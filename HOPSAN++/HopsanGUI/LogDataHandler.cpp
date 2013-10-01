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
#include "MainWindow.h"
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
    mpParentContainerObject = pParent;
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
    mpParentContainerObject = pParent;
}


void LogDataHandler::exportToPlo(QString filePath, QStringList variables)
{
    if(filePath.isEmpty()) return;    //Don't save anything if user presses cancel

    QFile file;
    file.setFileName(filePath);   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("Failed to open file for writing: " + filePath);
        return;
    }

    // Create a QTextStream object to stream the content of file
    QTextStream fileStream(&file);
    QString dateTimeString = QDateTime::currentDateTime().toString();
    QFileInfo ploFileInfo(filePath);
    QString modelPath = mpParentContainerObject->getModelFileInfo().filePath();
    QFileInfo modelFileInfo(modelPath);

    QStringList plotScaleStringList;
    QStringList startvaluesList;

    QList<SharedLogVariableDataPtrT> dataPtrs;
    dataPtrs.append(getPlotData(TIMEVARIABLENAME, -1)); //!< @todo this assumes that time exists and is named Time
    for(int v=0; v<variables.size(); ++v)
    {
        dataPtrs.append(getPlotData(variables[v],-1));
    }

    int nDataRows = dataPtrs[0]->getDataSize();
    int nDataCols = dataPtrs.size();

    // Write initial Header data
    fileStream << "    'VERSION'\n";
    fileStream << "    2\n";
    fileStream << "    '"<<ploFileInfo.baseName()<<".PLO' " << "'GUIVERSION " << QString(HOPSANGUIVERSION) << "' 'DATE "<<dateTimeString<<"'\n";
    fileStream << "    " << nDataCols  <<"    "<< nDataRows <<"\n";
    fileStream << "    'Time'";
    for(int i=1; i<dataPtrs.size(); ++i)
    {
        //! @todo fix this formating so that it looks nice in plo file (need to know length of previous
        fileStream << ",    '" << dataPtrs[i]->getSmartName()<<"'";
    }
    fileStream << "\n";

    // Write plotScalings line
    for(int i=0; i<dataPtrs.size(); ++i)
    {
        QString str;
        str.setNum(dataPtrs[i]->getPlotScale(),'E',6);
        plotScaleStringList.append(str); //Remember till later
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

    // Write data lines
    QString err;
    for(int row=0; row<nDataRows; ++row)
    {
        QString str;
        for(int col=0; col<dataPtrs.size(); ++col)
        {
            str.setNum(dataPtrs[col]->peekData(row,err),'E',6);
//            if(row == 0)
//            {
//                startvaluesList.append(str.setNum(dataPtrs[col]->peekData(row,err),'E',6));
//            }

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
    fileStream << "  "+ploFileInfo.baseName()+".PLO.DAT_-1" <<"\n";
    fileStream << "  "+modelFileInfo.fileName() <<"\n";
//    fileStream <<"   Variable     Startvalue     Scaling" <<"\n";
//    fileStream <<"------------------------------------------------------" <<"\n";
//    for(int i=0; i<dataPtrs.size(); ++i)
//    {
//        fileStream << "  " << dataPtrs[i]->getSmartName() << "     " << "STARTVALUEGOESHERE"<<"      "<<plotScaleStringList[i]<<"\n";
//    }

    file.close();
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
        QMessageBox::information(gpMainWindow->window(), gpMainWindow->tr("Hopsan"), "Unable to read .PLO file.");
        return;
    }

    QProgressDialog progressImportBar(tr("Importing PLO"), QString(), 0, 0, gpMainWindow);
    progressImportBar.setWindowModality(Qt::WindowModal);
    progressImportBar.setRange(0,0);
    progressImportBar.show();

    int nDataRows = 0;
    int nDataColumns = 0;
    int ploVersion = 0;

    QVector<PLOImportData> importedPLODataVector;

    // Stream all data and then manipulate
    QTextStream t(&file);

    // Read header data
    for (int lineNum=1; lineNum<7; ++lineNum)
    {
        QString line = t.readLine().trimmed();
        if(!line.isNull())
        {
            // Check PLO format version
            if(lineNum == 2)
            {
                ploVersion = line.toInt();
            }
            // Else check for num data info
            else if(lineNum == 4)
            {
                QStringList colsandrows = line.simplified().split(" ");
                nDataColumns = colsandrows[0].toInt();
                nDataRows = colsandrows[1].toInt();
                // Reserve memory for reading data
                importedPLODataVector.resize(nDataColumns);
                for(int c=0; c<nDataColumns; ++c)
                {
                    importedPLODataVector[c].mDataValues.reserve(nDataRows); //! @todo is it +- 0 or 1
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
                        importedPLODataVector[c].mDataValues.reserve(nDataRows); //! @todo is it +- 0 or 1
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
    }

    // Read the logged data
    for (int lineNum=7; lineNum<7+nDataRows; ++lineNum)
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

    if (importedPLODataVector.size() > 0)
    {
        ++mGenerationNumber;
        SharedLogVariableDataPtrT timeVecPtr(0);

        if (importedPLODataVector[0].mDataName == TIMEVARIABLENAME)
        {
            timeVecPtr = insertTimeVariable(importedPLODataVector.first().mDataValues);
            timeVecPtr->mpVariableDescription->mVariableSourceType = VariableDescription::ImportedVariableType;
        }

        for (int i=1; i<importedPLODataVector.size(); ++i)
        {
            VariableDescription varDesc;
            varDesc.mDataName = importedPLODataVector[i].mDataName;
            varDesc.mVariableSourceType = VariableDescription::ImportedVariableType;
            //! @todo what about reading the unit

            SharedLogVariableDataPtrT pNewData = insertVariableBasedOnDescription(varDesc, timeVecPtr, importedPLODataVector[i].mDataValues);
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
        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("CSV file could not be parsed.");
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

        //Ugly, assume that first vector is always time
        timeVecPtr = insertTimeVariable(data[0]);
        timeVecPtr->mpVariableDescription->mVariableSourceType = VariableDescription::ImportedVariableType;

        for (int i=1; i<data.size(); ++i)
        {
            VariableDescription varDesc;
            varDesc.mDataName = "CSV"+QString::number(i);
            varDesc.mVariableSourceType = VariableDescription::ImportedVariableType;

            /*SharedLogVariableDataPtrT pNewData = */insertVariableBasedOnDescription(varDesc, timeVecPtr, data[i]);
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
            SharedLogVariableDataPtrT pTime = insertTimeVariable(timeColumn);

            // Ok now we have the data lets add it as a variable
            for (int n=0; n<names.size(); ++n)
            {
                //! @todo what if data name already exists?

                VariableDescription varDesc;
                varDesc.mDataName = names[n];
                //varDesc.mDataUnit = "";
                //varDesc.mDataDescription = "";
                //varDesc.mAliasName  = "";
                varDesc.mVariableSourceType = VariableDescription::ImportedVariableType;

                //UniqueSharedTimeVectorPtrHelper helper;
                //SharedTimeVectorPtrT uniqeTimeVectorPtr = helper.makeSureUnique(timeColumn);

                insertVariableBasedOnDescription(varDesc, pTime, dataColumns[n]);
            }

            csvFile.close();

            // We do not want imported data to be removed automatically
            preventGenerationAutoRemoval(mGenerationNumber);

            emit newDataAvailable();
        }
        else
        {
            gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("Could not open data file:  "+csvFilePath);
        }
    }
    else
    {
        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("columns.size() != names.size() in:  LogDataHandler::importTimeVariablesFromCSVColumns()");
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
void LogDataHandler::collectPlotDataFromModel(bool overWriteLastGeneration)
{
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

                        timeVecPtr = insertTimeVariable(timeVec);

                        // Set the custom unit scaling to the default
                        QString defaultTimeUnit = gConfig.getDefaultUnit(TIMEVARIABLENAME);
                        if (defaultTimeUnit != timeVecPtr->getDataUnit())
                        {
                            timeVecPtr->setCustomUnitScale(UnitScale(defaultTimeUnit, gConfig.getUnitScale(TIMEVARIABLENAME, defaultTimeUnit)));
                        }

                        timeVectorObtained = true;
                    }

                    foundData=true;
                    VariableDescription varDesc;
                    varDesc.mModelPath = pModelObject->getParentContainerObject()->getModelFileInfo().fileName();
                    varDesc.mComponentName = pModelObject->getName();
                    varDesc.mPortName = (*pit)->getName();
                    varDesc.mDataName = varDescs[i].mName;
                    varDesc.mDataUnit = varDescs[i].mUnit;
                    varDesc.mDataDescription = varDescs[i].mDescription;
                    varDesc.mAliasName  = varDescs[i].mAlias;
                    varDesc.mVariableSourceType = VariableDescription::ModelVariableType;

                    SharedLogVariableDataPtrT pNewData = insertVariableBasedOnDescription(varDesc, timeVecPtr, dataVec);

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
}


//! @brief Returns the plot data for specified variable
//! @param[in] generation Generation of plot data
//! @param[in] componentName Name of component where variable is located
//! @param[in] portName Name of port where variable is located
//! @param[in] dataName Name of variable
//! @warning Do not call this function for multiports unless you know what you are doing. It will return the data from the first node only.
//! @deprecated
QVector<double> LogDataHandler::getPlotDataValues(int generation, QString componentName, QString portName, QString dataName)
{
    SharedLogVariableDataPtrT pData = getPlotData(generation, componentName, portName, dataName);
    if (pData)
    {
        return pData->getDataVectorCopy();
    }

    return QVector<double>();
}

QVector<double> LogDataHandler::getPlotDataValues(const QString fullName, int generation)
{
    SharedLogVariableDataPtrT pData = getPlotData(fullName, generation);
    if (pData)
    {
        return pData->getDataVectorCopy();
    }

    return QVector<double>();
}

//! @deprecated
SharedLogVariableDataPtrT LogDataHandler::getPlotData(int generation, QString componentName, QString portName, QString dataName)
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

    return getPlotData(concName, generation);
}

SharedLogVariableDataPtrT LogDataHandler::getPlotData(const QString fullName, const int generation)
{
    // First find the data variable
    LogDataMapT::iterator dit = mLogDataMap.find(fullName);
    if (dit != mLogDataMap.end())
    {
        return dit.value()->getDataGeneration(generation);
    }

    // Check alias names if not found with normal name
    for(dit=mLogDataMap.begin(); dit!=mLogDataMap.end(); ++dit)
    {
        if(dit.value().data()->getAliasName() == fullName)
        {
            return dit.value()->getDataGeneration(generation);
        }
    }
    return SharedLogVariableDataPtrT(0);
}

QVector<SharedLogVariableDataPtrT> LogDataHandler::getMultipleLogData(const QRegExp &rNameExp, const int generation) const
{
    QVector<SharedLogVariableDataPtrT> results;
    LogDataMapT::const_iterator it;
    for (it = mLogDataMap.begin(); it != mLogDataMap.end(); it++)
    {
        if (it.key().contains(rNameExp))
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

QVector<double> LogDataHandler::getTimeVectorCopy(int generation) const
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
    QString alias = QInputDialog::getText(gpMainWindow, "Define Variable Alias",
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
    if(!mpParentContainerObject->setVariableAlias(comp,port,var,alias))
    {
        return false;
    }

    LogVariableContainer *pData = mLogDataMap.value(fullName);
    if (pData)
    {
        // Undefine old alias first
        //! @todo this will remove alias from other if it alread exist mayb a question should be given to user
        if (mLogDataMap.contains(pData->getAliasName()))
        {
            undefinePlotAlias(pData->getAliasName());
        }
        //! @todo instead of bool return the uniqe changed alias should be returned
        mLogDataMap.insert(alias, pData);
        pData->setAliasName(alias);


        //! @todo This is madness, this code needs to be rewritten, maybe alias should not be changable from log data like this, or the code should be smarter
        //! @todo we also need a signal for when alias name is changed elsewhere
        // since the alias was undefined above, set it again
        mpParentContainerObject->setVariableAlias(comp,port,var,alias);
    }

    return true;
}


//! @brief Removes specified variable alias
//! @param[in] alias Alias to remove
void LogDataHandler::undefinePlotAlias(QString alias)
{
    LogVariableContainer *data = mLogDataMap.value(alias, 0);
    if (data)
    {
        QString comp,port,var;
        splitConcatName(data->getFullVariableName(), comp,port,var);
        mpParentContainerObject->setVariableAlias(comp,port,var,""); //! @todo maybe a remove alias function would be nice
        data->setAliasName("");

        mLogDataMap.remove(alias);
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

int LogDataHandler::getNumberOfGenerations() const
{
    int nGens=0;
    for(int i=getLowestGenerationNumber(); i<=getHighestGenerationNumber(); ++i)
    {
        bool genExists=false;
        LogDataMapT::const_iterator it;
        for (it=mLogDataMap.begin(); it!=mLogDataMap.end(); ++it)
        {
            if(it.value()->hasDataGeneration(i))
            {
                genExists=true;
            }
        }
        if(genExists)
        {
            qDebug() << "Generation " << i << " exists!";
            ++nGens;
        }
    }

    qDebug() << "nGens = " << nGens;

    return nGens;
}


//! @brief Limits number of plot generations to value specified in configuration
void LogDataHandler::limitPlotGenerations()
{
    if (getNumberOfGenerations() > gConfig.getGenerationLimit())
    {
        if(!gConfig.getAutoLimitLogDataGenerations())
        {
            QDialog *pDialog = new QDialog(gpMainWindow);
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
        int idx=getLowestGenerationNumber();
        while(getNumberOfGenerations() > gConfig.getGenerationLimit() && idx <= getHighestGenerationNumber())
        {
            // Remove old generation in each data variable container
            LogDataMapT::iterator dit = mLogDataMap.begin();
            for ( ; dit!=mLogDataMap.end(); ++dit)
            {
                dit.value()->removeDataGeneration(idx);
            }

            // Clear from generations cache object map
            mGenerationCacheMap.remove(idx);
            ++idx;
        }

//        // Remove old generation in each data variable container
//        LogDataMapT::iterator dit = mLogDataMap.begin();
//        for ( ; dit!=mLogDataMap.end(); ++dit)
//        {
//            dit.value()->removeGenerationsOlderThen(mGenerationNumber - gConfig.getGenerationLimit());
//        }

//        // Clear from generations cache object map
//        QList<int> gens = mGenerationCacheMap.keys();
//        for (int i=0; i<gens.size(); ++i)
//        {
//            if (gens[i] < (mGenerationNumber - gConfig.getGenerationLimit()) )
//            {
//                mGenerationCacheMap.remove(gens[i]);
//            }
//            else
//            {
//                break;
//            }
//        }
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

//    // Remember generation to keep
//    if (!mKeepGenerationsList.contains(gen))
//    {
//        mKeepGenerationsList.prepend(gen);
//    }
}

void LogDataHandler::allowGenerationAutoRemoval(const int gen)
{
    // Tag the generations in each log data variable container
    LogDataMapT::iterator dit = mLogDataMap.begin();
    for ( ; dit!=mLogDataMap.end(); ++dit)
    {
        dit.value()->allowAutoRemove(gen);
    }

//    // Remove generation to keep
    //    mKeepGenerationsList.removeOne(gen);
}

//! @brief Removes a generation (forced removal)
void LogDataHandler::removeGeneration(const int gen)
{
    LogDataMapT::iterator dit = mLogDataMap.begin();
    for ( ; dit!=mLogDataMap.end(); ++dit)
    {
        dit.value()->removeDataGeneration(gen,true);
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
    SharedLogVariableDataPtrT pData1 = getPlotData(a, -1);
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
    SharedLogVariableDataPtrT pData1 = getPlotData(a, -1);
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
    SharedLogVariableDataPtrT pData1 = getPlotData(a, -1);
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
    SharedLogVariableDataPtrT pData1 = getPlotData(a, -1);
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
    SharedLogVariableDataPtrT pData1 = getPlotData(a, -1);
    SharedLogVariableDataPtrT pData2 = getPlotData(b, -1);

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
    SharedLogVariableDataPtrT pData1 = getPlotData(a, -1);
    SharedLogVariableDataPtrT pData2 = getPlotData(b, -1);

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
    SharedLogVariableDataPtrT pData1 = getPlotData(a, -1);
    SharedLogVariableDataPtrT pData2 = getPlotData(b, -1);

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
    SharedLogVariableDataPtrT pData1 = getPlotData(a, -1);
    SharedLogVariableDataPtrT pData2 = getPlotData(b, -1);

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
    SharedLogVariableDataPtrT pData1 = getPlotData(a, -1);
    SharedLogVariableDataPtrT pData2;
    if(b!=TIMEVARIABLENAME)
    {
        pData2 = getPlotData(b, -1);
    }


    if( (pData1 == 0) || (pData2 == 0 && b != TIMEVARIABLENAME) )
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
    SharedLogVariableDataPtrT pData1 = getPlotData(a, -1);
    SharedLogVariableDataPtrT pData2;
    if(b!=TIMEVARIABLENAME)
    {
        pData2 = getPlotData(b, -1);
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
    SharedLogVariableDataPtrT pData1 = getPlotData(a, -1);
    SharedLogVariableDataPtrT pData2;
    if(b!=TIMEVARIABLENAME)
    {
        pData2 = getPlotData(b, -1);
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
    SharedLogVariableDataPtrT pDstData = getPlotData(dst, -1);
    SharedLogVariableDataPtrT pSrcData = getPlotData(src, -1);

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
    SharedLogVariableDataPtrT pDstData = getPlotData(dst, -1);
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
    SharedLogVariableDataPtrT pData1 = getPlotData(a, -1);
    if(pData1 != 0)
    {
        return pokeVariable(pData1, index, value);
    }
    gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("In Poke, No such variable: " + a);
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
            return true;
        }
    }
    gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("In Delete, No such variable: " + a);
    return false;
}

bool LogDataHandler::deleteVariable(SharedLogVariableDataPtrT a)
{
    return deleteVariable(a->getFullVariableName());
}

double LogDataHandler::peekVariable(const QString &a, const int index)
{
    SharedLogVariableDataPtrT pData1 = getPlotData(a, -1);
    if(pData1)
    {
        return peekVariable(pData1,index);
    }
    gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("In Peek, No such variable: " + a);
    return 0;
}

QString LogDataHandler::saveVariable(const QString &currName, const QString &newName)
{
    SharedLogVariableDataPtrT pCurrData = getPlotData(currName, -1);
    SharedLogVariableDataPtrT pNewData = getPlotData(newName, -1);
    // If curr data exist and new data does not exist
    if( (pNewData == 0) && (pCurrData != 0) )
    {
        SharedLogVariableDataPtrT pNewData = defineNewVariable(newName);
        if (pNewData)
        {
            pNewData->assignFrom(pCurrData);
            return pNewData->getFullVariableName();
        }
        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("Could not create variable: " + newName);
        return QString();
    }
    gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("Variable: " + currName + " does not exist, or Variable: " + newName + " already exist");
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
        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage(err);
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
        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage(err);
    }
    return r;
}

SharedLogVariableDataPtrT LogDataHandler::defineTempVariable(QString desiredname)
{
    QString numStr;
    numStr.setNum(mTempVarCtr);
    desiredname.append(numStr);
    SharedLogVariableDataPtrT pData = defineNewVariable(desiredname);
    pData->mpVariableDescription->mVariableSourceType = VariableDescription::TempVariableType;
    if (pData)
    {
       ++mTempVarCtr;
    }
    return pData;
}


void LogDataHandler::appendVariable(const QString &a, const double x, const double y)
{
    SharedLogVariableDataPtrT pData1 = getPlotData(a, -1);
    if(pData1)
    {
        pData1->append(x,y);
        return;
    }
    gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("No such variable: " + a);
    return;
}

SharedLogVariableDataPtrT LogDataHandler::defineNewVariable(const QString desiredname)
{
    if(mLogDataMap.find(desiredname) == mLogDataMap.end())
    {
        VariableDescription varDesc;
        varDesc.mDataName = desiredname;
        varDesc.mVariableSourceType = VariableDescription::ScriptVariableType;
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
    pData->mpVariableDescription->mDataUnit = rUnit;
    pData->mpVariableDescription->mDataUnit = rDescription;
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
    VariableDescription tempVariable;
    tempVariable.mComponentName = componentName;
    tempVariable.mPortName = portName;
    tempVariable.mDataName = dataName;
    tempVariable.mDataUnit = dataUnit;
    if(!mFavoriteVariables.contains(tempVariable))
    {
        mFavoriteVariables.append(tempVariable);
    }
    gpMainWindow->mpPlotWidget->mpPlotVariableTree->updateList();

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
            gpMainWindow->mpPlotWidget->mpPlotVariableTree->updateList();
            return;
        }
    }
}

QString LogDataHandler::plotVariable(const QString plotName, const QString fullVarName, const int gen, const int axis, QColor color)
{
    SharedLogVariableDataPtrT pData = getPlotData(fullVarName, gen);
    if(pData)
    {
        return gpPlotHandler->plotDataToWindow(plotName, pData, axis, color);
    }
    return "";
}

QString LogDataHandler::plotVariable(const QString plotName, const QString &rFullNameX, const QString &rFullNameY, const int gen, const int axis, QColor color)
{
    SharedLogVariableDataPtrT pDataX = getPlotData(rFullNameX, gen);
    SharedLogVariableDataPtrT pDataY = getPlotData(rFullNameY, gen);
    if (pDataX && pDataY)
    {
        return gpPlotHandler->plotDataToWindow(plotName, pDataX, pDataY, axis, color);
    }
    return "";
}

PlotWindow *LogDataHandler::plotVariable(PlotWindow *pPlotWindow, const QString fullVarName, const int gen, const int axis, QColor color)
{
    SharedLogVariableDataPtrT pData = getPlotData(fullVarName, gen);
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

QVector<SharedLogVariableDataPtrT> LogDataHandler::getOnlyVariablesAtGeneration(const int generation)
{
    QVector<SharedLogVariableDataPtrT> dataPtrVector;

    // First go through all data variable
    LogDataMapT::iterator dit = mLogDataMap.begin();
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


QStringList LogDataHandler::getLogDataVariableNames(QString separator, int generation)
{
    QStringList retval;
    LogDataMapT::iterator dit = mLogDataMap.begin();
    for ( ; dit!=mLogDataMap.end(); ++dit)
    {
        if(generation < 0 || dit.value()->hasDataGeneration(generation))
        {
            retval.append(dit.value()->getFullVariableNameWithSeparator(separator));
        }
    }
    return retval;
}


QString LogDataHandler::getNewCacheName()
{
    //The first dir is the main one, any other dirs have been appended later when taking ownership of someone elses data
    return mCacheDirs.first().absoluteFilePath("cf"+QString("%1").arg(mCacheSubDirCtr++));
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
        LogDataMapT::iterator odit; //odit = OtherDataIterator
        for (odit = pOtherHandler->mLogDataMap.begin(); odit!=pOtherHandler->mLogDataMap.end(); ++odit)
        {
            QString fullName = odit.key();
            LogDataMapT::iterator tdit = mLogDataMap.find(fullName); //tdit = ThisDataIterator
            if (tdit != mLogDataMap.end())
            {
                if (odit.value()->hasDataGeneration(generation))
                {
                    tdit.value()->addDataGeneration(mGenerationNumber, odit.value()->getDataGeneration(generation));
                    odit.value()->removeDataGeneration(generation, true);
                    tookOwnershipOfSomeData=true;
                }
            }
            else
            {
                if (odit.value()->hasDataGeneration(generation))
                {
                    LogVariableContainer *pNewContainer = new LogVariableContainer(*(odit.value()->getVariableDescription().data()), this);
                    pNewContainer->addDataGeneration(mGenerationNumber, odit.value()->getDataGeneration(generation));
                    mLogDataMap.insert(fullName, pNewContainer);
                    odit.value()->removeDataGeneration(generation, true);
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


SharedLogVariableDataPtrT LogDataHandler::insertVariableBasedOnDescription(VariableDescription &rVarDesc, SharedLogVariableDataPtrT pTimeVector, QVector<double> &rDataVector)
{
    SharedLogVariableDataPtrT pNewData;
    // First check if a data variable with this name alread exist
    QString fullName = rVarDesc.getFullName();
    LogDataMapT::iterator it = mLogDataMap.find(fullName);
    // If it exist insert into it
    if (it != mLogDataMap.end())
    {
        // Insert it into the generations map
        pNewData = it.value()->addDataGeneration(mGenerationNumber, pTimeVector, rDataVector);

        // Update alias if needed
        if ( rVarDesc.mAliasName != it.value()->getAliasName() )
        {
            // Remove old mention of alias
            mLogDataMap.remove(it.value()->getAliasName());

            // Update the local alias
            it.value()->setAliasName(rVarDesc.mAliasName);

            // Insert new alias kv pair
            mLogDataMap.insert(rVarDesc.mAliasName, it.value());
        }
    }
    else
    {
        // Create a new toplevel map item and insert data into the generations map
        LogVariableContainer *pDataContainer = new LogVariableContainer(rVarDesc, this);
        pNewData = pDataContainer->addDataGeneration(mGenerationNumber, pTimeVector, rDataVector);
        mLogDataMap.insert(fullName, pDataContainer);

        // Also insert alias if it exist
        if ( !rVarDesc.mAliasName.isEmpty() )
        {
            mLogDataMap.insert(rVarDesc.mAliasName, pDataContainer);
        }
    }
    return pNewData;
}


SharedLogVariableDataPtrT LogDataHandler::insertTimeVariable(QVector<double> &rTimeVector)
{
    //! @todo this should not be done/checked here every time should have been prepered someewhere else, but no point in doing it properly now since we must rewrite logdatahandler to be global anyway
    LogVariableContainer *pTime = mLogDataMap.value(TIMEVARIABLENAME);
    if (pTime)
    {
        return pTime->addDataGeneration(mGenerationNumber, SharedLogVariableDataPtrT(), rTimeVector); //Note! Time vector itself does not have a time vector it only has a data vector
    }
    else
    {
        VariableDescription varDesc;
        //varDesc.mModelPath = pModelObject->getParentContainerObject()->getModelFileInfo().fileName();
        varDesc.mDataName = TIMEVARIABLENAME; //!< @todo this name must be reserved
        varDesc.mDataUnit = "s";
        varDesc.mVariableSourceType = VariableDescription::ModelVariableType; //! @todo maybe timetype (dont know, check with old hopsan)

        return insertVariableBasedOnDescription(varDesc, SharedLogVariableDataPtrT(), rTimeVector);
    }
}


bool LogDataHandler::hasPlotData(const QString &rFullName, const int generation)
{
    return (getPlotData(rFullName, generation) != 0);
}
