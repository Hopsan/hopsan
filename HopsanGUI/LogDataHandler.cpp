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

#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QInputDialog>
#include <QDialogButtonBox>

#include "LogDataHandler.h"

#include "PlotWindow.h"
#include "DesktopHandler.h"
#include "GUIObjects/GUIContainerObject.h"
#include "MessageHandler.h"
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
    mNumPlotCurves = 0;
    mGenerationNumber = -1;

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


void LogDataHandler::exportToPlo(const QString &rFilePath, QList<HopsanVariable> variables, int version) const
{
    if ( (version < 1) || (version > 2) )
    {
        version = gpConfig->getPLOExportVersion();
    }

    if (variables.isEmpty())
    {
        gpMessageHandler->addWarningMessage("Trying to export nothing!");
        return;
    }

    if(rFilePath.isEmpty()) return;    //Don't save anything if user presses cancel

    QFile file;
    file.setFileName(rFilePath);   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMessageHandler->addErrorMessage("Failed to open file for writing: " + rFilePath);
        return;
    }

    // Create a QTextStream object to stream the content of file
    QTextStream fileStream(&file);
    fileStream.setRealNumberNotation(QTextStream::ScientificNotation);
    fileStream.setRealNumberPrecision(6);
    QString dateTimeString = QDateTime::currentDateTime().toString();
    QFileInfo ploFileInfo(rFilePath);
    QString modelPath = mpParentContainerObject->getModelFileInfo().filePath();
    QFileInfo modelFileInfo(modelPath);

    QList<int> gens;
    QList<int> lengths;
    QList<SharedVectorVariableT> timeVectors;
    QList<SharedVectorVariableT> freqVectors;
    int minLength=INT_MAX;
    foreach(HopsanVariable var, variables)
    {
        const int g = var.mpVariable->getGeneration();
        if (gens.isEmpty() || (gens.last() != g))
        {
            gens.append(g);
        }

        const int l = var.mpVariable->getDataSize();
        if (lengths.isEmpty() || (lengths.last() != l))
        {
            lengths.append(l);
            minLength = qMin(minLength, l);
        }

        SharedVectorVariableT pToF = var.mpVariable->getSharedTimeOrFrequencyVector();
        if (pToF)
        {
            if (pToF->getDataName() == TIMEVARIABLENAME )
            {
                if (timeVectors.isEmpty() || (timeVectors.last() != pToF))
                {
                    timeVectors.append(pToF);
                }
            }
            else if (pToF->getDataName() == FREQUENCYVARIABLENAME)
            {
                if (freqVectors.isEmpty() || (freqVectors.last() != pToF))
                {
                    freqVectors.append(pToF);
                }
            }
        }
    }

    if ( (timeVectors.size() > 0) && (freqVectors.size() > 0) )
    {
        gpMessageHandler->addErrorMessage(QString("In export PLO: You are mixing time and frequency variables, this is not supported!"));
    }
    if (gens.size() > 1)
    {
        gpMessageHandler->addWarningMessage(QString("In export PLO: Data had different generations, time vector may not be correct for all exported data!"));
    }
    if (lengths.size() > 1)
    {
        gpMessageHandler->addWarningMessage(QString("In export PLO: Data had different lengths, truncating to shortest data!"));
    }

    // We insert time or frequency last when we know what generation the data had, (to avoid taking last time generation that may belong to imported data)
    if (timeVectors.size() > 0)
    {
        variables.prepend(timeVectors.first());
    }
    else if (freqVectors.size() > 0)
    {
        variables.prepend(freqVectors.first());
    }

    // Now begin to write to pro file
    int nDataRows = minLength;
    int nDataCols = variables.size();

    // Write initial Header data
    if (version == 1)
    {
        fileStream << "    'VERSION'\n";
        fileStream << "    1\n";
        fileStream << "    '"<<ploFileInfo.baseName()<<".PLO'\n";
        if ((timeVectors.size() > 0) || (freqVectors.size() > 0))
        {
            fileStream << "    " << nDataCols-1  <<"    "<< nDataRows <<"\n";
        }
        else
        {
            fileStream << "    " << nDataCols  <<"    "<< nDataRows <<"\n";
        }
        fileStream << "    '" << variables.first().mpVariable->getSmartName() << "'";
        for(int i=1; i<variables.size(); ++i)
        {
            //! @todo fix this formating so that it looks nice in plo file (need to know length of previous
            fileStream << ",    '" << variables[i].mpVariable->getSmartName()<<"'";
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
            fileStream << "    '" << variables[0].mpVariable->getSmartName() << "'";
            for(int i=1; i<variables.size(); ++i)
            {
                //! @todo fix this formating so that it looks nice in plo file (need to know length of previous
                fileStream << ",    '" << variables[i].mpVariable->getSmartName()<<"'";
            }
        }
        else
        {
            fileStream << "    'NoData'";
        }
        fileStream << "\n";
    }

    // Write plotScalings line
    for(int i=0; i<variables.size(); ++i)
    {
        const double val =  variables[i].mpVariable->getPlotScale();
        if (val < 0)
        {
            fileStream << " " << val;
        }
        else
        {
            fileStream << "  " << val;
        }
    }
    fileStream << "\n";

    QVector< double > allData;
    allData.reserve(nDataRows*nDataCols);
    for(int var=0; var<variables.size(); ++var)
    {
        allData << variables[var].mpVariable->getDataVectorCopy();
    }

    // Write data lines
    for(int row=0; row<nDataRows; ++row)
    {
        for(int col=0; col<variables.size(); ++col)
        {
            const double val = allData[col*nDataRows+row];
            if (val < 0)
            {
                fileStream << " " << val;
            }
            else
            {
                fileStream << "  " << val;
            }
        }
        fileStream << "\n";
    }

    // Write plot data ending header
    if (version==1)
    {
        fileStream << "  "+ploFileInfo.baseName()+".PLO.DAT_-1" <<"\n";
        fileStream << "  "+modelFileInfo.fileName() <<"\n";
    }

    file.close();
}

void LogDataHandler::exportToCSV(const QString &rFilePath, const QList<HopsanVariable> &rVariables) const
{
    QFile file;
    file.setFileName(rFilePath);   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMessageHandler->addErrorMessage("Failed to open file for writing: " + rFilePath);
        return;
    }

    QTextStream fileStream(&file);  //Create a QTextStream object to stream the content of file

    // Save each variable to one line each
    for (int i=0; i<rVariables.size(); ++i)
    {
        SharedVectorVariableT pVariable = rVariables[i].mpVariable;
        fileStream << pVariable->getFullVariableName() << "," << pVariable->getAliasName() << "," << pVariable->getDataUnit() << ",";
        pVariable->sendDataToStream(fileStream, ",");
        fileStream << "\n";
    }
}

void LogDataHandler::exportGenerationToCSV(const QString &rFilePath, const int gen) const
{
    QList<HopsanVariable> vars = getAllNonAliasVariablesAtGeneration(gen);
    // Now export all of them
    exportToCSV(rFilePath, vars);
}

HopsanVariable LogDataHandler::insertNewHopsanVariable(const QString &rDesiredname, VariableTypeT type, const int gen)
{
    bool ok = isNameValid(rDesiredname);
    if (!ok)
    {
        gpMessageHandler->addErrorMessage(QString("Invalid variable name: %1").arg(rDesiredname));
    }
    if( ok )
    {
        SharedVariableDescriptionT pVarDesc = SharedVariableDescriptionT(new VariableDescription());
        pVarDesc->mDataName = rDesiredname;
        pVarDesc->mVariableSourceType = ScriptVariableType;
        SharedVectorVariableT pNewData = createFreeVariable(type, pVarDesc);
        return insertVariable(pNewData, "", gen);
    }
    return HopsanVariable();
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
    progressImportBar.show();
    progressImportBar.setMinimum(0);
    progressImportBar.setMaximum(0);

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
                    gpMessageHandler->addErrorMessage(fileInfo.fileName()+" Does not seem to be a plo file, Aborting import!");
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
                if ( (ploVersion == 1) && (line.startsWith("'"TIMEVARIABLENAME) || line.startsWith("'"FREQUENCYVARIABLENAME)) )
                {
                    // We add one to include "time or frequency x column"
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
            gpMessageHandler->addErrorMessage(QString("A parse error occured while parsing the header of: ")+fileInfo.fileName()+" Aborting import!");
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
        SharedVectorVariableT pTimeVec(0);
        SharedVectorVariableT pFreqVec(0);

        if (importedPLODataVector.first().mDataName == TIMEVARIABLENAME)
        {
            pTimeVec = insertTimeVectorVariable(importedPLODataVector.first().mDataValues, fileInfo.absoluteFilePath());
        }
        else if (importedPLODataVector.first().mDataName == FREQUENCYVARIABLENAME)
        {
            pFreqVec = insertFrequencyVectorVariable(importedPLODataVector.first().mDataValues, fileInfo.absoluteFilePath());
        }

        // First decide if we should skip the first column (if time or frequency vector)
        int i=0;
        if (pTimeVec || pFreqVec)
        {
            i=1;
        }
        // Go through all imported variable columns, and create the appropriate vector variable type and insert it
        // Note! You can not mix time frequency or plain vector types in the sam plo (v1) file
        for (; i<importedPLODataVector.size(); ++i)
        {
            SharedVariableDescriptionT pVarDesc = SharedVariableDescriptionT(new VariableDescription);
            pVarDesc->mDataName = importedPLODataVector[i].mDataName;
            //! @todo what about reading the unit

            SharedVectorVariableT pNewData;
            // Insert timedomain variable
            if (pTimeVec)
            {
                pNewData = insertTimeDomainVariable(pTimeVec, importedPLODataVector[i].mDataValues, pVarDesc, fileInfo.absoluteFilePath());
            }
            // Insert frequencydomain variable
            else if (pFreqVec)
            {
                pNewData = insertFrequencyDomainVariable(pFreqVec, importedPLODataVector[i].mDataValues, pVarDesc, fileInfo.absoluteFilePath());
            }
            // Insert plain vector variables
            else
            {
                pNewData = SharedVectorVariableT(new ImportedVectorVariable(importedPLODataVector[i].mDataValues, mGenerationNumber, pVarDesc,
                                                                            fileInfo.absoluteFilePath(), getGenerationMultiCache(mGenerationNumber)));
                insertVariable(pNewData);
            }
            pNewData->setPlotScale(importedPLODataVector[i].mPlotScale);
        }

        // We do not want imported data to be removed automatically
        preventGenerationAutoRemoval(mGenerationNumber);

        emit dataAdded();
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
        gpMessageHandler->addErrorMessage(QString("Could not open file: %1").arg(importFilePath));
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
            SharedVectorVariableT pTimeVec;
            //! @todo what if multiple subsystems with different time
            int timeIdx = allNames.indexOf(TIMEVARIABLENAME);
            if (timeIdx > -1)
            {
                pTimeVec = insertTimeVectorVariable(allDatas[timeIdx], fileInfo.absoluteFilePath());
            }

            for (int i=0; i<allNames.size(); ++i)
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

            emit dataAdded();
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
        gpMessageHandler->addErrorMessage("CSV file could not be parsed.");
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
        SharedVectorVariableT pTimeVec(0);

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

        emit dataAdded();
    }

    // Limit number of plot generations if there are too many
    limitPlotGenerations();
}


//! @todo this function assumes , separated format and only values,
//! @todo what if file does not have a time vector
void LogDataHandler::importTimeVariablesFromCSVColumns(const QString csvFilePath, QVector<int> datacolumns, QStringList datanames, QVector<int> timecolumns)
{
    if (datacolumns.size() == datanames.size() && datacolumns.size() == timecolumns.size())
    {
        //! @todo in the future use a HopsanCSV parser
        QFile csvFile(csvFilePath);
        csvFile.open(QIODevice::ReadOnly | QIODevice::Text);
        if (csvFile.isOpen())
        {
            QTextStream csvStream(&csvFile);

            // Make sure we have no time duplicates
            QMap<int, QVector<double> > uniqueTimeColumns;
            for (int i=0; i<timecolumns.size(); ++i)
            {
                uniqueTimeColumns.insert(timecolumns[i], QVector<double>() );
            }
            QList<int> uniqueTimeIds = uniqueTimeColumns.keys();

            QVector< QVector<double> > dataColumns;
            dataColumns.resize(datacolumns.size());

            while (!csvStream.atEnd())
            {
                //! @todo it fould be better here to read line by line into a large array and then extract sub vectors from that throu a smart matrix object
                QStringList row_fields = csvStream.readLine().split(",");
                if (row_fields.size() > 0)
                {
                    // Insert unique time vectors
                    for (int i=0; i<uniqueTimeIds.size(); ++i)
                    {
                        const int cid = uniqueTimeIds[i];
                        if (cid < row_fields.size())
                        {
                            uniqueTimeColumns[cid].push_back(row_fields[cid].toDouble());
                        }
                    }

                    // Insert data
                    for (int i=0; i<datacolumns.size(); ++i)
                    {
                        const int cid = datacolumns[i];
                        if ( cid < row_fields.size())
                        {
                            dataColumns[i].push_back(row_fields[cid].toDouble());
                        }
                    }
                }
            }

            //! @todo check if data was found
            ++mGenerationNumber;

            QFileInfo fileInfo(csvFilePath);
            QList<SharedVectorVariableT> timePtrs;

            // Insert the unique time vectors
            if (uniqueTimeIds.size() == 1)
            {
                SharedVectorVariableT pTimeVec = insertTimeVectorVariable(uniqueTimeColumns[uniqueTimeIds.first()], fileInfo.absoluteFilePath());
                timePtrs.append(pTimeVec);
            }
            else
            {
                // If we have more then one we need to give each time vector a unique name
                for (int i=0; i<uniqueTimeIds.size(); ++i)
                {
                    SharedVariableDescriptionT pDesc = createTimeVariableDescription();
                    pDesc->mDataName = QString("Time%1").arg(uniqueTimeIds[i]);
                    SharedVectorVariableT pTimeVar = insertCustomVectorVariable(uniqueTimeColumns[uniqueTimeIds[i]], pDesc, fileInfo.absoluteFilePath());
                    timePtrs.append(pTimeVar);
                }
            }

            // Ok now we have the data lets add it as a variable
            for (int n=0; n<datanames.size(); ++n)
            {
                //! @todo what if data name already exists?
                SharedVariableDescriptionT pVarDesc = SharedVariableDescriptionT(new VariableDescription);
                pVarDesc->mDataName = datanames[n];

                // Lookup time vector to use, and insert time domain data
                int tid = timecolumns[n];
                insertTimeDomainVariable(timePtrs[tid], dataColumns[n], pVarDesc, fileInfo.absoluteFilePath());
            }

            csvFile.close();

            // We do not want imported data to be removed automatically
            preventGenerationAutoRemoval(mGenerationNumber);

            emit dataAdded();
        }
        else
        {
            gpMessageHandler->addErrorMessage("Could not open data file:  "+csvFilePath);
        }
    }
    else
    {
        gpMessageHandler->addErrorMessage("columns.size() != names.size() in:  LogDataHandler::importTimeVariablesFromCSVColumns()");
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
    SharedVectorVariableT pTimeVec;

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

                        SharedVectorVariableT pNewData = insertTimeDomainVariable(pTimeVec, dataVec, pVarDesc);

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
        emit dataAdded();
        emit dataAddedFromModel(true);
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
    QList<HopsanVariable> vars = getAllNonAliasVariablesAtGeneration(gen);
    //! @todo what if you have removed all of the original variables and only want to export the alias variables
    //! @todo error message if nothing found

    // Ok now remove time vector as it will be added again in plo export
    // This is a hack, yes I know
    for (int i=0; i<vars.size(); ++i)
    {
        if (vars[i].mpVariable->getDataName() == TIMEVARIABLENAME)
        {
            vars.removeAt(i);
        }
    }

    // Now export all of them
    exportToPlo(rFilePath, vars, version);
}


//! @brief Returns the plot data for specified variable
//! @param[in] rName The full variable name
//! @param[in] generation Generation of plot data
//! @returns A copy of the variable data vector or an empty vector if variable not found
//! @warning Do not call this function for multiports unless you know what you are doing. It will return the data from the first node only.
QVector<double> LogDataHandler::copyVariableDataVector(const QString &rName, const int generation)
{
    SharedVectorVariableT pData = getVectorVariable(rName, generation);
    if (pData)
    {
        return pData->getDataVectorCopy();
    }

    return QVector<double>();
}

SharedVectorVariableT LogDataHandler::getVectorVariable(const QString &rName, const int generation) const
{
    return getHopsanVariable(rName, generation).mpVariable;
}

HopsanVariable LogDataHandler::getHopsanVariable(const QString &rName, const int generation) const
{
    // Find the data variable
    LogDataMapT::const_iterator dit = mLogDataMap.find(rName);
    if (dit != mLogDataMap.end())
    {
        return HopsanVariable(dit.value(), dit.value()->getDataGeneration(generation));
    }
    return HopsanVariable();
}

//! @brief Returns multiple logdatavariables based on regular expression search. Exluding temp variables but including aliases
//! @param [in] rNameExp The regular expression for the names to match
//! @param [in] generation The desired generation of the variable
QVector<SharedVectorVariableT> LogDataHandler::getMatchingVariablesAtGeneration(const QRegExp &rNameExp, const int generation) const
{
    QVector<SharedVectorVariableT> results;
    LogDataMapT::const_iterator it;
    for (it = mLogDataMap.begin(); it != mLogDataMap.end(); it++)
    {
        // Compare name with regexp
        if ( rNameExp.exactMatch(it.key()) )
        {
            SharedVectorVariableT pData = it.value()->getDataGeneration(generation);
            if (pData)
            {
                results.append(pData);
            }
        }
    }
    return results;
}

const QList<SharedVectorVariableContainerT> LogDataHandler::getAllVariableContainers() const
{
    return mLogDataMap.values();
}

const QList<SharedVectorVariableContainerT> LogDataHandler::getVariableContainersMatching(const QRegExp &rNameExp) const
{
    QList< SharedVectorVariableContainerT > results;
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

const QList<SharedVectorVariableContainerT> LogDataHandler::getVariableContainersMatching(const QRegExp &rNameExp, const int generation) const
{
    QList< SharedVectorVariableContainerT > results;
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

SharedVectorVariableContainerT LogDataHandler::getVariableContainer(const QString &rVarName) const
{
    return mLogDataMap.value(rVarName,SharedVectorVariableContainerT());
}


//! @brief Returns the time vector for specified generation
//! @param[in] generation Generation
const SharedVectorVariableT LogDataHandler::getTimeVectorVariable(int generation) const
{
    SharedVectorVariableContainerT pCont = mLogDataMap.value(TIMEVARIABLENAME);
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
        return SharedVectorVariableT();
    }
}

QVector<double> LogDataHandler::copyTimeVector(const int generation) const
{
    SharedVectorVariableT pTime = getTimeVectorVariable(generation);
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
    SharedVectorVariableContainerT pAliasContainer = getVariableContainer(rAlias);
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
        QList< SharedVectorVariableContainerT > vars = mLogDataMap.values();
        QList< SharedVectorVariableContainerT >::iterator it;
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
        dit.value()->allowGenerationAutoRemoval(gen, false);
    }
}

void LogDataHandler::allowGenerationAutoRemoval(const int gen)
{
    // Tag the generations in each log data variable container
    LogDataMapT::iterator dit = mLogDataMap.begin();
    for ( ; dit!=mLogDataMap.end(); ++dit)
    {
        dit.value()->allowGenerationAutoRemoval(gen,true);
    }
}

//! @brief Removes a generation
void LogDataHandler::removeGeneration(const int gen, const bool force)
{
    bool didRemoveSomething=false;
    // Note! Here we must iterate through a copy of the values from the map
    // if we use an iterator in the map we will crash if the removed generation is the final one
    // Then the data variable itself will also be removed and the iterator will become invalid
    QList< SharedVectorVariableContainerT > vars = mLogDataMap.values();
    QList< SharedVectorVariableContainerT >::iterator it;
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
    ++mNumPlotCurves;
}


//! @brief Decrements counter for number of open plot curves (in plot windows)
//! @note Used to decide if warning message shall be shown when closing model
//! @see PlotData::incrementOpenPlotCurves()
//! @see PlotData::hasOpenPlotCurves()
void LogDataHandler::decrementOpenPlotCurves()
{
    --mNumPlotCurves;
}


SharedVectorVariableT LogDataHandler::addVariableWithScalar(const SharedVectorVariableT a, const double x)
{
    SharedVectorVariableT pTempVar = createOrphanVariable(a->getSmartName()+"+"+QString::number(x), a->getVariableType());
    pTempVar->assignFrom(a);
    pTempVar->addToData(x);
    return pTempVar;
}


SharedVectorVariableT LogDataHandler::subVariableWithScalar(const SharedVectorVariableT a, const double x)
{
    SharedVectorVariableT pTempVar = createOrphanVariable(a->getSmartName()+"-"+QString::number(x), a->getVariableType());
    pTempVar->assignFrom(a);
    pTempVar->subFromData(x);
    return pTempVar;
}


SharedVectorVariableT LogDataHandler::mulVariableWithScalar(const SharedVectorVariableT a, const double x)
{
    SharedVectorVariableT pTempVar = createOrphanVariable(a->getSmartName()+"*"+QString::number(x), a->getVariableType());
    pTempVar->assignFrom(a);
    pTempVar->multData(x);
    return pTempVar;
}


SharedVectorVariableT LogDataHandler::divVariableWithScalar(const SharedVectorVariableT a, const double x)
{
    SharedVectorVariableT pTempVar = createOrphanVariable(a->getSmartName()+"/"+QString::number(x), a->getVariableType());
    pTempVar->assignFrom(a);
    pTempVar->divData(x);
    return pTempVar;
}


SharedVectorVariableT LogDataHandler::addVariables(const SharedVectorVariableT a, const SharedVectorVariableT b)
{
    SharedVectorVariableT pTempVar = createOrphanVariable(a->getSmartName()+"+"+b->getSmartName(), a->getVariableType());
    pTempVar->assignFrom(a);
    pTempVar->addToData(b);
    return pTempVar;
}


SharedVectorVariableT LogDataHandler::diffVariables(const SharedVectorVariableT a, const SharedVectorVariableT b)
{
    SharedVectorVariableT pTempVar = createOrphanVariable(a->getSmartName()+"_Diff", a->getVariableType());
    pTempVar->assignFrom(a);
    pTempVar->diffBy(b);
    return pTempVar;
}


SharedVectorVariableT LogDataHandler::integrateVariables(const SharedVectorVariableT a, const SharedVectorVariableT b)
{
    SharedVectorVariableT pTempVar = createOrphanVariable(a->getSmartName()+"_Int", a->getVariableType());
    pTempVar->assignFrom(a);
    pTempVar->integrateBy(b);
    return pTempVar;
}


SharedVectorVariableT LogDataHandler::lowPassFilterVariable(const SharedVectorVariableT a, const SharedVectorVariableT b, const double freq)
{
    SharedVectorVariableT pTempVar = createOrphanVariable(a->getSmartName()+"_Lp1", a->getVariableType());
    pTempVar->assignFrom(a);
    pTempVar->lowPassFilter(b, freq);
    return pTempVar;
}


bool LogDataHandler::deleteVariable(const QString &rVarName)
{
    // Find the container with given name, but only try to remove it if it is not already being removed
    LogDataMapT::iterator it = mLogDataMap.find(rVarName);
    if( (it != mLogDataMap.end()) && !mCurrentlyDeletingContainers.contains(it.value()) )
    {
        // Remember that this one is being deleted to avoid deleting the iterator agin if any subfunction results in a call back to this one
        mCurrentlyDeletingContainers.push_back(it.value());

        // Handle alias removal
        if (it.value()->isStoringAlias())
        {
            unregisterAlias(rVarName);
        }

        // Explicitly remove all generations, if you trigger a delete then you expect the data to be removed (otherwise it would remin untill all shared pointers dies)
        // Note! Anyone using a data vector shared pointer will still hang on to that as long as they wish
        it.value()->removeAllGenerations();

        // Remove data ptr from map, the actual container will be deleted automatically when is is no longer being used by anyone
        mLogDataMap.erase(it);
        mCurrentlyDeletingContainers.pop_back();

        emit dataRemoved();
        return true;
    }
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


SharedVectorVariableT LogDataHandler::elementWiseGT(SharedVectorVariableT pData, const double thresh)
{
    if (pData)
    {
        SharedVectorVariableT pTempVar = createOrphanVariable(pData->getSmartName()+"_gt", pData->getVariableType());
        QVector<double> res;
        pData->elementWiseGt(res,thresh);
        pTempVar->assignFrom(res);
        return pTempVar;
    }
    return SharedVectorVariableT();
}

SharedVectorVariableT LogDataHandler::elementWiseLT(SharedVectorVariableT pData, const double thresh)
{
    if (pData)
    {
        SharedVectorVariableT pTempVar = createOrphanVariable(pData->getSmartName()+"_lt", pData->getVariableType());
        QVector<double> res;
        pData->elementWiseLt(res,thresh);
        pTempVar->assignFrom(res);
        return pTempVar;
    }
    return SharedVectorVariableT();
}

//QString LogDataHandler::saveVariable(const QString &currName, const QString &newName)
//{
//    SharedVectorVariableT pCurrData = getVectorVariable(currName, -1);
//    SharedVectorVariableT pNewData = getVectorVariable(newName, -1);
//    // If curr data exist and new data does not exist
//    if( (pNewData == 0) && (pCurrData != 0) )
//    {
//        SharedVectorVariableT pNewData = defineNewVariable(newName);
//        if (pNewData)
//        {
//            pNewData->assignFrom(pCurrData);
//            return pNewData->getFullVariableName();
//        }
//        gpMessageHandler->addErrorMessage("Could not create variable: " + newName);
//        return QString();
//    }
//    gpMessageHandler->addErrorMessage("Variable: " + currName + " does not exist, or Variable: " + newName + " already exist");
//    return QString();
//}

SharedVectorVariableT LogDataHandler::subVariables(const SharedVectorVariableT a, const SharedVectorVariableT b)
{
    SharedVectorVariableT pTempVar = createOrphanVariable(a->getSmartName()+"-"+b->getSmartName(), a->getVariableType());
    pTempVar->assignFrom(a);
    pTempVar->subFromData(b);
    return pTempVar;
}

SharedVectorVariableT LogDataHandler::multVariables(const SharedVectorVariableT a, const SharedVectorVariableT b)
{
    SharedVectorVariableT pTempVar = createOrphanVariable(a->getSmartName()+"*"+b->getSmartName(), a->getVariableType());
    pTempVar->assignFrom(a);
    pTempVar->multData(b);
    return pTempVar;
}

SharedVectorVariableT LogDataHandler::divVariables(const SharedVectorVariableT a, const SharedVectorVariableT b)
{
    SharedVectorVariableT pTempVar = createOrphanVariable(a->getSmartName()+"/"+b->getSmartName(), a->getVariableType());
    pTempVar->assignFrom(a);
    pTempVar->divData(b);
    return pTempVar;
}

double LogDataHandler::pokeVariable(SharedVectorVariableT a, const int index, const double value)
{
    QString err;
    double r = a->pokeData(index,value,err);
    if (!err.isEmpty())
    {
        gpMessageHandler->addErrorMessage(err);
    }
    return r;
}


double LogDataHandler::peekVariable(SharedVectorVariableT a, const int index)
{
    QString err;
    double r = a->peekData(index, err);
    if (!err.isEmpty())
    {
        gpMessageHandler->addErrorMessage(err);
    }
    return r;
}


//! @brief Creates an orphan temp variable that will be deleted when its shared pointer reference counter reaches zero (when no one is using it)
//! @todo this function should not be inside logdatahdnler, it should be free spo that you do not need to use a log datahnadler to create a free variable
SharedVectorVariableT LogDataHandler::createOrphanVariable(const QString &rName, VariableTypeT type)
{
    SharedVariableDescriptionT varDesc = SharedVariableDescriptionT(new VariableDescription());
    varDesc->mDataName = rName;
    varDesc->mVariableSourceType = ScriptVariableType;
    SharedVectorVariableT pNewData = createFreeVariable(type,varDesc);
    pNewData->mGeneration = mGenerationNumber;
    return pNewData;
}


SharedVectorVariableT LogDataHandler::defineNewVectorVariable(const QString &rDesiredname, VariableTypeT type)
{
    bool ok = isNameValid(rDesiredname);
    if (!ok)
    {
        gpMessageHandler->addErrorMessage(QString("Invalid variable name: %1").arg(rDesiredname));
    }
    if( ok && (mLogDataMap.find(rDesiredname) == mLogDataMap.end()) )
    {
        SharedVariableDescriptionT pVarDesc = SharedVariableDescriptionT(new VariableDescription());
        pVarDesc->mDataName = rDesiredname;
        pVarDesc->mVariableSourceType = ScriptVariableType;
        SharedVectorVariableT pNewData = createFreeVariable(type, pVarDesc);
        insertVariable(pNewData);
        return pNewData;
    }
    return SharedVectorVariableT();
}


//! @brief Tells whether or not the model has open plot curves
//! @note Used to decide if warning message shall be shown when closing model
//! @see PlotData::incrementOpenPlotCurves()
//! @see PlotData::decrementOpenPlotCurves()
bool LogDataHandler::hasOpenPlotCurves()
{
    return (mNumPlotCurves > 0);
}

void LogDataHandler::closePlotsWithCurvesBasedOnOwnedData()
{
    emit closePlotsWithOwnedData();
}


PlotWindow *LogDataHandler::plotVariable(const QString plotName, const QString fullVarName, const int gen, const int axis, QColor color)
{
    HopsanVariable data = getHopsanVariable(fullVarName, gen);
    if(data)
    {
        return gpPlotHandler->plotDataToWindow(plotName, data, axis, color);
    }
    return 0;
}

PlotWindow *LogDataHandler::plotVariable(const QString plotName, const QString &rFullNameX, const QString &rFullNameY, const int gen, const int axis, QColor color)
{
    HopsanVariable xdata = getHopsanVariable(rFullNameX, gen);
    HopsanVariable ydata = getHopsanVariable(rFullNameY, gen);
    if (xdata && ydata)
    {
        return gpPlotHandler->plotDataToWindow(plotName, xdata, ydata, axis, color);
    }
    return 0;
}

PlotWindow *LogDataHandler::plotVariable(PlotWindow *pPlotWindow, const QString fullVarName, const int gen, const int axis, QColor color)
{
    HopsanVariable data = getHopsanVariable(fullVarName, gen);
    if(data)
    {
        return gpPlotHandler->plotDataToWindow(pPlotWindow, data, axis, color);
    }
    return 0;
}

//! @brief Get a list of all available variables at their respective higest (newest) generation. Aliases are excluded.
QList<HopsanVariable> LogDataHandler::getAllNonAliasVariablesAtRespectiveNewestGeneration()
{
    return getAllNonAliasVariablesAtGeneration(-1);
}

//! @brief Get a list of all available variables at a specific generation, variables that does not have this generation will not be included.  Aliases are excluded.
QList<HopsanVariable> LogDataHandler::getAllNonAliasVariablesAtGeneration(const int generation) const
{
    QList<HopsanVariable> dataList;
    dataList.reserve(mLogDataMap.size()); //(may not use all reserved space but that does not matter)

    // First go through all data variable
    LogDataMapT::const_iterator dit = mLogDataMap.begin();
    for ( ; dit!=mLogDataMap.end(); ++dit)
    {
        // Now try to find given generation
        SharedVectorVariableT pData = dit.value()->getNonAliasDataGeneration(generation);
        if (pData)
        {
            dataList.push_back(pData);
        }
    }
    return dataList;
}

QList<HopsanVariable> LogDataHandler::getAllVariablesAtRespectiveNewestGeneration()
{
    return getAllVariablesAtGeneration(-1);
}

QList<HopsanVariable> LogDataHandler::getAllVariablesAtGeneration(const int gen)
{
    QList<HopsanVariable> dataList;
    dataList.reserve(mLogDataMap.size());

    // First go through all data variable
    LogDataMapT::iterator dit = mLogDataMap.begin();
    for ( ; dit!=mLogDataMap.end(); ++dit)
    {
        SharedVectorVariableT pData = dit.value()->getDataGeneration(gen);
        if (pData)
        {
            dataList.append(HopsanVariable(dit.value(), pData));
        }
    }
    return dataList;
}

void LogDataHandler::getVariableNamesWithHighestGeneration(QStringList &rNames, QList<int> &rGens) const
{
    rNames.clear(); rGens.clear();
    rNames.reserve(mLogDataMap.size());
    rGens.reserve(mLogDataMap.size());

    LogDataMapT::const_iterator dit = mLogDataMap.begin();
    for ( ; dit!=mLogDataMap.end(); ++dit)
    {
        SharedVectorVariableT pData = dit.value()->getDataGeneration(-1);
        if(pData)
        {
            rNames.append(dit.key());
            rGens.append(pData->getGeneration());
        }
    }
}

int LogDataHandler::getCurrentGeneration() const
{
    return mGenerationNumber;
}


QStringList LogDataHandler::getVariableFullNames(const QString &rSeparator, const int generation) const
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

QList<HopsanVariable> LogDataHandler::getImportedVariablesForFile(const QString &rFileName)
{
    ImportedLogDataMapT::iterator fit;
    fit = mImportedLogDataMap.find(rFileName);
    if (fit != mImportedLogDataMap.end())
    {
        QList<HopsanVariable> results;
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
        return QList<HopsanVariable>();
    }
}

QList<int> LogDataHandler::getImportFileGenerations(const QString &rFilePath) const
{
    QList<int> results;
    ImportedLogDataMapT::const_iterator fit = mImportedLogDataMap.find(rFilePath);
    if (fit != mImportedLogDataMap.end())
    {
        // Add generation for each variable
        QMultiMap<QString, HopsanVariable>::const_iterator vit;
        for (vit=fit.value().begin(); vit!=fit.value().end(); ++vit)
        {
            const int g = vit.value().mpVariable->getGeneration();
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

void LogDataHandler::rememberIfImported(HopsanVariable data)
{
    // Remember the imported file in the import map, so we know what generations belong to which file
    if (data.mpVariable->isImported())
    {
        ImportedLogDataMapT::iterator fit; // File name iterator
        fit = mImportedLogDataMap.find(data.mpVariable->getImportedFileName());
        if (fit != mImportedLogDataMap.end())
        {
            fit.value().insertMulti(data.mpVariable->getFullVariableName(),data);
        }
        else
        {
            QMultiMap<QString,HopsanVariable> newFileMap;
            newFileMap.insert(data.mpVariable->getFullVariableName(),data);
            mImportedLogDataMap.insert(data.mpVariable->getImportedFileName(),newFileMap);
        }
        // connect delete signal so we know to remove this when generation is removed
        if (data.mpContainer)
        {
            connect(data.mpContainer.data(), SIGNAL(importedVariableBeingRemoved(SharedVectorVariableT)), this, SLOT(forgetImportedVariable(SharedVectorVariableT)));
        }

        // Make data description source know its imported
        data.mpVariable->mpVariableDescription->mVariableSourceType = ImportedVariableType;
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
            }

        }

        if (!tookOwnershipOfSomeData)
        {
            // Revert generation if no data was taken
            --mGenerationNumber;
        }
    }
    //! @todo favorite variables, plotwindows, imported data
}

void LogDataHandler::registerAlias(const QString &rFullName, const QString &rAlias)
{
    SharedVectorVariableContainerT pFullContainer = getVariableContainer(rFullName);
    if (pFullContainer)
    {
        // If alias is empty then we should unregister the alias
        if (rAlias.isEmpty())
        {
            const QList<int> fullGens = pFullContainer->getGenerations();
            SharedVectorVariableContainerT pAliasContainer;
            QString currentAliasName;
            bool didChangeAlias=false;
            for(int i=0; i<fullGens.size(); ++i)
            {
                SharedVectorVariableT pFullData = pFullContainer->getDataGeneration(fullGens[i]);
                if (pFullData->hasAliasName())
                {
                    // Prevent alias name lookup every time if alias name has not changed
                    if (currentAliasName != pFullData->getAliasName())
                    {
                        currentAliasName = pFullData->getAliasName();
                        pAliasContainer = getVariableContainer(currentAliasName);
                    }

                    // Remove the alias, here we assume that the alias name at this generation is actually an alias for the fullName (the registration should have taken care of that)
                    if (pAliasContainer)
                    {
                        //! @todo will this not trigger additional remove code for the actual variable also
                        pAliasContainer->removeDataGeneration(fullGens[i]);
                    }
                    pFullData->mpVariableDescription->mAliasName = rAlias;
                    didChangeAlias = true;
                }
            }
            if (didChangeAlias)
            {
                emit aliasChanged();
            }
        }
        else
        {
            QList<int> fullGens = pFullContainer->getGenerations();
            SharedVectorVariableContainerT pAliasContainer = getVariableContainer(rAlias);             // Check if alias already exist
            for(int i=fullGens.size()-1; i>=0; --i)
            {
                SharedVectorVariableT pAliasData;
                if (pAliasContainer)
                {
                    pAliasData = pAliasContainer->getDataGeneration(fullGens[i]);
                }
                // Check so that there will be no colision when merging alias
                if ( pAliasData && (pAliasData->getFullVariableName() != rFullName) )
                {
                    // Abort if we reach a generation that already exist and does not match
                    //! @todo we could keep going and try every gen (but then we do not want to duplicate this warning message)
                    gpMessageHandler->addWarningMessage("Alias collision when trying to merge alias, Aborting alias merge!");
                    break;
                }
                else
                {
                    // If we get here then we can merge (any previous data with alias name will be overwritten if fullname was same)
                    SharedVectorVariableT pData = pFullContainer->getDataGeneration(fullGens[i]);
                    // First unregister the old alias
                    if (pData->hasAliasName())
                    {
                        unregisterAlias(pData->getAliasName());
                    }
                    pData->mpVariableDescription->mAliasName = rAlias;
                    insertVariable(pData, rAlias, fullGens[i]);
                }
            }
            emit aliasChanged();
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
void LogDataHandler::forgetImportedVariable(SharedVectorVariableT pData)
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

SharedVectorVariableT LogDataHandler::insertCustomVectorVariable(const QVector<double> &rVector, SharedVariableDescriptionT pVarDesc)
{
    SharedVectorVariableT pVec = SharedVectorVariableT(new VectorVariable(rVector, mGenerationNumber, pVarDesc,
                                                                          getGenerationMultiCache(mGenerationNumber)));
    insertVariable(pVec);
    return pVec;
}

SharedVectorVariableT LogDataHandler::insertCustomVectorVariable(const QVector<double> &rVector, SharedVariableDescriptionT pVarDesc, const QString &rImportFileName)
{
    SharedVectorVariableT pVec = SharedVectorVariableT(new ImportedVectorVariable(rVector, mGenerationNumber, pVarDesc,
                                                                                  rImportFileName, getGenerationMultiCache(mGenerationNumber)));
    insertVariable(pVec);
    return pVec;
}

SharedVectorVariableT LogDataHandler::insertTimeVectorVariable(const QVector<double> &rTimeVector)
{
    return insertCustomVectorVariable(rTimeVector, createTimeVariableDescription());
//    SharedVectorVariableT pTimeVec = SharedVectorVariableT(new VectorVariable(rTimeVector, mGenerationNumber, createTimeVariableDescription(),
//                                                                        getGenerationMultiCache(mGenerationNumber)));
//    insertVariable(pTimeVec);
//    return pTimeVec;
}

SharedVectorVariableT LogDataHandler::insertTimeVectorVariable(const QVector<double> &rTimeVector, const QString &rImportFileName)
{
//    SharedVectorVariableT pTimeVec = SharedVectorVariableT(new ImportedVectorVariable(rTimeVector, mGenerationNumber, createTimeVariableDescription(),
//                                                                                rImportFileName, getGenerationMultiCache(mGenerationNumber)));
//    insertVariable(pTimeVec);
//    return pTimeVec;
    return insertCustomVectorVariable(rTimeVector, createTimeVariableDescription(), rImportFileName);
}

SharedVectorVariableT LogDataHandler::insertFrequencyVectorVariable(const QVector<double> &rFrequencyVector)
{
    return insertCustomVectorVariable(rFrequencyVector, createFrequencyVariableDescription());
//    SharedVectorVariableT pFreqVec = SharedVectorVariableT(new VectorVariable(rFrequencyVector, mGenerationNumber, createFrequencyVariableDescription(),
//                                                                              getGenerationMultiCache(mGenerationNumber)));
//    insertVariable(pFreqVec);
//    return pFreqVec;
}

SharedVectorVariableT LogDataHandler::insertFrequencyVectorVariable(const QVector<double> &rFrequencyVector, const QString &rImportFileName)
{
    return insertCustomVectorVariable(rFrequencyVector, createFrequencyVariableDescription(), rImportFileName);
//    SharedVectorVariableT pFreqVec = SharedVectorVariableT(new ImportedVectorVariable(rFrequencyVector, mGenerationNumber, createFrequencyVariableDescription(),
//                                                                                      rImportFileName, getGenerationMultiCache(mGenerationNumber)));
//    insertVariable(pFreqVec);
//    return pFreqVec;
}

SharedVectorVariableT LogDataHandler::insertTimeDomainVariable(SharedVectorVariableT pTimeVector, const QVector<double> &rDataVector, SharedVariableDescriptionT pVarDesc)
{
    SharedVectorVariableT pNewData = SharedVectorVariableT(new TimeDomainVariable(pTimeVector, rDataVector, mGenerationNumber, pVarDesc,
                                                                            getGenerationMultiCache(mGenerationNumber)));
    insertVariable(pNewData);
    return pNewData;
}

SharedVectorVariableT LogDataHandler::insertTimeDomainVariable(SharedVectorVariableT pTimeVector, const QVector<double> &rDataVector, SharedVariableDescriptionT pVarDesc, const QString &rImportFileName)
{
    SharedVectorVariableT pNewData = SharedVectorVariableT(new ImportedTimeDomainVariable(pTimeVector, rDataVector, mGenerationNumber, pVarDesc,
                                                                                    rImportFileName, getGenerationMultiCache(mGenerationNumber)));
    insertVariable(pNewData);
    return pNewData;
}

SharedVectorVariableT LogDataHandler::insertFrequencyDomainVariable(SharedVectorVariableT pFrequencyVector, const QVector<double> &rDataVector, SharedVariableDescriptionT pVarDesc)
{
    SharedVectorVariableT pNewData = SharedVectorVariableT(new FrequencyDomainVariable(pFrequencyVector, rDataVector, mGenerationNumber, pVarDesc,
                                                                                       getGenerationMultiCache(mGenerationNumber)));
    insertVariable(pNewData);
    return pNewData;
}

SharedVectorVariableT LogDataHandler::insertFrequencyDomainVariable(SharedVectorVariableT pFrequencyVector, const QVector<double> &rDataVector, SharedVariableDescriptionT pVarDesc, const QString &rImportFileName)
{
    SharedVectorVariableT pNewData = SharedVectorVariableT(new ImportedFrequencyDomainVariable(pFrequencyVector, rDataVector, mGenerationNumber, pVarDesc,
                                                                                               rImportFileName, getGenerationMultiCache(mGenerationNumber)));
    insertVariable(pNewData);
    return pNewData;
}

//! @brief Inserts a variable into the map creating a container if needed
//! @param[in] pVariable The variable to insert
//! @param[in] keyName An alternative keyName to use (used recursivly to set an alias, do not abuse this argument)
//! @param[in] gen An alternative generation number (-1 = current))
//! @returns A HopsanVariable object representing the inserted variable
HopsanVariable LogDataHandler::insertVariable(SharedVectorVariableT pVariable, QString keyName, int gen)
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

    SharedVectorVariableContainerT pDataContainer;
    // First check if a data variable with this name alread exist, if it exist then insert into it
    LogDataMapT::iterator it = mLogDataMap.find(keyName);
    if (it != mLogDataMap.end())
    {
        // Insert it into the generations map
        pDataContainer = it.value();
        pDataContainer->addDataGeneration(gen, pVariable);
    }
    else
    {
        // Create a new toplevel map item and insert data into the generations map
        pDataContainer = SharedVectorVariableContainerT(new VectorVariableContainer(keyName, this));
        pDataContainer->addDataGeneration(gen, pVariable);
        mLogDataMap.insert(keyName, pDataContainer);
    }

    // Also insert alias if it exist, but only if it is different from keyName (else we will have an endless loop in here)
    if ( pVariable->hasAliasName() && (pVariable->getAliasName() != keyName) )
    {
        insertVariable(pVariable, pVariable->getAliasName(), gen);
    }

    // Remember imported files in the import map, so that we know what generations belong to which file
    rememberIfImported(HopsanVariable(pDataContainer,pVariable));

    // Make the variable remember that this logdatahandler is its parent (creator)
    pVariable->mpParentLogDataHandler = this;

    return HopsanVariable(pDataContainer, pVariable);
}

bool LogDataHandler::hasVariable(const QString &rFullName, const int generation)
{
    return (getVectorVariable(rFullName, generation) != 0);
}
