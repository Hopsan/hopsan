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
#include "GraphicsView.h"
#include "GUIObjects/GUISystem.h"
#include "LogDataHandler.h"
#include "MainWindow.h"
#include "ModelHandler.h"
#include "version_gui.h"
#include "Widgets/DebuggerWidget.h"
#include "Widgets/HcomWidget.h"
#include "Widgets/LibraryWidget.h"
#include "Widgets/ProjectTabWidget.h"

ModelHandler::ModelHandler(QObject *parent)
    : QObject(parent)
{
    mNumberOfUntitledModels=0;
    mpProjectTabWidget = gpMainWindow->mpProjectTabs;
}

void ModelHandler::addModelWidget(ProjectTab *pModelWidget, QString name)
{
    pModelWidget->setParent(mpProjectTabWidget);    //! @todo Should probably use ModelHandler as parent

    mpProjectTabWidget->addTab(pModelWidget, name);
    mpProjectTabWidget->setCurrentWidget(pModelWidget);

    pModelWidget->setToolBarSimulationTimeParametersFromTab();

    emit newModelWidgetAdded();
}


//! @brief Adds a ProjectTab object (a new tab) to itself.
//! @see closeProjectTab(int index)
void ModelHandler::addNewProjectTab(QString modelName)
{
    modelName.append(QString::number(mNumberOfUntitledModels));

    ProjectTab *newTab = new ProjectTab(mpProjectTabWidget);    //! @todo Should probably use ModelHandler as parent
    newTab->getTopLevelSystem()->setName(modelName);

    addModelWidget(newTab, modelName);

    newTab->setToolBarSimulationTimeParametersFromTab();
    newTab->setSaved(true);
    mNumberOfUntitledModels += 1;
}

ProjectTab *ModelHandler::getModel(int idx)
{
    if(idx > 0 && idx < mModelPtrs.size()-1)
    {
        return 0;
    }
    return mModelPtrs[idx];
}


ProjectTab *ModelHandler::getCurrentModel()
{
    if(mModelPtrs.isEmpty())
    {
        return 0;
    }
    return mModelPtrs[mCurrentIdx];
}

SystemContainer *ModelHandler::getTopLevelSystem(int idx)
{
    if(idx > 0 && idx < mModelPtrs.size()-1)
    {
        return 0;
    }
    return mModelPtrs[idx]->getTopLevelSystem();
}

SystemContainer *ModelHandler::getCurrentTopLevelSystem()
{
    if(mModelPtrs.isEmpty())
    {
        return 0;
    }
    return mModelPtrs[mCurrentIdx]->getTopLevelSystem();
}

ContainerObject *ModelHandler::getContainer(int idx)
{
    if(idx > 0 && idx < mModelPtrs.size()-1)
    {
        return 0;
    }
    return mModelPtrs[idx]->getGraphicsView()->getContainerPtr();
}

ContainerObject *ModelHandler::getCurrentContainer()
{
    if(mModelPtrs.isEmpty())
    {
        return 0;
    }
    return mModelPtrs[mCurrentIdx]->getGraphicsView()->getContainerPtr();
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


//! @brief Loads a model from a file and opens it in a new project tab.
//! @param modelFileName is the path to the loaded file
//! @see loadModel()
//! @see saveModel(saveTarget saveAsFlag)
void ModelHandler::loadModel(QString modelFileName, bool ignoreAlreadyOpen)
{
    //! @todo maybe  write utility function that opens filel checks existance and sets fileinfo
    QFile file(modelFileName);   //Create a QFile object
    if(!file.exists())
    {
        qDebug() << "File not found: " + file.fileName();
        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("File not found: " + file.fileName());
        return;
    }
    QFileInfo fileInfo(file);

    //Make sure file not already open
    if(!ignoreAlreadyOpen)
    {
        for(int t=0; t!=mModelPtrs.size(); ++t)
        {
            if(this->getTopLevelSystem(t)->getModelFileInfo().filePath() == fileInfo.filePath())
            {
                QMessageBox::information(gpMainWindow, tr("Error"), tr("Unable to load model. File is already open."));
                return;
            }
        }
    }

    gpMainWindow->registerRecentModel(fileInfo);

    this->addModelWidget(new ProjectTab(/*this*/), fileInfo.baseName());
    ProjectTab *pCurrentTab = this->getCurrentModel();
    pCurrentTab->getTopLevelSystem()->getCoreSystemAccessPtr()->addSearchPath(fileInfo.absoluteDir().absolutePath());
    pCurrentTab->getTopLevelSystem()->setUndoEnabled(false, true);

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

        pCurrentTab->getTopLevelSystem()->setModelFileInfo(file); //Remember info about the file from which the data was loaded
        pCurrentTab->getTopLevelSystem()->setAppearanceDataBasePath(pCurrentTab->getTopLevelSystem()->getModelFileInfo().absolutePath());
        pCurrentTab->getTopLevelSystem()->loadFromDomElement(systemElement);

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
    pCurrentTab->setSaved(true);

    pCurrentTab->getTopLevelSystem()->setUndoEnabled(true, true);

    emit newModelWidgetAdded();
}

//! @brief Closes current project.
//! @param index defines which project to close.
//! @return true if closing went ok. false if the user canceled the operation.
//! @see closeAllProjectTabs()
bool ModelHandler::closeModel(int idx)
{
    if (!(this->getModel(idx)->isSaved()))
    {
        QString modelName;
        modelName = mpProjectTabWidget->tabText(idx);
        modelName.chop(1);
        QMessageBox msgBox;
        msgBox.setWindowIcon(gpMainWindow->windowIcon());
        msgBox.setText(QString("The model '").append(modelName).append("'").append(QString(" is not saved.")));
        msgBox.setInformativeText("Do you want to save your changes before closing?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);

        int answer = msgBox.exec();

        switch (answer)
        {
        case QMessageBox::Save:
            // Save was clicked
            getModel(idx)->save();
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


    if (getTopLevelSystem(idx)->getLogDataHandler()->hasOpenPlotCurves())
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

    //Disconnect signals
    //std::cout << "ProjectTabWidget: " << "Closing project: " << qPrintable(tabText(index)) << std::endl;
    //statusBar->showMessage(QString("Closing project: ").append(tabText(index)));
    disconnect(gpMainWindow->mpResetZoomAction,     SIGNAL(triggered()),    getModel(idx)->getGraphicsView(),   SLOT(resetZoom()));
    disconnect(gpMainWindow->mpZoomInAction,        SIGNAL(triggered()),    getModel(idx)->getGraphicsView(),   SLOT(zoomIn()));
    disconnect(gpMainWindow->mpZoomOutAction,       SIGNAL(triggered()),    getModel(idx)->getGraphicsView(),   SLOT(zoomOut()));
    disconnect(gpMainWindow->mpPrintAction,         SIGNAL(triggered()),    getModel(idx)->getGraphicsView(),   SLOT(print()));
    disconnect(gpMainWindow->mpExportPDFAction,     SIGNAL(triggered()),    getModel(idx)->getGraphicsView(),   SLOT(exportToPDF()));
    disconnect(gpMainWindow->mpExportPNGAction,     SIGNAL(triggered()),    getModel(idx)->getGraphicsView(),   SLOT(exportToPNG()));
    disconnect(gpMainWindow->mpCenterViewAction,    SIGNAL(triggered()),    getModel(idx)->getGraphicsView(),   SLOT(centerView()));

    disconnect(gpMainWindow,                                SIGNAL(simulateKeyPressed()),   getModel(idx),  SLOT(simulate()));
    disconnect(gpMainWindow->mpSaveAction,                  SIGNAL(triggered()),            getModel(idx),  SLOT(save()));
    disconnect(gpMainWindow->mpExportModelParametersAction, SIGNAL(triggered()),            getModel(idx),  SLOT(exportModelParameters()));

    getContainer(idx)->unmakeMainWindowConnectionsAndRefresh();

    getContainer(idx)->setUndoEnabled(false, true);  //This is necessary to prevent each component from registering it being deleted in the undo stack

    //Delete project tab, We dont need to call removeTab here, this seems to be handled automatically
    delete getModel(idx);

    return true;
}


//! @brief Closes all opened projects.
//! @return true if closing went ok. false if the user canceled the operation.
//! @see closeProjectTab(int index)
//! @see saveProjectTab()
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
    if(mModelPtrs.size() > 0) { mpProjectTabWidget->show(); }
    else { mpProjectTabWidget->hide(); }

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
    if(this->mModelPtrs.size() != 0)
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


void ModelHandler::launchDebugger()
{
    if(getCurrentTopLevelSystem() == 0) return;

    DebuggerWidget *pDebugger = new DebuggerWidget(getCurrentTopLevelSystem(), gpMainWindow);
    pDebugger->show();
    pDebugger->exec();
}
