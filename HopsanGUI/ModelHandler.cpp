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
//! @file   ModelHandler.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2013-05-16
//!
//! @brief Contains a model handler object
//!
//$Id$

#include <QFileDialog>
#include <QMessageBox>


//Hopsan includes
#include "common.h"
#include "global.h"
#include "Configuration.h"
#include "DesktopHandler.h"
#include "GraphicsView.h"
#include "GraphicsViewPort.h"
#include "GUIObjects/GUIContainerObject.h"
#include "MessageHandler.h"
#include "MainWindow.h"
#include "ModelHandler.h"
#include "SimulationThreadHandler.h"
#include "version_gui.h"
#include "Widgets/DebuggerWidget.h"
#include "MessageHandler.h"
#include "Widgets/FindWidget.h"
#include "Widgets/LibraryWidget.h"
#include "Widgets/ModelWidget.h"
#include "Widgets/ProjectTabWidget.h"
#include "Widgets/DataExplorer.h"
#include "Widgets/PlotWidget2.h"
#include "Widgets/TextEditorWidget.h"
#include "Utilities/GUIUtilities.h"

#ifdef USEZMQ
#include "RemoteSimulationUtils.h"
#endif

ModelHandler::ModelHandler(QObject *parent)
    : QObject(parent)
{
    mpDebugger = 0;

    mNumberOfUntitledModels=0;
    mNumberOfUntitledScripts=0;

    mpSimulationThreadHandler = new SimulationThreadHandler(); //!< @todo is this ever deleted

    mCurrentIdx = -1;

    mpFileWatcher = new QFileSystemWatcher(gpMainWindowWidget);

    connect(this, SIGNAL(checkMessages()),      gpMessageHandler,    SLOT(collectHopsanCoreMessages()), Qt::UniqueConnection);
}

void ModelHandler::addModelWidget(ModelWidget *pModelWidget, const QString &name, LoadOptions options)
{
    pModelWidget->setParent(gpCentralTabWidget);    //! @todo Should probably use ModelHandler as parent

    connect(pModelWidget->getTopLevelSystemContainer()->getLogDataHandler().data(), SIGNAL(dataAddedFromModel(bool)), gpMainWindow->mpShowLossesAction, SLOT(setEnabled(bool)));

    // If the Modelwidget should not be detatched then add it as a tab and switch to that tab
    if(!options.testFlag(Detatched))
    {
        mModelPtrs.append(pModelWidget);
        mCurrentIdx = mModelPtrs.size()-1;
        gpCentralTabWidget->setCurrentIndex(gpCentralTabWidget->insertTab(mCurrentIdx+1, pModelWidget, name));
        emit newModelWidgetAdded();
        emit modelChanged(pModelWidget);
    }
}


//! @brief Adds a ModelWidget object (a new tab) to itself.
//! @see closeModel(int index)
ModelWidget *ModelHandler::addNewModel(QString modelName, LoadOptions options)
{
    modelName.append(QString::number(mNumberOfUntitledModels));

    ModelWidget *pNewModelWidget = new ModelWidget(this,gpCentralTabWidget);    //! @todo Should probably use ModelHandler as parent
    pNewModelWidget->getTopLevelSystemContainer()->setName(modelName);

    connect(pNewModelWidget->getTopLevelSystemContainer()->getLogDataHandler().data(), SIGNAL(dataAddedFromModel(bool)), gpMainWindow->mpShowLossesAction, SLOT(setEnabled(bool)));

    addModelWidget(pNewModelWidget, modelName, options);

    pNewModelWidget->setSaved(true);
    mNumberOfUntitledModels += 1;

    return pNewModelWidget;
}


void ModelHandler::setCurrentModel(int idx)
{
    if ( (idx>=0) && (idx<mModelPtrs.size()) )
    {
        mCurrentIdx = idx;
        // Also switch tab if it is visible among the tabs
        if(gpCentralTabWidget->indexOf(mModelPtrs[idx]) != -1)
            gpCentralTabWidget->setCurrentWidget(mModelPtrs[idx]);
        refreshMainWindowConnections();
    }
}


void ModelHandler::setCurrentModel(ModelWidget *pWidget)
{
    setCurrentModel(mModelPtrs.indexOf(pWidget));
}

ModelWidget *ModelHandler::getModel(const QString &rModelFilePath)
{
    for (int i=0; i<mModelPtrs.size(); ++i)
    {
        if (mModelPtrs[i]->getTopLevelSystemContainer()->getModelFilePath() == rModelFilePath)
        {
            return mModelPtrs[i];
        }
    }
    return 0;
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
    return getModel(mCurrentIdx);
}

SystemObject *ModelHandler::getTopLevelSystem(const QString &rModelFilePath)
{
    ModelWidget *pMW = getModel(rModelFilePath);
    if (pMW)
    {
        return pMW->getTopLevelSystemContainer();
    }
    return 0;
}

SystemObject *ModelHandler::getTopLevelSystem(int idx)
{
    ModelWidget *pMW = getModel(idx);
    if (pMW)
    {
        return pMW->getTopLevelSystemContainer();
    }
    return 0;
}

SystemObject *ModelHandler::getCurrentTopLevelSystem()
{
    ModelWidget *pMW = getCurrentModel();
    if (pMW)
    {
        return pMW->getTopLevelSystemContainer();
    }
    return 0;
}

SystemObject *ModelHandler::getViewContainerObject(int idx)
{
    ModelWidget *pMW = getModel(idx);
    if (pMW)
    {
        return pMW->getViewContainerObject();
    }
    return 0;
}

SystemObject *ModelHandler::getCurrentViewContainerObject()
{
    ModelWidget *pMW = getCurrentModel();
    if (pMW)
    {
        return pMW->getViewContainerObject();
    }
    return 0;
}

QSharedPointer<LogDataHandler2> ModelHandler::getCurrentLogDataHandler()
{
    ModelWidget *pMW = getCurrentModel();
    if (pMW)
    {
        return pMW->getLogDataHandler();
    }
    return QSharedPointer<LogDataHandler2>(nullptr);
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
    QStringList modelFileNames = QFileDialog::getOpenFileNames(gpMainWindowWidget, tr("Choose Model File"),
                                                               gpConfig->getStringSetting(cfg::dir::loadmodel),
                                                         tr("Hopsan Model Files (*.hmf *.xml)"));
    for(const QString &modelFileName : modelFileNames) {
        loadModel(modelFileName);
        QFileInfo fileInfo = QFileInfo(modelFileName);
        gpConfig->setStringSetting(cfg::dir::loadmodel, fileInfo.absolutePath());
    }
}


//! @brief Loads a HCOM script from a file and opens it in a new project tab.
//! @see loadScriptFile(QString scriptFileName)
void ModelHandler::loadTextFile()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(gpMainWindowWidget, tr("Choose Text File"),
                                                          gpConfig->getStringSetting(cfg::dir::loadscript),
                                                                tr("All Supported Files (*.hcom *.hpp *.h *.cpp *.cc *.c *.xml *.mo *.py);;HCOM Script Files (*.hcom);;C++ Header Files (*.hpp *.h);;C++ Source Files (*.cpp *.cc *.c);;XML files (*.xml);;Modelica files (*.mo);;Python Script Files (*.py)"));
    for(const auto &fileName : fileNames)
    {
        QFileInfo fileInfo = QFileInfo(fileName);

        // Make sure file not already open
        for(auto &editor : mTextEditors)
        {
            if(editor->getFileInfo() == fileInfo)
            {
                QMessageBox::information(gpMainWindowWidget, tr("Error"), tr("Unable to load text file. File is already open."));
                continue;
            }
        }

        loadTextFile(fileName);
        gpConfig->setStringSetting(cfg::dir::loadscript, fileInfo.absolutePath());
    }
}


//! @brief Help function that loads a model from the text in a QAction object.
//! Used to facilitate recent models function.
void ModelHandler::loadModel(QAction *action)
{
    loadModel(action->text());
}


//! @brief Creates a new empty HCOM script and opens it in a new project tab
//! @see loadScriptFile()
void ModelHandler::newTextFile()
{
    TextEditorWidget *pNewEditor = new TextEditorWidget(QFileInfo(), HighlighterTypeEnum::Hcom, gpCentralTabWidget);
    gpCentralTabWidget->addTab(pNewEditor, "HcomScript"+QString::number(mNumberOfUntitledScripts));
    gpCentralTabWidget->setCurrentWidget(pNewEditor);
    mTextEditors.append(pNewEditor);
    mNumberOfUntitledScripts++;
}


void ModelHandler::loadModelParametersFromHpf()
{
    auto pModel = getCurrentModel();
    if (pModel) {
        pModel->importModelParametersFromHpf();
    }
}

void ModelHandler::loadModelParametersFromSsv()
{
    auto pModel = getCurrentModel();
    if (pModel) {
        pModel->importModelParametersFromSsv();
    }
}


//! @brief Loads a model from a file and opens it in a new project tab.
//! @param modelFileName is the path to the loaded file
//! @see loadModel()
//! @see saveModel(saveTarget saveAsFlag)
ModelWidget *ModelHandler::loadModel(QString modelFileName, LoadOptions options)
{
    //! @todo maybe  write utility function that opens file, checks existence and sets fileinfo
    QFile modelFile(modelFileName);   //Create a QFile object
    if(!modelFile.exists()) {
        gpMessageHandler->addErrorMessage("File not found: " + modelFile.fileName());
        return 0;
    }
    QFileInfo modelFileInfo(modelFile);

    // Make sure file not already open
    if(!options.testFlag(IgnoreAlreadyOpen)) {
        for(int t=0; t!=mModelPtrs.size(); ++t)
        {
            if(this->getTopLevelSystem(t)->getModelFileInfo().filePath() == modelFileInfo.filePath() && gpCentralTabWidget->indexOf(mModelPtrs[t]) > -1)
            {
                QMessageBox::information(gpMainWindowWidget, tr("Error"), tr("Unable to load model. File is already open."));
                return nullptr;
            }
        }
    }

    ModelWidget *pNewModel = new ModelWidget(this, gpCentralTabWidget);
    connect(pNewModel->getSimulationThreadHandler(), SIGNAL(startSimulation()), gpMainWindow, SLOT(hideSimulateButton()));
    connect(pNewModel->getSimulationThreadHandler(), SIGNAL(done(bool)), gpMainWindow, SLOT(showSimulateButton()));

    if(!options.testFlag(Detatched)) {
        gpMessageHandler->addInfoMessage("Loading model: "+modelFileInfo.absoluteFilePath());
    }

   addModelWidget(pNewModel, modelFileInfo.baseName(), options);
   bool loadOK = pNewModel->loadModel(modelFile);
   if (loadOK) {
       emit newModelWidgetAdded();
       if(!options.testFlag(Detatched)) {
           emit modelChanged(pNewModel);
           if(!options.testFlag(DontAddToRecentModels)) {
                gpConfig->addRecentModel(modelFileInfo.filePath());
           }
       }
       return pNewModel;
   }
   else
   {
        closeModel(pNewModel);
        return nullptr;
   }
}


//! @brief Loads a model from a file and opens it in a new project tab.
//! @param modelFileName is the path to the loaded file
//! @see loadModel()
//! @see saveModel(saveTarget saveAsFlag)
TextEditorWidget *ModelHandler::loadTextFile(QString fileName)
{
    QFile file(fileName);
    if(!file.exists())
    {
        gpMessageHandler->addErrorMessage("File not found: " + file.fileName());
        return nullptr;
    }
    QFileInfo fileInfo(file);

    //Abort if file is already open
    for(const auto& editor : mTextEditors) {
        if(fileInfo == editor->getFileInfo()) {
            gpCentralTabWidget->setCurrentWidget(editor);
            return editor;
        }
    }

    mpFileWatcher->addPath(fileName);
    TextEditorWidget *pNewEditor = new TextEditorWidget(QFileInfo(fileName), highlighterForExtension(fileInfo.suffix()), gpCentralTabWidget);
    gpCentralTabWidget->addTab(pNewEditor, fileInfo.fileName());
    gpCentralTabWidget->setCurrentWidget(pNewEditor);
    mTextEditors.append(pNewEditor);

    connect(mpFileWatcher, SIGNAL(fileChanged(QString)), pNewEditor, SLOT(fileChanged(QString)));

    return pNewEditor;
}


bool ModelHandler::closeModelByTabIndex(int tabIdx, bool force)
{
    for(int i=0; i<mModelPtrs.size(); ++i)
    {
        // When found close and return, else return false when loop ends
        if(mModelPtrs[i] == gpCentralTabWidget->widget(tabIdx))
        {
            return closeModel(i,force);
        }
    }

    //Also check script editors
    for(auto &editor : mTextEditors)
    {
        if(editor == gpCentralTabWidget->widget(tabIdx))
        {
            mpFileWatcher->removePath(editor->getFileInfo().absoluteFilePath());
            return closeScript(mTextEditors.indexOf(editor), force);
        }
    }

    gpCentralTabWidget->removeTab(tabIdx);
    return false;
}


bool ModelHandler::closeModel(ModelWidget *pModel, bool force)
{
    return closeModel(mModelPtrs.indexOf(pModel), force);
}


//! @brief Closes current project.
//! @param idx defines which project to close.
//! @param force Do not ask for saving model before closing
//! @return true if closing went ok. false if the user canceled the operation.
//! @see closeAllModels()
bool ModelHandler::closeModel(int idx, bool force)
{
    // Only remove if we found the model by index
    ModelWidget *pModelToClose = getModel(idx);
    if(pModelToClose)
    {
        if (!pModelToClose->isSaved() && !force)
        {
            QString modelName = pModelToClose->getTopLevelSystemContainer()->getName();
            QMessageBox msgBox;
            msgBox.setWindowIcon(gpMainWindowWidget->windowIcon());
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


//        if (pModelToClose->getTopLevelSystemContainer()->getLogDataHandler()->hasOpenPlotCurves())
//        {
//            QMessageBox msgBox;
//            msgBox.setWindowIcon(gpMainWindow->windowIcon());
//            msgBox.setText(QString("All open plot curves from this model will be lost."));
//            msgBox.setInformativeText("Are you sure you want to close?");
//            msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
//            msgBox.setDefaultButton(QMessageBox::Cancel);

//            int answer = msgBox.exec();
//            switch (answer)
//            {
//            case QMessageBox::Ok:
//                // Ok was clicked
//                getTopLevelSystem(idx)->getLogDataHandler()->closePlotsWithCurvesBasedOnOwnedData();
//                break;
//            case QMessageBox::Cancel:
//                // Cancel was clicked
//                return false;
//            default:
//                // should never be reached
//                return false;
//            }
//        }

        // Disconnect signals
        disconnectMainWindowConnections(pModelToClose);
//        pModelToClose->getViewContainerObject()->unmakeMainWindowConnectionsAndRefresh();

//        // Deactivate Undo to prevent each component from registering it being deleted in the undo stack (waste of time)
//        pModelToClose->getViewContainerObject()->setUndoEnabled(false, true);

        // Disconnect and delete model tab if any
        int tabIdx = gpCentralTabWidget->indexOf(pModelToClose);
        if (tabIdx >= 0)
        {
            // Disconnect all signals from this, to prevent a model change will be triggered when the tab is removed
            disconnect(gpCentralTabWidget->widget(tabIdx));
            // Remove the tab
            gpCentralTabWidget->removeTab(tabIdx);
        }

        // Remove and delete the model
        delete pModelToClose; //!< @todo it is very important (right now) that we delete before remove and --mCurrentIdx, since the delete will cause (undowidget trying to refresh from current widget) we can not remove the widgets before it has been deleted because of this. This behavior is really BAD and should be fixed. The destructor indirectly requires the use of one self by triggering a function in the undo widget
        mModelPtrs.removeAt(idx);

        // When a model widget is removed all previous indexes for later models will become incorrect,
        // lets set the new current to the latest in that case
        if (mCurrentIdx >= idx)
        {
            --mCurrentIdx;
        }

        // Refresh mainwindow sig/slot connections and button status to the new current model
        refreshMainWindowConnections();

        // Refresh toolbar connections if tab has been changed
        gpMainWindow->updateToolBarsToNewTab();

        // We are done removing the model widget
        return true;
    }
    return false;
}

bool ModelHandler::closeScript(int idx, bool force)
{
    // Only remove if we found the model by index
    TextEditorWidget *pEditor = mTextEditors[idx];
    if(pEditor)
    {
        if (!pEditor->isSaved() && !force)
        {
            QString modelName = pEditor->getFileInfo().fileName();
            QMessageBox msgBox;
            msgBox.setWindowIcon(gpMainWindowWidget->windowIcon());
            msgBox.setText(QString("Script file '").append(modelName).append("'  is not saved."));
            msgBox.setInformativeText("Do you want to save your changes before closing?");
            msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Save);

            int answer = msgBox.exec();
            switch (answer)
            {
            case QMessageBox::Save:
                // Save was clicked
                pEditor->save();
                if(!pEditor->isSaved())
                    return false;       //Save ws aborted
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

        // Disconnect signals
        disconnectMainWindowConnections(pEditor);

        // Disconnect and delete model tab if any
        int tabIdx = gpCentralTabWidget->indexOf(pEditor);
        if (tabIdx >= 0)
        {
            // Disconnect all signals from this, to prevent a text change will be triggered when the tab is removed
            disconnect(gpCentralTabWidget->widget(tabIdx));
            // Remove the tab
            gpCentralTabWidget->removeTab(tabIdx);
        }

        // Remove and delete the script editor
        delete pEditor; //!< @todo it is very important (right now) that we delete before remove and --mCurrentIdx, since the delete will cause (undowidget trying to refresh from current widget) we can not remove the widgets before it has been deleted because of this. This behavior is really BAD and should be fixed. The destructor indirectly requires the use of one self by triggering a function in the undo widget
        mTextEditors.removeAt(idx);

        // Refresh mainwindow sig/slot connections and button status to the new current model
        refreshMainWindowConnections();

        // Refresh toolbar connections if tab has been changed
        gpMainWindow->updateToolBarsToNewTab();

        // We are done removing the model widget
        return true;
    }
    return false;
}

//! @brief Closes all opened projects.
//! @return true if closing went ok. false if the user canceled the operation.
//! @param force Do not ask for saving model before closing
//! @see closeModel(int index)
//! @see saveModel()
bool ModelHandler::closeAllModels(bool force)
{
    gpConfig->clearLastSessionModels();

    while(mModelPtrs.size() > 0)
    {
        gpConfig->addLastSessionModel(mModelPtrs.last()->getViewContainerObject()->getModelFileInfo().filePath());
        if (!closeModel(mModelPtrs.size()-1,force))
        {
            return false;
        }
    }
    return true;
}

void ModelHandler::selectModelByTabIndex(int tabIdx)
{
    // Default is to have no current model, in case we click a tab that is not actually a model
    mCurrentIdx = -1;
    // Find which model is selected (if any)
    for(int i=0; i<mModelPtrs.size(); ++i)
    {
        if(mModelPtrs[i] == gpCentralTabWidget->widget(tabIdx))
        {
            mCurrentIdx=i;
        }
    }
    refreshMainWindowConnections();
}


void ModelHandler::refreshMainWindowConnections()
{
    // First disconnect connections from all models
    for(int i=0; i<mModelPtrs.size(); ++i)
    {
        disconnectMainWindowConnections(getModel(i));
        getViewContainerObject(i)->unmakeMainWindowConnectionsAndRefresh();
    }
    for(TextEditorWidget *scriptEditor : mTextEditors)
    {
        disconnectMainWindowConnections(scriptEditor);
    }

    // Now refresh actions (we need to do this before reconnecting to avoid sending signals)
    ModelWidget *pCurrentModel = getCurrentModel();
    if (pCurrentModel)
    {
        gpMainWindow->mpToggleRemoteCoreSimAction->setChecked(pCurrentModel->getUseRemoteSimulationCore());
        gpMainWindow->mpDataExplorer->setLogdataHandler(pCurrentModel->getLogDataHandler().data());
        gpMainWindow->mpImportDataFileAction->setEnabled(pCurrentModel->getLogDataHandler().data() != nullptr);
        gpPlotWidget->setLogDataHandler(pCurrentModel->getLogDataHandler().data());
    }
    else
    {
        gpMainWindow->mpToggleRemoteCoreSimAction->setChecked(false);
        gpMainWindow->mpDataExplorer->setLogdataHandler(nullptr);
        gpMainWindow->mpImportDataFileAction->setEnabled(false);
        if (gpPlotWidget)
        {
            gpPlotWidget->setLogDataHandler(nullptr);
        }
    }
    // Now reestablish connections to new model
    if(pCurrentModel)
    {
        connectMainWindowConnections(pCurrentModel);
        getCurrentViewContainerObject()->makeMainWindowConnectionsAndRefresh();
        getCurrentViewContainerObject()->updateMainWindowButtons();

        setToolBarSimulationTimeFromTab(pCurrentModel);

        if(gpLibraryWidget->mGfxType != pCurrentModel->getTopLevelSystemContainer()->getGfxType())
        {
            gpLibraryWidget->setGfxType(pCurrentModel->getTopLevelSystemContainer()->getGfxType());
        }
    }
    TextEditorWidget *pTextEditor = qobject_cast<TextEditorWidget*>(gpMainWindow->mpCentralTabs->currentWidget());
    if(pTextEditor)
    {
        connectMainWindowConnections(pTextEditor);
        gpFindWidget->setTextEditor(pTextEditor);
    }

    emit modelChanged(pCurrentModel);
}

void ModelHandler::disconnectMainWindowConnections(ModelWidget *pModel)
{
    //! @todo  Are these connections such connection that are supposed to be permanent connections? otherwise they should be in the disconnectMainWindowActions function
    disconnect(gpMainWindow->mpResetZoomAction,     SIGNAL(triggered()),    pModel->getGraphicsView(),   SLOT(resetZoom()));
    disconnect(gpMainWindow->mpZoomInAction,        SIGNAL(triggered()),    pModel->getGraphicsView(),   SLOT(zoomIn()));
    disconnect(gpMainWindow->mpZoomOutAction,       SIGNAL(triggered()),    pModel->getGraphicsView(),   SLOT(zoomOut()));
    disconnect(gpMainWindow->mpPrintAction,         SIGNAL(triggered()),    pModel->getGraphicsView(),   SLOT(print()));
    disconnect(gpMainWindow->mpExportPDFAction,     SIGNAL(triggered()),    pModel->getGraphicsView(),   SLOT(exportToPDF()));
    disconnect(gpMainWindow->mpExportPNGAction,     SIGNAL(triggered()),    pModel->getGraphicsView(),   SLOT(exportToPNG()));
    disconnect(gpMainWindow->mpCenterViewAction,    SIGNAL(triggered()),    pModel->getGraphicsView(),   SLOT(centerView()));

    disconnect(gpMainWindow->mpSimulationTimeEdit,          SIGNAL(simulationTimeChanged(QString,QString,QString)), pModel, SLOT(setTopLevelSimulationTime(QString,QString,QString)));
    disconnect(pModel, SIGNAL(simulationTimeChanged(QString,QString,QString)), gpMainWindow->mpSimulationTimeEdit, SLOT(displaySimulationTime(QString,QString,QString)));

    disconnect(gpMainWindow,                                SIGNAL(simulateKeyPressed()),   pModel,  SLOT(simulate_nonblocking()));
    disconnect(gpMainWindow->mpToggleRemoteCoreSimAction,   SIGNAL(triggered(bool)),        pModel,  SLOT(setUseRemoteSimulation(bool)));
    disconnect(gpMainWindow->mpSaveAction,                  SIGNAL(triggered()),            pModel,  SLOT(save()));
    disconnect(gpMainWindow->mpSaveAsAction,                SIGNAL(triggered()),            pModel,  SLOT(saveAs()));
    disconnect(gpMainWindow->mpExportModelParametersActionToHpf, SIGNAL(triggered()),       pModel,  SLOT(exportModelParametersToHpf()));
    disconnect(gpMainWindow->mpExportSimulationStateAction, SIGNAL(triggered()),            pModel,  SLOT(exportSimulationStates()));

    connect(pModel,                                         SIGNAL(modelSaved(ModelWidget*)),           SIGNAL(modelChanged(ModelWidget*)));
}

void ModelHandler::disconnectMainWindowConnections(TextEditorWidget* pScriptEditor)
{
    disconnect(gpMainWindow->mpSaveAction,      SIGNAL(triggered()),    pScriptEditor, SLOT(save()));
    disconnect(gpMainWindow->mpSaveAndRunAction,  SIGNAL(triggered()),    pScriptEditor,  SLOT(saveAndRun()));
    disconnect(gpMainWindow->mpSaveAsAction,      SIGNAL(triggered()),    pScriptEditor, SLOT(saveAs()));
    disconnect(gpMainWindow->mpCutAction,      SIGNAL(triggered()),    pScriptEditor, SLOT(cut()));
    disconnect(gpMainWindow->mpCopyAction,      SIGNAL(triggered()),    pScriptEditor, SLOT(copy()));
    disconnect(gpMainWindow->mpPasteAction,      SIGNAL(triggered()),    pScriptEditor, SLOT(paste()));
    disconnect(gpMainWindow->mpUndoAction,      SIGNAL(triggered()),    pScriptEditor, SLOT(undo()));
    disconnect(gpMainWindow->mpRedoAction,      SIGNAL(triggered()),    pScriptEditor, SLOT(redo()));
    disconnect(gpMainWindow->mpZoomInAction,      SIGNAL(triggered()),    pScriptEditor, SLOT(zoomIn()));
    disconnect(gpMainWindow->mpZoomOutAction,      SIGNAL(triggered()),    pScriptEditor, SLOT(zoomOut()));
    disconnect(gpMainWindow->mpPrintAction,      SIGNAL(triggered()),    pScriptEditor, SLOT(print()));
}


void ModelHandler::connectMainWindowConnections(ModelWidget *pModel)
{
    //! @todo  Are these connections such connection that are supposed to be permanent connections? otherwise they should be in the disconnectMainWindowActions function
    connect(gpMainWindow->mpResetZoomAction,    SIGNAL(triggered()),    pModel->getGraphicsView(),    SLOT(resetZoom()), Qt::UniqueConnection);
    connect(gpMainWindow->mpZoomInAction,       SIGNAL(triggered()),    pModel->getGraphicsView(),    SLOT(zoomIn()), Qt::UniqueConnection);
    connect(gpMainWindow->mpZoomOutAction,      SIGNAL(triggered()),    pModel->getGraphicsView(),    SLOT(zoomOut()), Qt::UniqueConnection);
    connect(gpMainWindow->mpPrintAction,        SIGNAL(triggered()),    pModel->getGraphicsView(),    SLOT(print()), Qt::UniqueConnection);
    connect(gpMainWindow->mpExportPDFAction,    SIGNAL(triggered()),    pModel->getGraphicsView(),    SLOT(exportToPDF()), Qt::UniqueConnection);
    connect(gpMainWindow->mpExportPNGAction,    SIGNAL(triggered()),    pModel->getGraphicsView(),    SLOT(exportToPNG()), Qt::UniqueConnection);
    connect(gpMainWindow->mpCenterViewAction,   SIGNAL(triggered()),    pModel->getGraphicsView(),    SLOT(centerView()), Qt::UniqueConnection);

    connect(gpMainWindow->mpSimulationTimeEdit, SIGNAL(simulationTimeChanged(QString,QString,QString)), pModel, SLOT(setTopLevelSimulationTime(QString,QString,QString)), Qt::UniqueConnection);
    connect(pModel, SIGNAL(simulationTimeChanged(QString,QString,QString)), gpMainWindow->mpSimulationTimeEdit, SLOT(displaySimulationTime(QString,QString,QString)), Qt::UniqueConnection);

    connect(gpMainWindow,                                       SIGNAL(simulateKeyPressed()),   pModel,    SLOT(simulate_nonblocking()), Qt::UniqueConnection);
    connect(gpMainWindow->mpToggleRemoteCoreSimAction,          SIGNAL(triggered(bool)),        pModel,    SLOT(setUseRemoteSimulation(bool)), Qt::UniqueConnection);
    connect(gpMainWindow->mpSaveAction,                         SIGNAL(triggered()),            pModel,    SLOT(save()), Qt::UniqueConnection);
    connect(gpMainWindow->mpSaveAsAction,                       SIGNAL(triggered()),            pModel,    SLOT(saveAs()), Qt::UniqueConnection);
    connect(gpMainWindow->mpExportModelParametersActionToHpf,   SIGNAL(triggered()),            pModel,    SLOT(exportModelParametersToHpf()), Qt::UniqueConnection);
    connect(gpMainWindow->mpExportModelParametersActionToSsv,   SIGNAL(triggered()),            pModel,    SLOT(exportModelParametersToSsv()), Qt::UniqueConnection);
    connect(gpMainWindow->mpExportSimulationStateAction,        SIGNAL(triggered()),            pModel,    SLOT(exportSimulationStates()), Qt::UniqueConnection);

    connect(pModel,                                         SIGNAL(modelSaved(ModelWidget*)),           SIGNAL(modelChanged(ModelWidget*)));
}

void ModelHandler::connectMainWindowConnections(TextEditorWidget* pScriptEditor)
{
    connect(gpMainWindow->mpSaveAction,      SIGNAL(triggered()),    pScriptEditor, SLOT(save()));
    connect(gpMainWindow->mpSaveAndRunAction, SIGNAL(triggered()),    pScriptEditor, SLOT(saveAndRun()));
    connect(gpMainWindow->mpSaveAsAction,      SIGNAL(triggered()),    pScriptEditor, SLOT(saveAs()));
    connect(gpMainWindow->mpCutAction,      SIGNAL(triggered()),    pScriptEditor, SLOT(cut()));
    connect(gpMainWindow->mpCopyAction,      SIGNAL(triggered()),    pScriptEditor, SLOT(copy()));
    connect(gpMainWindow->mpPasteAction,      SIGNAL(triggered()),    pScriptEditor, SLOT(paste()));
    connect(gpMainWindow->mpUndoAction,      SIGNAL(triggered()),    pScriptEditor, SLOT(undo()));
    connect(gpMainWindow->mpRedoAction,      SIGNAL(triggered()),    pScriptEditor, SLOT(redo()));
    connect(gpMainWindow->mpZoomInAction,      SIGNAL(triggered()),    pScriptEditor, SLOT(zoomIn()));
    connect(gpMainWindow->mpZoomOutAction,      SIGNAL(triggered()),    pScriptEditor, SLOT(zoomOut()));
    connect(gpMainWindow->mpPrintAction,      SIGNAL(triggered()),    pScriptEditor, SLOT(print()));
}

//! @brief Help function to update the toolbar simulation time parameters from a tab
void ModelHandler::setToolBarSimulationTimeFromTab(ModelWidget *pModel)
{
    gpMainWindow->mpSimulationTimeEdit->displaySimulationTime(pModel->getStartTime(), pModel->getTimeStep(), pModel->getStopTime());
}

void ModelHandler::saveState()
{
    mStateInfoIndex = gpCentralTabWidget->currentIndex();
    mStateInfoList.clear();

    while(!mModelPtrs.isEmpty())
    {
        ModelStateInfo info;
        ModelWidget *pModel = getModel(0);
        info.modelFile = pModel->getTopLevelSystemContainer()->getModelFileInfo().filePath();
        info.hasChanged = !pModel->isSaved();
        info.tabName = gpCentralTabWidget->tabText(gpCentralTabWidget->indexOf(pModel));
        info.logDataHandler = pModel->getTopLevelSystemContainer()->getLogDataHandler();
        info.viewPort = pModel->getGraphicsView()->getViewPort();

        //! @todo This code is duplicated from ModelWidget::saveModel(), make it a common function somehow
        //Save xml document
        QDomDocument domDocument;
        QDomElement hmfRoot = appendHMFRootElement(domDocument, HMF_VERSIONNUM, HOPSANGUIVERSION, getHopsanCoreVersion());
        pModel->getTopLevelSystemContainer()->saveToDomElement(hmfRoot);
        QString fileNameWithoutHmf = pModel->getTopLevelSystemContainer()->getModelFileInfo().fileName();
        fileNameWithoutHmf.chop(4);
        info.backupFile = gpDesktopHandler->getBackupPath()+fileNameWithoutHmf+"_savedstate.hmf";

        appendRootXMLProcessingInstruction(domDocument); //The xml "comment" on the first line
        bool savedOK = saveXmlFile(info.backupFile, gpMessageHandler, [&](){return domDocument;});
        if(!savedOK) {
            return;
        }

        pModel->setSaved(true);
        closeModel(0);
        //pTab->close();

        mStateInfoList.append(info);
    }
}

void ModelHandler::restoreState()
{
    int numTextTabs = gpCentralTabWidget->count()-1;
    for(int i=0; i<mStateInfoList.size(); ++i)
    {
        ModelStateInfo info = mStateInfoList[i];

        loadModel(info.backupFile, DontAddToRecentModels);
        getCurrentModel()->hasChanged();
        getCurrentTopLevelSystem()->setModelFile(info.modelFile);
//        QString basePath = QFileInfo(info.modelFile).absolutePath();
//        QStringListIterator objIt(getCurrentTopLevelSystem()->getModelObjectNames());
//            while (objIt.hasNext())
//            {
//                //getCurrentTopLevelSystem()->getModelObject(objIt.next())->getAppearanceData()->setBasePath(basePath);
//            }

        getCurrentModel()->setLogDataHandler(info.logDataHandler);
        info.logDataHandler.clear();

        getCurrentModel()->getGraphicsView()->setViewPort(info.viewPort);
        if(mModelPtrs.size() < i+1)
        {
            addNewModel();
        }
        gpCentralTabWidget->setCurrentIndex(1+i);
        this->mCurrentIdx = numTextTabs+i;
        gpCentralTabWidget->setTabText(1+i, info.tabName);

        //! @todo FIXA /Peter
//        getCurrentTopLevelSystem()->setLogDataHandler(mStateInfoLogDataHandlersList[i]);
//        mStateInfoLogDataHandlersList[i]->setParentContainerObject(getCurrentTopLevelSystem());
    }
    gpCentralTabWidget->setCurrentIndex(mStateInfoIndex);
    gpPlotWidget->updateList();
    mStateInfoList.clear();
}

void ModelHandler::revertCurrentModel()
{
    QMessageBox warningBox(QMessageBox::Warning, QObject::tr("Warning"),
                                QObject::tr("All changes made to current model or text file will be lost. Do you want to continue?"),
                                QMessageBox::NoButton, gpMainWindowWidget);
    warningBox.addButton("Yes", QMessageBox::AcceptRole);
    warningBox.addButton("No", QMessageBox::RejectRole);
    warningBox.setWindowIcon(gpMainWindowWidget->windowIcon());
    if(warningBox.exec() != QMessageBox::AcceptRole) {
        return;
    }

    ModelWidget *pModel = gpModelHandler->getCurrentModel();
    if (pModel)
    {
        pModel->revertModel();
        refreshMainWindowConnections();
    }

    TextEditorWidget *pTextEditor = qobject_cast<TextEditorWidget*>(gpMainWindow->mpCentralTabs->currentWidget());
    if(pTextEditor)
    {
        pTextEditor->reload();
    }
}

void ModelHandler::createLabviewWrapperFromCurrentModel()
{
    qobject_cast<SystemObject*>(getCurrentViewContainerObject())->exportToLabView();
}


void ModelHandler::exportCurrentModelToFMU1_32()
{
    qobject_cast<SystemObject*>(getCurrentViewContainerObject())->exportToFMU1_32();
}

void ModelHandler::exportCurrentModelToFMU1_64()
{
    qobject_cast<SystemObject*>(getCurrentViewContainerObject())->exportToFMU1_64();
}

void ModelHandler::exportCurrentModelToFMU2_32()
{
    qobject_cast<SystemObject*>(getCurrentViewContainerObject())->exportToFMU2_32();
}

void ModelHandler::exportCurrentModelToFMU2_64()
{
    qobject_cast<SystemObject*>(getCurrentViewContainerObject())->exportToFMU2_64();
}

void ModelHandler::exportCurrentModelToFMU3_32()
{
    qobject_cast<SystemObject*>(getCurrentViewContainerObject())->exportToFMU3_32();
}

void ModelHandler::exportCurrentModelToFMU3_64()
{
    qobject_cast<SystemObject*>(getCurrentViewContainerObject())->exportToFMU3_64();
}

void ModelHandler::exportCurrentModelToSimulink()
{
    qobject_cast<SystemObject*>(getCurrentViewContainerObject())->exportToSimulink();
}

void ModelHandler::exportCurrentModelToExe_32()
{
    qobject_cast<SystemObject*>(getCurrentViewContainerObject())->exportToExecutableModel("", ArchitectureEnumT::x86);
}

void ModelHandler::exportCurrentModelToExe_64()
{
    qobject_cast<SystemObject*>(getCurrentViewContainerObject())->exportToExecutableModel("", ArchitectureEnumT::x64);
}

void ModelHandler::showLosses(bool show)
{
    qobject_cast<SystemObject*>(getCurrentViewContainerObject())->showLosses(show);
}


void ModelHandler::measureSimulationTime()
{
    qobject_cast<SystemObject*>(getCurrentViewContainerObject())->measureSimulationTime();
}


void ModelHandler::launchDebugger()
{
    if(getCurrentTopLevelSystem() == 0) return;

    if(!mpDebugger || !mpDebugger->isVisible())
    {
        mpDebugger = new DebuggerWidget(getCurrentModel(), gpMainWindow);
        mpDebugger->show();
        mpDebugger->exec();
    }
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
        QVector<ModelWidget*> models = mModelPtrs.toVector();
        // Make sure that current model is first, as that will be used for simulation time settings
        int i = models.indexOf(getCurrentModel());
        if (i >= 0)
        {
            models.remove(i);
        }
        models.push_front(getCurrentModel());
        return simulateMultipleModels_nonblocking(models, modelsHaveNotChanged);
    }
    return false;
}


bool ModelHandler::simulateAllOpenModels_blocking(bool modelsHaveNotChanged)
{
    if(!mModelPtrs.isEmpty())
    {
        QVector<ModelWidget*> models = mModelPtrs.toVector();
        // Make sure that current model is first, as that will be used for simulation time settings
        int i = models.indexOf(getCurrentModel());
        if (i >= 0)
        {
            models.remove(i);
        }
        models.push_front(getCurrentModel());
        return simulateMultipleModels_blocking(models, modelsHaveNotChanged);
    }
    return false;
}



bool ModelHandler::simulateMultipleModels_nonblocking(QVector<ModelWidget*> models, bool noChange)
{
    if (!models.isEmpty())
    {
        // All systems will use start time, stop time and time step from the first model
        double startTime = models.first()->getStartTime().toDouble();
        double stopTime = models.first()->getStopTime().toDouble();
        size_t nSamples = models.first()->getTopLevelSystemContainer()->getNumberOfLogSamples();
        double logStartT = models.first()->getTopLevelSystemContainer()->getLogStartTime();

        QVector<SystemObject*> systemsVector;
        bool lockOK=false;
        int i;
        for(i=0; i<models.size(); ++i)
        {
            // Append system vector
            systemsVector.append(models[i]->getTopLevelSystemContainer());

            // Lock simulation mutex
            lockOK = models[i]->mSimulateMutex.tryLock();
            if (lockOK)
            {
                // Create relay connection, since we are using the external simulation thread handler
                connect(mpSimulationThreadHandler, SIGNAL(done(bool)), models[i], SIGNAL(simulationFinished()));
            }
            else
            {
                break;
            }
        }
        if (!lockOK)
        {
            // Unlock again
            for (int j=0; j<i; ++j)
            {
                // Remove and previously made relay connections
                disconnect(mpSimulationThreadHandler, SIGNAL(done(bool)), models[j], SIGNAL(simulationFinished()));
                models[j]->mSimulateMutex.unlock();
            }
            // Return with failure
            return false;
        }

        // Set simulation time and start simulation thread
        mpSimulationThreadHandler->setSimulationTimeVariables(startTime, stopTime, logStartT, nSamples);
        mpSimulationThreadHandler->initSimulateFinalize(systemsVector, noChange);
        return mpSimulationThreadHandler->wasSuccessful();
    }
    return false;
}



bool ModelHandler::simulateMultipleModels_blocking(QVector<ModelWidget*> models, bool noChanges)
{
    TicToc tictoc;
    if (!models.isEmpty())
    {
        // All systems will use start time, stop time and time step from the first model
        double startTime = models.first()->getStartTime().toDouble();
        double stopTime = models.first()->getStopTime().toDouble();
        size_t nSamples = models.first()->getTopLevelSystemContainer()->getNumberOfLogSamples();
        double logStartT = models.first()->getTopLevelSystemContainer()->getLogStartTime();

        QVector<SystemObject*> systemsVector;
        bool lockOK=false;
        int i;
        for(i=0; i<models.size(); ++i)
        {
            // Append system vector
            systemsVector.append(models[i]->getTopLevelSystemContainer());

            // Lock simulation mutex
            lockOK = models[i]->mSimulateMutex.tryLock();
            if (lockOK)
            {
                // Create relay connection, since we are using the external simulation thread handler
                connect(mpSimulationThreadHandler, SIGNAL(done(bool)), models[i], SIGNAL(simulationFinished()));
            }
            else
            {
                break;
            }
        }
        if (!lockOK)
        {
            // Unlock again
            for (int j=0; j<i; ++j)
            {
                // Remove and previously made relay connections
                disconnect(mpSimulationThreadHandler, SIGNAL(done(bool)), models[j], SIGNAL(simulationFinished()));
                models[j]->mSimulateMutex.unlock();
            }
            // Return with failure
            return false;
        }

        mpSimulationThreadHandler->setSimulationTimeVariables(startTime, stopTime, logStartT, nSamples);
        mpSimulationThreadHandler->setProgressDilaogBehaviour(true, false);
        mpSimulationThreadHandler->initSimulateFinalize_blocking(systemsVector, noChanges);
        tictoc.toc("simulateMultipleModels_blocking()");
        return mpSimulationThreadHandler->wasSuccessful();
    }

    tictoc.toc("simulateMultipleModels_blocking()");
    return false;
}
