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
        tmp = QDir(gDesktopHandler.getLogDataPath() + QString("/handler%1").arg(ctr));
        ++ctr;
    }while(tmp.exists());
    tmp.mkpath(tmp.absolutePath());
    mCacheDirs.append(tmp);
    mCacheSubDirCtr = 0;
}

LogDataHandler::~LogDataHandler()
{
    qDebug() << "in LogDataHandler destructor" << endl;

    // Clear all data
    QList< QPointer<LogVariableContainer> > data = mLogDataMap.values();
    for (int i=0; i<data.size(); ++i)
    {
        // Check if data container ptr != NULL, to prevent double deleting aliases (its a QPointer)
        if (data[i])
        {
            delete data[i];
        }
    }
    mLogDataMap.clear();
    // Clear generation Cache files (Individual files will remain if until all instances dies)
    mGenerationCacheMap.clear();

    // Remove the cache directory if it is empty, if it is not then cleanup should happen on program exit
    for (int i=0; i<mCacheDirs.size(); ++i)
    {
        mCacheDirs[i].rmdir(mCacheDirs[i].path());
    }
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
    QVector<SharedLogVariableDataPtrT> dataPtrs;
    for(int v=0; v<rVariables.size(); ++v)
    {
        // Do not try to export missing data
        SharedLogVariableDataPtrT pData = getLogVariableDataPtr(rVariables[v],-1);
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

void LogDataHandler::exportToPlo(const QString &rFilePath, const QVector<SharedLogVariableDataPtrT> &rVariables, int version) const
{
    if ( (version < 1) || (version > 2) )
    {
        version = gConfig.getPLOExportVersion();
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

    QVector<SharedLogVariableDataPtrT> dataPtrs = rVariables;
    QVector<int> gens;
    for(int v=0; v<dataPtrs.size(); ++v)
    {
        gens.append(dataPtrs.last()->getGeneration());
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
    SharedLogVariableDataPtrT pTime = getLogVariableDataPtr(TIMEVARIABLENAME, timeGen);
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

void LogDataHandler::exportToCSV(const QString &rFilePath, const QVector<SharedLogVariableDataPtrT> &rVariables) const
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
    QVector<SharedLogVariableDataPtrT> vars = getAllVariablesAtGeneration(gen);
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

void LogDataHandler::importFromPlo(QString rImportFilePath)
{
    if(rImportFilePath.isEmpty())
    {

        rImportFilePath = QFileDialog::getOpenFileName(0,tr("Choose Hopsan .plo File"),
                                                       gConfig.getPlotDataDir(),
                                                       tr("Hopsan File (*.plo)"));
    }
    if(rImportFilePath.isEmpty())
    {
        return;
    }

    QFile file(rImportFilePath);
    QFileInfo fileInfo(file);
    gConfig.setPlotDataDir(fileInfo.absolutePath());

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
        SharedLogVariableDataPtrT timeVecPtr(0);

        VariableUniqueDescription varUniqueDesc;
        varUniqueDesc.mVariableSourceType = ImportedVariableType;
        varUniqueDesc.mImportFileName = fileInfo.fileName();

        if (importedPLODataVector[0].mDataName == TIMEVARIABLENAME)
        {
            timeVecPtr = insertTimeVariable(importedPLODataVector.first().mDataValues, &varUniqueDesc);
        }

        for (int i=1; i<importedPLODataVector.size(); ++i)
        {
            VariableCommonDescription varCommonDesc;
            varCommonDesc.mDataName = importedPLODataVector[i].mDataName;
            //! @todo what about reading the unit

            SharedLogVariableDataPtrT pNewData = insertVariableBasedOnDescription(varCommonDesc, &varUniqueDesc, timeVecPtr, importedPLODataVector[i].mDataValues);
            pNewData->setPlotScale(importedPLODataVector[i].mPlotScale);
        }

        // We do not want imported data to be removed automatically
        preventGenerationAutoRemoval(mGenerationNumber);

        emit newDataAvailable();
    }

    // Limit number of plot generations if there are too many
    limitPlotGenerations();
}


void LogDataHandler::importFromCsv(QString rImportFilePath)
{
    if(rImportFilePath.isEmpty())
    {

        rImportFilePath = QFileDialog::getOpenFileName(0,tr("Choose .csv File"),
                                                       gConfig.getPlotDataDir(),
                                                       tr("Comma-separated values files (*.csv)"));
    }
    if(rImportFilePath.isEmpty())
    {
        return;
    }

    QFile file(rImportFilePath);
    QFileInfo fileInfo(file);
    gConfig.setPlotDataDir(fileInfo.absolutePath());

    CoreCSVParserAccess *pParser = new CoreCSVParserAccess(rImportFilePath);

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
        SharedLogVariableDataPtrT timeVecPtr(0);

        VariableUniqueDescription varUniqueDesc;
        varUniqueDesc.mVariableSourceType = ImportedVariableType;
        varUniqueDesc.mImportFileName = fileInfo.fileName();

        //Ugly, assume that first vector is always time
        timeVecPtr = insertTimeVariable(data[0], &varUniqueDesc);

        for (int i=1; i<data.size(); ++i)
        {
            VariableCommonDescription varDesc;
            varDesc.mDataName = "CSV"+QString::number(i);

            insertVariableBasedOnDescription(varDesc, &varUniqueDesc, timeVecPtr, data[i]);
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

            VariableUniqueDescription varUniqueDesc;
            varUniqueDesc.mVariableSourceType = ImportedVariableType;
            QFileInfo fileInfo(csvFilePath);
            varUniqueDesc.mImportFileName = fileInfo.fileName();

            SharedLogVariableDataPtrT pTime = insertTimeVariable(timeColumn, &varUniqueDesc);

            // Ok now we have the data lets add it as a variable
            for (int n=0; n<names.size(); ++n)
            {
                //! @todo what if data name already exists?

                VariableCommonDescription varDesc;
                varDesc.mDataName = names[n];
                insertVariableBasedOnDescription(varDesc, &varUniqueDesc, pTime, dataColumns[n]);
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
    // Remove old generation in each data variable container
    mLogDataMap.clear();

    // Clear from generations cache object map
    mGenerationCacheMap.clear();

    //Update generation number
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
    SharedLogVariableDataPtrT timeVecPtr;

    //! @todo why not run multiappend when overwriting generation ? Baecouse tehn we are not appending, need som common open mode
    if(!overWriteLastGeneration)
    {
        this->getOrCreateGenerationMultiCache(mGenerationNumber)->beginMultiAppend();
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
                if ( gConfig.getShowHiddenNodeDataVariables() || (varDescs[i].mNodeDataVariableType != "Hidden") )
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
                            QVector<double> timeVec = QVector<double>::fromStdVector(*pTimeVector);//!< @todo not copy here, should maybe rewrite makeSureUniqe to handled std::vector also
                            //SharedTimeVectorPtrT timeVecPtr = timeVecHelper.makeSureUnique(timeVec);

                            timeVecPtr = insertTimeVariable(timeVec, 0);

                            // Set the custom unit scaling to the default
                            QString defaultTimeUnit = gConfig.getDefaultUnit(TIMEVARIABLENAME);
                            if (defaultTimeUnit != timeVecPtr->getDataUnit())
                            {
                                timeVecPtr->setCustomUnitScale(UnitScale(defaultTimeUnit, gConfig.getUnitScale(TIMEVARIABLENAME, defaultTimeUnit)));
                            }

                            timeVectorObtained = true;
                        }

                        foundData=true;
                        VariableCommonDescription varDesc;
                        varDesc.mModelPath = pModelObject->getParentContainerObject()->getModelFileInfo().fileName();
                        varDesc.mComponentName = pModelObject->getName();
                        varDesc.mPortName = (*pit)->getName();
                        varDesc.mDataName = varDescs[i].mName;
                        varDesc.mDataUnit = varDescs[i].mUnit;
                        varDesc.mDataDescription = varDescs[i].mDescription;
                        varDesc.mAliasName  = varDescs[i].mAlias;
                        varDesc.mVariableSourceType = ModelVariableType;

                        SharedLogVariableDataPtrT pNewData = insertVariableBasedOnDescription(varDesc, 0, timeVecPtr, dataVec);

                        UnitScale us;
                        pModelObject->getCustomPlotUnitOrScale(varDesc.mPortName+"#"+varDesc.mDataName, us);
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
        this->getOrCreateGenerationMultiCache(mGenerationNumber)->endMultiAppend();
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
    QVector<SharedLogVariableDataPtrT> vars = getAllVariablesAtGeneration(gen);

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
    SharedLogVariableDataPtrT pData = getLogVariableDataPtr(generation, componentName, portName, dataName);
    if (pData)
    {
        return pData->getDataVectorCopy();
    }

    return QVector<double>();
}

QVector<double> LogDataHandler::copyLogDataVariableValues(const QString &rName, const int generation)
{
    SharedLogVariableDataPtrT pData = getLogVariableDataPtr(rName, generation);
    if (pData)
    {
        return pData->getDataVectorCopy();
    }

    return QVector<double>();
}

//! @deprecated
SharedLogVariableDataPtrT LogDataHandler::getLogVariableDataPtr(int generation, QString componentName, QString portName, QString dataName)
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

SharedLogVariableDataPtrT LogDataHandler::getLogVariableDataPtr(const QString &rName, const int generation) const
{
    // Find the data variable
    LogDataMapT::const_iterator dit = mLogDataMap.find(rName);
    if (dit != mLogDataMap.end())
    {
        return dit.value()->getDataGeneration(generation);
    }

    // If not found return 0
    return SharedLogVariableDataPtrT(0);
}

//! @brief Returns multiple logdatavariables based on regular expression search. Exluding temp variables.
//! @param [in] rNameExp The regular expression for the names to match
//! @param [in] generation The desired generation of the variable
QVector<SharedLogVariableDataPtrT> LogDataHandler::getMultipleLogVariableDataPtrs(const QRegExp &rNameExp, const int generation) const
{
    QVector<SharedLogVariableDataPtrT> results;
    LogDataMapT::const_iterator it;
    for (it = mLogDataMap.begin(); it != mLogDataMap.end(); it++)
    {
        // For any non temp variable compare name with regexp
        if ((it.value()->getVariableCommonDescription()->mVariableSourceType != TempVariableType) && rNameExp.exactMatch(it.key()))
        {
            SharedLogVariableDataPtrT pData = it.value()->getDataGeneration(generation);
            if (pData)
            {
                results.append(pData);
            }
        }
    }
    return results;
}

////! @todo maybe would be better if ONE getPlotData function could handle all cases
//SharedLogVariableDataPtrT LogDataHandler::getPlotDataByAlias(const QString alias, const int generation)
//{
//    // First find the data variable
//    AliasMapT::iterator dit = mPlotAliasMap.find(alias);
//    if (dit != mPlotAliasMap.end())
//    {
//        return dit.value()->getDataGeneration(generation);
//    }
//    return SharedLogVariableDataPtrT(0);
//}


//! @brief Returns the time vector for specified generation
//! @param[in] generation Generation
const SharedLogVariableDataPtrT LogDataHandler::getTimeVectorPtr(int generation) const
{
    LogVariableContainer *pCont = mLogDataMap.value(TIMEVARIABLENAME,0);
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
        return SharedLogVariableDataPtrT();
    }
}

QVector<double> LogDataHandler::copyTimeVector(const int generation) const
{
    SharedLogVariableDataPtrT pTime = getTimeVectorPtr(generation);
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


//! @brief Let's the user define a new alias for specified plot varaible
//! @param[in] componentName Name of component
//! @param[in] portName Name of port
//! @param[in] dataName Name of data variable
//! @param[in] dataUnit Unit of variable
void LogDataHandler::definePlotAlias(QString fullName)
{
    bool ok;
    QString alias = QInputDialog::getText(gpMainWindowWidget, gpMainWindowWidget->tr("Define Variable Alias"),
                                     "Alias:", QLineEdit::Normal, "", &ok);
    if(ok)
    {
       definePlotAlias(alias, fullName);
    }
}


//! @brief Defines a new alias for specified plot varaible
//! @param[in] alias Alias name for variable
//! @param[in] componentName Name of component
//! @param[in] portName Name of port
//! @param[in] dataName Name of data variable
//! @param[in] dataUnit Unit of variable
//! @todo this code should no longer be in LogDataHandler it should be in system or similar
bool LogDataHandler::definePlotAlias(const QString alias, const QString fullName)
{
    QString comp,port,var;
    splitConcatName(fullName, comp,port,var);
    // Try to set the new alias, abort if it did not work
    return mpParentContainerObject->setVariableAlias(comp,port,var,alias);

//    if(!mpParentContainerObject->setVariableAlias(comp,port,var,alias))
//    {
//        return false;
//    }

//    LogVariableContainer *pData = mLogDataMap.value(fullName);
//    if (pData)
//    {
//        // Undefine old alias first
//        //! @todo this will remove alias from other if it alread exist mayb a question should be given to user
//        if (mLogDataMap.contains(pData->getAliasName()))
//        {
//            undefinePlotAlias(pData->getAliasName());
//        }
//        //! @todo instead of bool return the uniqe changed alias should be returned
//        mLogDataMap.insert(alias, pData);
//        pData->setAliasName(alias);


//        //! @todo This is madness, this code needs to be rewritten, maybe alias should not be changable from log data like this, or the code should be smarter
//        //! @todo we also need a signal for when alias name is changed elsewhere
//        // since the alias was undefined above, set it again
//        mpParentContainerObject->setVariableAlias(comp,port,var,alias);
//    }

//    return true;
}


//! @brief Removes specified variable alias
//! @param[in] alias Alias to remove
void LogDataHandler::undefinePlotAlias(const QString &rAlias)
{
    LogVariableContainer *data = mLogDataMap.value(rAlias, 0);
    if (data)
    {
        QString comp,port,var;
        splitConcatName(data->getFullVariableName(), comp,port,var);
        mpParentContainerObject->setVariableAlias(comp,port,var,""); //!< @todo maybe a remove alias function would be nice
    }
}


//! @brief Returns plot variable for specified alias
//! @param[in] alias Alias of variable
QString LogDataHandler::getFullNameFromAlias(QString alias)
{
    //LogVariableContainer *pDataContainer = mPlotAliasMap.value(alias, 0);
    LogVariableContainer *data = mLogDataMap.value(alias, 0);
    if (data)
    {
        return data->getFullVariableName();
    }
    return QString();
}


//! @brief Returns plot alias for specified variable
//! @param[in] componentName Name of component
//! @param[in] portName Name of port
//! @param[in] dataName Name of data variable
QString LogDataHandler::getAliasFromFullName(QString fullName)
{
    LogVariableContainer *pDataContainer = mLogDataMap.value(fullName, 0);
    if (pDataContainer)
    {
        return pDataContainer->getAliasName();
    }
    return QString();
}

//! @todo logdatahandler needs to keep track of the generations the it contains, we should not need to ask for each one every time, this could take a long time
QList<int> LogDataHandler::getGenerations() const
{
    QMap<int, void*> allGens;

    // Ask each data variable what gens it is availible in
    LogDataMapT::const_iterator it;
    for (it=mLogDataMap.begin(); it!=mLogDataMap.end(); ++it)
    {
        QList<int> gens = it->data()->getGenerations();
        QList<int>::iterator it;
        for (it=gens.begin(); it!=gens.end(); ++it)
        {
            allGens.insert(*it, 0);
        }
    }

    // Now allGens should contain a unique set of all available generations
    return allGens.keys();
}

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
    if (numGens > gConfig.getGenerationLimit())
    {
        if(!gConfig.getAutoLimitLogDataGenerations())
        {
            QDialog *pDialog = new QDialog(gpMainWindowWidget);
            pDialog->setWindowTitle("Hopsan");
            QVBoxLayout *pLayout = new QVBoxLayout(pDialog);
            QLabel *pLabel = new QLabel("<b>Log data generation limit reached!</b><br><br>Generation limit: "+QString::number(gConfig.getGenerationLimit())+"<br>Number of data generations: "+QString::number(getNumberOfGenerations())+"<br><br><b>Discard "+QString::number(getNumberOfGenerations()-gConfig.getGenerationLimit())+" generations(s)?</b>");
            QCheckBox *pAutoLimitCheckBox = new QCheckBox("Automatically discard last generation", pDialog);
            pAutoLimitCheckBox->setChecked(false);
            QDialogButtonBox *pButtonBox = new QDialogButtonBox(pDialog);
            QPushButton *pDiscardButton = pButtonBox->addButton("Discard", QDialogButtonBox::AcceptRole);
            QPushButton *pKeepButton = pButtonBox->addButton("Keep", QDialogButtonBox::RejectRole);
            connect(pDiscardButton, SIGNAL(clicked()), pDialog, SLOT(accept()));
            connect(pKeepButton, SIGNAL(clicked()), pDialog, SLOT(reject()));
            connect(pAutoLimitCheckBox, SIGNAL(toggled(bool)), pKeepButton, SLOT(setDisabled(bool)));
            pLayout->addWidget(pLabel);
            pLayout->addWidget(pAutoLimitCheckBox);
            pLayout->addWidget(pButtonBox);

            int retval = pDialog->exec();

            gConfig.setAutoLimitLogDataGenerations(pAutoLimitCheckBox->isChecked());

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
// This is NEW code by Peter, but it is not working yet, do not remove it
        TicToc timer;
        int highest = getHighestGenerationNumber();
        // Note! Here we must iterate through a copy of the values from the map
        // if we use an iterator in the map we will crash if the removed generation is the final one
        // Then the data variable itself will also be removed and the iterator will become invalid
        QList<QPointer<LogVariableContainer> > vars = mLogDataMap.values();
        QList<QPointer<LogVariableContainer> >::iterator it;
        for ( it=vars.begin(); it!=vars.end(); ++it)
        {
            (*it)->purgeOldGenerations(highest-gConfig.getGenerationLimit(), gConfig.getGenerationLimit());
        }
        emit dataRemoved();
        //! @todo what about clearing mGenerationCacheMap
        timer.toc("removeOldGenerations");
        qDebug() << "Number of logdata variables: " << getNumVariables();
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
    // Note! Here we must iterate through a copy of the values from the map
    // if we use an iterator in the map we will crash if the removed generation is the final one
    // Then the data variable itself will also be removed and the iterator will become invalid
    QList<QPointer<LogVariableContainer> > vars = mLogDataMap.values();
    QList<QPointer<LogVariableContainer> >::iterator it;
    for ( it=vars.begin(); it!=vars.end(); ++it)
    {
        (*it)->removeDataGeneration(gen,force);
    }
    //! @todo this should not be emmited if no data was actaully removed (if force=false)
    emit dataRemoved();
    //! @todo what about generation cache map it is not cleared, and should we realy clear it always, what if there were some variables that hade autoremove disabled
}

ContainerObject *LogDataHandler::getParentContainerObject()
{
    return mpParentContainerObject;
}

const QList<QDir> &LogDataHandler::getCacheDirs() const
{
    return mCacheDirs;
}

SharedMultiDataVectorCacheT LogDataHandler::getOrCreateGenerationMultiCache(const int gen)
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
    SharedLogVariableDataPtrT pData1 = getLogVariableDataPtr(a, -1);
    if( (pData1 == NULL))
    {
        return NULL;
    }
    else
    {
        SharedLogVariableDataPtrT pTemp = addVariableWithScalar(pData1,x);
        return pTemp->getFullVariableName();
    }
}

SharedLogVariableDataPtrT LogDataHandler::addVariableWithScalar(const SharedLogVariableDataPtrT a, const double x)
{
    SharedLogVariableDataPtrT pTempVar = defineTempVariable(a->getFullVariableName()+"AddedWith"+QString::number(x));
    pTempVar->assignFrom(a);
    pTempVar->addToData(x);
    return pTempVar;
}


QString LogDataHandler::subVariableWithScalar(const QString &a, const double x)
{
    SharedLogVariableDataPtrT pData1 = getLogVariableDataPtr(a, -1);
    if( (pData1 == NULL))
    {
        return NULL;
    }
    else
    {
        SharedLogVariableDataPtrT pTemp = subVariableWithScalar(pData1,x);
        return pTemp->getFullVariableName();
    }
}

SharedLogVariableDataPtrT LogDataHandler::subVariableWithScalar(const SharedLogVariableDataPtrT a, const double x)
{
    SharedLogVariableDataPtrT pTempVar = defineTempVariable(a->getFullVariableName()+"SubtractedWith"+QString::number(x));
    pTempVar->assignFrom(a);
    pTempVar->subFromData(x);
    return pTempVar;
}


QString LogDataHandler::mulVariableWithScalar(const QString &a, const double x)
{
    SharedLogVariableDataPtrT pData1 = getLogVariableDataPtr(a, -1);
    if( (pData1 == NULL))
    {
        return NULL;
    }
    else
    {
        SharedLogVariableDataPtrT pTemp = mulVariableWithScalar(pData1,x);
        return pTemp->getFullVariableName();
    }
}

SharedLogVariableDataPtrT LogDataHandler::mulVariableWithScalar(const SharedLogVariableDataPtrT a, const double x)
{
    SharedLogVariableDataPtrT pTempVar = defineTempVariable(a->getFullVariableName()+"MultiplicatedWith"+QString::number(x));
    pTempVar->assignFrom(a);
    pTempVar->multData(x);
    return pTempVar;
}


QString LogDataHandler::divVariableWithScalar(const QString &a, const double x)
{
    SharedLogVariableDataPtrT pData1 = getLogVariableDataPtr(a, -1);
    if( (pData1 == NULL))
    {
        return NULL;
    }
    else
    {
        SharedLogVariableDataPtrT pTemp = divVariableWithScalar(pData1,x);
        return pTemp->getFullVariableName();
    }
}

SharedLogVariableDataPtrT LogDataHandler::divVariableWithScalar(const SharedLogVariableDataPtrT a, const double x)
{
    SharedLogVariableDataPtrT pTempVar = defineTempVariable(a->getFullVariableName()+"dividedWith"+QString::number(x));
    pTempVar->assignFrom(a);
    pTempVar->divData(x);
    return pTempVar;
}


QString LogDataHandler::addVariables(const QString &a, const QString &b)
{
    SharedLogVariableDataPtrT pData1 = getLogVariableDataPtr(a, -1);
    SharedLogVariableDataPtrT pData2 = getLogVariableDataPtr(b, -1);

    if( (pData1 == 0) || (pData2 == 0) )
    {
        return QString();
    }
    else
    {
        SharedLogVariableDataPtrT pTemp = addVariables(pData1,pData2);
        return pTemp->getFullVariableName();
    }
}

SharedLogVariableDataPtrT LogDataHandler::addVariables(const SharedLogVariableDataPtrT a, const SharedLogVariableDataPtrT b)
{
    SharedLogVariableDataPtrT pTempVar = defineTempVariable(a->getFullVariableName()+"plus"+b->getFullVariableName());
    pTempVar->assignFrom(a);
    pTempVar->addToData(b);
    return pTempVar;
}

QString LogDataHandler::subVariables(const QString &a, const QString &b)
{
    SharedLogVariableDataPtrT pData1 = getLogVariableDataPtr(a, -1);
    SharedLogVariableDataPtrT pData2 = getLogVariableDataPtr(b, -1);

    if( (pData1 == 0) || (pData2 == 0) )
    {
        return QString();
    }
    else
    {
        SharedLogVariableDataPtrT pTemp = subVariables(pData1,pData2);
        return pTemp->getFullVariableName();
    }
}

QString LogDataHandler::multVariables(const QString &a, const QString &b)
{
    SharedLogVariableDataPtrT pData1 = getLogVariableDataPtr(a, -1);
    SharedLogVariableDataPtrT pData2 = getLogVariableDataPtr(b, -1);

    if( (pData1 == 0) || (pData2 == 0) )
    {
        return QString();
    }
    else
    {
        SharedLogVariableDataPtrT pTemp = multVariables(pData1,pData2);
        return pTemp->getFullVariableName();
    }
}

QString LogDataHandler::divVariables(const QString &a, const QString &b)
{
    SharedLogVariableDataPtrT pData1 = getLogVariableDataPtr(a, -1);
    SharedLogVariableDataPtrT pData2 = getLogVariableDataPtr(b, -1);

    if( (pData1 == 0) || (pData2 == 0) )
    {
        return QString();
    }
    else
    {
        SharedLogVariableDataPtrT pTemp = divVariables(pData1,pData2);
        return pTemp->getFullVariableName();
    }
}

SharedLogVariableDataPtrT LogDataHandler::diffVariables(const SharedLogVariableDataPtrT a, const SharedLogVariableDataPtrT b)
{
    SharedLogVariableDataPtrT pTempVar = defineTempVariable(a->getFullVariableName()+"Diff");
    pTempVar->assignFrom(a);
    pTempVar->diffBy(b);
    return pTempVar;
}

QString LogDataHandler::diffVariables(const QString &a, const QString &b)
{
    SharedLogVariableDataPtrT pData1 = getLogVariableDataPtr(a, -1);
    SharedLogVariableDataPtrT pData2;
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
        SharedLogVariableDataPtrT pTemp = diffVariables(pData1,pData2);
        return pTemp->getFullVariableName();
    }
}

SharedLogVariableDataPtrT LogDataHandler::lowPassFilterVariable(const SharedLogVariableDataPtrT a, const SharedLogVariableDataPtrT b, const double freq)
{
    SharedLogVariableDataPtrT pTempVar = defineTempVariable(a->getFullVariableName()+"Lp1");
    pTempVar->assignFrom(a);
    pTempVar->lowPassFilter(b, freq);
    return pTempVar;
}

QString LogDataHandler::lowPassFilterVariable(const QString &a, const QString &b, const double freq)
{
    SharedLogVariableDataPtrT pData1 = getLogVariableDataPtr(a, -1);
    SharedLogVariableDataPtrT pData2;
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
        SharedLogVariableDataPtrT pTemp = lowPassFilterVariable(pData1,pData2,freq);
        return pTemp->getFullVariableName();
    }
}

SharedLogVariableDataPtrT LogDataHandler::fftVariable(const SharedLogVariableDataPtrT a, const SharedLogVariableDataPtrT b, const bool doPowerSpectrum)
{
    SharedLogVariableDataPtrT pTempVar = defineTempVariable(a->getFullVariableName()+"Lp1");
    pTempVar->assignFrom(a);
    pTempVar->frequencySpectrum(b, doPowerSpectrum);
    return pTempVar;
}

QString LogDataHandler::fftVariable(const QString &a, const QString &b, const bool doPowerSpectrum)
{
    SharedLogVariableDataPtrT pData1 = getLogVariableDataPtr(a, -1);
    SharedLogVariableDataPtrT pData2;
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
        SharedLogVariableDataPtrT pTemp = fftVariable(pData1,pData2,doPowerSpectrum);
        return pTemp->getFullVariableName();
    }
}



QString LogDataHandler::assignVariable(const QString &dst, const QString &src)
{
    SharedLogVariableDataPtrT pDstData = getLogVariableDataPtr(dst, -1);
    SharedLogVariableDataPtrT pSrcData = getLogVariableDataPtr(src, -1);

    if(pSrcData == 0)
    {
        return QString();
    }
    else if(pDstData == 0)
    {
        pDstData = defineNewVariable(dst);
    }
    return assignVariable(pDstData,pSrcData)->getFullVariableName();
}

QString LogDataHandler::assignVariable(const QString &dst, const QVector<double> &src)
{
    SharedLogVariableDataPtrT pDstData = getLogVariableDataPtr(dst, -1);
    if (!pDstData)
    {
        pDstData = defineNewVariable(dst);
    }

    // Check again if new def was succesfull
    if (pDstData)
    {
        pDstData->assignFrom(src);
    }
    return pDstData->getFullVariableName();
}

double LogDataHandler::pokeVariable(const QString &a, const int index, const double value)
{
    SharedLogVariableDataPtrT pData1 = getLogVariableDataPtr(a, -1);
    if(pData1 != 0)
    {
        return pokeVariable(pData1, index, value);
    }
    gpTerminalWidget->mpConsole->printErrorMessage("In Poke, No such variable: " + a);
    return 0;
}

bool LogDataHandler::deleteVariable(const QString &a)
{
    LogDataMapT::iterator it = mLogDataMap.find(a);
    if(it != mLogDataMap.end())
    {
        // Figure out if we are deleting by alias name or full name
        if (it.value()->getAliasName() == a)
        {
            return deleteVariable(it.value()->getFullVariableName());
        }
        else
        {
            // Remember alias if any
            QString alias = it.value()->getAliasName();
            // Delete the date
            it.value()->deleteLater();
            // Remove data ptr from map
            mLogDataMap.erase(it);
            // If an alias was present then also remove it from map
            if (!alias.isEmpty())
            {
                mLogDataMap.remove(alias);
            }
            emit dataRemoved();
            return true;
        }
    }
    gpTerminalWidget->mpConsole->printErrorMessage("In Delete, No such variable: " + a);
    return false;
}

//! @brief Returns the number of log data variables registered in this log data handler
//! @returns The number of registered log data variables
int LogDataHandler::getNumVariables() const
{
    return mLogDataMap.size();
}

bool LogDataHandler::deleteVariable(SharedLogVariableDataPtrT a)
{
    return deleteVariable(a->getFullVariableName());
}

double LogDataHandler::peekVariable(const QString &a, const int index)
{
    SharedLogVariableDataPtrT pData1 = getLogVariableDataPtr(a, -1);
    if(pData1)
    {
        return peekVariable(pData1,index);
    }
    gpTerminalWidget->mpConsole->printErrorMessage("In Peek, No such variable: " + a);
    return 0;
}

SharedLogVariableDataPtrT LogDataHandler::elementWiseGT(SharedLogVariableDataPtrT pData, const double thresh)
{
    if (pData)
    {
        SharedLogVariableDataPtrT pTempVar = defineTempVariable(pData->getFullVariableName()+"gt");
        QVector<double> res;
        pData->elementWiseGt(res,thresh);
        pTempVar->assignFrom(res);
        return pTempVar;
    }
    return SharedLogVariableDataPtrT();
}

SharedLogVariableDataPtrT LogDataHandler::elementWiseLT(SharedLogVariableDataPtrT pData, const double thresh)
{
    if (pData)
    {
        SharedLogVariableDataPtrT pTempVar = defineTempVariable(pData->getFullVariableName()+"gt");
        QVector<double> res;
        pData->elementWiseLt(res,thresh);
        pTempVar->assignFrom(res);
        return pTempVar;
    }
    return SharedLogVariableDataPtrT();
}

QString LogDataHandler::saveVariable(const QString &currName, const QString &newName)
{
    SharedLogVariableDataPtrT pCurrData = getLogVariableDataPtr(currName, -1);
    SharedLogVariableDataPtrT pNewData = getLogVariableDataPtr(newName, -1);
    // If curr data exist and new data does not exist
    if( (pNewData == 0) && (pCurrData != 0) )
    {
        SharedLogVariableDataPtrT pNewData = defineNewVariable(newName);
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

SharedLogVariableDataPtrT LogDataHandler::subVariables(const SharedLogVariableDataPtrT a, const SharedLogVariableDataPtrT b)
{
    SharedLogVariableDataPtrT pTempVar = defineTempVariable(a->getFullVariableName()+b->getFullVariableName());
    pTempVar->assignFrom(a);
    pTempVar->subFromData(b);
    return pTempVar;
}

SharedLogVariableDataPtrT LogDataHandler::multVariables(const SharedLogVariableDataPtrT a, const SharedLogVariableDataPtrT b)
{
    SharedLogVariableDataPtrT pTempVar = defineTempVariable(a->getFullVariableName()+b->getFullVariableName());
    pTempVar->assignFrom(a);
    pTempVar->multData(b);
    return pTempVar;
}

SharedLogVariableDataPtrT LogDataHandler::divVariables(const SharedLogVariableDataPtrT a, const SharedLogVariableDataPtrT b)
{
    SharedLogVariableDataPtrT pTempVar = defineTempVariable(a->getFullVariableName()+b->getFullVariableName());
    pTempVar->assignFrom(a);
    pTempVar->divData(b);
    return pTempVar;
}

//! @todo Should this function really return a value?
SharedLogVariableDataPtrT LogDataHandler::assignVariable(SharedLogVariableDataPtrT dst, const SharedLogVariableDataPtrT src)
{
    dst->assignFrom(src);
    return dst;
}

double LogDataHandler::pokeVariable(SharedLogVariableDataPtrT a, const int index, const double value)
{
    QString err;
    double r = a->pokeData(index,value,err);
    if (!err.isEmpty())
    {
        gpTerminalWidget->mpConsole->printErrorMessage(err);
    }
    return r;
}

SharedLogVariableDataPtrT LogDataHandler::saveVariable(SharedLogVariableDataPtrT a)
{
    SharedLogVariableDataPtrT pTempVar = defineTempVariable(a->getFullVariableName());
    pTempVar->assignFrom(a);
    return pTempVar;
}

double LogDataHandler::peekVariable(SharedLogVariableDataPtrT a, const int index)
{
    QString err;
    double r = a->peekData(index, err);
    if (!err.isEmpty())
    {
        gpTerminalWidget->mpConsole->printErrorMessage(err);
    }
    return r;
}

SharedLogVariableDataPtrT LogDataHandler::defineTempVariable(QString desiredname)
{
    QString numStr;
    numStr.setNum(mTempVarCtr);
    desiredname.append(numStr);
    SharedLogVariableDataPtrT pData = defineNewVariable(desiredname);
    pData->mpVariableCommonDescription->mVariableSourceType = TempVariableType;
    if (pData)
    {
       ++mTempVarCtr;
    }
    return pData;
}


void LogDataHandler::appendVariable(const QString &a, const double x, const double y)
{
    SharedLogVariableDataPtrT pData1 = getLogVariableDataPtr(a, -1);
    if(pData1)
    {
        pData1->append(x,y);
        return;
    }
    gpTerminalWidget->mpConsole->printErrorMessage("No such variable: " + a);
    return;
}

SharedLogVariableDataPtrT LogDataHandler::defineNewVariable(const QString desiredname)
{
    if(mLogDataMap.find(desiredname) == mLogDataMap.end())
    {
        VariableCommonDescription varDesc;
        varDesc.mDataName = desiredname;
        varDesc.mVariableSourceType = ScriptVariableType;
        LogVariableContainer *pDataContainer = new LogVariableContainer(varDesc, this);
        pDataContainer->addDataGeneration(mGenerationNumber, QVector<double>(), QVector<double>());
        mLogDataMap.insert(varDesc.getFullName(), pDataContainer);
        return pDataContainer->getDataGeneration(mGenerationNumber);
    }
    return SharedLogVariableDataPtrT();
}

SharedLogVariableDataPtrT LogDataHandler::defineNewVariable(const QString desiredname, const QString &rUnit, const QString &rDescription)
{
    SharedLogVariableDataPtrT pData = defineNewVariable(desiredname);
    pData->mpVariableCommonDescription->mDataUnit = rUnit;
    pData->mpVariableCommonDescription->mDataUnit = rDescription;
    return pData;
}

//SharedLogVariableDataPtrT LogDataHandler::defineNewVariable(const QString desiredname, QVector<double> x, QVector<double> y)
//{
//    VariableDescription varDesc;
//    varDesc.mDataName = desiredname;
//    varDesc.mVariableSourceType = VariableDescription::ScriptVariableType;
//    LogVariableContainer *pDataContainer = new LogVariableContainer(varDesc, this);
//    pDataContainer->addDataGeneration(mGenerationNumber, x, y);
//    mLogDataMap.insert(varDesc.getFullName(), pDataContainer);
//    return pDataContainer->getDataGeneration(mGenerationNumber);
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
    VariableCommonDescription tempVariable;
    tempVariable.mComponentName = componentName;
    tempVariable.mPortName = portName;
    tempVariable.mDataName = dataName;
    tempVariable.mDataUnit = dataUnit;
    if(!mFavoriteVariables.contains(tempVariable))
    {
        mFavoriteVariables.append(tempVariable);
    }
    gpPlotWidget->mpPlotVariableTree->updateList();

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
            gpPlotWidget->mpPlotVariableTree->updateList();
            return;
        }
    }
}

QString LogDataHandler::plotVariable(const QString plotName, const QString fullVarName, const int gen, const int axis, QColor color)
{
    SharedLogVariableDataPtrT pData = getLogVariableDataPtr(fullVarName, gen);
    if(pData)
    {
        return gpPlotHandler->plotDataToWindow(plotName, pData, axis, color);
    }
    return "";
}

QString LogDataHandler::plotVariable(const QString plotName, const QString &rFullNameX, const QString &rFullNameY, const int gen, const int axis, QColor color)
{
    SharedLogVariableDataPtrT pDataX = getLogVariableDataPtr(rFullNameX, gen);
    SharedLogVariableDataPtrT pDataY = getLogVariableDataPtr(rFullNameY, gen);
    if (pDataX && pDataY)
    {
        return gpPlotHandler->plotDataToWindow(plotName, pDataX, pDataY, axis, color);
    }
    return "";
}

PlotWindow *LogDataHandler::plotVariable(PlotWindow *pPlotWindow, const QString fullVarName, const int gen, const int axis, QColor color)
{
    SharedLogVariableDataPtrT pData = getLogVariableDataPtr(fullVarName, gen);
    if(pData)
    {
        return gpPlotHandler->plotDataToWindow(pPlotWindow, pData, axis, color);
    }
    return 0;
}

QVector<SharedLogVariableDataPtrT> LogDataHandler::getAllVariablesAtNewestGeneration()
{
    QVector<SharedLogVariableDataPtrT> dataPtrVector;

    // First go through all data variable
    LogDataMapT::iterator dit = mLogDataMap.begin();
    for ( ; dit!=mLogDataMap.end(); ++dit)
    {
        SharedLogVariableDataPtrT pData = dit.value()->getDataGeneration(-1);
        if (pData)
        {
            dataPtrVector.push_back(pData);
        }
    }

    return dataPtrVector;
}

QVector<SharedLogVariableDataPtrT> LogDataHandler::getAllVariablesAtGeneration(const int generation) const
{
    QVector<SharedLogVariableDataPtrT> dataPtrVector;

    // First go through all data variable
    LogDataMapT::const_iterator dit = mLogDataMap.begin();
    for ( ; dit!=mLogDataMap.end(); ++dit)
    {
        // Now try to find given generation
        SharedLogVariableDataPtrT pData = dit.value()->getDataGeneration(generation);
        if (pData)
        {
            dataPtrVector.push_back(pData);
        }
    }

    return dataPtrVector;
}

int LogDataHandler::getLatestGeneration() const
{
    return mGenerationNumber;
}


QStringList LogDataHandler::getLogDataVariableNames(const QString &rSeparator, const int generation) const
{
    QStringList retval;
    LogDataMapT::const_iterator dit = mLogDataMap.begin();
    for ( ; dit!=mLogDataMap.end(); ++dit)
    {
        if(generation < 0 || dit.value()->hasDataGeneration(generation))
        {
            retval.append(dit.value()->getFullVariableNameWithSeparator(rSeparator));
        }
    }
    return retval;
}

QList<QString> LogDataHandler::getImportedVariablesFileNames() const
{
    return mImportedLogDataMap.keys();
}

QList<SharedLogVariableDataPtrT> LogDataHandler::getImportedVariablesForFile(const QString &rFileName)
{
    ImportedLogDataMapT::iterator fit;
    fit = mImportedLogDataMap.find(rFileName);
    if (fit != mImportedLogDataMap.end())
    {
        return fit.value().values();
    }
    else
    {
        // Return empty list if file not found
        return QList<SharedLogVariableDataPtrT>();
    }
}


QString LogDataHandler::getNewCacheName()
{
    //The first dir is the main one, any other dirs have been appended later when taking ownership of someone elses data
    return mCacheDirs.first().absoluteFilePath("cf"+QString("%1").arg(mCacheSubDirCtr++));
}

void LogDataHandler::rememberIfImported(SharedLogVariableDataPtrT pData)
{
    // Remember the imported file in the import map, so we know what generations belong to which file
    if (pData->isImported())
    {
        ImportedLogDataMapT::iterator fit; // File name iterator
        fit = mImportedLogDataMap.find(pData->getImportedFromFileName());
        if (fit != mImportedLogDataMap.end())
        {
            fit.value().insert(pData->getFullVariableName(),pData);
        }
        else
        {
            QMap<QString,SharedLogVariableDataPtrT> newFileMap;
            newFileMap.insert(pData->getFullVariableName(),pData);
            mImportedLogDataMap.insert(pData->getImportedFromFileName(),newFileMap);
        }
        // connect delete signal so we know to remove this when generation is removed
        connect(pData->getLogVariableContainer(), SIGNAL(logVariableBeingRemoved(SharedLogVariableDataPtrT)), this, SLOT(forgetImportedLogDataVariable(SharedLogVariableDataPtrT)));
    }
}

void LogDataHandler::takeOwnershipOfData(LogDataHandler *pOtherHandler, int generation)
{
    // If generation < -1 then take everything
    if (generation < -1)
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
        if (pOtherHandler->mGenerationCacheMap.contains(generation))
        {
            mGenerationCacheMap.insert(mGenerationNumber, pOtherHandler->mGenerationCacheMap.value(generation));
            pOtherHandler->mGenerationCacheMap.remove(generation);

            // Only take ownership of dir if we do not already own it
            QDir cacheDir = mGenerationCacheMap.value(mGenerationNumber)->getCacheFileInfo().absoluteDir();
            if (!mCacheDirs.contains(cacheDir))
            {
                mCacheDirs.append(cacheDir);
                //! @todo this will leave the dir in the other object aswell but I dont think it will remove the files in the dir when it dies, may need to deal with that.
            }

            tookOwnershipOfSomeData = true;
        }

//        // Take ownership of keep generation list contents
//        if (pOtherHandler->mKeepGenerationsList.contains(generation))
//        {
//            mKeepGenerationsList.append(mGenerationNumber);
//            pOtherHandler->mKeepGenerationsList.removeOne(generation);
//            tookOwnershipOfSomeData=true;
//        }

        // Take the data, and only increment this->mGeneration if data was taken
        QList< QPointer<LogVariableContainer> > otherVariables = pOtherHandler->mLogDataMap.values(); // Need to take values and itterate through, else the iterator will become invalid when removing from other
        QList< QPointer<LogVariableContainer> >::iterator odit; //odit = OtherDataIterator
        for (odit=otherVariables.begin(); odit!=otherVariables.end(); ++odit)
        {
            QString fullName = (*odit)->getFullVariableName();
            LogDataMapT::iterator tdit = mLogDataMap.find(fullName); //tdit = ThisDataIterator
            if (tdit != mLogDataMap.end())
            {
                if ((*odit)->hasDataGeneration(generation))
                {
                    tdit.value()->addDataGeneration(mGenerationNumber, (*odit)->getDataGeneration(generation));
                    (*odit)->removeDataGeneration(generation, true);
                    tookOwnershipOfSomeData=true;
                }
            }
            else
            {
                if ((*odit)->hasDataGeneration(generation))
                {
                    LogVariableContainer *pNewContainer = new LogVariableContainer(*((*odit)->getVariableCommonDescription().data()), this);
                    pNewContainer->addDataGeneration(mGenerationNumber, (*odit)->getDataGeneration(generation));
                    mLogDataMap.insert(fullName, pNewContainer);
                    (*odit)->removeDataGeneration(generation, true);
                    tookOwnershipOfSomeData=true;
                }
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
    //! @todo favorite variables, plotwindows
}

void LogDataHandler::registerAlias(const QString &rFullName, const QString &rAlias)
{
    QPointer<LogVariableContainer> pDataContainer;

    // If alias is empty then we should unregister the alias
    if (rAlias.isEmpty())
    {
        pDataContainer = mLogDataMap.value(rFullName,0);
        if (pDataContainer)
        {
            QString oldAlias = pDataContainer->getAliasName();
            if (!oldAlias.isEmpty())
            {
                pDataContainer->setAliasName("");
                mLogDataMap.remove(oldAlias);
                emit newDataAvailable();
            }
        }
    }
    else
    {
        // Check if alias already exist, and if name is different
        pDataContainer = mLogDataMap.value(rAlias,0);

        // If alias not already present then register it, (if data exist)
        if (!pDataContainer)
        {
            pDataContainer = mLogDataMap.value(rFullName,0);
            if (pDataContainer)
            {
                mLogDataMap.insert(rAlias,pDataContainer);
                pDataContainer->setAliasName(rAlias);
                emit newDataAvailable();
            }
        }
        else if (pDataContainer->getFullVariableName() != rFullName)
        {
            // Ok we should reregister this alias
            mLogDataMap.remove(rAlias);
            mLogDataMap.insert(rAlias, pDataContainer);
            pDataContainer->setAliasName(rAlias);
            emit newDataAvailable();
        }
        // Else we should do nothing as the alias is already registered for the same fullName
    }
}

void LogDataHandler::unregisterAlias(const QString &rAlias)
{
    QPointer<LogVariableContainer> pDataContainer = mLogDataMap.value(rAlias, 0);
    if (pDataContainer)
    {
        pDataContainer->setAliasName("");
        mLogDataMap.remove(rAlias);
    }
}

//! @brief This slot should be signaled when a variable that might be registered as imported is removed
void LogDataHandler::forgetImportedLogDataVariable(SharedLogVariableDataPtrT pData)
{
    // If a data ptr was found then unregister it
    if (pData)
    {
        ImportedLogDataMapT::iterator fit;
        fit = mImportedLogDataMap.find(pData->getImportedFromFileName());
        if (fit != mImportedLogDataMap.end())
        {
            // Now see if we find the correct variable
            QMap<QString, SharedLogVariableDataPtrT>::iterator vit=fit.value().find(pData->getFullVariableName());
            if (vit != fit.value().end())
            {
                // Only remove if same variable (compare pointers)
                if (vit.value().data() == pData.data())
                {
                    // Erase the variable from file map
                    fit.value().erase(vit);
                    // Now erase the file level map if it has become empty
                    if (fit.value().isEmpty())
                    {
                        mImportedLogDataMap.erase(fit);
                    }
                }
            }
        }
    }
}



SharedLogVariableDataPtrT LogDataHandler::insertVariableBasedOnDescription(VariableCommonDescription &rVarComDesc, VariableUniqueDescription *pVarUniqDesc, SharedLogVariableDataPtrT pTimeVector, QVector<double> &rDataVector)
{
    SharedLogVariableDataPtrT pNewData;
    // First check if a data variable with this name alread exist
    QString fullName = rVarComDesc.getFullName();
    LogDataMapT::iterator it = mLogDataMap.find(fullName);
    // If it exist insert into it
    if (it != mLogDataMap.end())
    {
        // Replace common data if the new variable have a lower source value
        //! @todo this is needed to replace common descriptiuon in imported data if a model variable with same name shows up. but cuirrently some data may be lost (lime special unit)
        if (rVarComDesc.mVariableSourceType < it.value()->getVariableCommonDescription()->mVariableSourceType)
        {
            it.value()->setVariableCommonDescription(rVarComDesc);
        }

        // Insert it into the generations map
        pNewData = it.value()->addDataGeneration(mGenerationNumber, pTimeVector, rDataVector);

        // Update alias if needed
        if ( rVarComDesc.mAliasName != it.value()->getAliasName() )
        {
            // Remove old mention of alias
            mLogDataMap.remove(it.value()->getAliasName());

            // Update the local alias
            it.value()->setAliasName(rVarComDesc.mAliasName);

            // Insert new alias kv pair
            mLogDataMap.insert(rVarComDesc.mAliasName, it.value());
        }
    }
    else
    {
        // Create a new toplevel map item and insert data into the generations map
        LogVariableContainer *pDataContainer = new LogVariableContainer(rVarComDesc, this);
        pNewData = pDataContainer->addDataGeneration(mGenerationNumber, pTimeVector, rDataVector);
        mLogDataMap.insert(fullName, pDataContainer);

        // Also insert alias if it exist
        if ( !rVarComDesc.mAliasName.isEmpty() )
        {
            mLogDataMap.insert(rVarComDesc.mAliasName, pDataContainer);
        }
    }

    // Set the unique override description
    if (pVarUniqDesc)
    {
        pNewData->setVariableUniqueDescription(*pVarUniqDesc);
    }

    // Remember the imported file in the import map, so we know what generations belong to which file
    rememberIfImported(pNewData);

    return pNewData;
}


SharedLogVariableDataPtrT LogDataHandler::insertTimeVariable(QVector<double> &rTimeVector, VariableUniqueDescription *pVarUniqDesc)
{
    SharedLogVariableDataPtrT pNewData;
    //! @todo this should not be done/checked here every time should have been prepered someewhere else, but no point in doing it properly now since we must rewrite logdatahandler to be global anyway
    LogVariableContainer *pTime = mLogDataMap.value(TIMEVARIABLENAME);
    if (pTime)
    {
        pNewData = pTime->addDataGeneration(mGenerationNumber, SharedLogVariableDataPtrT(), rTimeVector); //Note! Time vector itself does not have a time vector it only has a data vector
    }
    else
    {
        VariableCommonDescription varCommonDesc;
        //varDesc.mModelPath = pModelObject->getParentContainerObject()->getModelFileInfo().fileName();
        varCommonDesc.mDataName = TIMEVARIABLENAME; //!< @todo this name must be reserved
        varCommonDesc.mDataUnit = "s";
        varCommonDesc.mVariableSourceType = ModelVariableType; //! @todo maybe timetype (dont know, check with old hopsan)
        pNewData = insertVariableBasedOnDescription(varCommonDesc, pVarUniqDesc, SharedLogVariableDataPtrT(), rTimeVector);
    }

    // Set the unique description if needed
    if (pVarUniqDesc)
    {
        pNewData->setVariableUniqueDescription(*pVarUniqDesc);
    }

    // Remember the imported file in the import map, so we know what generations belong to which file
    rememberIfImported(pNewData);

    return pNewData;
}


bool LogDataHandler::hasLogVariableData(const QString &rFullName, const int generation)
{
    return (getLogVariableDataPtr(rFullName, generation) != 0);
}
