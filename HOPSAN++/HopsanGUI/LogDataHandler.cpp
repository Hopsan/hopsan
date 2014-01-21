/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   LogDataHandler.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2012-12-18
//!
//! @brief Contains the LogData classes
//!
//$Id$

#include "LogDataHandler.h"

#include "PlotWindow.h"
#include "DesktopHandler.h"
#include "GUIObjects/GUIContainerObject.h"
#include "Widgets/HcomWidget.h"
#include "Widgets/ModelWidget.h"
#include "common.h"
#include "global.h"
#include "version_gui.h"
#include "Configuration.h"
#include "GUIPort.h"
#include "Utilities/GUIUtilities.h"

#include "PlotWindow.h"
#include "Widgets/PlotWidget.h"
#include "PlotHandler.h"

#include "ComponentUtilities/CSVParser.h"
#include "HopsanTypes.h"

//! @brief Constructor for plot data object
//! @param pParent Pointer to parent container object
LogDataHandler::LogDataHandler(ContainerObject *pParent) : QObject(pParent)
{
    mpParentContainerObject = 0;
    setParentContainerObject(pParent);
    mnPlotCurves = 0;
    mGenerationNumber = -1;
    mTempVarCtr = 0;

    // Create the temporary directory that will contain cache data
    int ctr=0;
    QDir tmp;
    do
    {
        tmp = QDir(gpDesktopHandler->getLogDataPath() + QString("/handler%1").arg(ctr));
        ++ctr;
    }while(tmp.exists());
    tmp.mkpath(tmp.absolutePath());
    mCacheDirs.append(tmp);
    mCacheSubDirCtr = 0;
}

LogDataHandler::~LogDataHandler()
{
    // Clear all data
    clear();
}

void LogDataHandler::setParentContainerObject(ContainerObject *pParent)
{
    if (mpParentContainerObject)
    {
        disconnect(0, 0, this, SLOT(registerAlias(QString,QString)));
    }
    mpParentContainerObject = pParent;
    connect(mpParentContainerObject, SIGNAL(aliasChanged(QString,QString)), this, SLOT(registerAlias(QString,QString)));
}


void LogDataHandler::exportToPlo(const QString &rFilePath, const QStringList &rVariables, const int version) const
{
    QVector<SharedVariablePtrT> dataPtrs;
    for(int v=0; v<rVariables.size(); ++v)
    {
        // Do not try to export missing data
        SharedVariablePtrT pData = getLogVariableDataPtr(rVariables[v],-1);
        if (pData)
        {
            dataPtrs.append(pData);
        }
        else
        {
            gpTerminalWidget->mpConsole->printWarningMessage(QString("In export PLO: %1 was not found, ignoring!").arg(rVariables[v]));
        }
    }
    exportToPlo(rFilePath, dataPtrs, version);
}

void LogDataHandler::exportToPlo(const QString &rFilePath, const QVector<SharedVariablePtrT> &rVariables, int version) const
{
    if ( (version < 1) || (version > 2) )
    {
        version = gpConfig->getPLOExportVersion();
    }

    if(rFilePath.isEmpty()) return;    //Don't save anything if user presses cancel

    QFile file;
    file.setFileName(rFilePath);   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpTerminalWidget->mpConsole->printErrorMessage("Failed to open file for writing: " + rFilePath);
        return;
    }

    // Create a QTextStream object to stream the content of file
    QTextStream fileStream(&file);
    QString dateTimeString = QDateTime::currentDateTime().toString();
    QFileInfo ploFileInfo(rFilePath);
    QString modelPath = mpParentContainerObject->getModelFileInfo().filePath();
    QFileInfo modelFileInfo(modelPath);

    //QStringList plotScaleStringList;
    //QStringList startvaluesList;

    QVector<SharedVariablePtrT> dataPtrs = rVariables;
    QVector<int> gens;
    gens.reserve(dataPtrs.size());
    for(int v=0; v<dataPtrs.size(); ++v)
    {
        gens.append(dataPtrs[v]->getGeneration());
    }

    int timeGen = -1;
    if (!gens.isEmpty())
    {
        if ( gens.count(gens.first()) != gens.size() )
        {
            gpTerminalWidget->mpConsole->printWarningMessage(QString("In export PLO: Data had different generations, time vector may not be correct for all exported data!"));
        }
        timeGen = gens.first();
    }

    // We insert time last when we know what generation the data had, (to avoid taking last time generation that may belong to imported data)
    SharedVariablePtrT pTime = getLogVariableDataPtr(TIMEVARIABLENAME, timeGen);
    if (pTime)
    {
        dataPtrs.prepend(pTime);
    }


    // Now begin to write to pro file
    int nDataRows = dataPtrs[0]->getDataSize();
    int nDataCols = dataPtrs.size();

    // Write initial Header data
    if (version == 1)
    {
        fileStream << "    'VERSION'\n";
        fileStream << "    1\n";
        fileStream << "    '"<<ploFileInfo.baseName()<<".PLO'\n";
        fileStream << "    " << nDataCols-1  <<"    "<< nDataRows <<"\n";
        fileStream << "    'Time'";
        for(int i=1; i<dataPtrs.size(); ++i)
        {
            //! @todo fix this formating so that it looks nice in plo file (need to know length of previous
            fileStream << ",    '" << dataPtrs[i]->getSmartName()<<"'";
        }
        fileStream << "\n";
    }
    else if (version == 2)
    {
        fileStream << "    'VERSION'\n";
        fileStream << "    2\n";
        fileStream << "    '"<<ploFileInfo.baseName()<<".PLO' " << "'" << modelFileInfo.fileName() << "' " << "'GUIVERSION " << QString(HOPSANGUIVERSION) << "' 'DATE "<<dateTimeString<<"'\n";
        fileStream << "    " << nDataCols  <<"    "<< nDataRows <<"\n";
        if (nDataCols > 0)
        {
            fileStream << "    '" << dataPtrs[0]->getSmartName() << "'";
            for(int i=1; i<dataPtrs.size(); ++i)
            {
                //! @todo fix this formating so that it looks nice in plo file (need to know length of previous
                fileStream << ",    '" << dataPtrs[i]->getSmartName()<<"'";
            }
        }
        else
        {
            fileStream << "    'NoData'";
        }
        fileStream << "\n";
    }

    // Write plotScalings line
    for(int i=0; i<dataPtrs.size(); ++i)
    {
        QString str;
        str.setNum(dataPtrs[i]->getPlotScale(),'E',6);
        //plotScaleStringList.append(str); //Remember till later
        if (str[0] == '-')
        {
            fileStream << " " << str;
        }
        else
        {
            fileStream << "  " << str;
        }
    }
    fileStream << "\n";

    // If data is cached to disk, we need to move it to ram before writing plo
    QVector<bool> wasCached;
    wasCached.reserve(dataPtrs.size());
    for(int col=0; col<dataPtrs.size(); ++col)
    {
        wasCached.push_back(dataPtrs[col]->isCachingDataToDisk());
        if (dataPtrs[col]->isCachingDataToDisk())
        {
            dataPtrs[col]->setCacheDataToDisk(false);
        }
    }

    // Write data lines
    QString err;
    for(int row=0; row<nDataRows; ++row)
    {
        QString str;
        for(int col=0; col<dataPtrs.size(); ++col)
        {
            str.setNum(dataPtrs[col]->peekData(row,err),'E',6);
            if (str[0] == '-')
            {
                fileStream << " " << str;
            }
            else
            {
                fileStream << "  " << str;
            }
        }
        fileStream << "\n";
    }

    // If data was cached to disk, we need to move it back to cache again
    for(int col=0; col<dataPtrs.size(); ++col)
    {
        dataPtrs[col]->setCacheDataToDisk(wasCached[col]);
    }

    // Write plot data ending header
    if (version==1)
    {
        fileStream << "  "+ploFileInfo.baseName()+".PLO.DAT_-1" <<"\n";
        fileStream << "  "+modelFileInfo.fileName() <<"\n";
    }

    file.close();
}

void LogDataHandler::exportToCSV(const QString &rFilePath, const QVector<SharedVariablePtrT> &rVariables) const
{
    QFile file;
    file.setFileName(rFilePath);   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpTerminalWidget->mpConsole->printErrorMessage("Failed to open file for writing: " + rFilePath);
        return;
    }

    QTextStream fileStream(&file);  //Create a QTextStream object to stream the content of file

    // Save each variable to one line each
    for (int i=0; i<rVariables.size(); ++i)
    {
        fileStream << rVariables[i]->getFullVariableName() << "," << rVariables[i]->getAliasName() << "," << rVariables[i]->getDataUnit() << ",";
        rVariables[i]->sendDataToStream(fileStream, ",");
        fileStream << "\n";
    }
}

void LogDataHandler::exportGenerationToCSV(const QString &rFilePath, const int gen) const
{
    QVector<SharedVariablePtrT> vars = getAllUniqueVariablesAtGeneration(gen);
    // Now export all of them
    exportToCSV(rFilePath, vars);
}

class PLOImportData
{
public:
      QString mDataName;
      double mPlotScale;
      double startvalue;
      QVector<double> mDataValues;
};

void LogDataHandler::importFromPlo(QString importFilePath)
{
    if(importFilePath.isEmpty())
    {

        importFilePath = QFileDialog::getOpenFileName(0,tr("Choose Hopsan .plo File"),
                                                       gpConfig->getPlotDataDir(),
                                                       tr("Hopsan File (*.plo)"));
    }
    if(importFilePath.isEmpty())
    {
        return;
    }

    QFile file(importFilePath);
    QFileInfo fileInfo(file);
    gpConfig->setPlotDataDir(fileInfo.absolutePath());

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::information(gpMainWindowWidget, gpMainWindowWidget->tr("Hopsan"), "Unable to read .PLO file.");
        return;
    }

    QProgressDialog progressImportBar(tr("Importing PLO"), QString(), 0, 0, gpMainWindowWidget);
    progressImportBar.setWindowModality(Qt::WindowModal);
    progressImportBar.setRange(0,0);
    progressImportBar.show();

    unsigned int nDataRows = 0;
    int nDataColumns = 0;
    int ploVersion = 0;

    QVector<PLOImportData> importedPLODataVector;

    // Stream all data and then manipulate
    QTextStream t(&file);

    // Read header data
    bool parseOK=true;
    for (unsigned int lineNum=1; lineNum<7; ++lineNum)
    {
        QString line = t.readLine().trimmed();
        if(!line.isNull())
        {
            // Check PLO format version
            if (lineNum == 1)
            {
                // Check if this seems to be a plo file
                if (line != "'VERSION'")
                {
                    gpTerminalWidget->mpConsole->printErrorMessage(fileInfo.fileName()+" Does not seem to be a plo file, Aborting import!");
                    return;
                }
            }
            else if(lineNum == 2)
            {
                ploVersion = line.toUInt(&parseOK);
            }
            // Else check for num data info
            else if(lineNum == 4)
            {
                bool colOK, rowOK;
                QStringList colsandrows = line.simplified().split(" ");
                nDataColumns = colsandrows[0].toUInt(&colOK);
                nDataRows = colsandrows[1].toUInt(&rowOK);
                parseOK = (colOK && rowOK);
                if (parseOK)
                {
                    // Reserve memory for reading data
                    importedPLODataVector.resize(nDataColumns);
                    for(int c=0; c<nDataColumns; ++c)
                    {
                        importedPLODataVector[c].mDataValues.reserve(nDataRows);
                    }
                }
            }
            // Else check for data header info
            else if(lineNum == 5)//(line.startsWith("'Time"))
            {
                if ((ploVersion == 1) && line.startsWith("'Time"))
                {
                    // We add one to include "time or x column"
                    nDataColumns+=1;
                    importedPLODataVector.resize(nDataColumns);
                    for(int c=0; c<nDataColumns; ++c)
                    {
                        importedPLODataVector[c].mDataValues.reserve(nDataRows);
                    }
                }

                QStringList dataheader = line.split(",");
                for(int c=0; c<nDataColumns; ++c)
                {
                    QString word;
                    word = dataheader[c].remove('\'');
                    importedPLODataVector[c].mDataName = word.trimmed();
                }
            }
            // Else check for plot scale
            else if (lineNum == 6)
            {
                // Read plotscales
                QTextStream linestream(&line);
                for(int c=0; c<nDataColumns; ++c)
                {
                    linestream >> importedPLODataVector[c].mPlotScale;
                }
            }
        }
        if (!parseOK)
        {
            gpTerminalWidget->mpConsole->printErrorMessage(QString("A parse error occured while parsing the header of: ")+fileInfo.fileName()+" Aborting import!");
            return;
        }
    }

    // Read the logged data
    for (unsigned int lineNum=7; lineNum<7+nDataRows; ++lineNum)
    {
        QString line = t.readLine().trimmed();
        if(!line.isNull())
        {
            // We check if data row first since it will happen most often
            QTextStream linestream(&line);
            for(int c=0; c<nDataColumns; ++c)
            {
                double tmp;
                linestream >> tmp;
                importedPLODataVector[c].mDataValues.append(tmp);
            }
        }
    }

    // Ignore the rest of the data for now

    // We are done reading, close the file
    file.close();

    // Insert data into log-data handler
    if (importedPLODataVector.size() > 0)
    {
        ++mGenerationNumber;
        SharedVariablePtrT pTimeVec(0);

        if (importedPLODataVector[0].mDataName == TIMEVARIABLENAME)
        {
            pTimeVec = insertTimeVectorVariable(importedPLODataVector.first().mDataValues, fileInfo.absoluteFilePath());

        }

        for (int i=1; i<importedPLODataVector.size(); ++i)
        {
            SharedVariableDescriptionT pVarDesc = SharedVariableDescriptionT(new VariableDescription);
            pVarDesc->mDataName = importedPLODataVector[i].mDataName;
            //! @todo what about reading the unit

            SharedVariablePtrT pNewData = insertTimeDomainVariable(pTimeVec,importedPLODataVector[i].mDataValues, pVarDesc, fileInfo.absoluteFilePath());
            pNewData->setPlotScale(importedPLODataVector[i].mPlotScale);
        }

        // We do not want imported data to be removed automatically
        preventGenerationAutoRemoval(mGenerationNumber);

        emit newDataAvailable();
    }

    // Limit number of plot generations if there are too many
    limitPlotGenerations();
}

void LogDataHandler::importFromCSV_AutoFormat(QString importFilePath)
{
    if(importFilePath.isEmpty())
    {

        importFilePath = QFileDialog::getOpenFileName(0,tr("Choose .csv File"),
                                                       gpConfig->getPlotDataDir(),
                                                       tr("Comma-separated values files (*.csv)"));
    }
    if(importFilePath.isEmpty())
    {
        return;
    }

    QFile file(importFilePath);
    if (file.open(QFile::ReadOnly))
    {
        QTextStream ts(&file);
        QStringList fields = ts.readLine().split(',');
        if (!fields.isEmpty())
        {
            bool ok;
            fields.front().toDouble(&ok);
            file.close();
            if (ok)
            {
                // If Ok then a number in first spot, seems to be a plain value csv
                importFromPlainColumnCsv(importFilePath);
            }
            else
            {
                // If NOT ok then likely text in first spot, seems to be a hopsan row csv file, or some other junk that will not work
                importHopsanRowCSV(importFilePath);
            }
        }
    }
    else
    {
        gpTerminalWidget->mpConsole->printErrorMessage(QString("Could not open file: %1").arg(importFilePath));
    }
    file.close();
}

void LogDataHandler::importHopsanRowCSV(QString importFilePath)
{
    if(importFilePath.isEmpty())
    {

        importFilePath = QFileDialog::getOpenFileName(0,tr("Choose .csv File"),
                                                       gpConfig->getPlotDataDir(),
                                                       tr("Hopsan row based csv files (*.csv)"));
    }
    if(importFilePath.isEmpty())
    {
        return;
    }

    QFile file(importFilePath);
    QFileInfo fileInfo(file);
    gpConfig->setPlotDataDir(fileInfo.absolutePath());

    bool parseOk = true;

    if (file.open(QFile::ReadOnly))
    {
        QStringList allNames, allAlias, allUnits;
        QList< QVector<double> > allDatas;
        int nDataValueElements = -1;
        QTextStream contentStream(&file);
        while (!contentStream.atEnd())
        {
            //! @todo this is not the most clever and speedy implementation
            QStringList lineFields = contentStream.readLine().split(',');
            if (lineFields.size() > 3)
            {
                // Read metadata
                allNames.append(lineFields[0]);
                allAlias.append(lineFields[1]);
                allUnits.append(lineFields[2]);

                allDatas.append(QVector<double>());
                QVector<double> &data = allDatas.last();
                if (nDataValueElements > -1)
                {
                    data.reserve(nDataValueElements);
                }
                for (int i=3; i<lineFields.size(); ++i)
                {
                    bool ok;
                    data.append(lineFields[i].toDouble(&ok));
                    if(!ok)
                    {
                        parseOk = false;
                    }
                }
                nDataValueElements = data.size();
            }
        }

        if (parseOk)
        {
            ++mGenerationNumber;

            // Figure out time
            SharedVariablePtrT pTimeVec;
            //! @todo what if multiple subsystems with different time
            int timeIdx = allNames.indexOf(TIMEVARIABLENAME);
            if (timeIdx > -1)
            {
                pTimeVec = insertTimeVectorVariable(allDatas[timeIdx], fileInfo.absoluteFilePath());
            }

            for (int i=1; i<allNames.size(); ++i)
            {
                // We already inserted time
                if (i == timeIdx)
                {
                    continue;
                }

                SharedVariableDescriptionT pVarDesc = SharedVariableDescriptionT(new VariableDescription);
                pVarDesc->mDataName = allNames[i];
                pVarDesc->mAliasName = allAlias[i];
                pVarDesc->mDataUnit = allUnits[i];
                insertTimeDomainVariable(pTimeVec, allDatas[i], pVarDesc, fileInfo.absoluteFilePath());
            }

            // We do not want imported data to be removed automatically
            preventGenerationAutoRemoval(mGenerationNumber);

            // Limit number of plot generations if there are too many
            limitPlotGenerations();

            emit newDataAvailable();
        }
    }
    file.close();
}


void LogDataHandler::importFromPlainColumnCsv(QString importFilePath)
{
    if(importFilePath.isEmpty())
    {

        importFilePath = QFileDialog::getOpenFileName(0,tr("Choose .csv File"),
                                                       gpConfig->getPlotDataDir(),
                                                       tr("Comma-separated values files (*.csv)"));
    }
    if(importFilePath.isEmpty())
    {
        return;
    }

    QFile file(importFilePath);
    QFileInfo fileInfo(file);
    gpConfig->setPlotDataDir(fileInfo.absolutePath());

    CoreCSVParserAccess *pParser = new CoreCSVParserAccess(importFilePath);

    if(!pParser->isOk())
    {
        gpTerminalWidget->mpConsole->printErrorMessage("CSV file could not be parsed.");
        return;
    }

    int cols = pParser->getNumberOfColumns();
    QList<QVector<double> > data;
    for(int c=0; c<cols; ++c)
    {
        QVector<double> vec = pParser->getColumn(c);
        data.append(vec);
    }

    delete(pParser);

    if (!data.isEmpty())
    {
        ++mGenerationNumber;
        SharedVariablePtrT pTimeVec(0);

        // Ugly, assume that first vector is always time
        pTimeVec = insertTimeVectorVariable(data[0], fileInfo.absoluteFilePath());

        for (int i=1; i<data.size(); ++i)
        {
            SharedVariableDescriptionT pVarDesc = SharedVariableDescriptionT(new VariableDescription);
            pVarDesc->mDataName = "CSV"+QString::number(i);
            insertTimeDomainVariable(pTimeVec, data[i], pVarDesc, fileInfo.absoluteFilePath());
        }

        // We do not want imported data to be removed automatically
        preventGenerationAutoRemoval(mGenerationNumber);

        emit newDataAvailable();
    }

    // Limit number of plot generations if there are too many
    limitPlotGenerations();
}


//! @todo this function assumes , separated format and only values,
//! @todo what if file does not have a time vector
void LogDataHandler::importTimeVariablesFromCSVColumns(const QString csvFilePath, QVector<int> columns, QStringList names, const int timeColumnId)
{
    if (columns.size() == names.size())
    {
        //! @todo in the future use a HopsanCSV parser
        QFile csvFile(csvFilePath);
        csvFile.open(QIODevice::ReadOnly | QIODevice::Text);
        if (csvFile.isOpen())
        {
            QTextStream csvStream(&csvFile);

            QVector<double> timeColumn;
            QVector< QVector<double> > dataColumns;
            dataColumns.resize(columns.size());
            while (!csvStream.atEnd())
            {
                QStringList row = csvStream.readLine().split(",");
                if (row.size() > 0)
                {
                    timeColumn.append(row[timeColumnId].toDouble());
                    for (int i=0; i<columns.size(); ++i)
                    {
                        if (columns[i] < row.size())
                        {
                            dataColumns[i].push_back(row[columns[i]].toDouble());
                        }
                    }
                }
            }

            //! @todo check if data was found
            ++mGenerationNumber;

            QFileInfo fileInfo(csvFilePath);
            SharedVariablePtrT pTimeVec = insertTimeVectorVariable(timeColumn, fileInfo.absoluteFilePath());

            // Ok now we have the data lets add it as a variable
            for (int n=0; n<names.size(); ++n)
            {
                //! @todo what if data name already exists?
                SharedVariableDescriptionT pVarDesc = SharedVariableDescriptionT(new VariableDescription);
                pVarDesc->mDataName = names[n];
                insertTimeDomainVariable(pTimeVec, dataColumns[n], pVarDesc, fileInfo.absoluteFilePath());
            }

            csvFile.close();

            // We do not want imported data to be removed automatically
            preventGenerationAutoRemoval(mGenerationNumber);

            emit newDataAvailable();
        }
        else
        {
            gpTerminalWidget->mpConsole->printErrorMessage("Could not open data file:  "+csvFilePath);
        }
    }
    else
    {
        gpTerminalWidget->mpConsole->printErrorMessage("columns.size() != names.size() in:  LogDataHandler::importTimeVariablesFromCSVColumns()");
    }
}


//! @brief Returns whether or not plot data is empty
bool LogDataHandler::isEmpty()
{
    return mLogDataMap.isEmpty();
}

void LogDataHandler::clear()
{
    // Clear all data containers
    QList< QPointer<LogVariableContainer> > data = mLogDataMap.values();
    for (int i=0; i<data.size(); ++i)
    {
        data[i]->deleteLater();
    }
    mLogDataMap.clear();

    // Remove imported variables
    mImportedLogDataMap.clear();

    // Clear generation Cache files (Individual files will remain if until all instances dies)
    mGenerationCacheMap.clear();

    // Remove the cache directory if it is empty, if it is not then cleanup should happen on program exit
    for (int i=0; i<mCacheDirs.size(); ++i)
    {
        mCacheDirs[i].rmdir(mCacheDirs[i].path());
    }

    // Reset generation number
    mGenerationNumber = -1;
}


//! @brief Collects plot data from last simulation
void LogDataHandler::collectLogDataFromModel(bool overWriteLastGeneration)
{
    TicToc tictoc;
    if(!overWriteLastGeneration)
    {
        ++mGenerationNumber;
    }

    if(mpParentContainerObject->getCoreSystemAccessPtr()->getNSamples() == 0)
    {
        return;         //Don't collect plot data if logging is disabled (to avoid empty generations)
    }

    //UniqueSharedTimeVectorPtrHelper timeVecHelper;
    bool foundData = false;
    bool timeVectorObtained = false;
    SharedVariablePtrT pTimeVec;

    //! @todo why not run multiappend when overwriting generation ? Baecouse tehn we are not appending, need som common open mode
    if(!overWriteLastGeneration)
    {
        this->getGenerationMultiCache(mGenerationNumber)->beginMultiAppend();
    }
    // Iterate components
    for(int m=0; m<mpParentContainerObject->getModelObjectNames().size(); ++m)
    {
        //! @todo getting names every time is very ineffecient it creates and copies a new vector every freaking time
        ModelObject *pModelObject = mpParentContainerObject->getModelObject(mpParentContainerObject->getModelObjectNames().at(m));

        for(QList<Port*>::iterator pit=pModelObject->getPortListPtrs().begin(); pit!=pModelObject->getPortListPtrs().end(); ++pit)
        {
            QVector<CoreVariableData> varDescs;
            mpParentContainerObject->getCoreSystemAccessPtr()->getVariableDescriptions(pModelObject->getName(), (*pit)->getName(), varDescs);

            // Iterate variables
            for(int i=0; i<varDescs.size(); ++i)
            {
                // Skip hidden variables
                if ( gpConfig->getShowHiddenNodeDataVariables() || (varDescs[i].mNodeDataVariableType != "Hidden") )
                {
                    // Fetch variable data
                    QVector<double> dataVec;
                    std::vector<double> *pTimeVector;
                    mpParentContainerObject->getCoreSystemAccessPtr()->getPlotData(pModelObject->getName(), (*pit)->getName(), varDescs[i].mName,
                                                                                   pTimeVector, dataVec);

                    // Prevent adding data if time or data vector was empty
                    if (!pTimeVector->empty() && !dataVec.isEmpty())
                    {
                        //! @todo Should be possible to have multiple timevectors per generation
                        // Store time data (only once)
                        if(!timeVectorObtained)
                        {
                            // Make sure we use the same time vector
                            pTimeVec = insertTimeVectorVariable(QVector<double>::fromStdVector(*pTimeVector)); //!< @todo here we need to copy (convert) from std vector to qvector, don know if that slows down (probably not much)

                            // Set the custom unit scaling to the default
                            QString defaultTimeUnit = gpConfig->getDefaultUnit(TIMEVARIABLENAME);
                            if (defaultTimeUnit != pTimeVec->getDataUnit())
                            {
                                pTimeVec->setCustomUnitScale(UnitScale(defaultTimeUnit, gpConfig->getUnitScale(TIMEVARIABLENAME, defaultTimeUnit)));
                            }

                            timeVectorObtained = true;
                        }

                        foundData=true;
                        SharedVariableDescriptionT pVarDesc = SharedVariableDescriptionT(new VariableDescription);
                        pVarDesc->mModelPath = pModelObject->getParentContainerObject()->getModelFileInfo().filePath();
                        pVarDesc->mComponentName = pModelObject->getName();
                        pVarDesc->mPortName = (*pit)->getName();
                        pVarDesc->mDataName = varDescs[i].mName;
                        pVarDesc->mDataUnit = varDescs[i].mUnit;
                        pVarDesc->mDataDescription = varDescs[i].mDescription;
                        pVarDesc->mAliasName  = varDescs[i].mAlias;
                        pVarDesc->mVariableSourceType = ModelVariableType;

                        SharedVariablePtrT pNewData = insertTimeDomainVariable(pTimeVec, dataVec, pVarDesc);

                        UnitScale us;
                        pModelObject->getCustomPlotUnitOrScale(pVarDesc->mPortName+"#"+pVarDesc->mDataName, us);
                        if (!us.mScale.isEmpty())
                        {
                            pNewData->setCustomUnitScale(us);
                        }
                    }
                }
            }
        }
    }
    if(!overWriteLastGeneration)
    {
        this->getGenerationMultiCache(mGenerationNumber)->endMultiAppend();
    }

    // Limit number of plot generations if there are too many
    limitPlotGenerations();

    // Increment generation counter
    if (foundData)
    {
        emit newDataAvailable();
    }
    else if (!overWriteLastGeneration)
    {
        // Revert generation number if no data was found, and we were not overwriting this generation
        --mGenerationNumber;
    }
    tictoc.toc("Collect plot data");
}

void LogDataHandler::exportGenerationToPlo(const QString &rFilePath, const int gen, const int version) const
{
    QVector<SharedVariablePtrT> vars = getAllUniqueVariablesAtGeneration(gen);

    // Ok now remove time vector as it will be added again in plo export
    // This is a hack, yes I know
    for (int i=0; i<vars.size(); ++i)
    {
        if (vars[i]->getDataName() == TIMEVARIABLENAME)
        {
            vars.remove(i);
        }
    }

    // Now export all of them
    exportToPlo(rFilePath, vars, version);
}


//! @brief Returns the plot data for specified variable
//! @param[in] generation Generation of plot data
//! @param[in] componentName Name of component where variable is located
//! @param[in] portName Name of port where variable is located
//! @param[in] dataName Name of variable
//! @warning Do not call this function for multiports unless you know what you are doing. It will return the data from the first node only.
//! @deprecated
QVector<double> LogDataHandler::copyLogDataVariableValues(int generation, QString componentName, QString portName, QString dataName)
{
    SharedVariablePtrT pData = getLogVariableDataPtr(generation, componentName, portName, dataName);
    if (pData)
    {
        return pData->getDataVectorCopy();
    }

    return QVector<double>();
}

QVector<double> LogDataHandler::copyLogDataVariableValues(const QString &rName, const int generation)
{
    SharedVariablePtrT pData = getLogVariableDataPtr(rName, generation);
    if (pData)
    {
        return pData->getDataVectorCopy();
    }

    return QVector<double>();
}

//! @deprecated
SharedVariablePtrT LogDataHandler::getLogVariableDataPtr(int generation, QString componentName, QString portName, QString dataName)
{
    //! @todo how to handle request by alias
    QString concName = componentName+"#"+portName+"#"+dataName;

    //! @todo this should probalby be handled in collectData
    if(mpParentContainerObject->getModelObject(componentName)->getPort(portName)->getPortType() == "PowerMultiportType" ||
       mpParentContainerObject->getModelObject(componentName)->getPort(portName)->getPortType() == "ReadMultiportType")
    {
        QString newPortName = mpParentContainerObject->getModelObject(componentName)->getPort(portName)->getConnectedPorts().first()->getName();
        QString newComponentName = mpParentContainerObject->getModelObject(componentName)->getPort(portName)->getConnectedPorts().first()->getParentModelObjectName();

        concName= newComponentName+"#"+newPortName+"#"+dataName;
    }

    return getLogVariableDataPtr(concName, generation);
}

SharedVariablePtrT LogDataHandler::getLogVariableDataPtr(const QString &rName, const int generation) const
{
    // Find the data variable
    LogDataMapT::const_iterator dit = mLogDataMap.find(rName);
    if (dit != mLogDataMap.end())
    {
        return dit.value()->getDataGeneration(generation);
    }

    // If not found return 0
    return SharedVariablePtrT(0);
}

//! @brief Returns multiple logdatavariables based on regular expression search. Exluding temp variables but including aliases
//! @param [in] rNameExp The regular expression for the names to match
//! @param [in] generation The desired generation of the variable
QVector<SharedVariablePtrT> LogDataHandler::getMatchingVariablesAtGeneration(const QRegExp &rNameExp, const int generation) const
{
    QVector<SharedVariablePtrT> results;
    LogDataMapT::const_iterator it;
    for (it = mLogDataMap.begin(); it != mLogDataMap.end(); it++)
    {
        // Compare name with regexp
        if ( rNameExp.exactMatch(it.key()) )
        {
            SharedVariablePtrT pData = it.value()->getDataGeneration(generation);
            if (pData)
            {
                results.append(pData);
            }
        }
    }
    return results;
}

const QList< QPointer<LogVariableContainer> > LogDataHandler::getAllLogVariableContainers() const
{
    return mLogDataMap.values();
}

const QList< QPointer<LogVariableContainer> > LogDataHandler::getLogDataContainersMatching(const QRegExp &rNameExp) const
{
    QList< QPointer<LogVariableContainer> > results;
    LogDataMapT::const_iterator it;
    for (it = mLogDataMap.begin(); it != mLogDataMap.end(); it++)
    {
        // Compare name with regexp
        if ( rNameExp.exactMatch(it.key()) )
        {
            results.append(it.value());
        }
    }
    return results;
}

const QList< QPointer<LogVariableContainer> > LogDataHandler::getLogDataContainersMatching(const QRegExp &rNameExp, const int generation) const
{
    QList< QPointer<LogVariableContainer> > results;
    LogDataMapT::const_iterator it;
    for (it = mLogDataMap.begin(); it != mLogDataMap.end(); it++)
    {
        // Compare name with regexp an check generation
        if ( rNameExp.exactMatch(it.key()) && it.value()->hasDataGeneration(generation) )
        {
            results.append(it.value());
        }
    }
    return results;
}

QPointer<LogVariableContainer> LogDataHandler::getLogVariableContainer(const QString &rVarName) const
{
    return mLogDataMap.value(rVarName,0);
}


//! @brief Returns the time vector for specified generation
//! @param[in] generation Generation
const SharedVariablePtrT LogDataHandler::getTimeVectorPtr(int generation) const
{
    LogVariableContainer *pCont = mLogDataMap.value(TIMEVARIABLENAME);
    if (pCont)
    {
        if (generation < 0)
        {
            generation = pCont->getHighestGeneration();
        }
        return pCont->getDataGeneration(generation);
    }
    else
    {
        return SharedVariablePtrT();
    }
}

QVector<double> LogDataHandler::copyTimeVector(const int generation) const
{
    SharedVariablePtrT pTime = getTimeVectorPtr(generation);
    if (pTime)
    {
        return pTime->getDataVectorCopy();
    }
    return QVector<double>();
}


////! @brief Returns whether or not the specified component exists in specified plot generation
////! @param[in] generation Generation
////! @param[in] componentName Component name
//bool LogDataHandler::componentHasPlotGeneration(int generation, QString fullName)
//{
//    LogDataMapT::iterator it = mAllPlotData.find(fullName);
//    if( it != mAllPlotData.end())
//    {
//        return it.value()->hasDataGeneration(generation);
//    }
//    else
//    {
//        return false;
//    }
//}


//! @brief Defines a new alias for specified variable (popup box)
//! @param[in] rFullName The Full name of the variable
void LogDataHandler::defineAlias(const QString &rFullName)
{
    bool ok;
    QString alias = QInputDialog::getText(gpMainWindowWidget, gpMainWindowWidget->tr("Define Variable Alias"),
                                          QString("Alias for: %1").arg(rFullName), QLineEdit::Normal, "", &ok);
    if(ok)
    {
        defineAlias(alias, rFullName);
    }
}


//! @brief Defines a new alias for specified variable
//! @param[in] rAlias Alias name for variable
//! @param[in] rFullName The Full name of the variable
bool LogDataHandler::defineAlias(const QString &rAlias, const QString &rFullName)
{
    // Try to set the new alias, abort if it did not work
    return mpParentContainerObject->setVariableAlias(rFullName,rAlias);
}


//! @brief Removes specified variable alias
//! @param[in] alias Alias to remove
void LogDataHandler::undefinePlotAlias(const QString &rAlias)
{
    QString fullName = getFullNameFromAlias(rAlias);
    if (!fullName.isEmpty())
    {
        mpParentContainerObject->setVariableAlias(fullName,""); //!< @todo maybe a remove alias function would be nice
        // Regardless of success or falure, lets remove the alias from log variables
        // Set variable will fail if you try to remove alias from an old component that has already been deleted
        unregisterAlias(rAlias);
    }
}


//! @brief Returns plot variable for specified alias
//! @param[in] rAlias Alias of variable
QString LogDataHandler::getFullNameFromAlias(const QString &rAlias)
{
    //! @todo should know what gens are aliases
    //! @todo what about multiple different namesunder the same alias (is that even possible right now)
    QPointer<LogVariableContainer> pAliasContainer = getLogVariableContainer(rAlias);
    if (pAliasContainer)
    {
        QList<int> gens = pAliasContainer->getGenerations();
        foreach(int g, gens)
        {
            if (pAliasContainer->getDataGeneration(g)->getAliasName() == rAlias)
            {
                return pAliasContainer->getDataGeneration(g)->getFullVariableName();
            }
        }
    }
    return QString();
}


////! @brief Returns plot alias for specified variable
////! @param[in] componentName Name of component
////! @param[in] portName Name of port
////! @param[in] dataName Name of data variable
//QString LogDataHandler::getAliasFromFullName(const QString &rFullName)
//{
//    LogVariableContainer *pDataContainer = getLogVariableContainer(rFullName);
//    if (pDataContainer)
//    {
//        return pDataContainer->getAliasName();
//    }
//    return QString();
//}

//! @todo logdatahandler needs to keep track of the generations the it contains, we should not need to ask for each one every time, this could take a long time
QList<int> LogDataHandler::getGenerations() const
{
    QMap<int, void*> allGens;

    // Ask each data variable what gens it is availible in
    LogDataMapT::const_iterator it;
    for (it=mLogDataMap.begin(); it!=mLogDataMap.end(); ++it)
    {
        QList<int> gens = it.value()->getGenerations();
        QList<int>::iterator it;
        for (it=gens.begin(); it!=gens.end(); ++it)
        {
            allGens.insert(*it, 0);
        }
    }

    // Now allGens should contain a unique set of all available generations
    return allGens.keys();
}

//! @todo maybe could speed this up by using the IndexIntervalCollection
int LogDataHandler::getLowestGenerationNumber() const
{
    int min=INT_MAX;
    // We must search through and ask all variables since they may have different sets of non-contious generations
    LogDataMapT::const_iterator it;
    for (it=mLogDataMap.begin(); it!=mLogDataMap.end(); ++it)
    {
        min = qMin(min,(it.value()->getLowestGeneration()));
    }
    return min;
}

//! @todo maybe could speed this up by using the IndexIntervalCollection
int LogDataHandler::getHighestGenerationNumber() const
{
    int max=INT_MIN;
    // We must search through and ask all variables since they may have different sets of non-contious generations
    LogDataMapT::const_iterator it;
    for (it=mLogDataMap.begin(); it!=mLogDataMap.end(); ++it)
    {
        max = qMax(max,(it.value()->getHighestGeneration()));
    }
    return max;
}

//! @todo should ignore tempvariables, maybe they should not even be in the logdata map, should have a separate one for temp variables
//! @todo maybe could speed this up by using the IndexIntervalCollection
void LogDataHandler::getLowestAndHighestGenerationNumber(int &rLowest, int &rHighest) const
{
    rLowest=INT_MAX;
    rHighest=INT_MIN;

    // We must search through and ask all variables since they may have different sets of non-contious generations
    LogDataMapT::const_iterator it;
    for (it=mLogDataMap.begin(); it!=mLogDataMap.end(); ++it)
    {
        rLowest = qMin(rLowest,(it.value()->getLowestGeneration()));
        rHighest = qMax(rHighest,(it.value()->getHighestGeneration()));
    }
}

//! @brief Ask each variable how many generations it has, and returns the maximum (the total number of availiable generations)
//! @note This function will become slower as the number of unique variable names grow
int LogDataHandler::getNumberOfGenerations() const
{
    int num = 0;
    // Lets loop through everyone and ask how many gens they have, the one with the most will be the total availible number
    LogDataMapT::const_iterator it;
    for (it=mLogDataMap.begin(); it!=mLogDataMap.end(); ++it)
    {
        num = qMax(it.value()->getNumGenerations(), num);
    }
    return num;
}


//! @brief Limits number of plot generations to value specified in configuration
void LogDataHandler::limitPlotGenerations()
{
    int numGens = getNumberOfGenerations() ;
    if (numGens > gpConfig->getGenerationLimit())
    {
        if(!gpConfig->getAutoLimitLogDataGenerations())
        {
            QDialog dialog(gpMainWindowWidget);
            dialog.setWindowTitle("Hopsan");
            QVBoxLayout *pLayout = new QVBoxLayout(&dialog);
            QLabel *pLabel = new QLabel("<b>Log data generation limit reached!</b><br><br>Generation limit: "+QString::number(gpConfig->getGenerationLimit())+
                                        "<br>Number of data generations: "+QString::number(numGens)+
                                        "<br><br><b>Discard "+QString::number(numGens-gpConfig->getGenerationLimit())+" generations(s)?</b>");
            QCheckBox *pAutoLimitCheckBox = new QCheckBox("Automatically discard old generations", &dialog);
            pAutoLimitCheckBox->setChecked(gpConfig->getAutoLimitLogDataGenerations());
            QDialogButtonBox *pButtonBox = new QDialogButtonBox(&dialog);
            QPushButton *pDiscardButton = pButtonBox->addButton("Discard", QDialogButtonBox::AcceptRole);
            QPushButton *pKeepButton = pButtonBox->addButton("Keep", QDialogButtonBox::RejectRole);
            connect(pDiscardButton, SIGNAL(clicked()), &dialog, SLOT(accept()));
            connect(pKeepButton, SIGNAL(clicked()), &dialog, SLOT(reject()));
            connect(pAutoLimitCheckBox, SIGNAL(toggled(bool)), pKeepButton, SLOT(setDisabled(bool)));
            pLayout->addWidget(pLabel);
            pLayout->addWidget(pAutoLimitCheckBox);
            pLayout->addWidget(pButtonBox);

            int retval = dialog.exec();
            gpConfig->setAutoLimitLogDataGenerations(pAutoLimitCheckBox->isChecked());

            if(retval == QDialog::Rejected)
            {
                return;
            }
        }

        // Remove old generation in each data variable container
//        TicToc timer;
//        int lowest, highest;
//        getLowestAndHighestGenerationNumber(lowest,highest);
//        qDebug() << "removeOldGenerations: " << lowest << " to " << highest;
//        while( (numGens > gConfig.getGenerationLimit()) && (lowest <= highest))
//        {
//            // Remove old generation in each data variable container, but do not force removal
//            removeGeneration(lowest, false);

//            // Clear from generations cache object map
//            //! @todo should we realy clear this, what if there were some variables that hade autoremove disabled
//            mGenerationCacheMap.remove(lowest);
//            ++lowest;
//            //! @todo since getNumGens may be slow, maybe removeGen should return booltrue if something was actually removed, then we can calc numGens locally
//            numGens = getNumberOfGenerations(); //Update num gens
//        }
//        timer.tocDbg("removeOldGenerations");

//---------------------------------------------------------------------------------------
// Note this code is not as smart as the old one but much faster
        TicToc timer;
        int highest = getHighestGenerationNumber();
        int highestToRemove = highest-gpConfig->getGenerationLimit();
        bool didRemoveSomething = false;
        // Note! Here we must iterate through a copy of the values from the map
        // if we use an iterator in the map we will crash if the removed generation is the final one
        // Then the data variable itself will also be removed and the iterator will become invalid
        QList< QPointer<LogVariableContainer> > vars = mLogDataMap.values();
        QList< QPointer<LogVariableContainer> >::iterator it;
        for ( it=vars.begin(); it!=vars.end(); ++it)
        {
            didRemoveSomething += (*it)->purgeOldGenerations(highestToRemove, gpConfig->getGenerationLimit());
        }

        if (didRemoveSomething)
        {
            emit dataRemoved();

            //! @todo this is not a good solution, signaling would be better but it would be nice not to make every thing into qobjects
            // Loop until we have removed every empty generation cache object
            QList<int> gens = mGenerationCacheMap.keys();
            for (int i=0; i<gens.size(); ++i)
            {
                const int g = gens[i];
                if (g>highestToRemove)
                {
                    break;
                }
                removeGenerationCacheIfEmpty(g);
            }
        }
        qDebug() << "Number of logdata variables: " << getNumVariables();
        timer.toc("removeOldGenerations");
//---------------------------------------------------------------------------------------
    }
}

void LogDataHandler::preventGenerationAutoRemoval(const int gen)
{
    // Tag the generations in each log data variable container
    LogDataMapT::iterator dit = mLogDataMap.begin();
    for ( ; dit!=mLogDataMap.end(); ++dit)
    {
        dit.value()->preventAutoRemove(gen);
    }
}

void LogDataHandler::allowGenerationAutoRemoval(const int gen)
{
    // Tag the generations in each log data variable container
    LogDataMapT::iterator dit = mLogDataMap.begin();
    for ( ; dit!=mLogDataMap.end(); ++dit)
    {
        dit.value()->allowAutoRemove(gen);
    }
}

//! @brief Removes a generation
void LogDataHandler::removeGeneration(const int gen, const bool force)
{
    bool didRemoveSomething=false;
    // Note! Here we must iterate through a copy of the values from the map
    // if we use an iterator in the map we will crash if the removed generation is the final one
    // Then the data variable itself will also be removed and the iterator will become invalid
    QList< QPointer<LogVariableContainer> > vars = mLogDataMap.values();
    QList< QPointer<LogVariableContainer> >::iterator it;
    for ( it=vars.begin(); it!=vars.end(); ++it)
    {
        if ((*it)->removeDataGeneration(gen,force))
        {
            // If the generation was removed then try to remove the cache object (if any), if it still have active subscribers it will remain
            removeGenerationCacheIfEmpty(gen);
            didRemoveSomething = true;
        }
    }

    // Only emit this signal if something was actually removed
    if (didRemoveSomething)
    {
        emit dataRemoved();
    }
}

ContainerObject *LogDataHandler::getParentContainerObject()
{
    return mpParentContainerObject;
}

const QList<QDir> &LogDataHandler::getCacheDirs() const
{
    return mCacheDirs;
}

//! @brief Will retreive an existing or create a new generation multi-chace object
SharedMultiDataVectorCacheT LogDataHandler::getGenerationMultiCache(const int gen)
{
    SharedMultiDataVectorCacheT pCache = mGenerationCacheMap.value(gen, SharedMultiDataVectorCacheT());
    if (!pCache)
    {
        pCache = SharedMultiDataVectorCacheT(new MultiDataVectorCache(getNewCacheName()));
        mGenerationCacheMap.insert(gen, pCache);
    }
    return pCache;
}


//! @brief Increments counter for number of open plot curves (in plot windows)
//! @note Used to decide if warning message shall be shown when closing model
//! @see PlotData::decrementOpenPlotCurves()
//! @see PlotData::hasOpenPlotCurves()
void LogDataHandler::incrementOpenPlotCurves()
{
    ++mnPlotCurves;
}


//! @brief Decrements counter for number of open plot curves (in plot windows)
//! @note Used to decide if warning message shall be shown when closing model
//! @see PlotData::incrementOpenPlotCurves()
//! @see PlotData::hasOpenPlotCurves()
void LogDataHandler::decrementOpenPlotCurves()
{
    --mnPlotCurves;
}


QString LogDataHandler::addVariableWithScalar(const QString &a, const double x)
{
    SharedVariablePtrT pData1 = getLogVariableDataPtr(a, -1);
    if( (pData1 == NULL))
    {
        return NULL;
    }
    else
    {
        SharedVariablePtrT pTemp = addVariableWithScalar(pData1,x);
        return pTemp->getFullVariableName();
    }
}

SharedVariablePtrT LogDataHandler::addVariableWithScalar(const SharedVariablePtrT a, const double x)
{
    SharedVariablePtrT pTempVar = createOrphanVariable(a->getFullVariableName()+"+"+QString::number(x));
    pTempVar->assignFrom(a);
    pTempVar->addToData(x);
    return pTempVar;
}


QString LogDataHandler::subVariableWithScalar(const QString &a, const double x)
{
    SharedVariablePtrT pData1 = getLogVariableDataPtr(a, -1);
    if( (pData1 == NULL))
    {
        return NULL;
    }
    else
    {
        SharedVariablePtrT pTemp = subVariableWithScalar(pData1,x);
        return pTemp->getFullVariableName();
    }
}

SharedVariablePtrT LogDataHandler::subVariableWithScalar(const SharedVariablePtrT a, const double x)
{
    SharedVariablePtrT pTempVar = createOrphanVariable(a->getFullVariableName()+"-"+QString::number(x));
    pTempVar->assignFrom(a);
    pTempVar->subFromData(x);
    return pTempVar;
}


QString LogDataHandler::mulVariableWithScalar(const QString &a, const double x)
{
    SharedVariablePtrT pData1 = getLogVariableDataPtr(a, -1);
    if( (pData1 == NULL))
    {
        return NULL;
    }
    else
    {
        SharedVariablePtrT pTemp = mulVariableWithScalar(pData1,x);
        return pTemp->getFullVariableName();
    }
}

SharedVariablePtrT LogDataHandler::mulVariableWithScalar(const SharedVariablePtrT a, const double x)
{
    SharedVariablePtrT pTempVar = createOrphanVariable(a->getFullVariableName()+"*"+QString::number(x));
    pTempVar->assignFrom(a);
    pTempVar->multData(x);
    return pTempVar;
}


QString LogDataHandler::divVariableWithScalar(const QString &a, const double x)
{
    SharedVariablePtrT pData1 = getLogVariableDataPtr(a, -1);
    if( (pData1 == NULL))
    {
        return NULL;
    }
    else
    {
        SharedVariablePtrT pTemp = divVariableWithScalar(pData1,x);
        return pTemp->getFullVariableName();
    }
}

SharedVariablePtrT LogDataHandler::divVariableWithScalar(const SharedVariablePtrT a, const double x)
{
    SharedVariablePtrT pTempVar = createOrphanVariable(a->getFullVariableName()+"/"+QString::number(x));
    pTempVar->assignFrom(a);
    pTempVar->divData(x);
    return pTempVar;
}


QString LogDataHandler::addVariables(const QString &a, const QString &b)
{
    SharedVariablePtrT pData1 = getLogVariableDataPtr(a, -1);
    SharedVariablePtrT pData2 = getLogVariableDataPtr(b, -1);

    if( (pData1 == 0) || (pData2 == 0) )
    {
        return QString();
    }
    else
    {
        SharedVariablePtrT pTemp = addVariables(pData1,pData2);
        return pTemp->getFullVariableName();
    }
}

SharedVariablePtrT LogDataHandler::addVariables(const SharedVariablePtrT a, const SharedVariablePtrT b)
{
    SharedVariablePtrT pTempVar = createOrphanVariable(a->getFullVariableName()+"+"+b->getFullVariableName());
    pTempVar->assignFrom(a);
    pTempVar->addToData(b);
    return pTempVar;
}

QString LogDataHandler::subVariables(const QString &a, const QString &b)
{
    SharedVariablePtrT pData1 = getLogVariableDataPtr(a, -1);
    SharedVariablePtrT pData2 = getLogVariableDataPtr(b, -1);

    if( (pData1 == 0) || (pData2 == 0) )
    {
        return QString();
    }
    else
    {
        SharedVariablePtrT pTemp = subVariables(pData1,pData2);
        return pTemp->getFullVariableName();
    }
}

QString LogDataHandler::multVariables(const QString &a, const QString &b)
{
    SharedVariablePtrT pData1 = getLogVariableDataPtr(a, -1);
    SharedVariablePtrT pData2 = getLogVariableDataPtr(b, -1);

    if( (pData1 == 0) || (pData2 == 0) )
    {
        return QString();
    }
    else
    {
        SharedVariablePtrT pTemp = multVariables(pData1,pData2);
        return pTemp->getFullVariableName();
    }
}

QString LogDataHandler::divVariables(const QString &a, const QString &b)
{
    SharedVariablePtrT pData1 = getLogVariableDataPtr(a, -1);
    SharedVariablePtrT pData2 = getLogVariableDataPtr(b, -1);

    if( (pData1 == 0) || (pData2 == 0) )
    {
        return QString();
    }
    else
    {
        SharedVariablePtrT pTemp = divVariables(pData1,pData2);
        return pTemp->getFullVariableName();
    }
}

SharedVariablePtrT LogDataHandler::diffVariables(const SharedVariablePtrT a, const SharedVariablePtrT b)
{
    SharedVariablePtrT pTempVar = createOrphanVariable(a->getFullVariableName()+"_Diff");
    pTempVar->assignFrom(a);
    pTempVar->diffBy(b);
    return pTempVar;
}

QString LogDataHandler::diffVariables(const QString &a, const QString &b)
{
    SharedVariablePtrT pData1 = getLogVariableDataPtr(a, -1);
    SharedVariablePtrT pData2;
    if(b!=TIMEVARIABLENAME)
    {
        pData2 = getLogVariableDataPtr(b, -1);
    }


    if( (pData1 == 0) || ((pData2 == 0) && (b != TIMEVARIABLENAME)) )
    {
        return QString();
    }
    else
    {
        SharedVariablePtrT pTemp = diffVariables(pData1,pData2);
        return pTemp->getFullVariableName();
    }
}

SharedVariablePtrT LogDataHandler::integrateVariables(const SharedVariablePtrT a, const SharedVariablePtrT b)
{
    SharedVariablePtrT pTempVar = createOrphanVariable(a->getFullVariableName()+"Int");
    pTempVar->assignFrom(a);
    pTempVar->integrateBy(b);
    return pTempVar;
}

QString LogDataHandler::integrateVariables(const QString &a, const QString &b)
{
    SharedVariablePtrT pData1 = getLogVariableDataPtr(a, -1);
    SharedVariablePtrT pData2;
    if(b!=TIMEVARIABLENAME)
    {
        pData2 = getLogVariableDataPtr(b, -1);
    }

    if( (pData1 == 0) || ((pData2 == 0) && (b != TIMEVARIABLENAME)) )
    {
        return QString();
    }
    else
    {
        SharedVariablePtrT pTemp = integrateVariables(pData1,pData2);
        return pTemp->getFullVariableName();
    }
}


SharedVariablePtrT LogDataHandler::lowPassFilterVariable(const SharedVariablePtrT a, const SharedVariablePtrT b, const double freq)
{
    SharedVariablePtrT pTempVar = createOrphanVariable(a->getFullVariableName()+"_Lp1");
    pTempVar->assignFrom(a);
    pTempVar->lowPassFilter(b, freq);
    return pTempVar;
}

QString LogDataHandler::lowPassFilterVariable(const QString &a, const QString &b, const double freq)
{
    SharedVariablePtrT pData1 = getLogVariableDataPtr(a, -1);
    SharedVariablePtrT pData2;
    if(b!=TIMEVARIABLENAME)
    {
        pData2 = getLogVariableDataPtr(b, -1);
    }


    if( (pData1 == 0) || (pData2 == 0 && b != TIMEVARIABLENAME) )
    {
        return QString();
    }
    else
    {
        SharedVariablePtrT pTemp = lowPassFilterVariable(pData1,pData2,freq);
        return pTemp->getFullVariableName();
    }
}

SharedVariablePtrT LogDataHandler::fftVariable(const SharedVariablePtrT a, const SharedVariablePtrT b, const bool doPowerSpectrum)
{
    SharedVariablePtrT pFFTVar = createOrphanVariable(a->getFullVariableName()+"_fft", FrequencyDomainType);
    pFFTVar = a->toFrequencySpectrum(b, doPowerSpectrum);
    return pFFTVar;
}

QString LogDataHandler::fftVariable(const QString &a, const QString &b, const bool doPowerSpectrum)
{
    SharedVariablePtrT pData1 = getLogVariableDataPtr(a, -1);
    SharedVariablePtrT pData2;
    if(b!=TIMEVARIABLENAME)
    {
        pData2 = getLogVariableDataPtr(b, -1);
    }


    if( (pData1 == 0) || (pData2 == 0 && b != TIMEVARIABLENAME) )
    {
        return QString();
    }
    else
    {
        SharedVariablePtrT pTemp = fftVariable(pData1,pData2,doPowerSpectrum);
        return pTemp->getFullVariableName();
    }
}


QString LogDataHandler::assignVariable(const QString &dst, const QString &src)
{
    SharedVariablePtrT pSrcData = getLogVariableDataPtr(src, -1);
    // If source does not exist then return unsuccessfully
    if(!pSrcData)
    {
        return QString();
    }

    // If dst does not exist, then try to create it
    SharedVariablePtrT pDstData = getLogVariableDataPtr(dst, -1);
    if(!pDstData)
    {
        pDstData = defineNewVariable(dst);
    }

    // If we were successfull in creating dst then use it else return unsuccessfully
    if (pDstData)
    {
        pDstData->assignFrom(pSrcData);
        return pDstData->getFullVariableName();
    }
    else
    {
        return QString();
    }
}

QString LogDataHandler::assignVariable(const QString &dst, const QVector<double> &src)
{
    SharedVariablePtrT pDstData = getLogVariableDataPtr(dst, -1);
    if (!pDstData)
    {
        pDstData = defineNewVariable(dst);
    }

    // Check again if new def was succesfull, else return empty
    if (pDstData)
    {
        pDstData->assignFrom(src);
        return pDstData->getFullVariableName();
    }
    else
    {
        return QString();
    }

}

double LogDataHandler::pokeVariable(const QString &a, const int index, const double value)
{
    SharedVariablePtrT pData1 = getLogVariableDataPtr(a, -1);
    if(pData1)
    {
        return pokeVariable(pData1, index, value);
    }
    gpTerminalWidget->mpConsole->printErrorMessage("In Poke, No such variable: " + a);
    return -1;
}

bool LogDataHandler::deleteVariable(const QString &rVarName)
{
    LogDataMapT::iterator it = mLogDataMap.find(rVarName);
    if(it != mLogDataMap.end())
    {
        // Handle alias removal
        if (it.value()->isStoringAlias())
        {
            unregisterAlias(rVarName);
            //! @todo should we delete the actual variable also?
        }
        // Delete the data
        it.value()->deleteLater();
        // Remove data ptr from map
        mLogDataMap.erase(it);
        emit dataRemoved();
        return true;
    }
    gpTerminalWidget->mpConsole->printErrorMessage("In Delete, No such variable: " + rVarName);
    return false;
}

//! @brief Remove a variable, but only imported generations
bool LogDataHandler::deleteImportedVariable(const QString &rVarName)
{
    bool didRemove=false;
    LogDataMapT::iterator it = mLogDataMap.find(rVarName);
    if(it != mLogDataMap.end())
    {
        didRemove = (*it)->removeAllImportedGenerations();
        if (didRemove)
        {
            emit dataRemoved();
        }
    }
    return didRemove;
}

//! @brief Returns the number of log data variables registered in this log data handler
//! @returns The number of registered log data variables
int LogDataHandler::getNumVariables() const
{
    return mLogDataMap.size();
}

//bool LogDataHandler::deleteVariable(SharedVariablePtrT pVariable)
//{
//    return deleteVariable(pVariable->getFullVariableName());
//}

double LogDataHandler::peekVariable(const QString &a, const int index)
{
    SharedVariablePtrT pData1 = getLogVariableDataPtr(a, -1);
    if(pData1)
    {
        return peekVariable(pData1,index);
    }
    gpTerminalWidget->mpConsole->printErrorMessage("In Peek, No such variable: " + a);
    return 0;
}

SharedVariablePtrT LogDataHandler::elementWiseGT(SharedVariablePtrT pData, const double thresh)
{
    if (pData)
    {
        SharedVariablePtrT pTempVar = createOrphanVariable(pData->getFullVariableName()+"_gt");
        QVector<double> res;
        pData->elementWiseGt(res,thresh);
        pTempVar->assignFrom(res);
        return pTempVar;
    }
    return SharedVariablePtrT();
}

SharedVariablePtrT LogDataHandler::elementWiseLT(SharedVariablePtrT pData, const double thresh)
{
    if (pData)
    {
        SharedVariablePtrT pTempVar = createOrphanVariable(pData->getFullVariableName()+"_lt");
        QVector<double> res;
        pData->elementWiseLt(res,thresh);
        pTempVar->assignFrom(res);
        return pTempVar;
    }
    return SharedVariablePtrT();
}

QString LogDataHandler::saveVariable(const QString &currName, const QString &newName)
{
    SharedVariablePtrT pCurrData = getLogVariableDataPtr(currName, -1);
    SharedVariablePtrT pNewData = getLogVariableDataPtr(newName, -1);
    // If curr data exist and new data does not exist
    if( (pNewData == 0) && (pCurrData != 0) )
    {
        SharedVariablePtrT pNewData = defineNewVariable(newName);
        if (pNewData)
        {
            pNewData->assignFrom(pCurrData);
            return pNewData->getFullVariableName();
        }
        gpTerminalWidget->mpConsole->printErrorMessage("Could not create variable: " + newName);
        return QString();
    }
    gpTerminalWidget->mpConsole->printErrorMessage("Variable: " + currName + " does not exist, or Variable: " + newName + " already exist");
    return QString();
}

SharedVariablePtrT LogDataHandler::subVariables(const SharedVariablePtrT a, const SharedVariablePtrT b)
{
    SharedVariablePtrT pTempVar = createOrphanVariable(a->getFullVariableName()+"-"+b->getFullVariableName());
    pTempVar->assignFrom(a);
    pTempVar->subFromData(b);
    return pTempVar;
}

SharedVariablePtrT LogDataHandler::multVariables(const SharedVariablePtrT a, const SharedVariablePtrT b)
{
    SharedVariablePtrT pTempVar = createOrphanVariable(a->getFullVariableName()+"*"+b->getFullVariableName());
    pTempVar->assignFrom(a);
    pTempVar->multData(b);
    return pTempVar;
}

SharedVariablePtrT LogDataHandler::divVariables(const SharedVariablePtrT a, const SharedVariablePtrT b)
{
    SharedVariablePtrT pTempVar = createOrphanVariable(a->getFullVariableName()+"/"+b->getFullVariableName());
    pTempVar->assignFrom(a);
    pTempVar->divData(b);
    return pTempVar;
}

double LogDataHandler::pokeVariable(SharedVariablePtrT a, const int index, const double value)
{
    QString err;
    double r = a->pokeData(index,value,err);
    if (!err.isEmpty())
    {
        gpTerminalWidget->mpConsole->printErrorMessage(err);
    }
    return r;
}

SharedVariablePtrT LogDataHandler::saveVariable(SharedVariablePtrT a)
{
    SharedVariablePtrT pTempVar = defineTempVariable(a->getFullVariableName());
    pTempVar->assignFrom(a);
    return pTempVar;
}

double LogDataHandler::peekVariable(SharedVariablePtrT a, const int index)
{
    QString err;
    double r = a->peekData(index, err);
    if (!err.isEmpty())
    {
        gpTerminalWidget->mpConsole->printErrorMessage(err);
    }
    return r;
}

//! @brief Creates a new temp variable (appending tempctr to name) that will be added to and manageed by the logdatahandler, it will not be automatically deleted unless its generation is auto-removed
//! @note Avoid this function if possible
SharedVariablePtrT LogDataHandler::defineTempVariable(const QString &rDesiredname)
{
    SharedVariablePtrT pData = defineNewVectorVariable_NoNameCheck(rDesiredname+QString("%1").arg(mTempVarCtr));
    if (pData)
    {
        pData->mpVariableDescription->mVariableSourceType = TempVariableType;
        ++mTempVarCtr;
    }
    return pData;
}

//! @brief Creates an orphan temp variable that will be deleted when its shared pointer reference counter reaches zero (when no one is using it)
SharedVariablePtrT LogDataHandler::createOrphanVariable(const QString &rName, VariableTypeT type)
{
    SharedVariableDescriptionT varDesc = SharedVariableDescriptionT(new VariableDescription());
    varDesc->mDataName = rName;
    varDesc->mVariableSourceType = ScriptVariableType;

    SharedVariablePtrT pNewVar;


    return SharedVariablePtrT(new VectorVariable(QVector<double>(), mGenerationNumber, varDesc, SharedMultiDataVectorCacheT()));
}


void LogDataHandler::appendVariable(const QString &a, const double x, const double y)
{
    SharedVariablePtrT pData1 = getLogVariableDataPtr(a, -1);
    if(pData1)
    {
        pData1->append(x,y);
        return;
    }
    gpTerminalWidget->mpConsole->printErrorMessage("No such variable: " + a);
    return;
}

SharedVariablePtrT LogDataHandler::defineNewVariable(const QString &rDesiredname, VariableTypeT type)
{
    bool ok = isNameValid(rDesiredname);
    if (!ok)
    {
        gpTerminalWidget->mpConsole->printErrorMessage(QString("Invalid variable name: %1").arg(rDesiredname));
    }
    if( ok && (mLogDataMap.find(rDesiredname) == mLogDataMap.end()) )
    {
        return defineNewVectorVariable_NoNameCheck(rDesiredname, type);
    }
    return SharedVariablePtrT();
}

//SharedVariablePtrT LogDataHandler::defineNewVariable(const QString &rDesiredname, const QString &rUnit, const QString &rDescription)
//{
//    SharedVariablePtrT pData = defineNewVariable(rDesiredname);
//    if (pData)
//    {
//        pData->mpVariableDescription->mDataUnit = rUnit;
//        pData->mpVariableDescription->mDataUnit = rDescription;
//    }
//    return pData;
//}

//! @brief Tells whether or not the model has open plot curves
//! @note Used to decide if warning message shall be shown when closing model
//! @see PlotData::incrementOpenPlotCurves()
//! @see PlotData::decrementOpenPlotCurves()
bool LogDataHandler::hasOpenPlotCurves()
{
    return (mnPlotCurves > 0);
}

void LogDataHandler::closePlotsWithCurvesBasedOnOwnedData()
{
    emit closePlotsWithOwnedData();
}


//! @brief Returns the list of favorite variables
LogDataHandler::FavoriteListT LogDataHandler::getFavoriteVariableList()
{
    return mFavoriteVariables;
}


//! @brief Defines a new favorite variable
//! @param[in] componentName Name of component
//! @param[in] portName Name of port
//! @param[in] dataName Name of data variable
//! @param[in] dataUnit Unit of the variable
void LogDataHandler::setFavoriteVariable(QString componentName, QString portName, QString dataName, QString dataUnit)
{
    VariableDescription tempVariable;
    tempVariable.mComponentName = componentName;
    tempVariable.mPortName = portName;
    tempVariable.mDataName = dataName;
    tempVariable.mDataUnit = dataUnit;
    if(!mFavoriteVariables.contains(tempVariable))
    {
        mFavoriteVariables.append(tempVariable);
    }
    gpPlotWidget->updateList();

    mpParentContainerObject->mpModelWidget->hasChanged();
}


//! @brief Removse a favorite variable
//! @param[in] componentName Name of component
void LogDataHandler::removeFavoriteVariableByComponentName(QString componentName)
{
    FavoriteListT::iterator it;
    for(it=mFavoriteVariables.begin(); it!=mFavoriteVariables.end(); ++it)
    {
        if((*it).mComponentName == componentName)
        {
            mFavoriteVariables.removeAll((*it));
            gpPlotWidget->updateList();
            return;
        }
    }
}

QString LogDataHandler::plotVariable(const QString plotName, const QString fullVarName, const int gen, const int axis, QColor color)
{
    SharedVariablePtrT pData = getLogVariableDataPtr(fullVarName, gen);
    if(pData)
    {
        return gpPlotHandler->plotDataToWindow(plotName, pData, axis, color);
    }
    return "";
}

QString LogDataHandler::plotVariable(const QString plotName, const QString &rFullNameX, const QString &rFullNameY, const int gen, const int axis, QColor color)
{
    SharedVariablePtrT pDataX = getLogVariableDataPtr(rFullNameX, gen);
    SharedVariablePtrT pDataY = getLogVariableDataPtr(rFullNameY, gen);
    if (pDataX && pDataY)
    {
        return gpPlotHandler->plotDataToWindow(plotName, pDataX, pDataY, axis, color);
    }
    return "";
}

PlotWindow *LogDataHandler::plotVariable(PlotWindow *pPlotWindow, const QString fullVarName, const int gen, const int axis, QColor color)
{
    SharedVariablePtrT pData = getLogVariableDataPtr(fullVarName, gen);
    if(pData)
    {
        return gpPlotHandler->plotDataToWindow(pPlotWindow, pData, axis, color);
    }
    return 0;
}

//! @brief Get a list of all available variables at their respective higest (newest) generation. Aliases are excluded.
QVector<SharedVariablePtrT> LogDataHandler::getAllUniqueVariablesAtNewestGeneration()
{
    QVector<SharedVariablePtrT> dataPtrVector;

    // First go through all data variable
    LogDataMapT::iterator dit = mLogDataMap.begin();
    for ( ; dit!=mLogDataMap.end(); ++dit)
    {
        SharedVariablePtrT pData = dit.value()->getNonAliasDataGeneration(-1);
        if (pData)
        {
            dataPtrVector.push_back(pData);
        }
        //! @todo what about alias duplicates
    }

    return dataPtrVector;
}

//! @brief Get a list of all available variables at a specific generation, variables that does not have this generation will not be included.  Aliases are excluded.
QVector<SharedVariablePtrT> LogDataHandler::getAllUniqueVariablesAtGeneration(const int generation) const
{
    QVector<SharedVariablePtrT> dataPtrVector;

    // First go through all data variable
    LogDataMapT::const_iterator dit = mLogDataMap.begin();
    for ( ; dit!=mLogDataMap.end(); ++dit)
    {
        // Now try to find given generation
        SharedVariablePtrT pData = dit.value()->getNonAliasDataGeneration(generation);
        if (pData)
        {
            dataPtrVector.push_back(pData);
        }
        //! @todo what about alias duplicates
    }

    return dataPtrVector;
}

//QStringList LogDataHandler::getLogDataVariableNames(const QString &rSeparator, const int generation) const
//{
//    QStringList retval;
//    LogDataMapT::const_iterator dit = mLogDataMap.begin();
//    for ( ; dit!=mLogDataMap.end(); ++dit)
//    {
//        if(generation < 0 || dit.value()->hasDataGeneration(generation))
//        {
//            if (dit.value().mIsAlias)
//            {
//                retval.append(dit.value().mpDataContainer->getAliasName());
//            }
//            else
//            {
//                retval.append(dit.value().mpDataContainer->getFullVariableNameWithSeparator(rSeparator));
//            }
//        }
//    }
//    return retval;
//}

void LogDataHandler::getLogDataVariableNamesWithHighestGeneration(QStringList &rNames, QList<int> &rGens) const
{
    rNames.clear(); rGens.clear();
    rNames.reserve(mLogDataMap.size());
    rGens.reserve(mLogDataMap.size());

    LogDataMapT::const_iterator dit = mLogDataMap.begin();
    for ( ; dit!=mLogDataMap.end(); ++dit)
    {
        SharedVariablePtrT pData = dit.value()->getDataGeneration(-1);
        if(pData)
        {
//            if (dit.value().mIsAlias)
//            {
//                rNames.append(dit.value().mpDataContainer->getAliasName());
//            }
//            else
//            {
//                rNames.append(dit.value().mpDataContainer->getFullVariableNameWithSeparator(rSeparator));
//            }
            rNames.append(dit.key());
            rGens.append(pData->getGeneration());
        }
    }
}

int LogDataHandler::getLatestGeneration() const
{
    return mGenerationNumber;
}


QStringList LogDataHandler::getLogDataVariableFullNames(const QString &rSeparator, const int generation) const
{
    QStringList retval;
    LogDataMapT::const_iterator dit = mLogDataMap.begin();
    for ( ; dit!=mLogDataMap.end(); ++dit)
    {
        //! @todo How to avoid aliases here FIXA  /Peter
        if(generation < 0 || dit.value()->hasDataGeneration(generation))
        {
            //retval.append(dit.value()->getFullVariableNameWithSeparator(rSeparator));
            retval.append(dit.key());
        }
    }
    return retval;
}

QList<QString> LogDataHandler::getImportedVariablesFileNames() const
{
    return mImportedLogDataMap.keys();
}

QList<SharedVariablePtrT> LogDataHandler::getImportedVariablesForFile(const QString &rFileName)
{
    ImportedLogDataMapT::iterator fit;
    fit = mImportedLogDataMap.find(rFileName);
    if (fit != mImportedLogDataMap.end())
    {
        QList<SharedVariablePtrT> results;
        // Iterate over all variables
        QStringList keys = QStringList(fit.value().keys());
        keys.removeDuplicates();
        QString key;
        Q_FOREACH(key, keys)
        {
            // value(key) Returns the most recently inserted value (latest generation)
            results.append(fit.value().value(key));
        }
        return results;
    }
    else
    {
        // Return empty list if file not found
        return QList<SharedVariablePtrT>();
    }
}

QList<int> LogDataHandler::getImportFileGenerations(const QString &rFilePath) const
{
    QList<int> results;
    ImportedLogDataMapT::const_iterator fit = mImportedLogDataMap.find(rFilePath);
    if (fit != mImportedLogDataMap.end())
    {
        // Add generation for each variable
        QMultiMap<QString, SharedVariablePtrT>::const_iterator vit;
        for (vit=fit.value().begin(); vit!=fit.value().end(); ++vit)
        {
            const int g = vit.value()->getGeneration();
            if (!results.contains(g))
            {
                results.append(g);
            }
        }
    }
    return results;
}

QMap<QString, QList<int> > LogDataHandler::getImportFilesAndGenerations() const
{
    QMap<QString, QList<int> > results;
    QList<QString> keys = mImportedLogDataMap.keys();
    Q_FOREACH(QString file, keys)
    {
        results.insert(file, getImportFileGenerations(file));
    }
    return results;
}

void LogDataHandler::removeImportedFileGenerations(const QString &rFileName)
{
    QList<int> gens = getImportFileGenerations(rFileName);
    int g;
    Q_FOREACH(g,gens)
    {
        removeGeneration(g, true);
    }
}


QString LogDataHandler::getNewCacheName()
{
    // The first dir is the main one, any other dirs have been appended later when taking ownership of someone elses data
    return mCacheDirs.first().absoluteFilePath("cf"+QString("%1").arg(mCacheSubDirCtr++));
}

void LogDataHandler::rememberIfImported(SharedVariablePtrT pData)
{
    // Remember the imported file in the import map, so we know what generations belong to which file
    if (pData->isImported())
    {
        ImportedLogDataMapT::iterator fit; // File name iterator
        fit = mImportedLogDataMap.find(pData->getImportedFileName());
        if (fit != mImportedLogDataMap.end())
        {
            fit.value().insertMulti(pData->getFullVariableName(),pData);
        }
        else
        {
            QMultiMap<QString,SharedVariablePtrT> newFileMap;
            newFileMap.insert(pData->getFullVariableName(),pData);
            mImportedLogDataMap.insert(pData->getImportedFileName(),newFileMap);
        }
        // connect delete signal so we know to remove this when generation is removed
        if (pData->getLogVariableContainer())
        {
            connect(pData->getLogVariableContainer(), SIGNAL(importedVariableBeingRemoved(SharedVariablePtrT)), this, SLOT(forgetImportedVariable(SharedVariablePtrT)));
        }

        // Make data description source know its imported
        pData->mpVariableDescription->mVariableSourceType = ImportedVariableType;
    }
}

//! @brief Removes a generation cache object from map if it has no subscribers
void LogDataHandler::removeGenerationCacheIfEmpty(const int gen)
{
    // Removes a generation cache object from map if it has no subscribers
    GenerationCacheMapT::iterator it = mGenerationCacheMap.find(gen);
    if ( (it!=mGenerationCacheMap.end()) && (it.value()->getNumSubscribers()==0) )
    {
        mGenerationCacheMap.erase(it);
    }
}

SharedVariablePtrT LogDataHandler::defineNewVectorVariable_NoNameCheck(const QString &rName, VariableTypeT type)
{
    //! @todo insertVariable maybe should be used instead
    SharedVariableDescriptionT pVarDesc = SharedVariableDescriptionT(new VariableDescription());
    pVarDesc->mDataName = rName;
    pVarDesc->mVariableSourceType = ScriptVariableType;
    LogVariableContainer *pDataContainer = new LogVariableContainer(pVarDesc->getFullName(), this);
    SharedVariablePtrT pNewData = createFreeVariable(type, pVarDesc);
    pDataContainer->addDataGeneration(mGenerationNumber, pNewData);
    mLogDataMap.insert(pVarDesc->getFullName(), pDataContainer);
    return pDataContainer->getDataGeneration(mGenerationNumber);
}

void LogDataHandler::takeOwnershipOfData(LogDataHandler *pOtherHandler, const int otherGeneration)
{
    // If otherGeneration < -1 then take everything
    if (otherGeneration < -1)
    {
        int minOGen = pOtherHandler->getLowestGenerationNumber();
        int maxOGen = pOtherHandler->getHighestGenerationNumber();
        // Since generations are not necessarily continous and same in all datavariables we try with every generation between min and max
        // We cant take them all at once, that colut change the internal ordering
        for (int i=minOGen; i<=maxOGen; ++i)
        {
            // Take one at a time
            takeOwnershipOfData(pOtherHandler, i);
        }
    }
    else
        // Take only specified generation (-1 = latest)
    {
        // Increment generation
        ++mGenerationNumber;
        bool tookOwnershipOfSomeData=false;

        // Take the generation cache map
        if (pOtherHandler->mGenerationCacheMap.contains(otherGeneration))
        {
            mGenerationCacheMap.insert(mGenerationNumber, pOtherHandler->mGenerationCacheMap.value(otherGeneration));
            pOtherHandler->mGenerationCacheMap.remove(otherGeneration);

            // Only take ownership of dir if we do not already own it
            QDir cacheDir = mGenerationCacheMap.value(mGenerationNumber)->getCacheFileInfo().absoluteDir();
            if (!mCacheDirs.contains(cacheDir))
            {
                mCacheDirs.append(cacheDir);
                //! @todo this will leave the dir in the other object aswell but I dont think it will remove the files in the dir when it dies, may need to deal with that.
            }

            tookOwnershipOfSomeData = true;
        }

        // Take the data, and only increment this->mGeneration if data was taken

        // Need to copy the map and iterate through copy, else the iterator will become invalid when removing from other
        LogDataMapT otherMap = pOtherHandler->mLogDataMap;
        LogDataMapT::iterator other_it;
        for (other_it=otherMap.begin(); other_it!=otherMap.end(); ++other_it)
        {
            if (other_it.value()->hasDataGeneration(otherGeneration))
            {
                QString keyName = other_it.key();
                insertVariable(other_it.value()->getDataGeneration(otherGeneration), keyName);

                // Remove from other
                other_it.value()->removeDataGeneration(otherGeneration, true);
                tookOwnershipOfSomeData=true;

                //! @todo how to detect if alias colision appear, how to notify user

//                LogDataMapT::iterator this_it = mLogDataMap.find(keyName);
//                if (this_it == mLogDataMap.end())
//                {
//                    // If variable does not exist, then create it
//                    LogVariableContainer *pNewContainer = new LogVariableContainer(this);
//                    pNewContainer->addDataGeneration(mGenerationNumber, other_it->mpDataContainer->getDataGeneration(otherGeneration));
//                    mLogDataMap.insert(keyName, LogDataStructT(pNewContainer,false));

//                    // Take alias
//                    const QString &aliasName = other_it->mpDataContainer->getAliasName();
//                    if (!aliasName.isEmpty())
//                    {
//                        LogDataMapT::iterator this_a_it = mLogDataMap.find(aliasName);
//                        if (this_a_it == mLogDataMap.end())
//                        {
//                            // Insert new alias
//                            mLogDataMap.insert(aliasName, LogDataStructT(pNewContainer,true));
//                        }
//                        else
//                        {
//                            //! @todo maybe handle this
//                            // If alias points to wrong thing then igonre it
//                            if (this_a_it->mpDataContainer->getFullVariableName() != keyName)
//                            {
//                                qWarning() << "In takeOwnership: Alias: "+aliasName+" LOST!!!";
//                            }
//                        }
//                    }

//                    // Remove from other, (note! Generation in alias in other will also be removed since they are pointing to the same thing)
//                    other_it->mpDataContainer->removeDataGeneration(otherGeneration, true);
//                    tookOwnershipOfSomeData=true;
//                }
//                else
//                {
//                    // Varaiable exist, append to it
//                    this_it.value().mpDataContainer->addDataGeneration(mGenerationNumber, other_it->mpDataContainer->getDataGeneration(otherGeneration));

//                    // check so that alias exist, if not add
//                    const QString &aliasName = other_it->mpDataContainer->getAliasName();
//                    if (!aliasName.isEmpty())
//                    {
//                        LogDataMapT::iterator this_a_it = mLogDataMap.find(aliasName);
//                        if (this_a_it == mLogDataMap.end())
//                        {
//                            // Insert new alias
//                            mLogDataMap.insert(aliasName, LogDataStructT(this_it.value().mpDataContainer,true));
//                        }
//                        else
//                        {
//                            //! @todo maybe handle this
//                            // If alias points to wrong thing then igonre it
//                            if (this_a_it->mpDataContainer->getFullVariableName() != keyName)
//                            {
//                                qWarning() << "In takeOwnership: Alias: "+aliasName+" LOST!!!";
//                            }
//                        }
//                    }

//                    // Remove from other, (note! Generation in alias in other will also be removed since they are pointing to the same thing)
//                    other_it->mpDataContainer->removeDataGeneration(otherGeneration, true);
//                    tookOwnershipOfSomeData=true;
//                }
            }

        }

        if (tookOwnershipOfSomeData)
        {
            // If we did actually take some data then it might be a good idea to make sure our tempvariable counter is set high enough to avoid colision
            mTempVarCtr = qMax(mTempVarCtr, pOtherHandler->mTempVarCtr);
        }
        else
        {
            // Revert generation if no data was taken
            --mGenerationNumber;
        }
    }
    //! @todo favorite variables, plotwindows, imported data
}

void LogDataHandler::registerAlias(const QString &rFullName, const QString &rAlias)
{
    QPointer<LogVariableContainer> pFullContainer = getLogVariableContainer(rFullName);
    if (pFullContainer)
    {
        // If alias is empty then we should unregister the alias
        if (rAlias.isEmpty())
        {
            const QList<int> fullGens = pFullContainer->getGenerations();
            QPointer<LogVariableContainer> pAliasContainer;
            QString currentAliasName;
            for(int i=0; i<fullGens.size(); ++i)
            {
                SharedVariablePtrT pFullData = pFullContainer->getDataGeneration(fullGens[i]);
                if (pFullData->hasAliasName())
                {
                    // Prevent alias name lookup every time if alias name has not changed
                    if (currentAliasName != pFullData->getAliasName())
                    {
                        currentAliasName = pFullData->getAliasName();
                        pAliasContainer = getLogVariableContainer(currentAliasName);
                    }

                    // Remove the alias, here we assume that the alias name at this generation is actually an alias for the fullName (the registration should have taken care of that)
                    if (pAliasContainer)
                    {
                        //! @todo will this not trigger additional remove code for the actual variable also
                        pAliasContainer->removeDataGeneration(fullGens[i]);
                    }
                    pFullData->mpVariableDescription->mAliasName = rAlias;
                }
            }

            //! @todo maybe use different signal for remove
            emit newDataAvailable();
        }
        else
        {
            QList<int> fullGens = pFullContainer->getGenerations();
            QPointer<LogVariableContainer> pAliasContainer = getLogVariableContainer(rAlias);             // Check if alias already exist
            for(int i=fullGens.size()-1; i>=0; --i)
            {
                SharedVariablePtrT pAliasData;
                if (pAliasContainer)
                {
                    pAliasData = pAliasContainer->getDataGeneration(fullGens[i]);
                }
                // Check so that there will be no colision when merging alias
                if ( pAliasData && (pAliasData->getFullVariableName() != rFullName) )
                {
                    // Abort if we reach a generation that already exist and does not match
                    //! @todo we could keep going and try every gen (but then we do not want to duplicate this warning message)
                    gpTerminalWidget->mpConsole->printWarningMessage("Alias collision when trying to merge alias, Aborting alias merge!");
                    break;
                }
                else
                {
                    // If we get here then we can merge (any previous data with alias name will be overwritten if fullname was same)
                    SharedVariablePtrT pData = pFullContainer->getDataGeneration(fullGens[i]);
                    // First unregister the old alias
                    if (pData->hasAliasName())
                    {
                        unregisterAlias(pData->getAliasName());
                    }
                    pData->mpVariableDescription->mAliasName = rAlias;
                    insertVariable(pData, rAlias, fullGens[i]);
                }
            }
            emit newDataAvailable();
        }
    }
}


void LogDataHandler::unregisterAlias(const QString &rAlias)
{
    //! @todo the container should know which generations are aliases for faster removal
    QString fullName = getFullNameFromAlias(rAlias);
    if (!fullName.isEmpty())
    {
        registerAlias(fullName, "");
    }
}

//! @brief This slot should be signaled when a variable that might be registered as imported is removed
void LogDataHandler::forgetImportedVariable(SharedVariablePtrT pData)
{
    if (pData)
    {
        // First find the correct file sub map
        ImportedLogDataMapT::iterator fit;
        fit = mImportedLogDataMap.find(pData->getImportedFileName());
        if (fit != mImportedLogDataMap.end())
        {
            // Now remove the sub map entery
            // Only remove if same variable (same key,value) (compare pointers)
            fit.value().remove(pData->getFullVariableName(),pData);
            // Now erase the file level map if it has become empty
            if (fit.value().isEmpty())
            {
                mImportedLogDataMap.erase(fit);
            }
        }
        //! @todo this code assumes that imported data can not have aliases (full name can be a short one like an alias but is technically not an alias)
    }
}

SharedVariablePtrT LogDataHandler::insertTimeVectorVariable(const QVector<double> &rTimeVector)
{
    SharedVariablePtrT pTimeVec = SharedVariablePtrT(new VectorVariable(rTimeVector, mGenerationNumber, createTimeVariableDescription(),
                                                                        getGenerationMultiCache(mGenerationNumber)));
    insertVariable(pTimeVec);
    return pTimeVec;
}

SharedVariablePtrT LogDataHandler::insertTimeVectorVariable(const QVector<double> &rTimeVector, const QString &rImportFileName)
{
    SharedVariablePtrT pTimeVec = SharedVariablePtrT(new ImportedVectorVariable(rTimeVector, mGenerationNumber, createTimeVariableDescription(),
                                                                                rImportFileName, getGenerationMultiCache(mGenerationNumber)));
    insertVariable(pTimeVec);
    return pTimeVec;
}

SharedVariablePtrT LogDataHandler::insertTimeDomainVariable(SharedVariablePtrT pTimeVector, const QVector<double> &rDataVector, SharedVariableDescriptionT pVarDesc)
{
    SharedVariablePtrT pNewData = SharedVariablePtrT(new TimeDomainVariable(pTimeVector, rDataVector, mGenerationNumber, pVarDesc,
                                                                            getGenerationMultiCache(mGenerationNumber)));
    insertVariable(pNewData);
    return pNewData;
}

SharedVariablePtrT LogDataHandler::insertTimeDomainVariable(SharedVariablePtrT pTimeVector, const QVector<double> &rDataVector, SharedVariableDescriptionT pVarDesc, const QString &rImportFileName)
{
    SharedVariablePtrT pNewData = SharedVariablePtrT(new ImportedTimeDomainVariable(pTimeVector, rDataVector, mGenerationNumber, pVarDesc,
                                                                                    rImportFileName, getGenerationMultiCache(mGenerationNumber)));
    insertVariable(pNewData);
    return pNewData;
}

//! @brief Inserts a variable into the map creating a container if needed
//! @param[in] pVariable The variable to insert
//! @param[in] keyName An alternative keyName to use (used recursivly to set an alias, do not abuse this argument)
//! @param[in] gen An alternative generation number (-1 = current))
void LogDataHandler::insertVariable(SharedVariablePtrT pVariable, QString keyName, int gen)
{
    // If keyName is empty then use fullName
    if (keyName.isEmpty())
    {
        keyName = pVariable->getFullVariableName();
    }

    // If gen -1 then use current generation
    if (gen<0)
    {
        gen = mGenerationNumber;
    }

    // First check if a data variable with this name alread exist
    LogDataMapT::iterator it = mLogDataMap.find(keyName);
    // If it exist insert into it
    if (it != mLogDataMap.end())
    {
        // Insert it into the generations map
        it.value()->addDataGeneration(gen, pVariable);
    }
    else
    {
        // Create a new toplevel map item and insert data into the generations map
        LogVariableContainer *pDataContainer = new LogVariableContainer(keyName, this);
        pDataContainer->addDataGeneration(gen, pVariable);
        mLogDataMap.insert(keyName, pDataContainer);
    }

    // Also insert alias if it exist, but only if it is different from keyName (else we will have an endless loop in here)
    if ( pVariable->hasAliasName() && (pVariable->getAliasName() != keyName) )
    {
        insertVariable(pVariable, pVariable->getAliasName(), gen);
    }

    // Remember imported files in the import map, so that we know what generations belong to which file
    rememberIfImported(pVariable);
}

bool LogDataHandler::hasLogVariableData(const QString &rFullName, const int generation)
{
    return (getLogVariableDataPtr(rFullName, generation) != 0);
}
