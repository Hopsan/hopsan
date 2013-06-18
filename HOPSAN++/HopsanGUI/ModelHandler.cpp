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
//! @file   ModelHandler.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2013-05-16
//!
//! @brief Contains a model handler object
//!
//$Id$

#include "common.h"
#include "Configuration.h"
#include "DesktopHandler.h"
#include "GraphicsView.h"
#include "GUIObjects/GUISystem.h"
#include "LogDataHandler.h"
#include "MainWindow.h"
#include "ModelHandler.h"
#include "SimulationThreadHandler.h"
#include "version_gui.h"
#include "Widgets/DebuggerWidget.h"
#include "Widgets/HcomWidget.h"
#include "Widgets/LibraryWidget.h"
#include "Widgets/ProjectTabWidget.h"

ModelHandler::ModelHandler(QObject *parent)
    : QObject(parent)
{
    mNumberOfUntitledModels=0;

    mpSimulationThreadHandler = new SimulationThreadHandler();

    mCurrentIdx = -1;

    connect(this, SIGNAL(checkMessages()),      gpMainWindow->mpTerminalWidget,    SLOT(checkMessages()), Qt::UniqueConnection);
}

void ModelHandler::addModelWidget(ModelWidget *pModelWidget, const QString &name, bool hidden)
{
    pModelWidget->setParent(gpMainWindow->mpCentralTabs);    //! @todo Should probably use ModelHandler as parent

    mModelPtrs.append(pModelWidget);
    mCurrentIdx = mModelPtrs.size()-1;

    // If the Modelwidget should not be hidden then add it as a tab and switch to that tab
    if(!hidden)
    {
        gpMainWindow->mpCentralTabs->setCurrentIndex(gpMainWindow->mpCentralTabs->addTab(pModelWidget, name));
        pModelWidget->setToolBarSimulationTimeParametersFromTab();
    }

    //! @todo Should we emit this also when hidden?
    emit newModelWidgetAdded();
}


//! @brief Adds a ModelWidget object (a new tab) to itself.
//! @see closeModel(int index)
ModelWidget *ModelHandler::addNewModel(QString modelName, bool hidden)
{
    modelName.append(QString::number(mNumberOfUntitledModels));

    ModelWidget *pNewModelWidget = new ModelWidget(this,gpMainWindow->mpCentralTabs);    //! @todo Should probably use ModelHandler as parent
    pNewModelWidget->getTopLevelSystem()->setName(modelName);

    addModelWidget(pNewModelWidget, modelName, hidden);

//    if(!hidden)
//    {
//        pNewModelWidget->setToolBarSimulationTimeParametersFromTab();
//    }
    pNewModelWidget->setSaved(true);
    mNumberOfUntitledModels += 1;

    return pNewModelWidget;
}


void ModelHandler::setCurrentModel(int idx)
{
    mCurrentIdx = idx;
    // Also switch tab if it is visible among the tabs
    if(gpMainWindow->mpCentralTabs->indexOf(mModelPtrs[idx]) != -1)
        gpMainWindow->mpCentralTabs->setCurrentWidget(mModelPtrs[idx]);
}


void ModelHandler::setCurrentModel(ModelWidget *pWidget)
{
    setCurrentModel(mModelPtrs.indexOf(pWidget));
}


ModelWidget *ModelHandler::getModel(int idx)
{
    if( (idx >= 0) && (idx < mModelPtrs.size()) )
    {
        return mModelPtrs[idx];
    }
    return 0;
}


ModelWidget *ModelHandler::getCurrentModel()
{
    if( mModelPtrs.isEmpty() || (mCurrentIdx < 0) )
    {
        return 0;
    }
    return mModelPtrs[mCurrentIdx];
}

SystemContainer *ModelHandler::getTopLevelSystem(int idx)
{
    ModelWidget *pMW = getModel(idx);
    if (pMW)
    {
        return pMW->getTopLevelSystem();
    }
    return 0;
}

SystemContainer *ModelHandler::getCurrentTopLevelSystem()
{
    ModelWidget *pMW = getCurrentModel();
    if (pMW)
    {
        return pMW->getTopLevelSystem();
    }
    return 0;
}

ContainerObject *ModelHandler::getContainer(int idx)
{
    ModelWidget *pMW = getModel(idx);
    if (pMW)
    {
        return pMW->getGraphicsView()->getContainerPtr();
    }
    return 0;
}

ContainerObject *ModelHandler::getCurrentContainer()
{
    ModelWidget *pMW = getCurrentModel();
    if (pMW)
    {
        return pMW->getGraphicsView()->getContainerPtr();
    }
    return 0;
}

int ModelHandler::count() const
{
    return mModelPtrs.size();
}


//! @brief Loads a model from a file and opens it in a new project tab.
//! @see loadModel(QString modelFileName)
//! @see Model(saveTarget saveAsFlag)
void ModelHandler::loadModel()
{
    QDir fileDialogOpenDir;
    QString modelFileName = QFileDialog::getOpenFileName(gpMainWindow, tr("Choose Model File"),
                                                         gConfig.getLoadModelDir(),
                                                         tr("Hopsan Model Files (*.hmf *.xml)"));
    if(!modelFileName.isEmpty())
    {
        loadModel(modelFileName);
        QFileInfo fileInfo = QFileInfo(modelFileName);
        gConfig.setLoadModelDir(fileInfo.absolutePath());
    }
}


//! @brief Help function that loads a model from the text in a QAction object.
//! Used to facilitate recent models function.
void ModelHandler::loadModel(QAction *action)
{
    loadModel(action->text());
}


void ModelHandler::loadModelParameters()
{
    qobject_cast<SystemContainer*>(getCurrentContainer())->loadParameterFile();
}


//! @brief Loads a model from a file and opens it in a new project tab.
//! @param modelFileName is the path to the loaded file
//! @see loadModel()
//! @see saveModel(saveTarget saveAsFlag)
ModelWidget *ModelHandler::loadModel(QString modelFileName, bool ignoreAlreadyOpen, bool hidden)
{
    //! @todo maybe  write utility function that opens filel checks existance and sets fileinfo
    QFile file(modelFileName);   //Create a QFile object
    if(!file.exists())
    {
        qDebug() << "File not found: " + file.fileName();
        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("File not found: " + file.fileName());
        return 0;
    }
    QFileInfo fileInfo(file);

    //Make sure file not already open
    if(!ignoreAlreadyOpen)
    {
        for(int t=0; t!=mModelPtrs.size(); ++t)
        {
            if(this->getTopLevelSystem(t)->getModelFileInfo().filePath() == fileInfo.filePath() && gpMainWindow->mpCentralTabs->indexOf(mModelPtrs[t]) > -1)
            {
                QMessageBox::information(gpMainWindow, tr("Error"), tr("Unable to load model. File is already open."));
                return 0;
            }
        }
    }

    gpMainWindow->registerRecentModel(fileInfo);

    ModelWidget *pNewModel = new ModelWidget(this, gpMainWindow->mpCentralTabs);
    this->addModelWidget(pNewModel, fileInfo.baseName(), hidden);
    pNewModel->getTopLevelSystem()->getCoreSystemAccessPtr()->addSearchPath(fileInfo.absoluteDir().absolutePath());
    pNewModel->getTopLevelSystem()->setUndoEnabled(false, true);

    if(!hidden)
        gpMainWindow->mpTerminalWidget->mpConsole->printInfoMessage("Loading model: "+fileInfo.absoluteFilePath());

    //Check if this is an expected hmf xml file
    //! @todo maybe write helpfunction that does this directly in system (or container)
    QDomDocument domDocument;
    QDomElement hmfRoot = loadXMLDomDocument(file, domDocument, HMF_ROOTTAG);
    if (!hmfRoot.isNull())
    {
        //! @todo check if we could load else give error message and dont attempt to load
        QDomElement systemElement = hmfRoot.firstChildElement(HMF_SYSTEMTAG);

        // Check if Format version OK
        QString hmfFormatVersion = hmfRoot.attribute(HMF_VERSIONTAG, "0");
        if (!verifyHmfFormatVersion(hmfFormatVersion))
        {
            gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("Model file format: "+hmfFormatVersion+", is to old. Try to update (resave) the model in an previous version of Hopsan");
        }
        else if (hmfFormatVersion < HMF_VERSIONNUM)
        {
            gpMainWindow->mpTerminalWidget->mpConsole->printWarningMessage("Model file is saved with an older version of Hopsan, but versions should be compatible.");
        }

        pNewModel->getTopLevelSystem()->setModelFileInfo(file); //Remember info about the file from which the data was loaded
        pNewModel->getTopLevelSystem()->setAppearanceDataBasePath(pNewModel->getTopLevelSystem()->getModelFileInfo().absolutePath());
        pNewModel->getTopLevelSystem()->loadFromDomElement(systemElement);

        //! @todo not hardcoded strings
        //! @todo in the future not only debug message but an actual check that libs are present
        QDomElement reqDom = hmfRoot.firstChildElement("requirements");
        QDomElement compLib = reqDom.firstChildElement("componentlibrary");
        while (!compLib.isNull())
        {
            gpMainWindow->mpTerminalWidget->mpConsole->printDebugMessage("This model MIGHT require Lib: " + compLib.text());
            compLib = compLib.nextSiblingElement("componentlibrary");
        }
    }
    else
    {
        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage(QString("Model does not contain a HMF root tag: ")+HMF_ROOTTAG);
    }
    pNewModel->setSaved(true);

    pNewModel->getTopLevelSystem()->setUndoEnabled(true, true);

    emit newModelWidgetAdded();

    return pNewModel;
}

void ModelHandler::setCurrentTopLevelSimulationTimeParameters(const QString startTime, const QString timeStep, const QString stopTime)
{
    if (count() > 0)
    {
        getCurrentModel()->setTopLevelSimulationTime(startTime, timeStep, stopTime);
    }
}


bool ModelHandler::closeModelByTabIndex(int tabIdx)
{
    for(int i=0; i<mModelPtrs.size(); ++i)
    {
        // When found close and return, else return false when loop ends
        if(mModelPtrs[i] == gpMainWindow->mpCentralTabs->widget(tabIdx))
        {
            return closeModel(i);
        }
    }
    return false;
}


bool ModelHandler::closeModel(ModelWidget *pModel)
{
    return closeModel(mModelPtrs.indexOf(pModel));
}


//! @brief Closes current project.
//! @param index defines which project to close.
//! @return true if closing went ok. false if the user canceled the operation.
//! @see closeAllModels()
bool ModelHandler::closeModel(int idx)
{
    // Only remove if we found the model by index
    ModelWidget *pModelToClose = getModel(idx);
    if(pModelToClose)
    {
        if (!pModelToClose->isSaved())
        {
            QString modelName = pModelToClose->getTopLevelSystem()->getName();
            QMessageBox msgBox;
            msgBox.setWindowIcon(gpMainWindow->windowIcon());
            msgBox.setText(QString("The model '").append(modelName).append("'  is not saved."));
            msgBox.setInformativeText("Do you want to save your changes before closing?");
            msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Save);

            int answer = msgBox.exec();
            switch (answer)
            {
            case QMessageBox::Save:
                // Save was clicked
                pModelToClose->save();
                break;
            case QMessageBox::Discard:
                // Don't Save was clicked
                break;
            case QMessageBox::Cancel:
                // Cancel was clicked
                return false;
            default:
                // should never be reached
                return false;
            }
        }


        if (pModelToClose->getTopLevelSystem()->getLogDataHandler()->hasOpenPlotCurves())
        {
            QMessageBox msgBox;
            msgBox.setWindowIcon(gpMainWindow->windowIcon());
            msgBox.setText(QString("All open plot curves from this model will be lost."));
            msgBox.setInformativeText("Are you sure you want to close?");
            msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Cancel);

            int answer = msgBox.exec();
            switch (answer)
            {
            case QMessageBox::Ok:
                // Ok was clicked
                getTopLevelSystem(idx)->getLogDataHandler()->closePlotsWithCurvesBasedOnOwnedData();
                break;
            case QMessageBox::Cancel:
                // Cancel was clicked
                return false;
            default:
                // should never be reached
                return false;
            }
        }

        // Disconnect signals
        disconnect(gpMainWindow->mpResetZoomAction,     SIGNAL(triggered()),    pModelToClose->getGraphicsView(),   SLOT(resetZoom()));
        disconnect(gpMainWindow->mpZoomInAction,        SIGNAL(triggered()),    pModelToClose->getGraphicsView(),   SLOT(zoomIn()));
        disconnect(gpMainWindow->mpZoomOutAction,       SIGNAL(triggered()),    pModelToClose->getGraphicsView(),   SLOT(zoomOut()));
        disconnect(gpMainWindow->mpPrintAction,         SIGNAL(triggered()),    pModelToClose->getGraphicsView(),   SLOT(print()));
        disconnect(gpMainWindow->mpExportPDFAction,     SIGNAL(triggered()),    pModelToClose->getGraphicsView(),   SLOT(exportToPDF()));
        disconnect(gpMainWindow->mpExportPNGAction,     SIGNAL(triggered()),    pModelToClose->getGraphicsView(),   SLOT(exportToPNG()));
        disconnect(gpMainWindow->mpCenterViewAction,    SIGNAL(triggered()),    pModelToClose->getGraphicsView(),   SLOT(centerView()));

        disconnect(gpMainWindow,                                SIGNAL(simulateKeyPressed()),   pModelToClose,  SLOT(simulate()));
        disconnect(gpMainWindow->mpSaveAction,                  SIGNAL(triggered()),            pModelToClose,  SLOT(save()));
        disconnect(gpMainWindow->mpExportModelParametersAction, SIGNAL(triggered()),            pModelToClose,  SLOT(exportModelParameters()));

        getContainer(idx)->unmakeMainWindowConnectionsAndRefresh();

        // Deactivate Undo to prevent each component from registering it being deleted in the undo stack
        getContainer(idx)->setUndoEnabled(false, true);

        // Delete model tab if any
        gpMainWindow->mpCentralTabs->removeTab(gpMainWindow->mpCentralTabs->indexOf(pModelToClose));

        // Remove and delete the model
        mModelPtrs.removeAt(idx);
        delete pModelToClose;

        // When a model widget is removed all previous indexes for later models will become incorrect,
        // lets set the new current to the latest in that case
        if (mCurrentIdx >= idx)
        {
            --mCurrentIdx;
        }

        // Refresh toolbar connections if tab has been changed
        gpMainWindow->updateToolBarsToNewTab();

        // We are done removing the model widget
        return true;
    }
    return false;
}


//! @brief Closes all opened projects.
//! @return true if closing went ok. false if the user canceled the operation.
//! @see closeModel(int index)
//! @see saveModel()
bool ModelHandler::closeAllModels()
{
    gConfig.clearLastSessionModels();

    while(mModelPtrs.size() > 0)
    {
        mCurrentIdx = mModelPtrs.size()-1;
        gConfig.addLastSessionModel(getCurrentTopLevelSystem()->getModelFileInfo().filePath());
        if (!closeModel(mModelPtrs.size()-1))
        {
            return false;
        }
    }
    return true;
}


void ModelHandler::modelChanged()
{
    //! @todo We might need to change this
    //if(mModelPtrs.size() > 0) { gpMainWindow->mpCentralTabs->show(); }
    //else { gpMainWindow->mpCentralTabs->hide(); }

    mCurrentIdx = -1;
    for(int i=0; i<mModelPtrs.size(); ++i)
    {
        if(mModelPtrs[i] == gpMainWindow->mpCentralTabs->currentWidget())
        {
            mCurrentIdx=i;
        }
    }

    for(int i=0; i<mModelPtrs.size(); ++i)
    {
        //If you add a disconnect here, remember to also add it to the close tab function!
        //! @todo  Are these connections such connection that are supposed to be permanent conections? otherwise they should be in the disconnectMainWindowActions function
        disconnect(gpMainWindow->mpResetZoomAction,       SIGNAL(triggered()),        getModel(i)->getGraphicsView(),  SLOT(resetZoom()));
        disconnect(gpMainWindow->mpZoomInAction,          SIGNAL(triggered()),        getModel(i)->getGraphicsView(),  SLOT(zoomIn()));
        disconnect(gpMainWindow->mpZoomOutAction,         SIGNAL(triggered()),        getModel(i)->getGraphicsView(),  SLOT(zoomOut()));
        disconnect(gpMainWindow->mpPrintAction,           SIGNAL(triggered()),        getModel(i)->getGraphicsView(),  SLOT(print()));
        disconnect(gpMainWindow->mpExportPDFAction,       SIGNAL(triggered()),        getModel(i)->getGraphicsView(),  SLOT(exportToPDF()));
        disconnect(gpMainWindow->mpExportPNGAction,       SIGNAL(triggered()),        getModel(i)->getGraphicsView(),  SLOT(exportToPNG()));
        disconnect(gpMainWindow->mpCenterViewAction,      SIGNAL(triggered()),        getModel(i)->getGraphicsView(),  SLOT(centerView()));

        getContainer(i)->unmakeMainWindowConnectionsAndRefresh();

        //disconnect(gpMainWindow,                    SIGNAL(simulateKeyPressed()),   getModel(i),  SLOT(simulate()));
        disconnect(gpMainWindow,                        SIGNAL(simulateKeyPressed()),   getModel(i),  SLOT(simulate_nonblocking()));
        disconnect(gpMainWindow->mpCoSimulationAction,  SIGNAL(triggered()),            getModel(i),  SLOT(startCoSimulation()));
        disconnect(gpMainWindow->mpSaveAction,          SIGNAL(triggered()),            getModel(i),  SLOT(save()));
        disconnect(gpMainWindow->mpSaveAsAction,        SIGNAL(triggered()),            getModel(i),  SLOT(saveAs()));
        disconnect(gpMainWindow->mpExportModelParametersAction,   SIGNAL(triggered()),            getModel(i),  SLOT(exportModelParameters()));
    }
    if(this->mModelPtrs.size() != 0 && getCurrentModel())
    {
        //connect(gpMainWindow,                       SIGNAL(simulateKeyPressed()),   getCurrentModel(),        SLOT(simulate()), Qt::UniqueConnection);
        connect(gpMainWindow,                                   SIGNAL(simulateKeyPressed()),   getCurrentModel(),    SLOT(simulate_nonblocking()), Qt::UniqueConnection);
        connect(gpMainWindow->mpCoSimulationAction,             SIGNAL(triggered()),            getCurrentModel(),    SLOT(startCoSimulation()), Qt::UniqueConnection);
        connect(gpMainWindow->mpSaveAction,                     SIGNAL(triggered()),            getCurrentModel(),    SLOT(save()), Qt::UniqueConnection);
        connect(gpMainWindow->mpSaveAsAction,                   SIGNAL(triggered()),            getCurrentModel(),    SLOT(saveAs()), Qt::UniqueConnection);
        connect(gpMainWindow->mpExportModelParametersAction,    SIGNAL(triggered()),            getCurrentModel(),    SLOT(exportModelParameters()), Qt::UniqueConnection);

        connect(gpMainWindow->mpResetZoomAction,    SIGNAL(triggered()),    getCurrentModel()->getGraphicsView(),    SLOT(resetZoom()), Qt::UniqueConnection);
        connect(gpMainWindow->mpZoomInAction,       SIGNAL(triggered()),    getCurrentModel()->getGraphicsView(),    SLOT(zoomIn()), Qt::UniqueConnection);
        connect(gpMainWindow->mpZoomOutAction,      SIGNAL(triggered()),    getCurrentModel()->getGraphicsView(),    SLOT(zoomOut()), Qt::UniqueConnection);
        connect(gpMainWindow->mpPrintAction,        SIGNAL(triggered()),    getCurrentModel()->getGraphicsView(),    SLOT(print()), Qt::UniqueConnection);
        connect(gpMainWindow->mpExportPDFAction,    SIGNAL(triggered()),    getCurrentModel()->getGraphicsView(),    SLOT(exportToPDF()), Qt::UniqueConnection);
        connect(gpMainWindow->mpExportPNGAction,    SIGNAL(triggered()),    getCurrentModel()->getGraphicsView(),    SLOT(exportToPNG()), Qt::UniqueConnection);
        connect(gpMainWindow->mpCenterViewAction,   SIGNAL(triggered()),    getCurrentModel()->getGraphicsView(),    SLOT(centerView()), Qt::UniqueConnection);

        getCurrentContainer()->makeMainWindowConnectionsAndRefresh();

        getCurrentContainer()->updateMainWindowButtons();
        getCurrentModel()->setToolBarSimulationTimeParametersFromTab();


        if(gpMainWindow->mpLibrary->mGfxType != getCurrentTopLevelSystem()->getGfxType())
        {
            gpMainWindow->mpLibrary->setGfxType(getCurrentTopLevelSystem()->getGfxType());
        }

        gpMainWindow->mpToggleNamesAction->setChecked(!getCurrentContainer()->areSubComponentNamesHidden());
        gpMainWindow->mpTogglePortsAction->setChecked(!getCurrentContainer()->areSubComponentPortsHidden());
        gpMainWindow->mpShowLossesAction->setChecked(getCurrentContainer()->areLossesVisible());
    }
}

void ModelHandler::saveState()
{
    mStateInfoBackupList.clear();
    mStateInfoHasChanged.clear();
    mStateInfoHmfList.clear();
    mStateInfoModels.clear();
    mStateInfoTabNames.clear();
    mStateInfoLogDataHandlersList.clear();

    while(!mModelPtrs.isEmpty())
    {
        ModelWidget *pModel = getModel(0);
        mStateInfoHmfList << pModel->getTopLevelSystem()->getModelFileInfo().filePath();
        mStateInfoHasChanged << !pModel->isSaved();
        mStateInfoTabNames << gpMainWindow->mpCentralTabs->tabText(gpMainWindow->mpCentralTabs->indexOf(pModel));
        pModel->getTopLevelSystem()->getLogDataHandler()->setParent(0);       //Make sure it is not removed when deleting the container object
        mStateInfoLogDataHandlersList << pModel->getTopLevelSystem()->getLogDataHandler();
        if(!pModel->isSaved())
        {
            //! @todo This code is duplicated from ModelWidget::saveModel(), make it a common function somehow
                //Save xml document
            QDomDocument domDocument;
            QDomElement hmfRoot = appendHMFRootElement(domDocument, HMF_VERSIONNUM, HOPSANGUIVERSION, getHopsanCoreVersion());
            pModel->getTopLevelSystem()->saveToDomElement(hmfRoot);
            QString fileNameWithoutHmf = getCurrentTopLevelSystem()->getModelFileInfo().fileName();
            fileNameWithoutHmf.chop(4);
            mStateInfoBackupList << gDesktopHandler.getBackupPath()+fileNameWithoutHmf+"_savedstate.hmf";
            QFile xmlhmf(gDesktopHandler.getBackupPath()+fileNameWithoutHmf+"_savedstate.hmf");
            if (!xmlhmf.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
            {
                return;
            }
            QTextStream out(&xmlhmf);
            appendRootXMLProcessingInstruction(domDocument); //The xml "comment" on the first line
            domDocument.save(out, XMLINDENTATION);
            xmlhmf.close();
            pModel->setSaved(true);
            closeModel(0);
            //pTab->close();
        }
        else
        {
            mStateInfoBackupList << "";
            closeModel(0);
            //pTab->close();
        }
    }
}

void ModelHandler::restoreState()
{
    for(int i=0; i<mStateInfoHmfList.size(); ++i)
    {
        if(mStateInfoHasChanged[i])
        {
            loadModel(mStateInfoBackupList[i]);
            getCurrentModel()->hasChanged();
            getCurrentTopLevelSystem()->setModelFile(mStateInfoHmfList[i]);
            QString basePath = QFileInfo(mStateInfoHmfList[i]).absolutePath();
            QStringListIterator objIt(getCurrentTopLevelSystem()->getModelObjectNames());
            while (objIt.hasNext())
            {
                getCurrentTopLevelSystem()->getModelObject(objIt.next())->getAppearanceData()->setBasePath(basePath);
            }
        }
        else
        {
            loadModel(mStateInfoHmfList[i]);
        }
        gpMainWindow->mpCentralTabs->setTabText(i, mStateInfoTabNames[i]);
        getCurrentTopLevelSystem()->setLogDataHandler(mStateInfoLogDataHandlersList[i]);
    }
}

void ModelHandler::createLabviewWrapperFromCurrentModel()
{
    qobject_cast<SystemContainer*>(getCurrentContainer())->exportToLabView();
}


void ModelHandler::exportCurrentModelToFMU()
{
    qobject_cast<SystemContainer*>(getCurrentContainer())->exportToFMU();
}


void ModelHandler::exportCurrentModelToSimulink()
{
    qobject_cast<SystemContainer*>(getCurrentContainer())->exportToSimulink();
}


void ModelHandler::exportCurrentModelToSimulinkCoSim()
{
    qobject_cast<SystemContainer*>(getCurrentContainer())->exportToSimulinkCoSim();
}


void ModelHandler::showLosses(bool show)
{
    qobject_cast<SystemContainer*>(getCurrentContainer())->showLosses(show);
}


void ModelHandler::measureSimulationTime()
{
    qobject_cast<SystemContainer*>(getCurrentContainer())->measureSimulationTime();
}


void ModelHandler::launchDebugger()
{
    if(getCurrentTopLevelSystem() == 0) return;

    DebuggerWidget *pDebugger = new DebuggerWidget(getCurrentTopLevelSystem(), gpMainWindow);
    pDebugger->show();
    pDebugger->exec();
}


void ModelHandler::openAnimation()
{
    if(!mModelPtrs.isEmpty())
    {
        getCurrentModel()->openAnimation();
    }
}


bool ModelHandler::simulateAllOpenModels_nonblocking(bool modelsHaveNotChanged)
{
    if(!mModelPtrs.isEmpty())
    {
        //All systems will use start time, stop time and time step from this system
        SystemContainer *pMainSystem = getCurrentTopLevelSystem();

            //Setup simulation parameters
        double startTime = getCurrentModel()->getStartTime().toDouble();
        double stopTime = getCurrentModel()->getStopTime().toDouble();
        size_t nSamples = pMainSystem->getNumberOfLogSamples();

        // Ask core to initialize simulation
        QVector<SystemContainer*> systemsVector;
        for(int i=0; i<mModelPtrs.size(); ++i)
        {
            systemsVector.append(getTopLevelSystem(i));
        }

        mpSimulationThreadHandler->setSimulationTimeVariables(startTime, stopTime, nSamples);
        mpSimulationThreadHandler->initSimulateFinalize(systemsVector, modelsHaveNotChanged);

        //! @todo fix return code (maybe remove)
        return true;
    }
    return false;
}


bool ModelHandler::simulateAllOpenModels_blocking(bool modelsHaveNotChanged)
{
    if(!mModelPtrs.isEmpty())
    {
        //All systems will use start time, stop time and time step from this system
        SystemContainer *pMainSystem = getCurrentTopLevelSystem();

            //Setup simulation parameters
        double startTime = getCurrentModel()->getStartTime().toDouble();
        double stopTime = getCurrentModel()->getStopTime().toDouble();
        size_t nSamples = pMainSystem->getNumberOfLogSamples();

        // Ask core to initialize simulation
        QVector<SystemContainer*> systemsVector;
        for(int i=0; i<mModelPtrs.size(); ++i)
        {
            systemsVector.append(getTopLevelSystem(i));
        }

        mpSimulationThreadHandler->setSimulationTimeVariables(startTime, stopTime, nSamples);
        mpSimulationThreadHandler->setProgressDilaogBehaviour(true, false);
        mpSimulationThreadHandler->initSimulateFinalize_blocking(systemsVector, modelsHaveNotChanged);

        //! @todo fix return code (maybe remove)
        return true;
    }
    return false;
}

