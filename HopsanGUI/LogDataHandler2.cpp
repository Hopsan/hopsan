/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

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
#include "ComponentSystem.h"

#ifdef USEHDF5
#include "hopsanhdf5exporter.h"
#endif


//! @brief Constructor for plot data object
//! @param pParent Pointer to parent container object
LogDataHandler2::LogDataHandler2(ModelWidget *pParentModel) : QObject(pParentModel)
{
    setParentModel(pParentModel);

    // Create the temporary directory that will contain cache data
    int ctr=0;
    QDir desiredLogCacheDir;
    do
    {
        desiredLogCacheDir = QDir(gpDesktopHandler->getLogDataPath() + QString("handler%1").arg(ctr));
        ++ctr;
    }while(desiredLogCacheDir.exists());
    desiredLogCacheDir.mkpath(desiredLogCacheDir.absolutePath());
    mCacheDirs.append(desiredLogCacheDir);
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
        disconnect(0, 0, this, SLOT(registerQuantity(QString,QString)));
    }
    mpParentModel = pParentModel;
    connect(mpParentModel, SIGNAL(aliasChanged(QString,QString)), this, SLOT(registerAlias(QString,QString)));
    connect(mpParentModel, SIGNAL(quantityChanged(QString,QString)), this, SLOT(registerQuantity(QString,QString)));
}

ModelWidget *LogDataHandler2::getParentModel()
{
    return mpParentModel;
}

void LogDataHandler2::createEmptyGeneration()
{
    ++mCurrentGenerationNumber;
}


void LogDataHandler2::exportToPlo(const QString &rFilePath, QList<SharedVectorVariableT> variables, int version) const
{
    if ( (version < 1) || (version > 3) )
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
    for(SharedVectorVariableT var : variables) {
        const int g = var->getGeneration();
        if (gens.isEmpty() || (gens.last() != g)) {
            gens.append(g);
        }

        const int l = var->getDataSize();
        if (lengths.isEmpty() || (lengths.last() != l)) {
            lengths.append(l);
            minLength = qMin(minLength, l);
        }

        SharedVectorVariableT pToF = var->getSharedTimeOrFrequencyVector();
        if (pToF) {
            if (pToF->getDataName() == TIMEVARIABLENAME) {
                if (timeVectors.isEmpty() || (timeVectors.last() != pToF)) {
                    timeVectors.append(pToF);
                }
            }
            else if (pToF->getDataName() == FREQUENCYVARIABLENAME) {
                if (freqVectors.isEmpty() || (freqVectors.last() != pToF)) {
                    freqVectors.append(pToF);
                }
            }
        }
    }

    if ( (timeVectors.size() > 0) && (freqVectors.size() > 0) ) {
        gpMessageHandler->addErrorMessage(QString("In export PLO: You are mixing time and frequency variables, this is not supported!"));
    }
    if (gens.size() > 1) {
        gpMessageHandler->addWarningMessage(QString("In export PLO: Data had different generations, time vector may not be correct for all exported data!"));
    }
    if (lengths.size() > 1) {
        gpMessageHandler->addWarningMessage(QString("In export PLO: Data had different lengths, truncating to shortest data!"));
    }

    // We insert time or frequency last when we know what generation the data had, (to avoid taking last time generation that may belong to imported data)
    if (timeVectors.size() > 0) {
        variables.prepend(timeVectors.first());
    }
    else if (freqVectors.size() > 0) {
        variables.prepend(freqVectors.first());
    }

    // Now begin to write to pro file
    int nDataRows = minLength;
    int nDataCols = variables.size();

    // Write initial Header data
    if (version == 1 || version == 2)
    {
        fileStream << "    'VERSION'\n";
        fileStream << "    " << version << "\n";
        fileStream << "    '"<<ploFileInfo.baseName()<<".PLO'\n";
        // PLO Version 1 and 2 does not count the time or frequency vector in the number of data columns
        if ( ((version == 1) || (version == 2)) && ((timeVectors.size() > 0) || (freqVectors.size() > 0)) )
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
    else if (version == 3)
    {
        fileStream << "    'VERSION'\n";
        fileStream << "    3\n";
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
    // Note! Plotscale was removed in HopsanNG 0.7, for backward compatibility with PLO v1, write ones
    // Plot v2 prints Quantity names if known, else scale one
    //! @todo In format 3 we should likely write Quantity:Unit to clarify the values unit
    for(int i=0; i<variables.size(); ++i)
    {
        const QString &q = variables[i]->getDataQuantity();
        if (version==1 || q.isEmpty())
        {
            fileStream << "  " << 1.0;
        }
        else
        {
            fileStream << "  " << q;
        }
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
    if (version < 3)
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

void LogDataHandler2::exportGenerationToCSV(const QString &rFilePath, int gen) const
{
    if (gen == -1)
    {
        gen = mCurrentGenerationNumber;
    }

    QList<SharedVectorVariableT> vars = getAllNonAliasVariablesAtGeneration(gen);
    // Now export all of them
    exportToCSV(rFilePath, vars);
}


void LogDataHandler2::exportToHDF5(const QString &rFilePath, const QList<SharedVectorVariableT> &rVariables) const
{
#ifdef USEHDF5
    QList<hopsan::HVector<double> > dataVectors;
    HopsanHDF5Exporter *pExporter = new HopsanHDF5Exporter(hopsan::HString(rFilePath.toStdString().c_str()), hopsan::HString(mpParentModel->getTopLevelSystemContainer()->getModelFileInfo().fileName().toStdString().c_str()), hopsan::HString(QString("HopsanGUI %1").arg(HOPSANGUIVERSION).toStdString().c_str()));
    for (const SharedVectorVariableT &rVar : rVariables) {
        QStringList systemHierarchy;
        QString componentName,portName,variableName;
        splitFullVariableName(rVar->getFullVariableName(),systemHierarchy,componentName,portName,variableName);
        hopsan::HVector<double> dataVector(rVar->getDataVectorCopy().toStdVector());

        hopsan::HString systemHierarchyStr = systemHierarchy.join(".").toStdString().c_str();
        pExporter->addVariable(systemHierarchyStr,componentName.toStdString().c_str(),portName.toStdString().c_str(),variableName.toStdString().c_str(),rVar->getAliasName().toStdString().c_str(),rVar->getDataUnit().toStdString().c_str(),rVar->getDataQuantity().toStdString().c_str(),dataVector);
    }

    bool success = pExporter->writeToFile();

    if (!success) {
        gpMessageHandler->addErrorMessage(pExporter->getLastError().c_str());
    }
#else
    gpMessageHandler->addErrorMessage("HDF5 is not Supported in this build");
#endif
}

void LogDataHandler2::exportGenerationToHDF5(const QString &rFilePath, int gen) const
{
    if (gen == -1)
    {
        gen = mCurrentGenerationNumber;
    }

    //! @todo use a enum for choosing export format
    QList<SharedVectorVariableT> vars = getAllNonAliasVariablesAtGeneration(gen);
    // Now export all of them
    exportToHDF5(rFilePath, vars);
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
        // Assign generation cache file
        pNewData->mpCachedDataVector->switchCacheFile(getGenerationMultiCache(mCurrentGenerationNumber));
        pNewData->mpCachedDataVector->setCached(gpConfig->getCacheLogData());
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
            else if(lineNum == 5)
            {
                if ( ((ploVersion == 1) || (ploVersion == 2)) && (line.startsWith("'" TIMEVARIABLENAME) || line.startsWith("'" FREQUENCYVARIABLENAME)) )
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
            // Right now we ignore numeric plotscale, as we removed plotscale from data variables, we look for quantities instead

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
        QString firstLine = ts.readLine();
        if(firstLine.isEmpty()) {
            gpMessageHandler->addErrorMessage("CSV file is empty: "+importFilePath);
            return;
        }
        QStringList firstRow = firstLine.split(',');

        QStringList firstColumn;
        firstColumn.push_back(firstRow.first());
        while(!ts.atEnd()) {
            QStringList row = ts.readLine().split(',');
            if(row.isEmpty()) {
                firstColumn.push_back(QString());
            }
            else {
                firstColumn.push_back(row.first());
            }
        }
        file.close();
        if (!firstRow.isEmpty())
        {
            bool columnWise = true;
            bool hopsanCSV = false;
            int linesToSkip = 0;
            if(!isNumber(firstColumn.first()) && isNumber(firstColumn.last())) {
                columnWise = true;
                hopsanCSV = false;
                while(!firstColumn.empty() && !isNumber(firstColumn.first())) {
                    linesToSkip++;
                    firstColumn.pop_front();
                }
            }
            else if(!isNumber(firstRow.first()) && isNumber(firstRow.last())) {
                columnWise = false;
                while(!firstRow.empty() && !isNumber(firstRow.first())) {
                    linesToSkip++;
                    firstRow.pop_front();
                }
                if(linesToSkip == 3) {
                    hopsanCSV = true;
                }
            }

            if (columnWise) {
                importFromPlainColumnCsv(importFilePath, ',', linesToSkip);
            }
            else if (!columnWise && !hopsanCSV)
            {
                importFromPlainRowCsv(importFilePath, ',', linesToSkip);
            }
            else {
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


void LogDataHandler2::importFromPlainColumnCsv(QString importFilePath, const QChar separator, const int rowsToSkip, const int timecolumn)
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

    QTextStream ts(&file);
    QStringList names;
    file.open(QFile::ReadOnly);
    if(rowsToSkip > 0) {
        names = ts.readLine().split(separator);
    }
    for(auto& name : names) {
        name.remove("\"");
    }
    file.close();

    CoreCSVParserAccess csvparser(importFilePath,separator,rowsToSkip);

    if(!csvparser.isOk())
    {
        gpMessageHandler->addErrorMessage("CSV file could not be parsed.");
        return;
    }

    int cols = csvparser.getNumberOfColumns();
    QList<QVector<double> > data;
    for(int c=0; c<cols; ++c)
    {
        QVector<double> vec;
        csvparser.getColumn(c,vec);
        data.append(vec);
    }

    if (!data.isEmpty() && timecolumn<data.size())
    {
        ++mCurrentGenerationNumber;
        SharedVectorVariableT pTimeVec(0);

        pTimeVec = insertTimeVectorVariable(data[timecolumn], fileInfo.absoluteFilePath());

        SharedVectorVariableT pNewData;
        for (int i=1; i<data.size(); ++i)
        {
            SharedVariableDescriptionT pVarDesc = SharedVariableDescriptionT(new VariableDescription);
            if (names.size() > i) {
                pVarDesc->mDataName = names[i];
            }
            else {
                pVarDesc->mDataName = "CSV"+QString::number(i);
            }
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

void LogDataHandler2::importFromPlainRowCsv(QString importFilePath, const QChar separator, const int columnsToSkip, const int timeRow)
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
            QStringList lineFields = contentStream.readLine().split(separator);
            if (lineFields.size() > columnsToSkip)
            {
                // Read metadata
                if(columnsToSkip > 0) {
                    allNames.append(lineFields[0]);
                }

                allDatas.append(QVector<double>());
                QVector<double> &data = allDatas.last();
                if (nDataValueElements > -1)
                {
                    data.reserve(nDataValueElements);
                }
                for (int i=columnsToSkip; i<lineFields.size(); ++i)
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
            if(timeRow > -1) {
                pTimeVec = insertTimeVectorVariable(allDatas[timeRow], fileInfo.absoluteFilePath());
            }

            SharedVectorVariableT pNewData;
            for (int i=0; i<allNames.size(); ++i)
            {
                // We already inserted time
                if (i == timeRow)
                {
                    continue;
                }

                SharedVariableDescriptionT pVarDesc = SharedVariableDescriptionT(new VariableDescription);
                if(allNames.size() > i) {
                    pVarDesc->mDataName = allNames[i];
                }

                if(!pTimeVec.isNull()) {
                    pNewData = insertTimeDomainVariable(pTimeVec, allDatas[i], pVarDesc, fileInfo.absoluteFilePath());
                }
                else {
                    pNewData = insertCustomVectorVariable(allDatas[i], pVarDesc, fileInfo.absoluteFilePath());
                }
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
        git.value()->clear(true);
        git.value()->deleteLater();
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
    if (!(mpParentModel && mpParentModel->getTopLevelSystemContainer()))
    {
        return;
    }
    SystemObject *pTopLevelSystem = mpParentModel->getTopLevelSystemContainer();
    if (pTopLevelSystem->getCoreSystemAccessPtr()->getNumLogSamples() == 0)
    {
        //Don't collect plot data if logging is disabled (to avoid empty generations)
        return;
    }

    // Increment generation number if we are not overwriting last generation
    if(!overWriteLastGeneration)
    {
        ++mCurrentGenerationNumber;
    }

    auto pGMC = this->getGenerationMultiCache(mCurrentGenerationNumber);
    //! @todo why not run multiappend when overwriting generation ? Because then we are not appending, need some common open mode
    if(!overWriteLastGeneration)
    {
        TicToc tt(TicToc::TextOutput::DebugMessage);
        pGMC->beginMultiAppend();
        tt.toc(QString("Opening cache file: %1").arg(pGMC->getCacheFileInfo().absoluteFilePath()));
    }

    // Block signaling while collecting all data to avoid signal spamming and extreme slowdown
    this->blockSignals(true);

    TicToc tictoc(TicToc::TextOutput::DebugMessage);
    auto sizeBefore = pGMC->getCacheSize();
    QMap<std::vector<double>*, SharedVectorVariableT> generationTimeVectors;
    bool foundData = collectLogDataFromSystem(pTopLevelSystem, QStringList(), generationTimeVectors);
    auto sizeAfter = pGMC->getCacheSize();
    const double cachedSize_mb = (sizeAfter-sizeBefore)*1.0e-6;
    const double collect_ms = tictoc.toc("Collecting all log data");
    gpMessageHandler->addDebugMessage(QString("Wrote to disk: %1 MB data at %2 MB/s").arg(cachedSize_mb).arg( cachedSize_mb*1.0e3/collect_ms));

    this->blockSignals(false);

    if(!overWriteLastGeneration)
    {
        TicToc tt(TicToc::TextOutput::DebugMessage);
        pGMC->endMultiAppend();
        tt.toc("Closing cache file");
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
}

bool LogDataHandler2::collectLogDataFromSystem(SystemObject *pCurrentSystem, const QStringList &rSystemHieararchy, QMap<std::vector<double>*, SharedVectorVariableT> &rGenTimeVectors)
{
    SharedSystemHierarchyT sharedSystemHierarchy(new QStringList(rSystemHieararchy));
    bool foundData=false, foundDataInSubsys=false;

    // Store the systems own time vector
    auto pCoreSysTimeVector = pCurrentSystem->getCoreSystemAccessPtr()->getLogTimeData();
    if (pCoreSysTimeVector && !pCoreSysTimeVector->empty())
    {
        // Check so that we have not already stored this time vector in this generation
        if (!rGenTimeVectors.contains(pCoreSysTimeVector))
        {
            //! @todo here we need to copy (convert) from std vector to qvector, don know if that slows down (probably not much)
            auto time_vec = QVector<double>::fromStdVector(*pCoreSysTimeVector);
            time_vec.resize(pCurrentSystem->getCoreSystemAccessPtr()->getCoreSystemPtr()->getNumActuallyLoggedSamples());
            auto pSysTimeVector = insertTimeVectorVariable(time_vec, sharedSystemHierarchy);
            rGenTimeVectors.insert(pCoreSysTimeVector, pSysTimeVector);
        }
    }

    // Iterate components
    QList<ModelObject*> currentLevelModelObjects = pCurrentSystem->getModelObjects();
    for(ModelObject* pModelObject : currentLevelModelObjects) {
        // Ignore "system port model objects" they are not actually a component and will be listed anyway
        if (pModelObject->getTypeName() == HOPSANGUISYSTEMPORTTYPENAME)
        {
            continue;
        }

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
                        pVarDesc->mModelPath = pModelObject->getParentSystemObject()->getModelFilePath();
                        pVarDesc->mpSystemHierarchy = sharedSystemHierarchy;
                        pVarDesc->mComponentName = pModelObject->getName();
                        pVarDesc->mPortName = pPort->getName();
                        pVarDesc->mDataName = varDesc.mName;
                        pVarDesc->mDataUnit = varDesc.mUnit;
                        pVarDesc->mDataQuantity = varDesc.mQuantity;
                        pVarDesc->mDataDescription = varDesc.mDescription;
                        pVarDesc->mAliasName  = varDesc.mAlias;
                        pVarDesc->mVariableSourceType = ModelVariableType;
                        pVarDesc->mModelInvertPlot = pModelObject->getInvertPlotVariable(pPort->getName()+"#"+varDesc.mName);
                        pVarDesc->mLocalInvertInvertPlot = false;
                        pVarDesc->mCustomLabel = pModelObject->getVariablePlotLabel(pPort->getName()+"#"+varDesc.mName);

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
                            auto time_vec = QVector<double>::fromStdVector(*pCoreVarTimeVector);
                            time_vec.resize(pCurrentSystem->getCoreSystemAccessPtr()->getCoreSystemPtr()->getNumActuallyLoggedSamples());
                            auto pVarTimeVec = insertTimeVectorVariable(time_vec, SharedSystemHierarchyT());
                            pNewData = insertTimeDomainVariable(pVarTimeVec, dataVec, pVarDesc);
                        }
                    }
                }
            }
        }

        // If this is a subsystem, then go into it
        if (pModelObject->type() == SystemObjectType )
        {
            QStringList subsysHierarchy = rSystemHieararchy;
            subsysHierarchy << pModelObject->getName();
            bool foundDataInThisSubsys = collectLogDataFromSystem(qobject_cast<SystemObject*>(pModelObject), subsysHierarchy, rGenTimeVectors);
            foundDataInSubsys = foundDataInSubsys || foundDataInThisSubsys;
        }
    }
    return (foundData || foundDataInSubsys);
}

void LogDataHandler2::collectLogDataFromRemoteModel(QVector<RemoteResultVariable> &rResultVariables, bool overWriteLastGeneration)
{
    TicToc tictoc;
    if(!overWriteLastGeneration)
    {
        ++mCurrentGenerationNumber;
    }

    if(rResultVariables.size() == 0)
    {
        return;         //Don't collect plot data if logging is disabled (to avoid empty generations)
    }


    bool foundData = false;

    //! @todo why not run multiappend when overwriting generation ? Because then we are not appending, need some common open mode
    if(!overWriteLastGeneration)
    {
        this->getGenerationMultiCache(mCurrentGenerationNumber)->beginMultiAppend();
    }

    QList<SharedVariableDescriptionT> varDescs;
    varDescs.reserve(rResultVariables.size());
    QMap<size_t, SharedSystemHierarchyT> systemTimeVarIdAndSysname;
    QMap<QString, SharedVectorVariableT> systemHierarchy2TimeVariable;

    // Iterate over variables
    for(int v=0; v<rResultVariables.size(); ++v)
    {
        foundData=true;
        QStringList sh;
        QString comp, port, data;
        splitFullVariableName(rResultVariables[v].fullname,sh,comp,port,data);

        SharedVariableDescriptionT pVarDesc = SharedVariableDescriptionT(new VariableDescription);
        pVarDesc->mModelPath = "RemoteModel";
        pVarDesc->mComponentName = comp;
        pVarDesc->mpSystemHierarchy = SharedSystemHierarchyT(new QStringList(sh)); //!< @todo would be nice if we could avoid duplicates here
        pVarDesc->mPortName = port;
        pVarDesc->mDataName = data;
        pVarDesc->mDataUnit = rResultVariables[v].unit;
        pVarDesc->mDataQuantity = rResultVariables[v].quantity;
        pVarDesc->mDataDescription = "";
        pVarDesc->mAliasName  = rResultVariables[v].alias;
        pVarDesc->mVariableSourceType = ModelVariableType;

        varDescs.push_back(pVarDesc);

        if (data == TIMEVARIABLENAME)
        {
            systemTimeVarIdAndSysname.insert(v,pVarDesc->mpSystemHierarchy);
        }
    }

    // First process time variables
    for (auto it=systemTimeVarIdAndSysname.begin(); it!=systemTimeVarIdAndSysname.end(); ++it)
    {
        size_t v = it.key();
        QVector<double> timeData = QVector<double>::fromStdVector(rResultVariables[v].data);
        auto pNewData = insertTimeVectorVariable(timeData, it.value());
        systemHierarchy2TimeVariable.insert(it.value()->join("$"), pNewData);
    }

    auto timeIds = systemTimeVarIdAndSysname.keys();
    // Block signaling while collecting all data to avoid signal spamming and extreme slowdown
    this->blockSignals(true);
    for(int v=0; v<rResultVariables.size(); ++v)
    {
        // skip variables that were time variables
        if (timeIds.contains(v))
        {
            continue;
        }

        QVector<double> newData = QVector<double>::fromStdVector(rResultVariables[v].data);

        // Lookup time variable
        SharedVectorVariableT pTime = systemHierarchy2TimeVariable.value(varDescs[v]->mpSystemHierarchy->join("$"));
        if (pTime)
        {
            varDescs[v]->mpSystemHierarchy = pTime->mpVariableDescription->mpSystemHierarchy; // Replace systems hierarchy with shared version (to save some memory)
        }

        // Insert time domain variable
        insertTimeDomainVariable(pTime, newData, varDescs[v]);
    }
    this->blockSignals(false);

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


//! @brief Returns multiple logdata variables based on regular expression search. Excluding temp variables but including aliases
//! @param [in] rNameExp The regular expression for the names to match
//! @param [in] generation The desired generation of the variable
QList<SharedVectorVariableT> LogDataHandler2::getMatchingVariablesAtGeneration(const QRegExp &rNameExp, int generation, const VariableNameTypeT nametype) const
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
        return pGen->getMatchingVariables(rNameExp, nametype);
    }
    return QList<SharedVectorVariableT>();
}

QList<SharedVectorVariableT> LogDataHandler2::getMatchingVariablesFromAllGenerations(const QRegExp &rNameExp, const VariableNameTypeT nametype) const
{
    QList<SharedVectorVariableT> allData;
    for (auto pGen : mGenerationMap.values())
    {
        QList<SharedVectorVariableT> data = pGen->getMatchingVariables(rNameExp, nametype);
        allData.append(data);
    }
    return allData;
}


//! @brief Returns the time vector for specified generation
//! @param[in] generation Generation
const SharedVectorVariableT LogDataHandler2::getTimeVectorVariable(int generation) const
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

SharedVectorVariableT LogDataHandler2::elementWisePower(SharedVectorVariableT a, const double x)
{
    SharedVectorVariableT pTempVar = createOrphanVariable(a->getSmartName()+"*"+QString::number(x), a->getVariableType());
    pTempVar->assignFrom(a);
    pTempVar->powerData(x);
    return pTempVar;
}

void LogDataHandler2::setGenerationTimePlotOffset(int generation, double offset)
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
        // Now set the offset
        return pGen->setTimeOffset(offset);
    }
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

//! @brief Remove a variable or unregister an alias variable
//! @param[in] rVarName The name of the variable to remove (or alsi to unregister)
//! @param[in] generation The generation to remove from (or -2 == All, -1 == Current)
//! @returns true if something was removed
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
        // Need to work with a copy of values in case last variable is removed, then generation will be removed as well ( and iterator in map will become invalid )
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


PlotWindow *LogDataHandler2::plotVariable(const QString plotName, const QString fullVarName, const int gen, const int axis, PlotCurveStyle style)
{
    SharedVectorVariableT data = getVectorVariable(fullVarName, gen);
    if(data)
    {
        return gpPlotHandler->plotDataToWindow(plotName, data, axis, style);
    }
    return 0;
}

PlotWindow *LogDataHandler2::plotVariable(const QString plotName, const QString &rFullNameX, const QString &rFullNameY, const int gen, const int axis, PlotCurveStyle style)
{
    SharedVectorVariableT xdata = getVectorVariable(rFullNameX, gen);
    SharedVectorVariableT ydata = getVectorVariable(rFullNameY, gen);
    if (xdata && ydata)
    {
        return gpPlotHandler->plotDataToWindow(plotName, xdata, ydata, axis, style);
    }
    return 0;
}

PlotWindow *LogDataHandler2::plotVariable(PlotWindow *pPlotWindow, const QString fullVarName, const int gen, const int axis, PlotCurveStyle style)
{
    SharedVectorVariableT data = getVectorVariable(fullVarName, gen);
    if(data)
    {
        return gpPlotHandler->plotDataToWindow(pPlotWindow, data, axis, true, style);
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
        auto avars = git.value()->getAllAliasVariables();
        for (auto &var : avars)
        {
            data.insert(var->getAliasName(), var);
        }
        auto vars = git.value()->getAllNonAliasVariables();
        for (auto &var : vars)
        {
            data.insert(var->getFullVariableName(), var);
        }
    }
    return data.values();
}

QList<SharedVectorVariableT> LogDataHandler2::getMatchingVariablesAtRespectiveNewestGeneration(const QRegExp &rNameExp, const VariableNameTypeT nametype) const
{
    QMap<QString, SharedVectorVariableT> data;
    // Iterated generations and collect variables in a map
    // For newer generations replace old data values
    for (auto git = mGenerationMap.begin(); git != mGenerationMap.end(); ++git)
    {
        auto vars = git.value()->getMatchingVariables(rNameExp, nametype);
        if (nametype == Alias)
        {
            for (auto &var : vars)
            {
                data.insert(var->getAliasName(), var);
            }
        }
        else if (nametype == FullName)
        {
            for (auto &var : vars)
            {
                data.insert(var->getFullVariableName(), var);
            }
        }
        else
        {
            for (auto &var : vars)
            {
                // This might cause duplicates of alias name
                data.insert(var->getSmartName(), var);
            }
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

int LogDataHandler2::getHighestModelGeneration() const
{
    if (!mGenerationMap.isEmpty())
    {
        QMapIterator<int, LogDataGeneration*> it(mGenerationMap);
        it.toBack();
        while (it.hasPrevious())
        {
            it.previous();
            if (!it.value()->isImported())
            {
                return it.key();
            }
        }
    }
    // Note! -1 May be treated as latest or current
    return -1;
}

const LogDataGeneration *LogDataHandler2::getCurrentGeneration() const
{
    return mGenerationMap.value(mCurrentGenerationNumber, 0);
}

const LogDataGeneration *LogDataHandler2::getGeneration(const int gen) const
{
    return mGenerationMap.value(gen, 0);
}

bool LogDataHandler2::hasGeneration(const int gen) const
{
    return mGenerationMap.contains(gen);
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
    auto pGen = mGenerationMap.value(gen, nullptr);
    if (pGen)
    {
        bool rc = pGen->registerQuantity(rFullName, rQuantity);
        if (rc)
        {
            emit quantityChanged();
        }
        return rc;
    }
    return false;
}

bool LogDataHandler2::registerQuantity(const QString &rFullName, const QString &rQuantity)
{
    return registerQuantity(rFullName, rQuantity, -1);
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

    emit dataAdded();

    return pVariable;
}

bool LogDataHandler2::hasVariable(const QString &rFullName, const int generation)
{
    return (getVectorVariable(rFullName, generation) != 0);
}


