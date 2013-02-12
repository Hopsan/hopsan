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
        tmp = QDir(LOGDATACACHE + QString("/handler%1").arg(ctr));
        ++ctr;
    }while(tmp.exists());
    tmp.mkpath(tmp.absolutePath());
    mCacheDir = tmp;
    mCacheSubDirCtr = 0;
}

LogDataHandler::~LogDataHandler()
{
    qDebug() << "in LogDataHandler destructor" << endl;

    // Clear all data
    QList<LogVariableContainer*> data = mLogDataMap.values();
    for (int i=0; i<data.size(); ++i)
    {
        delete data[i];
    }
    mLogDataMap.clear();
    // Clear generation Cache files (Individual files will remain if until all instances dies)
    mGenerationCacheMap.clear();

    // Remove the cache directory if it is empty, if it is not then cleanup should happen on program exit
    mCacheDir.rmdir(mCacheDir.path());
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

    QString modelPathwayy = gpMainWindow->mpProjectTabs->getCurrentContainer()->getModelFileInfo().filePath();
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
        str.setNum(dataPtrs[0]->mSharedTimeVectorPtr->at(i),'E',6);
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

    UniqueSharedTimeVectorPtrHelper timeVecHelper;
    bool foundData = false;
    //for(QVector<HopImpData>::iterator git=hopOldVector.begin(); git!=hopOldVector.end(); ++git)
    {
        bool timeVectorObtained = false;
        SharedTimeVectorPtrT timeVecPtr = timeVecHelper.makeSureUnique(hopOldVector.first().mDataValues);
        //! @todo Should be possible to have multiple timevectors per generation
        //Store time data (only once)
        if(!timeVectorObtained)
        {
            mTimeVectorPtrs.append(timeVecPtr);
            timeVectorObtained = true;
        }


        for (int i=1; i<hopOldVector.size(); ++i)
        {
            foundData=true;
            VariableDescription varDesc;
            varDesc.mDataName = hopOldVector[i].mDataName;
            varDesc.mVarType = VariableDescription::I;
            //! @todo what about reading the unit

            // First check if a data variable with this name alread exist
            QString catName = varDesc.getFullName();
            LogDataMapT::iterator dit = mLogDataMap.find(catName);
            // If it exist insert into it
            if (dit != mLogDataMap.end())
            {
                // Insert it into the generations map
                dit.value()->addDataGeneration(mGenerationNumber, timeVecPtr, hopOldVector[i].mDataValues);
            }
            else
            {
                // Create a new toplevel map item and insert data into the generations map
                LogVariableContainer *pDataContainer = new LogVariableContainer(varDesc, this);
                pDataContainer->addDataGeneration(mGenerationNumber, hopOldVector.first().mDataValues, hopOldVector[i].mDataValues);
                mLogDataMap.insert(catName, pDataContainer);
            }
        }

        if (foundData)
        {
            ++mGenerationNumber;
            emit newDataAvailable();
        }
    }

       //    //Store time data (only once)
//    if(!HopOldVector.isEmpty())
//    {
//        mTimeVectors.append(HopOldVector.first().mDataValues);
//        //timeVectorObtained = true;

//        for (int i=1; i<HopOldVector.size(); ++i)
//        {

//            //QPair<QVector<double>, QVector<double> > dataImported;

//            //Store variable data

//            ImpdataMap.insert(HopOldVector[i].mDataName, HopOldVector[i].mDataValues);
//            ImpPortMap.insert(HopOldVector[i].mDataName, ImpdataMap);

//            ImpcomponentMap.insert(HopOldVector[i].mDataName, ImpPortMap);

//            //register alias
//            definePlotAlias(HopOldVector[i].mDataName, HopOldVector[i].mDataName, HopOldVector[i].mDataName, HopOldVector[i].mDataName);
//            setFavoriteVariable(HopOldVector[i].mDataName, HopOldVector[i].mDataName, HopOldVector[i].mDataName, HopOldVector[i].mDataName);

//        }

//    }

//    if(!ImpcomponentMap.isEmpty())     //Don't insert a generation if no plot data was collected (= model is empty)
//    {
//        mPlotData.append(ImpcomponentMap);
//    }

//    //Limit number of plot generations if there are too many
    limitPlotGenerations();
    //gpMainWindow->mpPlotWidget->mpPlotVariableTree->updateList();




}


//! @brief Returns whether or not plot data is empty
bool LogDataHandler::isEmpty()
{
    return mLogDataMap.isEmpty();
}


//! @brief Collects plot data from last simulation
void LogDataHandler::collectPlotDataFromModel()
{
    if(mpParentContainerObject->getCoreSystemAccessPtr()->getNSamples() == 0)
    {
        return;         //Don't collect plot data if logging is disabled (to avoid empty generations)
    }

    //bool timeVectorObtained = false;
    UniqueSharedTimeVectorPtrHelper timeVecHelper;
    bool foundData = false;
    bool timeVectorObtained = false;

    this->getGenerationMultiCache(mGenerationNumber)->beginMultiAppend();
    //Iterate components
    for(int m=0; m<mpParentContainerObject->getModelObjectNames().size(); ++m)
    {
        //! @todo getting names every time is very ineffecient it creates and copies a new vector every freaking time
        ModelObject *pModelObject = mpParentContainerObject->getModelObject(mpParentContainerObject->getModelObjectNames().at(m));

        for(QList<Port*>::iterator pit=pModelObject->getPortListPtrs().begin(); pit!=pModelObject->getPortListPtrs().end(); ++pit)
        {
            //QVector<QString> names;
            //QVector<QString> units;
            QVector<CoreVariableData> varDescs;
            //mpParentContainerObject->getCoreSystemAccessPtr()->getPlotDataNamesAndUnits(pModelObject->getName(), (*pit)->getPortName(), names, units);
            mpParentContainerObject->getCoreSystemAccessPtr()->getVariableDescriptions(pModelObject->getName(), (*pit)->getPortName(), varDescs);

            //Iterate variables
            for(int i=0; i<varDescs.size(); ++i)
            {
                //Fetch variables
                QPair<QVector<double>, QVector<double> > data;
                mpParentContainerObject->getCoreSystemAccessPtr()->getPlotData(pModelObject->getName(), (*pit)->getPortName(), varDescs[i].mName, data);

                // Prevent adding data if time or data vector was empty
                if (!data.first.isEmpty() && !data.second.isEmpty())
                {
                    // Make sure we use the same time vector
                    SharedTimeVectorPtrT timeVecPtr = timeVecHelper.makeSureUnique(data.first);

                    //! @todo Should be possible to have multiple timevectors per generation
                    //Store time data (only once)
                    if(!timeVectorObtained)
                    {
                        //! @todo this vector is never cleared when generations are removed
                        mTimeVectorPtrs.append(timeVecPtr);
                        timeVectorObtained = true;
                    }


                    foundData=true;
                    VariableDescription varDesc;
                    varDesc.mModelPath = pModelObject->getParentContainerObject()->getModelFileInfo().fileName();
                    varDesc.mComponentName = pModelObject->getName();
                    varDesc.mPortName = (*pit)->getPortName();
                    varDesc.mDataName = varDescs[i].mName;
                    varDesc.mDataUnit = varDescs[i].mUnit;
                    varDesc.mAliasName  = varDescs[i].mAlias;
                    varDesc.mVarType = VariableDescription::M;

                    // First check if a data variable with this name alread exist
                    QString catName = varDesc.getFullName();
                    LogDataMapT::iterator it = mLogDataMap.find(catName);
                    // If it exist insert into it
                    if (it != mLogDataMap.end())
                    {
                        // Insert it into the generations map
                        it.value()->addDataGeneration(mGenerationNumber, timeVecPtr, data.second);

                        // Update alias if needed
                        if ( varDesc.mAliasName != it.value()->getAliasName() )
                        {
                            // Remove old mention of alias
                            mLogDataMap.remove(it.value()->getAliasName());

                            // Update the local alias
                            it.value()->setAliasName(varDesc.mAliasName);

                            // Insert new alias kv pair
                            mLogDataMap.insert(varDesc.mAliasName, it.value());
                        }
                    }
                    else
                    {
                        // Create a new toplevel map item and insert data into the generations map
                        LogVariableContainer *pDataContainer = new LogVariableContainer(varDesc, this);
                        pDataContainer->addDataGeneration(mGenerationNumber, timeVecPtr, data.second);
                        mLogDataMap.insert(catName, pDataContainer);

                        // Also insert alias if it exist
                        if ( !varDesc.mAliasName.isEmpty() )
                        {
                            mLogDataMap.insert(varDesc.mAliasName, pDataContainer);
                        }
                    }
                }
            }
        }
    }
    this->getGenerationMultiCache(mGenerationNumber)->endMultiAppend();

    // Iterate and create/add aliases
//    //! @todo maybe a function that retreives alias and fullnames in one
//    QStringList aliasNames = mpParentContainerObject->getAliasNames();

//    AliasMapT::iterator ait = mPlotAliasMap.begin();
//    while(ait!=mPlotAliasMap.end())
//    {
//        if (!aliasNames.contains(ait.key()))
//        {
//            ait.value()->setAliasName("");
//            mPlotAliasMap.erase(ait);
//            ait=mPlotAliasMap.begin(); //Restart since itarator breaks
//        }
//        else
//        {
//            ++ait;
//        }
//    }

//    for (int i=0; i<aliasNames.size(); ++i)
//    {
//        QString fullName = mpParentContainerObject->getFullNameFromAlias(aliasNames[i]);
////        DataMapT::iterator it = mAllPlotData.find(fullName);
////        if (it != mAllPlotData.end())
////        {
////            it->second->setAliasName(aliasNames[i]);
////        }

//        //! @todo This will not work when a alias have chenged as this function will reject it, but that coude should change anyhow
//        //! @todo Do not register parameter aliases as variable aliases
//        definePlotAlias(aliasNames[i], fullName);
//    }


    // Limit number of plot generations if there are too many
    limitPlotGenerations();

    // Increment generation counter
    if (foundData)
    {
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
        return pData->getDataVector();
    }

    return QVector<double>();
}

QVector<double> LogDataHandler::getPlotDataValues(const QString fullName, int generation)
{
    SharedLogVariableDataPtrT pData = getPlotData(fullName, generation);
    if (pData)
    {
        return pData->getDataVector();
    }

    return QVector<double>();
}

//! @deprecated
SharedLogVariableDataPtrT LogDataHandler::getPlotData(int generation, QString componentName, QString portName, QString dataName)
{
    //! @todo how to handle request by alias
    QString concName = componentName+"#"+portName+"#"+dataName;

    //! @todo this should probalby be handled in collectData
    if(mpParentContainerObject->getModelObject(componentName)->getPort(portName)->getPortType() == "POWERMULTIPORT" ||
       mpParentContainerObject->getModelObject(componentName)->getPort(portName)->getPortType() == "READMULTIPORT")
    {
        QString newPortName = mpParentContainerObject->getModelObject(componentName)->getPort(portName)->getConnectedPorts().first()->getPortName();
        QString newComponentName = mpParentContainerObject->getModelObject(componentName)->getPort(portName)->getConnectedPorts().first()->getGuiModelObject()->getName();

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
QVector<double> LogDataHandler::getTimeVector(int generation)
{
    return *mTimeVectorPtrs.at(generation);
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


//! @brief Limits number of plot generations to value specified in configuration
void LogDataHandler::limitPlotGenerations()
{
    if ( (mGenerationNumber - gConfig.getGenerationLimit()) > 0 )
    {
        LogDataMapT::iterator dit = mLogDataMap.begin();
        for ( ; dit!=mLogDataMap.end(); ++dit)
        {
            dit.value()->removeGenerationsOlderThen(mGenerationNumber - gConfig.getGenerationLimit());
        }

        // Clear from generations cache object map
        QMap<int, SharedMultiDataVectorCacheT>::iterator it;
        for (it = mGenerationCacheMap.begin(); it != mGenerationCacheMap.end(); ++it)
        {
            if (it.key() < (mGenerationNumber - gConfig.getGenerationLimit()) )
            {
                mGenerationCacheMap.erase(it);
            }
            else
            {
                break;
            }
        }
    }

}

ContainerObject *LogDataHandler::getParentContainerObject()
{
    return mpParentContainerObject;
}

QDir LogDataHandler::getCacheDir() const
{
    return mCacheDir;
}

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
    pTempVar->assignToData(a);
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
    pTempVar->assignToData(a);
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
    pTempVar->assignToData(a);
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
    pTempVar->assignToData(a);
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
    pTempVar->assignToData(a);
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

QString LogDataHandler::assignVariable(const QString &a, const QString &b)
{
    SharedLogVariableDataPtrT pData1 = getPlotData(a, -1);
    SharedLogVariableDataPtrT pData2 = getPlotData(b, -1);

    if(pData2 == 0)
    {
        return QString();
    }
    else if(pData1 == 0)
    {
        VariableDescription varDesc;
        varDesc.mDataName = a;
        LogVariableContainer *pDataContainer = new LogVariableContainer(varDesc, this);
        pDataContainer->addDataGeneration(mGenerationNumber, QVector<double>(), QVector<double>());
        mLogDataMap.insert(a, pDataContainer);
        ++mGenerationNumber;
        pData1 = getPlotData(a,-1);
        SharedLogVariableDataPtrT pTemp = assignVariable(pData1,pData2);
        return pTemp->getFullVariableName();
    }
    else
    {
        SharedLogVariableDataPtrT pTemp = assignVariable(pData1,pData2);
        return pTemp->getFullVariableName();
    }
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
        mLogDataMap.erase(it);
        return true;
    }
    gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("No such variable: " + a);
    return false;
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
            pNewData->assignToData(pCurrData);
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
    pTempVar->assignToData(a);
    pTempVar->subFromData(b);
    return pTempVar;
}

SharedLogVariableDataPtrT LogDataHandler::multVariables(const SharedLogVariableDataPtrT a, const SharedLogVariableDataPtrT b)
{
    SharedLogVariableDataPtrT pTempVar = defineTempVariable(a->getFullVariableName()+b->getFullVariableName());
    pTempVar->assignToData(a);
    pTempVar->multData(b);
    return pTempVar;
}

SharedLogVariableDataPtrT LogDataHandler::divVariables(const SharedLogVariableDataPtrT a, const SharedLogVariableDataPtrT b)
{
    SharedLogVariableDataPtrT pTempVar = defineTempVariable(a->getFullVariableName()+b->getFullVariableName());
    pTempVar->assignToData(a);
    pTempVar->divData(b);
    return pTempVar;
}

//! @todo Should this function really return a value?
SharedLogVariableDataPtrT LogDataHandler::assignVariable(SharedLogVariableDataPtrT a, const SharedLogVariableDataPtrT b)
{
    a->assignToData(b);
    return a;
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

bool LogDataHandler::deleteVariable(SharedLogVariableDataPtrT a)
{
    LogDataMapT::iterator it = mLogDataMap.find(a->getFullVariableName());
    if(it != mLogDataMap.end())
    {
        it.value()->removeAllGenerations();
        mLogDataMap.erase(it);
        return true;
    }
    return false;
}

SharedLogVariableDataPtrT LogDataHandler::saveVariable(SharedLogVariableDataPtrT a)
{
    SharedLogVariableDataPtrT pTempVar = defineTempVariable(a->getFullVariableName());
    pTempVar->assignToData(a);
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

SharedLogVariableDataPtrT LogDataHandler::defineNewVariable(const QString desiredname)
{
    if(mLogDataMap.find(desiredname) == mLogDataMap.end())
    {
        VariableDescription varDesc;
        varDesc.mDataName = desiredname;
        varDesc.mVarType = VariableDescription::S;
        LogVariableContainer *pDataContainer = new LogVariableContainer(varDesc, this);
        pDataContainer->addDataGeneration(mGenerationNumber, QVector<double>(), QVector<double>());
        mLogDataMap.insert(varDesc.getFullName(), pDataContainer);
        return pDataContainer->getDataGeneration(mGenerationNumber);
    }
    return SharedLogVariableDataPtrT();
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

    mpParentContainerObject->mpParentProjectTab->hasChanged();
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
    return mCacheDir.absoluteFilePath("g"+QString("%1").arg(mCacheSubDirCtr++));
}
