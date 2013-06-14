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
#include "Widgets/ProjectTabWidget.h"
#include "common.h"
#include "version_gui.h"
#include "Configuration.h"
#include "GUIPort.h"
#include "Utilities/GUIUtilities.h"

#include "PlotWindow.h"
#include "Widgets/PlotWidget.h"
#include "PlotHandler.h"

//! @brief Constructor for plot data object
//! @param pParent Pointer to parent container object
LogDataHandler::LogDataHandler(ContainerObject *pParent) : QObject(pParent)
{
    mpParentContainerObject = pParent;
    mnPlotCurves = 0;
    mGenerationNumber = 0;
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

    QTextStream fileStream(&file);  //Create a QTextStream object to stream the content of file
    QDateTime dateTime = QDateTime::currentDateTime();
    QString dateTimeString = dateTime.toString();
    QFileInfo fii(filePath);
    QString namez = fii.baseName();
    QStringList scalingValuesList;
    QStringList startvaluesList;
    QVector<double> scalings;
    //QString ScaleVal;

    QString modelPathwayy = gpMainWindow->mpModelHandler->getCurrentContainer()->getModelFileInfo().filePath();
    QFileInfo fiz(modelPathwayy);
    QString namemodel = fiz.baseName();

    QList<SharedLogVariableDataPtrT> dataPtrs;
    for(int v=0; v<variables.size(); ++v)
    {
        dataPtrs.append(getPlotData(variables[v],-1));
    }

    // Write initial comment
    fileStream << "    'VERSION' " << QString(HOPSANGUIVERSION) << " " << dateTimeString << "\n";
    fileStream << "    1 " << "\n";
    fileStream << "    '"<<namez<<".PLO"<<"'"<<"\n";
    fileStream << "        " << dataPtrs.size() <<"    "<< dataPtrs[0]->getDataSize()<<"\n";
    fileStream << "    'Time      '";
    for(int i=0; i<dataPtrs.size(); ++i)
    {
        fileStream << ",    'Y" << i<<"      '";
    }
    fileStream <<",    '"<< "\n";

    // Write time and data vectors
    QString str;
    for(int i=0; i<dataPtrs.size()+1; ++i)
    {
        scalingValuesList.append(str.setNum(1.0,'E',6));
        fileStream <<"  "<< str;
        for(int j=0; j<12-str.size(); ++j)
        {
            fileStream << " ";
        }
    }
    fileStream << "\n";


    QString err;
    for(int i=0; i<dataPtrs[0]->getDataSize(); ++i)
    {
        str.setNum(dataPtrs[0]->getSharedTimePointer()->peekData(i,err),'E',6);
        fileStream <<"  "<<str;
        for(int j=0; j<12-str.size(); ++j)
        {
            fileStream << " ";
        }

        for(int k=0; k<dataPtrs.size(); ++k)
        {
            str.setNum(dataPtrs[k]->peekData(i,err),'E',6);
            //scalings = dataPtrs[k]->mDataVector;
            if(i == 0)
            {
                startvaluesList.append(str.setNum(dataPtrs[k]->peekData(i,err),'E',6));
            }

            fileStream <<"  "<< str;
            for(int j=0; j<12-str.size(); ++j)
            {
                fileStream << " ";
            }
        }
        fileStream << "\n";
    }
    fileStream << "  "+namez+".PLO.DAT_-1" <<"\n";
    fileStream << "  "+namemodel+".for" <<"\n";
    fileStream <<"   Variable     Startvalue     Scaling" <<"\n";
    fileStream <<"------------------------------------------------------" <<"\n";
    for(int i=0; i<dataPtrs.size(); ++i)
    {
        fileStream << "  Y" << i << "     " << startvaluesList[i]<<"      "<<scalingValuesList[i]<<"\n";
    }

    file.close();
}

class HopImpData
{
public:
      QString mDataName;
      double scale;
      double startvalue;
      QVector<double> mDataValues;
};

void LogDataHandler::importFromPlo()
{
    QString ImportFileName = QFileDialog::getOpenFileName(0,tr("Choose Old Hopsan File"),
                                                          gConfig.getModelicaModelsDir(),
                                                          tr("Old Hopsan File (*.plo)"));
    if(ImportFileName.isEmpty())
    {
        return;
    }

    QFile file(ImportFileName);
    QFileInfo fileInfo(file);
    gConfig.setModelicaModelsDir(fileInfo.absolutePath());

    QProgressDialog progressImportBar(tr("Initializing"), QString(), 0, 0, gpMainWindow);
    progressImportBar.show();

    progressImportBar.setMaximum(10);
    progressImportBar.setWindowModality(Qt::WindowModal);
    progressImportBar.setWindowTitle(tr("Importing PLO"));
    progressImportBar.setValue(0);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::information(gpMainWindow->window(), gpMainWindow->tr("Hopsan"), "Unable to read Old Hopsan file.");
        return;
    }


    QString tt;
    //QStringList Imcode;
    int colNum = 0;
    int lineNumber = 0;
    int tempcounter = 999000;
    int rowdepth = 0;
    //HopImpData stateList;
    QVector<HopImpData> hopOldVector;

    // Stream all data and then manipulate
    QTextStream t(&file);
    //Imcode = t.readAll();
    while(true)
    {
        QString line = t.readLine();
        if(line.isNull())
        {
            break;
        }
        else
        {
            QString linet = line.trimmed();
            progressImportBar.setValue(1);
            progressImportBar.setLabelText("Reading file (.plo).");
            ++lineNumber;

            if(lineNumber == 4)
            {
                QStringList tempholder = linet.split(" ");
                colNum = tempholder[0].toInt();
                hopOldVector.resize(colNum+1);
                rowdepth = tempholder[4].toInt();
                progressImportBar.setMaximum(rowdepth);

            }

            else if(linet.startsWith("'Time"))
            {

                QStringList tempheader = linet.split(",");
                tempcounter = lineNumber;
                for(int yy=0; yy<colNum+1; ++yy)
                {
                    QString word;
                    //int bb = tempheader[yy].size();
                    word = tempheader[yy].remove(QRegExp("'"));
                    word = word.remove(QRegExp(" "));
                    hopOldVector[yy].mDataName = word.trimmed();
                    progressImportBar.setLabelText("Finding variables.");

                }
                //tt = tempheader[0];


            }
            else if (lineNumber > 6)
           {
               if( (lineNumber< (rowdepth+tempcounter+1)))
                {
                    //rowdepth = depth;
                    //break;
                   int bb = 0;
                   int occurences = linet.count(QRegExp(" -"));
                   for(int nn=0;nn<occurences+1; ++nn)
                   {
                       if(occurences == 0)
                       {
                           break;
                       }
                       else if (nn == occurences)
                       {
                           break;
                       }
                       else
                       {
                           QString cc = " -";
                           int pp = linet.indexOf(cc,bb);
                           linet = linet.insert(pp, QString(" "));
                           bb = pp + 5;
                       }
                   }

                    QStringList tempdataObj = linet.split("  ");

                    for(int kk=0; kk<colNum+1; ++kk)
                    {
                        hopOldVector[kk].mDataValues.append(tempdataObj[kk].toDouble());
                    }
                }
               else
               {
                   break;
               }

            }
            //            else if (depth > (rowdepth-1+5))
            //            {
            //                for(int ss; ss<counter+1; ++ss)
            //                {
            //                    int aa = QString::compare(HopOldVector[ss].ImpHopString, linet, Qt::CaseInsensitive);
            //                    if(aa == 0)
            //                    {
            //                        HopOldVector[ss].startvalue = linet.section(" ",1,1).toDouble();
            //                        HopOldVector[ss].scale = linet.section(" ",2,2).toDouble();
            //                    }
            //                }

            //            }



        }
    }
    file.close();

    progressImportBar.setValue(10);
    progressImportBar.setLabelText("Data Fetched.");

    //UniqueSharedTimeVectorPtrHelper timeVecHelper;
    bool foundData = false;
    //for(QVector<HopImpData>::iterator git=hopOldVector.begin(); git!=hopOldVector.end(); ++git)
    {
//        bool timeVectorObtained = false;
        //SharedTimeVectorPtrT timeVecPtr = timeVecHelper.makeSureUnique(hopOldVector.first().mDataValues);
        //! @todo Should be possible to have multiple timevectors per generation
        //Store time data (only once)
//        if(!timeVectorObtained)
//        {
//            //! @todo this should not be done/checked here every time should have been prepered someewhere else, but no point in doing it properly now since we must rewrite logdatahandler to be global anyway
//            LogVariableContainer *pTime = mLogDataMap.value("time");
//            if (pTime)
//            {
//                pTime->addDataGeneration(mGenerationNumber, SharedTimeVectorPtrT(), hopOldVector.first().mDataValues); //Note! Time vector itself does not have a time vector it only has a data vector
//            }
//            else
//            {
//                VariableDescription varDesc;
//                varDesc.mDataName = "time"; //! @todo this name must be reserved
//                varDesc.mDataUnit = "s";
//                varDesc.mDataDescription = "time";
//                varDesc.mVarType = VariableDescription::ModelVariableType; //! @todo maybe timetype (dont know, check with old hopsan)
//                insertVariableBasedOnDescription(varDesc, SharedTimeVectorPtrT(), hopOldVector.first().mDataValues);
//            }

//            timeVectorObtained = true;
//        }

        SharedLogVariableDataPtrT timeVecPtr;
        if (hopOldVector.size() > 1)
        {
            timeVecPtr = insertTimeVariable(hopOldVector.first().mDataValues);
        }

        for (int i=1; i<hopOldVector.size(); ++i)
        {
            foundData=true;
            VariableDescription varDesc;
            varDesc.mDataName = hopOldVector[i].mDataName;
            varDesc.mVarType = VariableDescription::ImportedVariableType;
            //! @todo what about reading the unit

//            //! @todo use the insert function
//            // First check if a data variable with this name alread exist
//            QString catName = varDesc.getFullName();
//            LogDataMapT::iterator dit = mLogDataMap.find(catName);
//            // If it exist insert into it
//            if (dit != mLogDataMap.end())
//            {
//                // Insert it into the generations map
//                dit.value()->addDataGeneration(mGenerationNumber, timeVecPtr, hopOldVector[i].mDataValues);
//            }
//            else
//            {
//                // Create a new toplevel map item and insert data into the generations map
//                LogVariableContainer *pDataContainer = new LogVariableContainer(varDesc, this);
//                pDataContainer->addDataGeneration(mGenerationNumber, hopOldVector.first().mDataValues, hopOldVector[i].mDataValues);
//                mLogDataMap.insert(catName, pDataContainer);
//            }
            insertVariableBasedOnDescription(varDesc, timeVecPtr, hopOldVector[i].mDataValues);
        }

        if (foundData)
        {
            ++mGenerationNumber;
            emit newDataAvailable();
        }
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
                varDesc.mVarType = VariableDescription::ImportedVariableType;

                //UniqueSharedTimeVectorPtrHelper helper;
                //SharedTimeVectorPtrT uniqeTimeVectorPtr = helper.makeSureUnique(timeColumn);

                insertVariableBasedOnDescription(varDesc, pTime, dataColumns[n]);
            }

            csvFile.close();

            ++mGenerationNumber;
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


//! @brief Collects plot data from last simulation
void LogDataHandler::collectPlotDataFromModel(bool overWriteLastGeneration)
{
    if(overWriteLastGeneration)
    {
        --mGenerationNumber;
    }

    if(mpParentContainerObject->getCoreSystemAccessPtr()->getNSamples() == 0)
    {
        return;         //Don't collect plot data if logging is disabled (to avoid empty generations)
    }

    //UniqueSharedTimeVectorPtrHelper timeVecHelper;
    bool foundData = false;
    bool timeVectorObtained = false;
    SharedLogVariableDataPtrT timeVecPtr;

    //! @todo why not run multiappend when overwriting generation ?
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
                    varDesc.mVarType = VariableDescription::ModelVariableType;

                    insertVariableBasedOnDescription(varDesc, timeVecPtr, dataVec);
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
        ++mGenerationNumber;
    }
}


////! @brief Renames a component in the plot data storage
////! @note Always call this after renaming an object, or plot data for the object will not be accessible.
////! @param[in] oldName Previous name of object
////! @param[in] newName New name of object
//void LogDataHandler::updateObjectName(QString oldName, QString newName)
//{
//    //! @todo FIXA /Peter (or do we even need it at all)
////    for(int i=0; i<mPlotData.size(); ++i)
////    {
////        if(mPlotData.at(i).contains(oldName))
////        {
////            DataMapT generation;
////            generation = mPlotData.at(i);
////            DataMapT oldPlotData;
////            oldPlotData = mPlotData.at(i).find(oldName).value();
////            generation.insert(newName, oldPlotData);
////            generation.remove(oldName);
////            mPlotData.removeAt(i);
////            mPlotData.insert(i, generation);
////        }
////    }
//}


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
    return SharedLogVariableDataPtrT(0);
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
    LogVariableContainer *pCont = mLogDataMap.value("time",0);
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
    //    //! @todo Check that alias is not already used, and deal with it somehow if it is
    //    if (mPlotAliasMap.contains(alias))
    //    {
    //        return false;
    //    }
    //    else
    //    {
    //        LogDataMapT::iterator dit = mLogDataMap.find(fullName);
    //        if (dit!=mLogDataMap.end())
    //        {
    //            // First remove old alias
    //            mPlotAliasMap.remove(dit.value()->getAliasName());

    //            // Assign alias
    //            dit.value()->setAliasName(alias);

    //            // Insert into alias map
    //            mPlotAliasMap.insert(alias, dit.value().data());

    //

    LogVariableContainer *pData = mLogDataMap.value(fullName);
    if (pData)
    {
        // Undefine old alias first
        //! @todo this will remove alias from other if it alread exist mayb a question should be given to user
        if (mLogDataMap.contains(pData->getAliasName()))
        {
            undefinePlotAlias(pData->getAliasName());
        }

        QString comp,port,var;
        splitConcatName(fullName, comp,port,var);
        mpParentContainerObject->setVariableAlias(comp,port,var,alias);
        //! @todo instead of bool return the uniqe changed alias should be returned
        mLogDataMap.insert(alias, pData);
        pData->setAliasName(alias);
    }

    return true;
}


//! @brief Removes specified variable alias
//! @param[in] alias Alias to remove
void LogDataHandler::undefinePlotAlias(QString alias)
{
    //    LogVariableContainer *pDataContainer = mPlotAliasMap.value(alias, 0);
    //    if(pDataContainer)
    //    {
    //        QString fullName = pDataContainer->getFullVariableName();


    LogVariableContainer *data = mLogDataMap.value(alias, 0);
    if (data)
    {
        QString comp,port,var;
        splitConcatName(data->getFullVariableName(), comp,port,var);
        mpParentContainerObject->setVariableAlias(comp,port,var,""); //! @todo maybe a remove alias function would be nice
        data->setAliasName("");

        mLogDataMap.remove(alias);
    }
    //        mPlotAliasMap.remove(alias);
    //    }
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


//! @brief Limits number of plot generations to value specified in configuration
void LogDataHandler::limitPlotGenerations()
{
    if ( (mGenerationNumber - gConfig.getGenerationLimit()) > 0 )
    {
        // Remove old generation in each data varaible container
        LogDataMapT::iterator dit = mLogDataMap.begin();
        for ( ; dit!=mLogDataMap.end(); ++dit)
        {
            dit.value()->removeGenerationsOlderThen(mGenerationNumber - gConfig.getGenerationLimit());
        }

        // Clear from generations cache object map
        QList<int> gens = mGenerationCacheMap.keys();
        for (int i=0; i<gens.size(); ++i)
        {
            if (gens[i] < (mGenerationNumber - gConfig.getGenerationLimit()) )
            {
                mGenerationCacheMap.remove(gens[i]);
            }
            else
            {
                break;
            }
        }
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

    // Remember generation to keep
    if (!mKeepGenerationsList.contains(gen))
    {
        mKeepGenerationsList.prepend(gen);
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

    // Remove generation to keep
    mKeepGenerationsList.removeOne(gen);
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
    SharedLogVariableDataPtrT pTempVar = defineTempVariable(a->getFullVariableName()+b->getFullVariableName());
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
    if(b!="time")
    {
        pData2 = getPlotData(b, -1);
    }


    if( (pData1 == 0) || (pData2 == 0 && b != "time") )
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
    if(b!="time")
    {
        pData2 = getPlotData(b, -1);
    }


    if( (pData1 == 0) || (pData2 == 0 && b != "time") )
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
    if(b!="time")
    {
        pData2 = getPlotData(b, -1);
    }


    if( (pData1 == 0) || (pData2 == 0 && b != "time") )
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
    gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("No such variable: " + a);
    return 0;
}

bool LogDataHandler::deleteVariable(const QString &a)
{
    LogDataMapT::iterator it = mLogDataMap.find(a);
    if(it != mLogDataMap.end())
    {
        it.value()->removeAllGenerations();
        it.value()->deleteLater();
        mLogDataMap.erase(it);
        return true;
    }
    gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("No such variable: " + a);
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
    gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("No such variable: " + a);
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
    if (pData)
    {
       ++mTempVarCtr;
    }
    return pData;
}

void LogDataHandler::appendVariable(const SharedLogVariableDataPtrT a, const double x, const double y)
{
    a->append(x,y);
    return;
}

void LogDataHandler::appendVariable(const QString &a, const double x, const double y)
{
    SharedLogVariableDataPtrT pData1 = getPlotData(a, -1);
    if(pData1)
    {
        appendVariable(pData1,x,y);
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
        varDesc.mVarType = VariableDescription::ScriptVariableType;
        LogVariableContainer *pDataContainer = new LogVariableContainer(varDesc, this);
        pDataContainer->addDataGeneration(mGenerationNumber, QVector<double>(), QVector<double>());
        mLogDataMap.insert(varDesc.getFullName(), pDataContainer);
        return pDataContainer->getDataGeneration(mGenerationNumber);
    }
    return SharedLogVariableDataPtrT();
}

SharedLogVariableDataPtrT LogDataHandler::defineNewVariable(const QString desiredname, QVector<double> x, QVector<double> y)
{
    VariableDescription varDesc;
    varDesc.mDataName = desiredname;
    varDesc.mVarType = VariableDescription::ScriptVariableType;
    LogVariableContainer *pDataContainer = new LogVariableContainer(varDesc, this);
    pDataContainer->addDataGeneration(mGenerationNumber, x, y);
    mLogDataMap.insert(varDesc.getFullName(), pDataContainer);
    return pDataContainer->getDataGeneration(mGenerationNumber);
}



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
    // Since we post increment the counter after sucessfull import / collect we need to subtract one
    return mGenerationNumber-1;
}


QStringList LogDataHandler::getPlotDataNames()
{
    QStringList retval;
    LogDataMapT::iterator dit = mLogDataMap.begin();
    for ( ; dit!=mLogDataMap.end(); ++dit)
    {
        retval.append(dit.value()->getFullVariableNameWithSeparator("."));
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
        }

        // Take the data
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
                }
            }
        }

        //! @todo what about collision with tempvariable names
        // Take the tempvariable counter
        //! @todo do this, or is it even necessary

        // Take ownership of keep generation list contents
        if (pOtherHandler->mKeepGenerationsList.contains(generation))
        {
            mKeepGenerationsList.append(mGenerationNumber);
            pOtherHandler->mKeepGenerationsList.removeOne(generation);
        }

        // Increment generation
        ++mGenerationNumber;
    }

    //! @todo favorite variables, tempvar counter
}


void LogDataHandler::insertVariableBasedOnDescription(VariableDescription &rVarDesc, SharedLogVariableDataPtrT pTimeVector, QVector<double> &rDataVector)
{
    // First check if a data variable with this name alread exist
    QString fullName = rVarDesc.getFullName();
    LogDataMapT::iterator it = mLogDataMap.find(fullName);
    // If it exist insert into it
    if (it != mLogDataMap.end())
    {
        // Insert it into the generations map
        it.value()->addDataGeneration(mGenerationNumber, pTimeVector, rDataVector);

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
        pDataContainer->addDataGeneration(mGenerationNumber, pTimeVector, rDataVector);
        mLogDataMap.insert(fullName, pDataContainer);

        // Also insert alias if it exist
        if ( !rVarDesc.mAliasName.isEmpty() )
        {
            mLogDataMap.insert(rVarDesc.mAliasName, pDataContainer);
        }
    }
}


SharedLogVariableDataPtrT LogDataHandler::insertTimeVariable(QVector<double> &rTimeVector)
{
    //! @todo this should not be done/checked here every time should have been prepered someewhere else, but no point in doing it properly now since we must rewrite logdatahandler to be global anyway
    LogVariableContainer *pTime = mLogDataMap.value("Time");
    if (pTime)
    {
        pTime->addDataGeneration(mGenerationNumber, SharedLogVariableDataPtrT(), rTimeVector); //Note! Time vector itself does not have a time vector it only has a data vector
        return  pTime->getDataGeneration(-1);
    }
    else
    {
        VariableDescription varDesc;
        //varDesc.mModelPath = pModelObject->getParentContainerObject()->getModelFileInfo().fileName();
        varDesc.mDataName = "Time"; //! @todo this name must be reserved
        varDesc.mDataUnit = "s";
        varDesc.mVarType = VariableDescription::ModelVariableType; //! @todo maybe timetype (dont know, check with old hopsan)

        insertVariableBasedOnDescription(varDesc, SharedLogVariableDataPtrT(), rTimeVector);
        return  mLogDataMap.value("Time")->getDataGeneration(mGenerationNumber);
    }
}
