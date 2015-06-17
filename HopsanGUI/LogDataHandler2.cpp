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
//! @file   LogDataHandler2.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2012-12-18
//!
//! @brief Contains the LogData classes
//!
//$Id$

#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QDialogButtonBox>
#include <QPushButton>

#include "LogDataHandler2.h"
#include "LogDataGeneration.h"


#include "PlotWindow.h"
#include "DesktopHandler.h"
#include "GUIObjects/GUIContainerObject.h"
#include "GUIObjects/GUISystem.h"
#include "MessageHandler.h"
#include "Widgets/ModelWidget.h"
#include "common.h"
#include "global.h"
#include "version_gui.h"
#include "Configuration.h"
#include "GUIPort.h"
#include "Utilities/GUIUtilities.h"

#include "PlotWindow.h"
#include "PlotHandler.h"

#include "ComponentUtilities/CSVParser.h"
#include "HopsanTypes.h"

//UnitScale figureOutCutomUnitScale2(QString quantity, double scale)
//{
//    QList<UnitScale> uss;
//    gpConfig->getUnitScales(quantity,uss);
//    Q_FOREACH(const UnitScale &us, uss)
//    {
//        double s = us.toDouble();
//        if (fuzzyEqual(us.toDouble(), scale, 0.1*qMin(s,scale)))
//        {
//            return us;
//        }
//    }
//    return UnitScale();
//}


//! @brief Constructor for plot data object
//! @param pParent Pointer to parent container object
LogDataHandler2::LogDataHandler2(ModelWidget *pParentModel) : QObject(pParentModel)
{
    mpParentModel = 0;
    setParentModel(pParentModel);
    mNumPlotCurves = 0;
    mCurrentGenerationNumber = -1;

    // Create the temporary directory that will contain cache data
    int ctr=0;
    QDir tmp;
    do
    {
        tmp = QDir(gpDesktopHandler->getLogDataPath() + QString("handler%1").arg(ctr));
        ++ctr;
    }while(tmp.exists());
    tmp.mkpath(tmp.absolutePath());
    mCacheDirs.append(tmp);
    mCacheSubDirCtr = 0;
}

LogDataHandler2::~LogDataHandler2()
{
    // Clear all data
    clear();
}

void LogDataHandler2::setParentModel(ModelWidget *pParentModel)
{
    if (mpParentModel)
    {
        disconnect(0, 0, this, SLOT(registerAlias(QString,QString)));
    }
    mpParentModel = pParentModel;
    connect(mpParentModel, SIGNAL(aliasChanged(QString,QString)), this, SLOT(registerAlias(QString,QString)));
}

ModelWidget *LogDataHandler2::getParentModel()
{
    return mpParentModel;
}


void LogDataHandler2::exportToPlo(const QString &rFilePath, QList<SharedVectorVariableT> variables, int version) const
{
    if ( (version < 1) || (version > 2) )
    {
        version = gpConfig->getIntegerSetting(CFG_PLOEXPORTVERSION);
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
    QString modelPath = mpParentModel->getTopLevelSystemContainer()->getModelFileInfo().filePath(); //!< @todo ModelPath info should be in model maybe (but what about external systems in that case) /Peter
    QFileInfo modelFileInfo(modelPath);

    QList<int> gens;
    QList<int> lengths;
    QList<SharedVectorVariableT> timeVectors;
    QList<SharedVectorVariableT> freqVectors;
    int minLength=INT_MAX;
    foreach(SharedVectorVariableT var, variables)
    {
        const int g = var->getGeneration();
        if (gens.isEmpty() || (gens.last() != g))
        {
            gens.append(g);
        }

        const int l = var->getDataSize();
        if (lengths.isEmpty() || (lengths.last() != l))
        {
            lengths.append(l);
            minLength = qMin(minLength, l);
        }

        SharedVectorVariableT pToF = var->getSharedTimeOrFrequencyVector();
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
        fileStream << "    '" << variables.first()->getSmartName() << "'";
        for(int i=1; i<variables.size(); ++i)
        {
            //! @todo fix this formating so that it looks nice in plo file (need to know length of previous
            fileStream << ",    '" << variables[i]->getSmartName()<<"'";
        }
        fileStream << "\n";
    }
    else if (version == 2)
    {
        fileStream << "    'VERSION'\n";
        fileStream << "    2\n";
        fileStream << "    '"<<ploFileInfo.baseName()<<".PLO' " << "'" << modelFileInfo.fileName() << "' '" << dateTimeString << "' 'HopsanGUI " << QString(HOPSANGUIVERSION) << "'\n";
        fileStream << "    " << nDataCols  <<"    "<< nDataRows <<"\n";
        if (nDataCols > 0)
        {
            fileStream << "    '" << variables[0]->getSmartName() << "'";
            for(int i=1; i<variables.size(); ++i)
            {
                //! @todo fix this formating so that it looks nice in plo file (need to know length of previous
                fileStream << ",    '" << variables[i]->getSmartName()<<"'";
            }
        }
        else
        {
            fileStream << "    'NoData'";
        }
        fileStream << "\n";
    }

    // Write plotScalings line
    // Note! Plotscale removed in 0.7 for backward compatibility write ones
    for(int i=0; i<variables.size(); ++i)
    {
        fileStream << "  " << 1.0;
    }
    fileStream << "\n";

    QVector< double > allData;
    allData.reserve(nDataRows*nDataCols);
    for(int var=0; var<variables.size(); ++var)
    {
        allData << variables[var]->getDataVectorCopy();
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

void LogDataHandler2::exportToCSV(const QString &rFilePath, const QList<SharedVectorVariableT> &rVariables) const
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
        SharedVectorVariableT pVariable = rVariables[i];
        fileStream << pVariable->getFullVariableName() << "," << pVariable->getAliasName() << "," << pVariable->getDataUnit() << ",";
        pVariable->sendDataToStream(fileStream, ",");
        fileStream << "\n";
    }
}

void LogDataHandler2::exportGenerationToCSV(const QString &rFilePath, const int gen) const
{
    QList<SharedVectorVariableT> vars = getAllNonAliasVariablesAtGeneration(gen);
    // Now export all of them
    exportToCSV(rFilePath, vars);
}

SharedVectorVariableT LogDataHandler2::insertNewVectorVariable(const QString &rDesiredname, VariableTypeT type, const int gen)
{
    //! @todo should prevent negative gen maybe
    bool ok = isNameValid(rDesiredname);
    if (!ok)
    {
        gpMessageHandler->addErrorMessage(QString("Invalid variable name: %1").arg(rDesiredname));
    }
    if( ok )
    {
        SharedVectorVariableT pNewData = createOrphanVariable(rDesiredname, type);
        return insertVariable(pNewData, "", gen);
    }
    return SharedVectorVariableT();
}

SharedVectorVariableT LogDataHandler2::insertNewVectorVariable(SharedVectorVariableT pVariable, const int gen)
{
    return insertVariable(pVariable, "", gen);
}

class PLOImportData
{
public:
      QString mDataName;
      QString mPlotScale;
      double startvalue;
      QVector<double> mDataValues;
};

void LogDataHandler2::importFromPlo(QString importFilePath)
{
    if(importFilePath.isEmpty())
    {

        importFilePath = QFileDialog::getOpenFileName(0,tr("Choose Hopsan .plo File"),
                                                       gpConfig->getStringSetting(CFG_PLOTDATADIR),
                                                       tr("Hopsan File (*.plo)"));
    }
    if(importFilePath.isEmpty())
    {
        return;
    }

    QFile file(importFilePath);
    QFileInfo fileInfo(file);
    gpConfig->setStringSetting(CFG_PLOTDATADIR, fileInfo.absolutePath());

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
                if ( (ploVersion == 1) && (line.startsWith("'" TIMEVARIABLENAME) || line.startsWith("'" FREQUENCYVARIABLENAME)) )
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
                // Read plot scales
                QTextStream linestream(&line);
                for(int c=0; c<nDataColumns; ++c)
                {
                    linestream >> importedPLODataVector[c].mPlotScale;
                }
            }
        }
        if (!parseOK)
        {
            gpMessageHandler->addErrorMessage(QString("A parse error occurred while parsing the header of: ")+fileInfo.fileName()+" Aborting import!");
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
        ++mCurrentGenerationNumber;
        SharedVectorVariableT pTimeVec(0);
        SharedVectorVariableT pFreqVec(0);

        if (importedPLODataVector.first().mDataName == TIMEVARIABLENAME)
        {
            pTimeVec = insertTimeVectorVariable(importedPLODataVector.first().mDataValues, fileInfo.absoluteFilePath());
//            UnitScale us = figureOutCutomUnitScale2(TIMEVARIABLENAME, importedPLODataVector.first().mPlotScale);
//            if (!us.isOne() && !us.isEmpty())
//            {
//                //! @todo handle this somehow /Peter
//                //pTimeVec->setCustomUnitScale(us);
//            }
        }
        else if (importedPLODataVector.first().mDataName == FREQUENCYVARIABLENAME)
        {
            pFreqVec = insertFrequencyVectorVariable(importedPLODataVector.first().mDataValues, fileInfo.absoluteFilePath());
//            UnitScale us = figureOutCutomUnitScale2(FREQUENCYVARIABLENAME, importedPLODataVector.first().mPlotScale);
//            if (!us.isOne() && !us.isEmpty())
//            {
//                //! @todo handle this somehow /Peter
//                //pFreqVec->setCustomUnitScale(us);
//            }
        }

        // First decide if we should skip the first column (if time or frequency vector)
        int i=0;
        if (pTimeVec || pFreqVec)
        {
            i=1;
        }
        // Go through all imported variable columns, and create the appropriate vector variable type and insert it
        // Note! You can not mix time frequency or plain vector types in the same plo (v1) file
        SharedVectorVariableT pNewData;
        for (; i<importedPLODataVector.size(); ++i)
        {
            SharedVariableDescriptionT pVarDesc = SharedVariableDescriptionT(new VariableDescription);
            pVarDesc->mDataName = importedPLODataVector[i].mDataName;

            bool isNumber;
            importedPLODataVector[i].mPlotScale.toDouble(&isNumber);
            if (!isNumber)
            {
                pVarDesc->mDataQuantity = importedPLODataVector[i].mPlotScale;
                pVarDesc->mDataUnit = gpConfig->getBaseUnit(pVarDesc->mDataQuantity);
            }
            // Right now we ignore nummeric plotscale, as we removed plotscale from data variables, we look for quantities instead

            // Insert time domain variable
            if (pTimeVec)
            {
                pNewData = insertTimeDomainVariable(pTimeVec, importedPLODataVector[i].mDataValues, pVarDesc, fileInfo.absoluteFilePath());
            }
            // Insert frequency domain variable
            else if (pFreqVec)
            {
                pNewData = insertFrequencyDomainVariable(pFreqVec, importedPLODataVector[i].mDataValues, pVarDesc, fileInfo.absoluteFilePath());
            }
            // Insert plain vector variables
            else
            {
                pNewData = SharedVectorVariableT(new ImportedVectorVariable(importedPLODataVector[i].mDataValues, mCurrentGenerationNumber, pVarDesc,
                                                                            fileInfo.absoluteFilePath(), getGenerationMultiCache(mCurrentGenerationNumber)));
                insertVariable(pNewData);
            }
        }

        if(pNewData)
        {
            mImportedGenerationsMap.insert(pNewData->getGeneration(), pNewData->getImportedFileName());
        }

        emit dataAdded();
    }

    // Limit number of plot generations if there are too many
    limitPlotGenerations();
}

void LogDataHandler2::importFromCSV_AutoFormat(QString importFilePath)
{
    if(importFilePath.isEmpty())
    {

        importFilePath = QFileDialog::getOpenFileName(0,tr("Choose .csv File"),
                                                       gpConfig->getStringSetting(CFG_PLOTDATADIR),
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

void LogDataHandler2::importHopsanRowCSV(QString importFilePath)
{
    if(importFilePath.isEmpty())
    {

        importFilePath = QFileDialog::getOpenFileName(0,tr("Choose .csv File"),
                                                       gpConfig->getStringSetting(CFG_PLOTDATADIR),
                                                       tr("Hopsan row based csv files (*.csv)"));
    }
    if(importFilePath.isEmpty())
    {
        return;
    }

    QFile file(importFilePath);
    QFileInfo fileInfo(file);
    gpConfig->setStringSetting(CFG_PLOTDATADIR, fileInfo.absolutePath());

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
            ++mCurrentGenerationNumber;

            // Figure out time
            SharedVectorVariableT pTimeVec;
            //! @todo what if multiple subsystems with different time
            int timeIdx = allNames.indexOf(TIMEVARIABLENAME);
            if (timeIdx > -1)
            {
                pTimeVec = insertTimeVectorVariable(allDatas[timeIdx], fileInfo.absoluteFilePath());
            }

            SharedVectorVariableT pNewData;
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
                pNewData = insertTimeDomainVariable(pTimeVec, allDatas[i], pVarDesc, fileInfo.absoluteFilePath());
            }

            if(pNewData)
            {
                mImportedGenerationsMap.insert(pNewData->getGeneration(), pNewData->getImportedFileName());
            }

            // Limit number of plot generations if there are too many
            limitPlotGenerations();

            emit dataAdded();
        }
    }
    file.close();
}


void LogDataHandler2::importFromPlainColumnCsv(QString importFilePath)
{
    if(importFilePath.isEmpty())
    {

        importFilePath = QFileDialog::getOpenFileName(0,tr("Choose .csv File"),
                                                       gpConfig->getStringSetting(CFG_PLOTDATADIR),
                                                       tr("Comma-separated values files (*.csv)"));
    }
    if(importFilePath.isEmpty())
    {
        return;
    }

    QFile file(importFilePath);
    QFileInfo fileInfo(file);
    gpConfig->setStringSetting(CFG_PLOTDATADIR, fileInfo.absolutePath());

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
        ++mCurrentGenerationNumber;
        SharedVectorVariableT pTimeVec(0);

        // Ugly, assume that first vector is always time
        pTimeVec = insertTimeVectorVariable(data[0], fileInfo.absoluteFilePath());

        SharedVectorVariableT pNewData;
        for (int i=1; i<data.size(); ++i)
        {
            SharedVariableDescriptionT pVarDesc = SharedVariableDescriptionT(new VariableDescription);
            pVarDesc->mDataName = "CSV"+QString::number(i);
            pNewData = insertTimeDomainVariable(pTimeVec, data[i], pVarDesc, fileInfo.absoluteFilePath());
        }

        if(pNewData)
        {
            mImportedGenerationsMap.insert(pNewData->getGeneration(), pNewData->getImportedFileName());
        }

        emit dataAdded();
    }

    // Limit number of plot generations if there are too many
    limitPlotGenerations();
}


//! @todo this function assumes , separated format and only values,
//! @todo what if file does not have a time vector
void LogDataHandler2::importTimeVariablesFromCSVColumns(const QString csvFilePath, QVector<int> datacolumns, QStringList datanames, QVector<int> timecolumns)
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
                //! @todo it would be better here to read line by line into a large array and then extract sub vectors from that through a smart matrix object
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
            ++mCurrentGenerationNumber;

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
            SharedVectorVariableT pNewData;
            for (int n=0; n<datanames.size(); ++n)
            {
                //! @todo what if data name already exists?
                SharedVariableDescriptionT pVarDesc = SharedVariableDescriptionT(new VariableDescription);
                pVarDesc->mDataName = datanames[n];

                // Lookup time vector to use, and insert time domain data
                int tid = timecolumns[n];
                pNewData = insertTimeDomainVariable(timePtrs[tid], dataColumns[n], pVarDesc, fileInfo.absoluteFilePath());
            }

            csvFile.close();

            if(pNewData)
            {
                mImportedGenerationsMap.insert(pNewData->getGeneration(), pNewData->getImportedFileName());
            }

            emit dataAdded();
        }
        else
        {
            gpMessageHandler->addErrorMessage("Could not open data file:  "+csvFilePath);
        }
    }
    else
    {
        gpMessageHandler->addErrorMessage("columns.size() != names.size() in:  LogDataHandler2::importTimeVariablesFromCSVColumns()");
    }
}


//! @brief Returns whether or not plot data is empty
bool LogDataHandler2::isEmpty()
{
    return mGenerationMap.isEmpty();
}

void LogDataHandler2::clear()
{
    // Clear all data generations
    for (auto git = mGenerationMap.begin(); git!=mGenerationMap.end(); ++git)
    {
        delete git.value();
    }
    mGenerationMap.clear();

    // Remove imported variables
    mImportedGenerationsMap.clear();

    // Clear generation Cache files (Individual files will remain if until all instances dies)
    mGenerationCacheMap.clear();

    // Remove the cache directory if it is empty, if it is not then cleanup should happen on program exit
    for (int i=0; i<mCacheDirs.size(); ++i)
    {
        mCacheDirs[i].rmdir(mCacheDirs[i].path());
    }

    // Reset generation number
    mCurrentGenerationNumber = -1;
}




//! @brief Collects plot data from last simulation
void LogDataHandler2::collectLogDataFromModel(bool overWriteLastGeneration)
{
    TicToc tictoc;

    if (!(mpParentModel && mpParentModel->getTopLevelSystemContainer()))
    {
        return;
    }
    SystemContainer *pTopLevelSystem = mpParentModel->getTopLevelSystemContainer();
    if (pTopLevelSystem->getCoreSystemAccessPtr()->getNSamples() == 0)
    {
        //Don't collect plot data if logging is disabled (to avoid empty generations)
        return;
    }

    // Increment generation number if we are not overwriting last generation
    if(!overWriteLastGeneration)
    {
        ++mCurrentGenerationNumber;
    }


    //! @todo why not run multiappend when overwriting generation ? Because then we are not appending, need some common open mode
    if(!overWriteLastGeneration)
    {
        this->getGenerationMultiCache(mCurrentGenerationNumber)->beginMultiAppend();
    }

    QMap<std::vector<double>*, SharedVectorVariableT> generationTimeVectors;
    bool foundData = collectLogDataFromSystem(pTopLevelSystem, QStringList(), generationTimeVectors);

    if(!overWriteLastGeneration)
    {
        this->getGenerationMultiCache(mCurrentGenerationNumber)->endMultiAppend();
    }

    // Limit number of plot generations if there are too many
    limitPlotGenerations();

    // If we found data then emit signals
    if (foundData)
    {
        emit dataAdded();
        emit dataAddedFromModel(true);
    }
    // If we did not find data, and if we were NOT overwriting last generation then decrement generation counter
    else if (!overWriteLastGeneration)
    {
        // Revert generation number if no data was found
        --mCurrentGenerationNumber;
    }
    tictoc.toc("Collect plot data");
}

bool LogDataHandler2::collectLogDataFromSystem(SystemContainer *pCurrentSystem, const QStringList &rSystemHieararchy, QMap<std::vector<double>*, SharedVectorVariableT> &rGenTimeVectors)
{
    SharedSystemHierarchyT sharedSystemHierarchy(new QStringList(rSystemHieararchy));
    bool foundData=false, foundDataInSubsys=false;

    // Store the systems own time vector
    auto pCoreSysTimeVector = pCurrentSystem->getCoreSystemAccessPtr()->getLogTimeData();
    if (pCoreSysTimeVector && !pCoreSysTimeVector->empty())
    {
        // Check so taht we have not already stored this time vector in this generation
        if (!rGenTimeVectors.contains(pCoreSysTimeVector))
        {
            //! @todo here we need to copy (convert) from std vector to qvector, don know if that slows down (probably not much)
            auto pSysTimeVector = insertTimeVectorVariable(QVector<double>::fromStdVector(*pCoreSysTimeVector), sharedSystemHierarchy);
            rGenTimeVectors.insert(pCoreSysTimeVector, pSysTimeVector);
        }
    }

    // Iterate components
    QList<ModelObject*> currentLevelModelObjects = pCurrentSystem->getModelObjects();
    foreach(ModelObject* pModelObject, currentLevelModelObjects)
    {
        // First treat as an ordinary component (even if it is a subsystem), to get variables in the current system
        auto ports = pModelObject->getPortListPtrs();
        for(auto &pPort : ports)
        {
            QVector<CoreVariableData> varDescs;
            pCurrentSystem->getCoreSystemAccessPtr()->getVariableDescriptions(pModelObject->getName(),
                                                                              pPort->getName(),
                                                                              varDescs);

            // Iterate variables
            for(auto &varDesc : varDescs)
            {
                // Skip hidden variables
                if ( gpConfig->getBoolSetting(CFG_SHOWHIDDENNODEDATAVARIABLES) || (varDesc.mNodeDataVariableType != "Hidden") )
                {
                    // Fetch variable data
                    QVector<double> dataVec;
                    std::vector<double> *pCoreVarTimeVector=0;
                    pCurrentSystem->getCoreSystemAccessPtr()->getPlotData(pModelObject->getName(),
                                                                          pPort->getName(),
                                                                          varDesc.mName,
                                                                          pCoreVarTimeVector,
                                                                          dataVec);

                    // Prevent adding data if time or data vector was empty
                    if (!pCoreVarTimeVector->empty() && !dataVec.isEmpty())
                    {
                        foundData=true;
                        SharedVariableDescriptionT pVarDesc = SharedVariableDescriptionT(new VariableDescription);
                        pVarDesc->mModelPath = pModelObject->getParentContainerObject()->getModelFileInfo().filePath();
                        pVarDesc->mpSystemHierarchy = sharedSystemHierarchy;
                        pVarDesc->mComponentName = pModelObject->getName();
                        pVarDesc->mPortName = pPort->getName();
                        pVarDesc->mDataName = varDesc.mName;
                        pVarDesc->mDataUnit = varDesc.mUnit;
                        pVarDesc->mDataQuantity = varDesc.mQuantity;
                        pVarDesc->mDataDescription = varDesc.mDescription;
                        pVarDesc->mAliasName  = varDesc.mAlias;
                        pVarDesc->mVariableSourceType = ModelVariableType;

                        SharedVectorVariableT pNewData;

                        // Lookup which time vector from system parent or system grand parent to use
                        auto pSysTimeVector = rGenTimeVectors.value(pCoreVarTimeVector);

                        // Insert variable with parent system time vector if  that is what it is using
                        if (pSysTimeVector)
                        {
                            pNewData = insertTimeDomainVariable(pSysTimeVector, dataVec, pVarDesc);
                        }
                        // Else create a unique variable time vector for this component
                        else
                        {
                            auto pVarTimeVec = insertTimeVectorVariable(QVector<double>::fromStdVector(*pCoreVarTimeVector), SharedSystemHierarchyT());
                            pNewData = insertTimeDomainVariable(pVarTimeVec, dataVec, pVarDesc);
                        }
                    }
                }
            }
        }

        // If this is a subsystem, then go into it
        if (pModelObject->type() == SystemContainerType )
        {
            QStringList subsysHierarchy = rSystemHieararchy;
            subsysHierarchy << pModelObject->getName();
            bool foundDataInThisSubsys = collectLogDataFromSystem(qobject_cast<SystemContainer*>(pModelObject), subsysHierarchy, rGenTimeVectors);
            foundDataInSubsys = foundDataInSubsys || foundDataInThisSubsys;
        }
    }
    return (foundData || foundDataInSubsys);
}

void LogDataHandler2::collectLogDataFromRemoteModel(std::vector<std::string> &rNames, std::vector<double> &rData, bool overWriteLastGeneration)
{
    TicToc tictoc;
    if(!overWriteLastGeneration)
    {
        ++mCurrentGenerationNumber;
    }

    if(rData.size() == 0)
    {
        return;         //Don't collect plot data if logging is disabled (to avoid empty generations)
    }


    bool foundData = false;

    std::vector<double> *pCoreSysTimeVector=0, *pPrevInsertedCoreVarTimeVector=0;
    SharedVectorVariableT pSysTimeVec, pVarTimeVec;

    //! @todo why not run multiappend when overwriting generation ? Because then we are not appending, need some common open mode
    if(!overWriteLastGeneration)
    {
        this->getGenerationMultiCache(mCurrentGenerationNumber)->beginMultiAppend();
    }

    QList<SharedVariableDescriptionT> varDescs;
    varDescs.reserve(rNames.size());
    QMap<size_t, SharedSystemHierarchyT> systemTimeVarIdAndSysname;
    QMap<QString, SharedVectorVariableT> systemHierarchy2TimeVariable;

    // Iterate over variables
    for(int v=0; v<rNames.size(); ++v)
    {
        foundData=true;
        QStringList sh;
        QString comp, port, data;
        splitFullVariableName(QString::fromStdString(rNames[v]),sh,comp,port,data);


        SharedVariableDescriptionT pVarDesc = SharedVariableDescriptionT(new VariableDescription);
        pVarDesc->mModelPath = "UNKNOWN";
        pVarDesc->mComponentName = comp;
        pVarDesc->mpSystemHierarchy = SharedSystemHierarchyT(new QStringList(sh)); //!< @todo would be nice if we could avoid duplicates here
        pVarDesc->mPortName = port;
        pVarDesc->mDataName = data;
        pVarDesc->mDataUnit = "UNKOWN";
        pVarDesc->mDataDescription = "UNKOWN";
        pVarDesc->mAliasName  = "";
        pVarDesc->mVariableSourceType = ModelVariableType;

        varDescs.push_back(pVarDesc);

        if (data == TIMEVARIABLENAME)
        {
            systemTimeVarIdAndSysname.insert(v,pVarDesc->mpSystemHierarchy);
        }
    }

    // Figure out number of plot elements for each variable
    long int nElements = rData.size() / rNames.size();
    qDebug() << "nElements: " << nElements;

    // First process time variables
    for (auto it=systemTimeVarIdAndSysname.begin(); it!=systemTimeVarIdAndSysname.end(); ++it)
    {
        QVector<double> timeData;
        timeData.reserve(nElements);
        size_t v = it.key();
        for (size_t i=nElements*v; i<nElements*(v+1); ++i)
        {
            timeData.push_back(rData[i]);
        }

        auto pNewData = insertTimeVectorVariable(timeData, it.value());
        systemHierarchy2TimeVariable.insert(it.value()->join("$"), pNewData);
    }

    auto timeIds = systemTimeVarIdAndSysname.keys();
    for(size_t v=0; v<rNames.size(); ++v)
    {
        // skip variables that were time variables
        if (timeIds.contains(v))
        {
            continue;
        }

        QVector<double> newData;
        newData.reserve(nElements);
        for (size_t i=nElements*v; i<nElements*(v+1); ++i)
        {
            newData.push_back(rData[i]);
        }

        // Lookup time variable
        SharedVectorVariableT pTime = systemHierarchy2TimeVariable.value(varDescs[v]->mpSystemHierarchy->join("$"));
        if (pTime)
        {
            varDescs[v]->mpSystemHierarchy = pTime->mpVariableDescription->mpSystemHierarchy; // Replace systems hierachy with shared version (to save some memmory)
        }

        // Insert time domain variable
        insertTimeDomainVariable(pTime, newData, varDescs[v]);
    }

    if(!overWriteLastGeneration)
    {
        this->getGenerationMultiCache(mCurrentGenerationNumber)->endMultiAppend();
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
        --mCurrentGenerationNumber;
    }
    tictoc.toc("Collect plot data");
}

void LogDataHandler2::exportGenerationToPlo(const QString &rFilePath, int gen, const int version) const
{
    if (gen == -1)
    {
        gen = mCurrentGenerationNumber;
    }

    QList<SharedVectorVariableT> vars = getAllNonAliasVariablesAtGeneration(gen);
    //! @todo what if you have removed all of the original variables and only want to export the alias variables
    //! @todo error message if nothing found

    // Ok now remove time vector as it will be added again in plo export
    // This is a hack, yes I know
    for (int i=0; i<vars.size(); ++i)
    {
        if (vars[i]->getDataName() == TIMEVARIABLENAME)
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
QVector<double> LogDataHandler2::copyVariableDataVector(const QString &rName, const int generation)
{
    SharedVectorVariableT pData = getVectorVariable(rName, generation);
    if (pData)
    {
        return pData->getDataVectorCopy();
    }

    return QVector<double>();
}

SharedVectorVariableT LogDataHandler2::getVectorVariable(const QString &rName, int generation) const
{
    // If gen < 0 use current generation
    if (generation < 0)
    {
        generation = mCurrentGenerationNumber;
    }

    // Find the generation
    auto *pGen = mGenerationMap.value(generation, 0);
    if(pGen)
    {
        // Now find variable in generation
        return pGen->getVariable(rName);
    }
    return SharedVectorVariableT();
}


//! @brief Returns multiple logdatavariables based on regular expression search. Excluding temp variables but including aliases
//! @param [in] rNameExp The regular expression for the names to match
//! @param [in] generation The desired generation of the variable
QList<SharedVectorVariableT> LogDataHandler2::getMatchingVariablesAtGeneration(const QRegExp &rNameExp, int generation) const
{
    // Should we take current
    if (generation < 0)
    {
        generation = mCurrentGenerationNumber;
    }

    // Find the generation
    auto *pGen = mGenerationMap.value(generation, 0);
    if(pGen)
    {
        // Now find variable in generation
        return pGen->getMatchingVariables(rNameExp);
    }
    return QList<SharedVectorVariableT>();
}

QList<SharedVectorVariableT> LogDataHandler2::getMatchingVariablesFromAllGeneration(const QRegExp &rNameExp) const
{
    QList<SharedVectorVariableT> allData;
    for (auto pGen : mGenerationMap.values())
    {
        QList<SharedVectorVariableT> data = pGen->getMatchingVariables(rNameExp);
        allData.append(data);
    }
    return allData;
}


//! @brief Returns the time vector for specified generation
//! @param[in] generation Generation
const SharedVectorVariableT LogDataHandler2::getTimeVectorVariable(int generation) const
{
    // Find the generation
    auto *pGen = mGenerationMap.value(generation, 0);
    if(pGen)
    {
        // Now find variable in generation
        return pGen->getVariable(TIMEVARIABLENAME);
    }
    return SharedVectorVariableT();
}

QVector<double> LogDataHandler2::copyTimeVector(const int generation) const
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
//bool LogDataHandler2::componentHasPlotGeneration(int generation, QString fullName)
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




////! @brief Returns plot variable for specified alias
////! @param[in] rAlias Alias of variable
////! @param[in] gen Generation to check, -1 = Current, -2 = Newest Available, >= 0 = Specific generation to check
//QString LogDataHandler2::getFullNameFromAlias(const QString &rAlias, const int gen) const
//{
////    SharedVectorVariableContainerT pAliasContainer = getVariableContainer(rAlias);
////    if (pAliasContainer && pAliasContainer->isStoringAlias())
////    {
////        bool isStoringAlias = false;
////        int g = INT_MIN;
////        // Check if current
////        if (gen == -1)
////        {
////            g = mGenerationNumber;
////        }
////        // Check specific
////        else if (gen >= 0)
////        {
////            g = gen;
////        }
////        // First match (newest availible)
////        else if (gen == -2)
////        {
////            g = pAliasContainer->getHighestGeneration();
////        }
////        isStoringAlias = pAliasContainer->isGenerationAlias(g);

////        // Note there could be multiple different names under the same alias, but generation is used to choose
////        if (isStoringAlias && (pAliasContainer->getDataGeneration(g)->getAliasName() == rAlias) )
////        {
////            return pAliasContainer->getDataGeneration(g)->getFullVariableName();
////        }
////    }
////    return QString();
//}


QList<int> LogDataHandler2::getGenerations() const
{
    return mGenerationMap.keys();
}

int LogDataHandler2::getLowestGenerationNumber() const
{
    if (!mGenerationMap.isEmpty())
    {
        return mGenerationMap.begin().key();
    }
    return -1;
}

int LogDataHandler2::getHighestGenerationNumber() const
{
    if (!mGenerationMap.isEmpty())
    {
        return (--(mGenerationMap.end())).key();
    }
    return -1;
}

//! @brief Ask each variable how many generations it has, and returns the maximum (the total number of available generations)
//! @note This function will become slower as the number of unique variable names grow
int LogDataHandler2::getNumberOfGenerations() const
{
    return mGenerationMap.size();
}


//! @brief Limits number of plot generations to value specified in configuration
void LogDataHandler2::limitPlotGenerations()
{
    int numGens = getNumberOfGenerations() ;
    const int generationLimit = gpConfig->getGenerationLimit();
    if (numGens > generationLimit)
    {
        if(!gpConfig->getBoolSetting(CFG_AUTOLIMITGENERATIONS))
        {
            QDialog dialog(gpMainWindowWidget);
            dialog.setWindowTitle("Hopsan");
            QVBoxLayout *pLayout = new QVBoxLayout(&dialog);
            QLabel *pLabel = new QLabel("<b>Log data generation limit reached!</b><br><br>Generation limit: "+QString::number(generationLimit)+
                                        "<br>Number of data generations: "+QString::number(numGens)+
                                        "<br><br><b>Discard "+QString::number(numGens-generationLimit)+" generations(s)?</b>");
            QCheckBox *pAutoLimitCheckBox = new QCheckBox("Automatically discard old generations", &dialog);
            pAutoLimitCheckBox->setChecked(gpConfig->getBoolSetting(CFG_AUTOLIMITGENERATIONS));
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
            gpConfig->setBoolSetting(CFG_AUTOLIMITGENERATIONS, pAutoLimitCheckBox->isChecked());

            if(retval == QDialog::Rejected)
            {
                return;
            }
        }


        TicToc timer;
        int highestGeneration = getHighestGenerationNumber();
        int lowestGeneration = getLowestGenerationNumber();
        int highestToRemove = highestGeneration-generationLimit+1;
        bool didRemoveSomething = false;

        // Only do the purge if the lowest generation is under upper limit
        if (lowestGeneration <= highestToRemove)
        {
            // Note!
            // Here we must iterate through a copy of the map
            // if we use an iterator in the original map then the iterator will become invalid if an item is removed
            GenerationMapT gens = mGenerationMap;
            for (auto it=gens.begin(); it!=gens.end(); ++it)
            {
                const int g = it.key();

                // Only break loop when we have deleted or pruned all below purge limit
                if ( (g > highestToRemove) )
                {
                    break;
                }
                else
                {
                    // Try to remove each generation
                    didRemoveSomething += removeGeneration(g, false);
                }
            }
        }

        if (didRemoveSomething)
        {
            emit dataRemoved();

            //! @todo this is not a good solution, signaling would be better but it would be nice not to make every thing into qobjects
            // Loop until we have removed every empty generation cache object
            QList<int> gens = mGenerationCacheMap.keys();
            for (int g : gens)
            {
                if (g>highestToRemove)
                {
                    break;
                }
                removeGenerationCacheIfEmpty(g);
            }
        }
        timer.toc("removeOldGenerations");
    }
}

//! @brief Removes a generation
bool LogDataHandler2::removeGeneration(const int gen, const bool force)
{
    auto git = mGenerationMap.find(gen);
    if (git != mGenerationMap.end())
    {
        LogDataGeneration *pGen = git.value();
        if (force || !pGen->isImported())
        {
            int nVarsPre = pGen->getNumVariables();
            bool genEmpty = pGen->clear(force);
            int nVarsPost = pGen->getNumVariables();

            // Only delete the generation if it really becomes empty
            //! @todo a problem here is that limit will always go in here and call "prune (clear)" in the generation, wasting time, could be a problem if very many generations /Peter
            if (genEmpty)
            {
                delete pGen;
                mGenerationMap.erase(git);
//                if (pGen->getNumKeepVariables() != 0)
//                {
//                    mNumKeepGenerations--;
//                }
                // If the generation was removed then try to remove the cache object (if any), if it still have active subscribers it will remain
                removeGenerationCacheIfEmpty(gen);

                // Remove if it was an imported generation
                mImportedGenerationsMap.remove(gen);

                // Emit signal
                emit dataRemoved();

                return true;
            }
            // If we did not clear all data, then at least prune the log data cache to conserve disk space
            // but only if we actually removed some variables, otherwise there is no point in pruning (which takes time)
            else if (nVarsPre != nVarsPost)
            {
                //! @todo, pruning may take time, maybe we should have config option for this
                pruneGenerationCache(gen, pGen);

                // Emit signal
                emit dataRemoved();

                return true;
            }
        }
    }
    return false;
}

const QList<QDir> &LogDataHandler2::getCacheDirs() const
{
    return mCacheDirs;
}

//! @brief Will retrieve an existing or create a new generation multi-cache object
SharedMultiDataVectorCacheT LogDataHandler2::getGenerationMultiCache(const int gen)
{
    SharedMultiDataVectorCacheT pCache = mGenerationCacheMap.value(gen, SharedMultiDataVectorCacheT());
    if (!pCache)
    {
        pCache = SharedMultiDataVectorCacheT(new MultiDataVectorCache(getNewCacheName()));
        mGenerationCacheMap.insert(gen, pCache);
    }
    return pCache;
}

void LogDataHandler2::pruneGenerationCache(const int generation)
{
    pruneGenerationCache(generation, mGenerationMap.value(generation, 0));
}

void LogDataHandler2::pruneGenerationCache(const int generation, LogDataGeneration *pGeneration)
{
    if (pGeneration)
    {
        QString prevName;
        SharedMultiDataVectorCacheT pPrevCache = mGenerationCacheMap.value(generation, SharedMultiDataVectorCacheT());
        if (pPrevCache)
        {
            prevName = pPrevCache->getCacheFileInfo().baseName();
        }

        SharedMultiDataVectorCacheT pCache = SharedMultiDataVectorCacheT(new MultiDataVectorCache(getNewCacheName(prevName)));
        pGeneration->switchGenerationDataCache(pCache);

        // Replace old generation
        mGenerationCacheMap.insert(generation, pCache);
    }
}


//! @brief Increments counter for number of open plot curves (in plot windows)
//! @note Used to decide if warning message shall be shown when closing model
//! @see PlotData::decrementOpenPlotCurves()
//! @see PlotData::hasOpenPlotCurves()
void LogDataHandler2::incrementOpenPlotCurves()
{
    ++mNumPlotCurves;
}


//! @brief Decrements counter for number of open plot curves (in plot windows)
//! @note Used to decide if warning message shall be shown when closing model
//! @see PlotData::incrementOpenPlotCurves()
//! @see PlotData::hasOpenPlotCurves()
void LogDataHandler2::decrementOpenPlotCurves()
{
    --mNumPlotCurves;
}


SharedVectorVariableT LogDataHandler2::addVariableWithScalar(const SharedVectorVariableT a, const double x)
{
    SharedVectorVariableT pTempVar = createOrphanVariable(a->getSmartName()+"+"+QString::number(x), a->getVariableType());
    pTempVar->assignFrom(a);
    pTempVar->addToData(x);
    return pTempVar;
}


SharedVectorVariableT LogDataHandler2::subVariableWithScalar(const SharedVectorVariableT a, const double x)
{
    SharedVectorVariableT pTempVar = createOrphanVariable(a->getSmartName()+"-"+QString::number(x), a->getVariableType());
    pTempVar->assignFrom(a);
    pTempVar->subFromData(x);
    return pTempVar;
}


SharedVectorVariableT LogDataHandler2::mulVariableWithScalar(const SharedVectorVariableT a, const double x)
{
    SharedVectorVariableT pTempVar = createOrphanVariable(a->getSmartName()+"*"+QString::number(x), a->getVariableType());
    pTempVar->assignFrom(a);
    pTempVar->multData(x);
    return pTempVar;
}


SharedVectorVariableT LogDataHandler2::divVariableWithScalar(const SharedVectorVariableT a, const double x)
{
    SharedVectorVariableT pTempVar = createOrphanVariable(a->getSmartName()+"/"+QString::number(x), a->getVariableType());
    pTempVar->assignFrom(a);
    pTempVar->divData(x);
    return pTempVar;
}


SharedVectorVariableT LogDataHandler2::addVariables(const SharedVectorVariableT a, const SharedVectorVariableT b)
{
    SharedVectorVariableT pTempVar = createOrphanVariable(a->getSmartName()+"+"+b->getSmartName(), a->getVariableType());
    pTempVar->assignFrom(a);
    pTempVar->addToData(b);
    return pTempVar;
}


SharedVectorVariableT LogDataHandler2::diffVariables(const SharedVectorVariableT a, const SharedVectorVariableT b)
{
    SharedVectorVariableT pTempVar = createOrphanVariable(a->getSmartName()+"_Diff", a->getVariableType());
    pTempVar->assignFrom(a);
    pTempVar->diffBy(b);
    return pTempVar;
}


SharedVectorVariableT LogDataHandler2::integrateVariables(const SharedVectorVariableT a, const SharedVectorVariableT b)
{
    SharedVectorVariableT pTempVar = createOrphanVariable(a->getSmartName()+"_Int", a->getVariableType());
    pTempVar->assignFrom(a);
    pTempVar->integrateBy(b);
    return pTempVar;
}


SharedVectorVariableT LogDataHandler2::lowPassFilterVariable(const SharedVectorVariableT a, const SharedVectorVariableT b, const double freq)
{
    SharedVectorVariableT pTempVar = createOrphanVariable(a->getSmartName()+"_Lp1", a->getVariableType());
    pTempVar->assignFrom(a);
    pTempVar->lowPassFilter(b, freq);
    return pTempVar;
}


////! @brief Returns the number of log data variables registered in this log data handler
////! @returns The number of registered log data variables
//int LogDataHandler2::getNumVariables() const
//{
//    return -1;//mLogDataMap.size();
//}


SharedVectorVariableT LogDataHandler2::elementWiseGT(SharedVectorVariableT pData, const double thresh)
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

SharedVectorVariableT LogDataHandler2::elementWiseLT(SharedVectorVariableT pData, const double thresh)
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

//QString LogDataHandler2::saveVariable(const QString &currName, const QString &newName)
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

SharedVectorVariableT LogDataHandler2::subVariables(const SharedVectorVariableT a, const SharedVectorVariableT b)
{
    SharedVectorVariableT pTempVar = createOrphanVariable(a->getSmartName()+"-"+b->getSmartName(), a->getVariableType());
    pTempVar->assignFrom(a);
    pTempVar->subFromData(b);
    return pTempVar;
}

SharedVectorVariableT LogDataHandler2::multVariables(const SharedVectorVariableT a, const SharedVectorVariableT b)
{
    SharedVectorVariableT pTempVar = createOrphanVariable(a->getSmartName()+"*"+b->getSmartName(), a->getVariableType());
    pTempVar->assignFrom(a);
    pTempVar->multData(b);
    return pTempVar;
}

SharedVectorVariableT LogDataHandler2::divVariables(const SharedVectorVariableT a, const SharedVectorVariableT b)
{
    SharedVectorVariableT pTempVar = createOrphanVariable(a->getSmartName()+"/"+b->getSmartName(), a->getVariableType());
    pTempVar->assignFrom(a);
    pTempVar->divData(b);
    return pTempVar;
}




//! @brief Creates an orphan temp variable that will be deleted when its shared pointer reference counter reaches zero (when no one is using it)
//! @details This function will not insert the variable into the generation but but it will assign the current generation number anyway, we wont get that if using createFreeVariable
SharedVectorVariableT LogDataHandler2::createOrphanVariable(const QString &rName, VariableTypeT type)
{
    SharedVariableDescriptionT varDesc = SharedVariableDescriptionT(new VariableDescription());
    varDesc->mDataName = rName;
    varDesc->mVariableSourceType = ScriptVariableType;
    SharedVectorVariableT pNewData = createFreeVariable(type,varDesc);
    pNewData->mGeneration = mCurrentGenerationNumber;
    return pNewData;
}

bool LogDataHandler2::removeVariable(const QString &rVarName, int generation)
{
    // If generation = -1 then use current generation
    if (generation == -1)
    {
        generation = mCurrentGenerationNumber;
    }

    bool didRemove=false;

    // If gen < -1 remove variable in all gens
    if ( generation == -2)
    {
        // Need to work with a copy of values in case last variable is removed, then generation will be removed as well ( and iterator in map will becom invalid )
        QList<LogDataGeneration*> gens = mGenerationMap.values();
        for(auto pGen: gens)
        {
            didRemove += pGen->removeVariable(rVarName);
        }
    }
    // Else remove in specific generation
    else
    {
        auto *pGen = mGenerationMap.value(generation, 0);
        if(pGen)
        {
            // Now remove variable in generation
            didRemove = pGen->removeVariable(rVarName);
        }
    }

    if (didRemove)
    {
        emit dataRemoved();
    }
    return didRemove;

    //! @todo what about unregistering ALIAS, maybe handled inside generation
}


SharedVectorVariableT LogDataHandler2::defineNewVectorVariable(const QString &rDesiredname, VariableTypeT type)
{
    if (!isNameValid(rDesiredname))
    {
        gpMessageHandler->addErrorMessage(QString("Invalid variable name: %1").arg(rDesiredname));
        return SharedVectorVariableT();
    }

    auto pGen = getCurrentGeneration();
    if (pGen && !pGen->haveVariable(rDesiredname))
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
bool LogDataHandler2::hasOpenPlotCurves()
{
    return (mNumPlotCurves > 0);
}

void LogDataHandler2::closePlotsWithCurvesBasedOnOwnedData()
{
    emit closePlotsWithOwnedData();
}


PlotWindow *LogDataHandler2::plotVariable(const QString plotName, const QString fullVarName, const int gen, const int axis, QColor color)
{
    SharedVectorVariableT data = getVectorVariable(fullVarName, gen);
    if(data)
    {
        return gpPlotHandler->plotDataToWindow(plotName, data, axis, color);
    }
    return 0;
}

PlotWindow *LogDataHandler2::plotVariable(const QString plotName, const QString &rFullNameX, const QString &rFullNameY, const int gen, const int axis, QColor color)
{
    SharedVectorVariableT xdata = getVectorVariable(rFullNameX, gen);
    SharedVectorVariableT ydata = getVectorVariable(rFullNameY, gen);
    if (xdata && ydata)
    {
        return gpPlotHandler->plotDataToWindow(plotName, xdata, ydata, axis, color);
    }
    return 0;
}

PlotWindow *LogDataHandler2::plotVariable(PlotWindow *pPlotWindow, const QString fullVarName, const int gen, const int axis, QColor color)
{
    SharedVectorVariableT data = getVectorVariable(fullVarName, gen);
    if(data)
    {
        return gpPlotHandler->plotDataToWindow(pPlotWindow, data, axis, true, color);
    }
    return 0;
}


//! @brief Get a list of all available variables at a specific generation, variables that does not have this generation will not be included.  Aliases are excluded.
QList<SharedVectorVariableT> LogDataHandler2::getAllNonAliasVariablesAtGeneration(const int generation) const
{
    auto pGen = getGeneration(generation);
    if (pGen)
    {
        return pGen->getAllNonAliasVariables();
    }
    return QList<SharedVectorVariableT>();
}

QList<SharedVectorVariableT> LogDataHandler2::getAllVariablesAtGeneration(const int generation)
{
    auto pGen = getGeneration(generation);
    if (pGen)
    {
        return pGen->getAllVariables();
    }
    return QList<SharedVectorVariableT>();
}

QList<SharedVectorVariableT> LogDataHandler2::getAllVariablesAtCurrentGeneration()
{
    return getAllVariablesAtGeneration(mCurrentGenerationNumber);
}

QList<SharedVectorVariableT> LogDataHandler2::getAllVariablesAtRespectiveNewestGeneration()
{
    QMap<QString, SharedVectorVariableT> data;
    // Iterated generations and collect variables in a map
    // For newer generations replace old data values
    for (auto git = mGenerationMap.begin(); git != mGenerationMap.end(); ++git)
    {
        auto vars = git.value()->getAllVariables();
        for (auto var : vars)
        {
            data.insert(var->getSmartName(), var); //0 is dummy value
        }
    }
    return data.values();
}

QList<SharedVectorVariableT> LogDataHandler2::getMatchingVariablesAtRespectiveNewestGeneration(const QRegExp &rNameExp) const
{
    QMap<QString, SharedVectorVariableT> data;
    // Iterated generations and collect variables in a map
    // For newer generations replace old data values
    for (auto git = mGenerationMap.begin(); git != mGenerationMap.end(); ++git)
    {
        auto vars = git.value()->getMatchingVariables(rNameExp);
        for (auto var : vars)
        {
            data.insert(var->getSmartName(), var); //0 is dummy value
        }
    }
    return data.values();
}

QList<int> LogDataHandler2::getImportedGenerations() const
{
    return mImportedGenerationsMap.keys();
}


//void LogDataHandler2::getVariableNamesWithHighestGeneration(QStringList &rNames, QList<int> &rGens) const
//{
//    rNames.clear(); rGens.clear();
//    rNames.reserve(mLogDataMap.size());
//    rGens.reserve(mLogDataMap.size());

//    LogDataMapT::const_iterator dit = mLogDataMap.begin();
//    for ( ; dit!=mLogDataMap.end(); ++dit)
//    {
//        SharedVectorVariableT pData = dit.value()->getDataGeneration(-1);
//        if(pData)
//        {
//            rNames.append(dit.key());
//            rGens.append(pData->getGeneration());
//        }
//    }
//}

int LogDataHandler2::getCurrentGenerationNumber() const
{
    return mCurrentGenerationNumber;
}

const LogDataGeneration *LogDataHandler2::getCurrentGeneration() const
{
    return mGenerationMap.value(mCurrentGenerationNumber, 0);
}

const LogDataGeneration *LogDataHandler2::getGeneration(const int gen) const
{
    return mGenerationMap.value(gen, 0);
}

void LogDataHandler2::getVariableGenerationInfo(const QString &rFullName, int &rLowest, int &rHighest) const
{
    rLowest = -1;
    rHighest = -1;

    // Search lowest
    for (auto it = mGenerationMap.begin(); it!=mGenerationMap.end(); ++it)
    {
        if (it.value()->haveVariable(rFullName))
        {
            rLowest = it.key();
            break;
        }
    }

    // Search highest
    QMapIterator< int, LogDataGeneration*> rit(mGenerationMap);
    rit.toBack();
    while (rit.hasPrevious())
    {
        auto item = rit.previous();
        if (item.value()->haveVariable(rFullName))
        {
            rHighest = item.key();
            break;
        }
    }
}


QStringList LogDataHandler2::getVariableFullNames(int generation) const
{
    // If gen < 0 use current generation
    if (generation < 0)
    {
        generation = mCurrentGenerationNumber;
    }

    auto pGen = getGeneration(generation);
    if (pGen)
    {
        //! @todo fix gen < -1 (all gens)
        return pGen->getVariableFullNames();
    }
    return QStringList();
}


LogDataHandler2::ImportedGenerationsMapT LogDataHandler2::getImportFilesAndGenerations() const
{
    return mImportedGenerationsMap;
}


bool LogDataHandler2::isGenerationImported(int generation)
{
    // If gen < 0 use current generation
    if (generation < 0)
    {
        generation = mCurrentGenerationNumber;
    }

    auto pGen = getGeneration(generation);
    if (pGen)
    {
        return pGen->isImported();
    }
    return false;
}


QString LogDataHandler2::getNewCacheName(const QString &rDesiredName)
{
    // The first directory is the main one, any other directories have been appended later when taking ownership of someone else's data
    QDir dir = mCacheDirs.first();
    if (rDesiredName.isEmpty())
    {
        return dir.absoluteFilePath(QString("cf%1").arg(mCacheSubDirCtr++));
    }
    else
    {
        return dir.absoluteFilePath(QString("%1_%2").arg(rDesiredName).arg(mCacheSubDirCtr++));
    }
}


//! @brief Removes a generation cache object from map if it has no subscribers
void LogDataHandler2::removeGenerationCacheIfEmpty(const int gen)
{
    // Removes a generation cache object from map if it has no subscribers
    GenerationCacheMapT::iterator it = mGenerationCacheMap.find(gen);
    if ( (it!=mGenerationCacheMap.end()) && (it.value()->getNumSubscribers()==0) )
    {
        mGenerationCacheMap.erase(it);
    }
}

void LogDataHandler2::takeOwnershipOfData(LogDataHandler2 *pOtherHandler, const int otherGeneration)
{
    // If otherGeneration < -1 then take everything
    if (otherGeneration < -1)
    {
        int minOGen = pOtherHandler->getLowestGenerationNumber();
        int maxOGen = pOtherHandler->getHighestGenerationNumber();
        // Since generations are not necessarily continuous and same in all data variables we try with every generation between min and max
        // We cant take them all at once, that could change the internal ordering
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
        ++mCurrentGenerationNumber;
        bool tookOwnershipOfSomeData=false;

        // Take the generation cache map
        if (pOtherHandler->mGenerationCacheMap.contains(otherGeneration))
        {
            mGenerationCacheMap.insert(mCurrentGenerationNumber, pOtherHandler->mGenerationCacheMap.value(otherGeneration));
            pOtherHandler->mGenerationCacheMap.remove(otherGeneration);

            // Only take ownership of dir if we do not already own it
            QDir cacheDir = mGenerationCacheMap.value(mCurrentGenerationNumber)->getCacheFileInfo().absoluteDir();
            if (!mCacheDirs.contains(cacheDir))
            {
                mCacheDirs.append(cacheDir);
                //! @todo this will leave the dir in the other object as well but I don't think it will remove the files in the dir when it dies, may need to deal with that.
            }

            tookOwnershipOfSomeData = true;
        }

        // Take the data, and only increment this->mGeneration if data was taken
        LogDataGeneration *pOtherGen = pOtherHandler->mGenerationMap.value(otherGeneration, 0);
        LogDataGeneration::VariableMapT &otherMap = pOtherGen->mVariables;
        for (auto other_it=otherMap.begin(); other_it!=otherMap.end(); ++other_it)
        {
            QString keyName = other_it.key();
            insertVariable(other_it.value(), keyName);

            tookOwnershipOfSomeData=true;
        }
        // Remove from other
        pOtherHandler->removeGeneration(otherGeneration, true);

        if (!tookOwnershipOfSomeData)
        {
            // Revert generation if no data was taken
            --mCurrentGenerationNumber;
        }
    }
    //! @todo plotwindows, imported data
}

void LogDataHandler2::registerAlias(const QString &rFullName, const QString &rAlias, int gen)
{
    if (gen < 0)
    {
        gen = mCurrentGenerationNumber;
    }
    auto pGen = mGenerationMap.value(gen, 0);
    if (pGen)
    {
        if(pGen->registerAlias(rFullName, rAlias))
        {
            emit aliasChanged();
        }
    }
}


void LogDataHandler2::unregisterAlias(const QString &rAlias, int gen)
{
    if (gen < 0)
    {
        gen = mCurrentGenerationNumber;
    }
    auto pGen = mGenerationMap.value(gen, 0);
    if (pGen)
    {
        if(pGen->unregisterAlias(rAlias))
        {
            emit aliasChanged();
        }
    }
}

bool LogDataHandler2::registerQuantity(const QString &rFullName, const QString &rQuantity, int gen)
{
    if (gen < 0)
    {
        gen = mCurrentGenerationNumber;
    }
    auto pGen = mGenerationMap.value(gen, 0);
    if (pGen)
    {
        return pGen->registerQuantity(rFullName, rQuantity);
    }
    return false;
}

//! @brief This slot should be signaled when a variable that might be registered as imported is removed
void LogDataHandler2::forgetImportedVariable(SharedVectorVariableT pData)
{
//    if (pData)
//    {
//        // First find the correct file sub map
//        ImportedLogDataMapT::iterator fit;
//        fit = mImportedLogDataMap.find(pData->getImportedFileName());
//        if (fit != mImportedLogDataMap.end())
//        {
//            // Now remove the sub map entry
//            // Only remove if same variable (same key,value) (compare pointers)
//            fit.value().remove(pData->getFullVariableName(),pData);
//            // Now erase the file level map if it has become empty
//            if (fit.value().isEmpty())
//            {
//                mImportedLogDataMap.erase(fit);
//            }
//        }
//        //! @todo this code assumes that imported data can not have aliases (full name can be a short one like an alias but is technically not an alias)
    //    }
}

SharedVectorVariableT LogDataHandler2::insertCustomVectorVariable(const QVector<double> &rVector, SharedVariableDescriptionT pVarDesc)
{
    SharedVectorVariableT pVec = SharedVectorVariableT(new VectorVariable(rVector, mCurrentGenerationNumber, pVarDesc,
                                                                          getGenerationMultiCache(mCurrentGenerationNumber)));
    insertVariable(pVec);
    return pVec;
}

SharedVectorVariableT LogDataHandler2::insertCustomVectorVariable(const QVector<double> &rVector, SharedVariableDescriptionT pVarDesc, const QString &rImportFileName)
{
    SharedVectorVariableT pVec = SharedVectorVariableT(new ImportedVectorVariable(rVector, mCurrentGenerationNumber, pVarDesc,
                                                                                  rImportFileName, getGenerationMultiCache(mCurrentGenerationNumber)));
    insertVariable(pVec);
    return pVec;
}

SharedVectorVariableT LogDataHandler2::insertTimeVectorVariable(const QVector<double> &rTimeVector, SharedSystemHierarchyT pSysHierarchy)
{
    SharedVariableDescriptionT desc = createTimeVariableDescription();
    desc->mpSystemHierarchy = pSysHierarchy;
    return insertCustomVectorVariable(rTimeVector, desc);
}

SharedVectorVariableT LogDataHandler2::insertTimeVectorVariable(const QVector<double> &rTimeVector, const QString &rImportFileName)
{
    return insertCustomVectorVariable(rTimeVector, createTimeVariableDescription(), rImportFileName);
}

SharedVectorVariableT LogDataHandler2::insertFrequencyVectorVariable(const QVector<double> &rFrequencyVector, SharedSystemHierarchyT pSysHierarchy)
{
    SharedVariableDescriptionT desc = createFrequencyVariableDescription();
    desc->mpSystemHierarchy = pSysHierarchy;
    return insertCustomVectorVariable(rFrequencyVector, desc);
}

SharedVectorVariableT LogDataHandler2::insertFrequencyVectorVariable(const QVector<double> &rFrequencyVector, const QString &rImportFileName)
{
    return insertCustomVectorVariable(rFrequencyVector, createFrequencyVariableDescription(), rImportFileName);
}

SharedVectorVariableT LogDataHandler2::insertTimeDomainVariable(SharedVectorVariableT pTimeVector, const QVector<double> &rDataVector, SharedVariableDescriptionT pVarDesc)
{
    SharedVectorVariableT pNewData = SharedVectorVariableT(new TimeDomainVariable(pTimeVector, rDataVector, mCurrentGenerationNumber, pVarDesc,
                                                                                  getGenerationMultiCache(mCurrentGenerationNumber)));
    insertVariable(pNewData);
    return pNewData;
}

SharedVectorVariableT LogDataHandler2::insertTimeDomainVariable(SharedVectorVariableT pTimeVector, const QVector<double> &rDataVector, SharedVariableDescriptionT pVarDesc, const QString &rImportFileName)
{
    SharedVectorVariableT pNewData = SharedVectorVariableT(new ImportedTimeDomainVariable(pTimeVector, rDataVector, mCurrentGenerationNumber, pVarDesc,
                                                                                          rImportFileName, getGenerationMultiCache(mCurrentGenerationNumber)));
    insertVariable(pNewData);
    return pNewData;
}

SharedVectorVariableT LogDataHandler2::insertFrequencyDomainVariable(SharedVectorVariableT pFrequencyVector, const QVector<double> &rDataVector, SharedVariableDescriptionT pVarDesc)
{
    SharedVectorVariableT pNewData = SharedVectorVariableT(new FrequencyDomainVariable(pFrequencyVector, rDataVector, mCurrentGenerationNumber, pVarDesc,
                                                                                       getGenerationMultiCache(mCurrentGenerationNumber)));
    insertVariable(pNewData);
    return pNewData;
}

SharedVectorVariableT LogDataHandler2::insertFrequencyDomainVariable(SharedVectorVariableT pFrequencyVector, const QVector<double> &rDataVector, SharedVariableDescriptionT pVarDesc, const QString &rImportFileName)
{
    SharedVectorVariableT pNewData = SharedVectorVariableT(new ImportedFrequencyDomainVariable(pFrequencyVector, rDataVector, mCurrentGenerationNumber, pVarDesc,
                                                                                               rImportFileName, getGenerationMultiCache(mCurrentGenerationNumber)));
    insertVariable(pNewData);
    return pNewData;
}

//! @brief Inserts a variable into the map creating a generation if needed
//! @param[in] pVariable The variable to insert
//! @param[in] keyName An alternative keyName to use (used recursively to set an alias, do not abuse this argument)
//! @param[in] gen An alternative generation number (-1 = current))
//! @returns A SharedVectorVariableT object representing the inserted variable
SharedVectorVariableT LogDataHandler2::insertVariable(SharedVectorVariableT pVariable, QString keyName, int gen)
{
    bool isAlias=false;

    // If keyName is empty then use fullName, (this is not an alias)
    if (keyName.isEmpty())
    {
        keyName = pVariable->getFullVariableName();
    }
    // If keyName is not empty, it might be an alias if it is different from the fullName
    else if ( keyName != pVariable->getFullVariableName() )
    {
        isAlias = true;
    }


    // If gen -1 then use current generation
    if (gen<0)
    {
        gen = mCurrentGenerationNumber;
    }

    //! @todo is it here we need to create generation ?
    LogDataGeneration *pGen = mGenerationMap.value(gen, 0);
    // If generation does not exist, the we need to create it
    if (!pGen)
    {
        pGen = new LogDataGeneration(pVariable->getImportedFileName());
        mGenerationMap.insert(gen, pGen );
    }

    // Make the variable remember that this LogDataHandler is its parent (creator)
    pVariable->mpParentLogDataHandler = this;

    // Now add variable
    pGen->addVariable(keyName, pVariable, isAlias);
    pVariable->mGeneration = gen;

    // Also insert alias if it exist, but only if it is different from keyName (else we will have an endless loop in here)
    if ( pVariable->hasAliasName() && (pVariable->getAliasName() != keyName) )
    {
        insertVariable(pVariable, pVariable->getAliasName(), gen);
    }

    // Make data description source know its imported
    if (pVariable->isImported())
    {
        pVariable->mpVariableDescription->mVariableSourceType = ImportedVariableType;
    }

    return pVariable;
}

bool LogDataHandler2::hasVariable(const QString &rFullName, const int generation)
{
    return (getVectorVariable(rFullName, generation) != 0);
}


