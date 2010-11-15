//!
//! @file   ProjectTabWidget.cpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-02-05
//!
//! @brief Contain classes for Project Tabs
//!
//$Id$

#include <QtGui>
#include <QSizePolicy>
#include <QHash>
#include <QtXml>
//#include <QDomDocument>
//#include <QDomElement>
//#include <QDomText>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cassert>

#include "ProjectTabWidget.h"
#include "MainWindow.h"
#include "GUIPort.h"
#include "MessageWidget.h"
#include "InitializationThread.h"
#include "SimulationThread.h"
#include "ProgressBarThread.h"
#include "UndoStack.h"
#include "LibraryWidget.h"
#include "GUIObjects/GUIObject.h"
#include "GUIConnector.h"
#include "GraphicsScene.h"
#include "GraphicsView.h"
#include "GUIObjects/GUISystem.h"
#include "PlotWidget.h"

#include "version.h"
#include "Utilities/GUIUtilities.h"
#include "loadObjects.h"


//! @class ProjectTab
//! @brief The ProjectTab class is a Widget to contain a simulation model
//!
//! ProjectTab contains a drawing space to create models.
//!


//! Constructor.
//! @param parent defines a parent to the new instanced object.
ProjectTab::ProjectTab(ProjectTabWidget *parent)
    : QWidget(parent)
{
    mpParentProjectTabWidget = parent;
    mpSystem = new GUISystem(this, 0);

    //MainWindow *pMainWindow = mpParentProjectTabWidget->mpParentMainWindow;
    connect(this, SIGNAL(checkMessages()), gpMainWindow->mpMessageWidget, SLOT(checkMessages()));

    emit checkMessages();

    double timeStep = mpSystem->getCoreSystemAccessPtr()->getDesiredTimeStep();

    gpMainWindow->setTimeStepInToolBar(timeStep);

    mIsSaved = true;

    mpGraphicsView  = new GraphicsView(this);
    mpGraphicsView->setScene(mpSystem->getContainedScenePtr());

    QVBoxLayout *tabLayout = new QVBoxLayout;
    tabLayout->addWidget(mpGraphicsView);
    this->setLayout(tabLayout);
}


//! Should be called when a model has changed in some sense,
//! e.g. a component added or a connection has changed.
void ProjectTab::hasChanged()
{
    if (mIsSaved)
    {
        QString tabName = mpParentProjectTabWidget->tabText(mpParentProjectTabWidget->currentIndex());

        tabName.append("*");
        mpParentProjectTabWidget->setTabText(mpParentProjectTabWidget->currentIndex(), tabName);

        mIsSaved = false;
    }
}


//! Returns whether or not the current project is saved
bool ProjectTab::isSaved()
{
    return mIsSaved;
}


//! Set function to tell the tab whether or not it is saved
void ProjectTab::setSaved(bool value)
{
    mIsSaved = value;
}



//! Simulates the model in the tab in a separate thread, the GUI runs a progressbar parallel to the simulation.
bool ProjectTab::simulate()
{

    MessageWidget *pMessageWidget = gpMainWindow->mpMessageWidget;

    mpSystem->updateStartTime();
    mpSystem->updateStopTime();
    mpSystem->updateTimeStep();

        //Setup simulation parameters
    double startTime = mpSystem->getStartTime();
    double finishTime = mpSystem->getStopTime();
    double dt = finishTime - startTime;
    size_t nSteps = dt/mpSystem->getCoreSystemAccessPtr()->getDesiredTimeStep();
    size_t nSamples = mpSystem->getNumberOfLogSamples();

    if(!mpSystem->getCoreSystemAccessPtr()->isSimulationOk())
    {
        emit checkMessages();
        return false;
    }

    qDebug() << "Initializing simulation: " << startTime << nSteps << finishTime;

        //Ask core to initialize simulation
    InitializationThread actualInitialization(mpSystem->getCoreSystemAccessPtr(), startTime, finishTime, nSamples, this);
    actualInitialization.start();
    actualInitialization.setPriority(QThread::HighestPriority);

    ProgressBarThread progressThread(this);
    QProgressDialog progressBar(tr("Initializing simulation..."), tr("&Abort initialization"), 0, 0, this);
    if(gpMainWindow->mEnableProgressBar)
    {
        progressBar.setWindowModality(Qt::WindowModal);
        progressBar.setWindowTitle(tr("Simulate!"));
        size_t i=0;
        while (actualInitialization.isRunning())
        {
            progressThread.start();
            progressThread.setPriority(QThread::TimeCriticalPriority);//(QThread::LowestPriority);
            progressThread.wait();
            progressBar.setValue(i++);
            if (progressBar.wasCanceled())
            {
                mpSystem->getCoreSystemAccessPtr()->stop();
            }
        }
        progressBar.setValue(i);
    }

    actualInitialization.wait(); //Make sure actualSimulation do not goes out of scope during simulation
    actualInitialization.quit();


        //Ask core to execute (and finalize) simulation
    if (!progressBar.wasCanceled())
    {
        QTime simTimer;
        simTimer.start();
        SimulationThread actualSimulation(mpSystem->getCoreSystemAccessPtr(), startTime, finishTime, this);
        actualSimulation.start();
        actualSimulation.setPriority(QThread::HighestPriority);
            //! @todo TimeCriticalPriority seem to work on dual core, is it a problem on single core machines only?
        //actualSimulation.setPriority(QThread::TimeCriticalPriority); //No bar appears in Windows with this prio

        if(gpMainWindow->mEnableProgressBar)
        {
            progressBar.setLabelText(tr("Running simulation..."));
            progressBar.setCancelButtonText(tr("&Abort simulation"));
            progressBar.setMinimum(0);
            progressBar.setMaximum(nSteps);
            while (actualSimulation.isRunning())
            {
               progressThread.start();
               progressThread.setPriority(QThread::LowestPriority);
               progressThread.wait();
               progressBar.setValue((size_t)(mpSystem->getCoreSystemAccessPtr()->getCurrentTime()/dt * nSteps));
               if (progressBar.wasCanceled())
               {
                  mpSystem->getCoreSystemAccessPtr()->stop();
               }
            }
            progressThread.quit();
            progressBar.setValue((size_t)(mpSystem->getCoreSystemAccessPtr()->getCurrentTime()/dt * nSteps));
        }

        actualSimulation.wait(); //Make sure actualSimulation do not goes out of scope during simulation
        actualSimulation.quit();
        QString timeString;
        timeString.setNum(simTimer.elapsed());
        pMessageWidget->printGUIInfoMessage(QString("Simulation time: ").append(timeString).append(" ms"));
    }

    if (progressBar.wasCanceled())
    {
        pMessageWidget->printGUIMessage(QString(tr("Simulation of '").append(mpSystem->getCoreSystemAccessPtr()->getRootSystemName()).append(tr("' was terminated!"))));
    }
    else
    {
        pMessageWidget->printGUIMessage(QString(tr("Simulated '").append(mpSystem->getCoreSystemAccessPtr()->getRootSystemName()).append(tr("' successfully!"))));
        emit simulationFinished();
        //this->mpParentProjectTabWidget->mpParentMainWindow->mpPlotWidget->mpVariableList->updateList();
    }
    emit checkMessages();

    return !(progressBar.wasCanceled());
}


//! Slot that saves current project to old file name if it exists.
//! @see saveProjectTab(int index)
void ProjectTab::save()
{
    saveModel(EXISTINGFILE);
}


//! Slot that saves current project to a new model file.
//! @see saveProjectTab(int index)
void ProjectTab::saveAs()
{
    saveModel(NEWFILE);
}


//! Saves the model and the viewport settings in the tab to a model file.
//! @param saveAsFlag tells whether or not an already existing file name shall be used
//! @see saveProjectTab()
//! @see loadModel()
void ProjectTab::saveModel(saveTarget saveAsFlag)
{
        //Remove the asterix if tab goes from unsaved to saved
    if(!mIsSaved)
    {
        QString tabName = mpParentProjectTabWidget->tabText(mpParentProjectTabWidget->currentIndex());
        tabName.chop(1);
        mpParentProjectTabWidget->setTabText(mpParentProjectTabWidget->currentIndex(), tabName);
        std::cout << "ProjectTabWidget: " << qPrintable(QString("Project: ").append(tabName).append(QString(" saved"))) << std::endl;
        this->setSaved(true);
    }

    //MainWindow *pMainWindow = mpParentProjectTabWidget->mpParentMainWindow;


    if((mpSystem->mModelFileInfo.filePath().isEmpty()) || (saveAsFlag == NEWFILE))
    {
        QDir fileDialogSaveDir;
        QString modelFilePath;
        modelFilePath = QFileDialog::getSaveFileName(this, tr("Save Model File"),
                                                             fileDialogSaveDir.currentPath() + QString(MODELPATH),
                                                             tr("Hopsan Model Files (*.hmf)"));
        mpSystem->mModelFileInfo.setFile(modelFilePath);
    }

    //! @todo quickhack to avoid saving hmfx over hmf
    if (mpSystem->mModelFileInfo.filePath().endsWith("x"))
    {
        QString tmp = mpSystem->mModelFileInfo.filePath();
        tmp.chop(1);
        mpSystem->mModelFileInfo.setFile(tmp);
    }

    QFile file(mpSystem->mModelFileInfo.filePath());   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
    {
        qDebug() << "Failed to open file for writing: " + mpSystem->mModelFileInfo.filePath();
        return;
    }

    //Sets the model name (must set this name before saving or else systemports wont know the real name of their rootsystem parent)
    mpSystem->getCoreSystemAccessPtr()->setRootSystemName(mpSystem->mModelFileInfo.baseName());
    mpParentProjectTabWidget->setTabText(mpParentProjectTabWidget->currentIndex(), mpSystem->mModelFileInfo.fileName());

    QTextStream modelFile(&file);  //Create a QTextStream object to stream the content of file


    writeHeader(modelFile);



    modelFile << "SIMULATIONTIME " << gpMainWindow->getStartTimeFromToolBar() << " " << gpMainWindow->getTimeStepFromToolBar() << " " <<  gpMainWindow->getFinishTimeFromToolBar() << "\n";
    modelFile << "VIEWPORT " << (mpGraphicsView->horizontalScrollBar()->value() + mpGraphicsView->width()/2 - mpGraphicsView->pos().x()) / mpGraphicsView->mZoomFactor << " " <<
                                (mpGraphicsView->verticalScrollBar()->value() + mpGraphicsView->height()/2 - mpGraphicsView->pos().x()) / mpGraphicsView->mZoomFactor << " " <<
                                mpGraphicsView->mZoomFactor << "\n";
    modelFile << "--------------------------------------------------------------\n";
    modelFile << "USERICON " << addQuotes(mpSystem->getUserIconPath()) << "\n";
    modelFile << "ISOICON " << addQuotes(mpSystem->getIsoIconPath()) << "\n";

    //Calculate the position of the subsystem ports:
    GUISystem::GUIModelObjectMapT::iterator it;
    QLineF line;
    double angle, x, y;

    double xMax = mpSystem->mGUIModelObjectMap.begin().value()->x() + mpSystem->mGUIModelObjectMap.begin().value()->rect().width()/2.0;
    double xMin = mpSystem->mGUIModelObjectMap.begin().value()->x() + mpSystem->mGUIModelObjectMap.begin().value()->rect().width()/2.0;
    double yMax = mpSystem->mGUIModelObjectMap.begin().value()->y() + mpSystem->mGUIModelObjectMap.begin().value()->rect().height()/2.0;
    double yMin = mpSystem->mGUIModelObjectMap.begin().value()->y() + mpSystem->mGUIModelObjectMap.begin().value()->rect().height()/2.0;

    for(it = mpSystem->mGUIModelObjectMap.begin(); it!=mpSystem->mGUIModelObjectMap.end(); ++it)
    {
        if (it.value()->x()+it.value()->rect().width()/2.0 < xMin)
            xMin = it.value()->x()+it.value()->rect().width()/2.0;
        if (it.value()->x()+it.value()->rect().width()/2.0 > xMax)
            xMax = it.value()->x()+it.value()->rect().width()/2.0;
        if (it.value()->y()+it.value()->rect().height()/2.0 < yMin)
            yMin = it.value()->y()+it.value()->rect().height()/2.0;
        if (it.value()->y()+it.value()->rect().height()/2.0 > yMax)
            yMax = it.value()->y()+it.value()->rect().height()/2.0;
    }

    QPointF center = QPointF((xMax+xMin)/2, (yMax+yMin)/2);
    double w = xMax-xMin;
    double h = yMax-yMin;

    for(it = mpSystem->mGUIModelObjectMap.begin(); it!=mpSystem->mGUIModelObjectMap.end(); ++it)
    {
        if(it.value()->getTypeName() == "SystemPort")
        {
            line = QLineF(center.x(), center.y(), it.value()->x()+it.value()->rect().width()/2, it.value()->y()+it.value()->rect().height()/2);
            //getCurrentTab()->mpGraphicsScene->addLine(line); //debug-grej
            angle = line.angle()*3.141592/180.0;
            mpSystem->calcSubsystemPortPosition(w, h, angle, x, y);
            x = (x/w+1)/2; //Change coordinate system
            y = (-y/h+1)/2; //Change coordinate system
            modelFile << "PORT " << addQuotes(it.value()->getName()) <<" " << x << " " << y << " " << it.value()->rotation() << "\n";
        }
    }
        modelFile << "--------------------------------------------------------------\n";

    //QHash<QString, GUIObject*>::iterator it;
    for(it = mpSystem->mGUIModelObjectMap.begin(); it!=mpSystem->mGUIModelObjectMap.end(); ++it)
    {
        if ( it.value()->getTypeName() == QString("Subsystem") )
        {
            it.value()->saveToTextStream(modelFile, "BEGINSUBSYSTEM");
            modelFile << "ENDSUBSYSTEM" << "\n"; //!< @todo Do this in some better way, end subsystem is needed by core (but not gui as embedded systems are not suportet (yet))
        }
        else if (it.value()->getTypeName() == QString("SystemPort"))
        {
            it.value()->saveToTextStream(modelFile, "SYSTEMPORT");
        }
        else
        {
            it.value()->saveToTextStream(modelFile, "COMPONENT");
        }
    }

    modelFile << "--------------------------------------------------------------\n";

    for(int i = 0; i != mpSystem->mSubConnectorList.size(); ++i)
    {
        mpSystem->mSubConnectorList[i]->saveToTextStream(modelFile, "CONNECT");
    }
    modelFile << "--------------------------------------------------------------\n";
    file.close();

    qDebug() << "saving to xml";
    //Save xml document
    QDomDocument domDocument;
    QDomElement hmfRoot = appendHMFRootElement(domDocument);

    //Save the model component hierarcy
    //! @todo maybe use a saveload object instead of calling save imediately (only load object exist for now), or maybe this is fine
    mpSystem->saveToDomElement(hmfRoot);

    //Save to file
    const int IndentSize = 4;
    QFile xmlhmf(mpSystem->mModelFileInfo.filePath()+"x");
    if (!xmlhmf.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
    {
        qDebug() << "Failed to open file for writing: " << mpSystem->mModelFileInfo.filePath() << "x";
        return;
    }
    QTextStream out(&xmlhmf);
    appendRootXMLProcessingInstruction(domDocument); //The xml "comment" on the first line
    domDocument.save(out, IndentSize);
}



//! @class ProjectTabWidget
//! @brief The ProjectTabWidget class is a container class for ProjectTab class
//!
//! ProjectTabWidget contains ProjectTabWidget widgets.
//!


//! Constructor.
//! @param parent defines a parent to the new instanced object.
ProjectTabWidget::ProjectTabWidget(MainWindow *parent)
        :   QTabWidget(parent)
{
    //mpParentMainWindow = parent;
    //MainWindow *pMainWindow = (qobject_cast<MainWindow *>(parent)); //Ugly!!!

    connect(this, SIGNAL(checkMessages()), gpMainWindow->mpMessageWidget, SLOT(checkMessages()));

    setTabsClosable(true);
    mNumberOfUntitledTabs = 0;

    mpCopyData = new QString;

    connect(this,SIGNAL(currentChanged(int)),SLOT(tabChanged()));
    connect(this,SIGNAL(tabCloseRequested(int)),SLOT(closeProjectTab(int)));
    connect(this,SIGNAL(tabCloseRequested(int)),SLOT(tabChanged()));

    connect(gpMainWindow->newAction, SIGNAL(triggered()), this, SLOT(addNewProjectTab()));
    connect(gpMainWindow->openAction, SIGNAL(triggered()), this, SLOT(loadModel()));
}


//!  Tells current tab to export itself to PDF. This is needed because a direct connection to current tab would be too complicated.


//! Returns a pointer to the currently active project tab - be sure to check that the number of tabs is not zero before calling this
ProjectTab *ProjectTabWidget::getCurrentTab()
{
    return qobject_cast<ProjectTab *>(currentWidget());
}


//! Returns a pointer to the currently active project tab - be sure to check that the number of tabs is not zero before calling this
ProjectTab *ProjectTabWidget::getTab(int index)
{
    return qobject_cast<ProjectTab *>(widget(index));
}


//! Returns a pointer to the currently system model - be sure to check that the number of tabs is not zero before calling this
GUISystem *ProjectTabWidget::getCurrentSystem()
{
    return getCurrentTab()->mpSystem;
}


//! Returns a pointer to the currently system model - be sure to check that the number of tabs is not zero before calling this
GUISystem *ProjectTabWidget::getSystem(int index)
{
    return getTab(index)->mpSystem;
}


//! Adds an existing ProjectTab object to itself.
//! @see closeProjectTab(int index)
void ProjectTabWidget::addProjectTab(ProjectTab *projectTab, QString tabName)
{
    projectTab->setParent(this);

    addTab(projectTab, tabName);
    setCurrentWidget(projectTab);

    emit newTabAdded();
}


//! Adds a ProjectTab object (a new tab) to itself.
//! @see closeProjectTab(int index)
void ProjectTabWidget::addNewProjectTab(QString tabName)
{
    tabName.append(QString::number(mNumberOfUntitledTabs));

    ProjectTab *newTab = new ProjectTab(this);
    //newTab->mIsSaved = false;

    newTab->mpSystem->setName(tabName);

    //addTab(newTab, tabName.append(QString("*")));
    this->addTab(newTab, tabName);
    this->setCurrentWidget(newTab);

    mNumberOfUntitledTabs += 1;
}


//! Closes current project.
//! @param index defines which project to close.
//! @return true if closing went ok. false if the user canceled the operation.
//! @see closeAllProjectTabs()
bool ProjectTabWidget::closeProjectTab(int index)
{
    if (!(this->getCurrentTab()->isSaved()))
    {
        QString modelName;
        modelName = tabText(index);
        modelName.chop(1);
        QMessageBox msgBox;
        msgBox.setText(QString("The model '").append(modelName).append("'").append(QString(" is not saved.")));
        msgBox.setInformativeText("Do you want to save your changes before closing?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);

        int answer = msgBox.exec();

        switch (answer)
        {
        case QMessageBox::Save:
            // Save was clicked
            std::cout << "ProjectTabWidget: " << "Save and close" << std::endl;
            getTab(index)->save();
            removeTab(index);
            return true;
        case QMessageBox::Discard:
            // Don't Save was clicked
            removeTab(index);
            return true;
        case QMessageBox::Cancel:
            // Cancel was clicked
            std::cout << "ProjectTabWidget: " << "Cancel closing" << std::endl;
            return false;
        default:
            // should never be reached
            return false;
        }
    }
    else
    {
        std::cout << "ProjectTabWidget: " << "Closing project: " << qPrintable(tabText(index)) << std::endl;
        //statusBar->showMessage(QString("Closing project: ").append(tabText(index)));
        disconnect(gpMainWindow->resetZoomAction, SIGNAL(triggered()),getTab(index)->mpGraphicsView,SLOT(resetZoom()));
        disconnect(gpMainWindow->zoomInAction, SIGNAL(triggered()),getTab(index)->mpGraphicsView,SLOT(zoomIn()));
        disconnect(gpMainWindow->zoomOutAction, SIGNAL(triggered()),getTab(index)->mpGraphicsView,SLOT(zoomOut()));
        disconnect(gpMainWindow->exportPDFAction, SIGNAL(triggered()), getTab(index)->mpGraphicsView,SLOT(exportToPDF()));
        disconnect(gpMainWindow->centerViewAction,SIGNAL(triggered()),getTab(index)->mpGraphicsView,SLOT(centerView()));
        disconnect(gpMainWindow->hideNamesAction, SIGNAL(triggered()),getSystem(index),SLOT(hideNames()));
        disconnect(gpMainWindow->showNamesAction, SIGNAL(triggered()),getSystem(index),SLOT(showNames()));
        disconnect(gpMainWindow->mpStartTimeLineEdit, SIGNAL(editingFinished()), getSystem(index), SLOT(updateStartTime()));
        disconnect(gpMainWindow->mpTimeStepLineEdit, SIGNAL(editingFinished()), getSystem(index), SLOT(updateTimeStep()));
        disconnect(gpMainWindow->mpFinishTimeLineEdit, SIGNAL(editingFinished()), getSystem(index), SLOT(updateStopTime()));
        disconnect(gpMainWindow->disableUndoAction,SIGNAL(triggered()),getSystem(index), SLOT(disableUndo()));
        disconnect(gpMainWindow->simulateAction, SIGNAL(triggered()), getTab(index), SLOT(simulate()));
        disconnect(gpMainWindow->mpStartTimeLineEdit, SIGNAL(editingFinished()), getSystem(index),SLOT(updateStartTime()));
        disconnect(gpMainWindow->mpFinishTimeLineEdit, SIGNAL(editingFinished()), getSystem(index),SLOT(updateStopTime()));
        disconnect(gpMainWindow->mpTimeStepLineEdit, SIGNAL(editingFinished()), getSystem(index),SLOT(updateTimeStep()));
        disconnect(gpMainWindow->saveAction, SIGNAL(triggered()), getTab(index), SLOT(save()));
        disconnect(gpMainWindow->saveAsAction, SIGNAL(triggered()), getTab(index), SLOT(saveAs()));

        removeTab(index);
        return true;
    }
}


//! Closes all opened projects.
//! @return true if closing went ok. false if the user canceled the operation.
//! @see closeProjectTab(int index)
//! @see saveProjectTab()
bool ProjectTabWidget::closeAllProjectTabs()
{
    gpMainWindow->mLastSessionModels.clear();

    while(count() > 0)
    {
        setCurrentIndex(count()-1);
        gpMainWindow->mLastSessionModels.append(getCurrentSystem()->mModelFileInfo.filePath());
        if (!closeProjectTab(count()-1))
        {
            return false;
        }
    }
    return true;
}


//! Loads a model from a file and opens it in a new project tab.
//! @see loadModel(QString modelFileName)
//! @see saveModel(saveTarget saveAsFlag)
void ProjectTabWidget::loadModel()
{
    QDir fileDialogOpenDir;
    QString modelFileName = QFileDialog::getOpenFileName(this, tr("Choose Model File"),
                                                         fileDialogOpenDir.currentPath() + QString(MODELPATH),
                                                         tr("Hopsan Model Files (*.hmf *.hmfx)"));
    loadModel(modelFileName);

    emit newTabAdded();
}


//! Loads a model from a file and opens it in a new project tab.
//! @param modelFileName is the path to the loaded file
//! @see loadModel()
//! @see saveModel(saveTarget saveAsFlag)
void ProjectTabWidget::loadModel(QString modelFileName)
{
    if (modelFileName.isEmpty())
        return;

    QFile file(modelFileName);   //Create a QFile object

    if(!file.exists())
    {
        qDebug() << "Failed to open file, file not found: " + file.fileName();
        return;
    }

    QFileInfo fileInfo(file);

    //Make sure file not already open
    for(int t=0; t!=this->count(); ++t)
    {
        if( (this->tabText(t) == fileInfo.fileName()) || (this->tabText(t) == (fileInfo.fileName() + "*")) )
        {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::information(this, tr("Error"), tr("Unable to load model. File is already open."));
            return;
        }
    }

    gpMainWindow->registerRecentModel(fileInfo);

    this->addProjectTab(new ProjectTab(this), fileInfo.fileName());
    ProjectTab *pCurrentTab = qobject_cast<ProjectTab *>(currentWidget());
    pCurrentTab->setSaved(true);

    //Temporary hack to atempt loading xml model files
    if (modelFileName.endsWith("x"))
    {
        QDomDocument domDocument;
        QString errorStr;
        int errorLine, errorColumn;
        if (!domDocument.setContent(&file, false, &errorStr, &errorLine, &errorColumn))
        {
            QMessageBox::information(window(), tr("Hopsan GUI"),
                                     tr("Parse error at line %1, column %2:\n%3")
                                     .arg(errorLine)
                                     .arg(errorColumn)
                                     .arg(errorStr));
        }
        else
        {
            QDomElement hmfRoot = domDocument.documentElement();
            if (hmfRoot.tagName() != HMF_ROOTTAG)
            {
                QMessageBox::information(window(), tr("Hopsan GUI"),
                                         "The file is not an Hopsan Model File file. Incorrect hmf root tag name: "
                                         + hmfRoot.tagName() + "!=" + HMF_ROOTTAG);
            }
            else
            {
                //Do some header read stuff
                //! @todo clean this up and check for existance .isNull can be used
                //! @todo use the unused info
                QDomElement versionInfo = hmfRoot.firstChildElement("hopsanversions");
                QDomElement modelProperties = hmfRoot.firstChildElement("modelproperties");
                QDomElement systemElement = hmfRoot.firstChildElement(HMF_SYSTEMTAG);
                pCurrentTab->mpSystem->mModelFileInfo.setFile(file); //Remember info about the file from which the data was loaded
                pCurrentTab->mpSystem->loadFromDomElement(systemElement);
            }
        }
    }
    else
    {
        pCurrentTab->mpSystem->loadFromHMF(modelFileName);
    }
}


void ProjectTabWidget::tabChanged()
{
    for(size_t i=0; i<count(); ++i)
    {
            //If you add a disconnect here, remember to also add it to the close tab function!
        disconnect(gpMainWindow->resetZoomAction, SIGNAL(triggered()),getTab(i)->mpGraphicsView,SLOT(resetZoom()));
        disconnect(gpMainWindow->zoomInAction, SIGNAL(triggered()),getTab(i)->mpGraphicsView,SLOT(zoomIn()));
        disconnect(gpMainWindow->zoomOutAction, SIGNAL(triggered()),getTab(i)->mpGraphicsView,SLOT(zoomOut()));
        disconnect(gpMainWindow->exportPDFAction, SIGNAL(triggered()), getTab(i)->mpGraphicsView,SLOT(exportToPDF()));
        disconnect(gpMainWindow->centerViewAction,SIGNAL(triggered()),getTab(i)->mpGraphicsView,SLOT(centerView()));
        disconnect(gpMainWindow->hideNamesAction, SIGNAL(triggered()),getSystem(i),SLOT(hideNames()));
        disconnect(gpMainWindow->showNamesAction, SIGNAL(triggered()),getSystem(i),SLOT(showNames()));
        disconnect(gpMainWindow->mpStartTimeLineEdit, SIGNAL(editingFinished()), getSystem(i), SLOT(updateStartTime()));
        disconnect(gpMainWindow->mpTimeStepLineEdit, SIGNAL(editingFinished()), getSystem(i), SLOT(updateTimeStep()));
        disconnect(gpMainWindow->mpFinishTimeLineEdit, SIGNAL(editingFinished()), getSystem(i), SLOT(updateStopTime()));
        disconnect(gpMainWindow->disableUndoAction,SIGNAL(triggered()),getSystem(i), SLOT(disableUndo()));
        disconnect(gpMainWindow->simulateAction, SIGNAL(triggered()), getTab(i), SLOT(simulate()));
        disconnect(gpMainWindow->mpStartTimeLineEdit, SIGNAL(editingFinished()), getSystem(i),SLOT(updateStartTime()));
        disconnect(gpMainWindow->mpFinishTimeLineEdit, SIGNAL(editingFinished()), getSystem(i),SLOT(updateStopTime()));
        disconnect(gpMainWindow->mpTimeStepLineEdit, SIGNAL(editingFinished()), getSystem(i),SLOT(updateTimeStep()));
        disconnect(gpMainWindow->saveAction, SIGNAL(triggered()), getTab(i), SLOT(save()));
        disconnect(gpMainWindow->saveAsAction, SIGNAL(triggered()), getTab(i), SLOT(saveAs()));
        disconnect(gpMainWindow->cutAction, SIGNAL(triggered()), getSystem(i),SLOT(cutSelected()));
        disconnect(gpMainWindow->copyAction, SIGNAL(triggered()), getSystem(i),SLOT(copySelected()));
        disconnect(gpMainWindow->pasteAction, SIGNAL(triggered()), getSystem(i),SLOT(paste()));
    }
    if(this->count() != 0)
    {
        connect(gpMainWindow->resetZoomAction, SIGNAL(triggered()),getCurrentTab()->mpGraphicsView,SLOT(resetZoom()));
        connect(gpMainWindow->zoomInAction, SIGNAL(triggered()),getCurrentTab()->mpGraphicsView,SLOT(zoomIn()));
        connect(gpMainWindow->zoomOutAction, SIGNAL(triggered()),getCurrentTab()->mpGraphicsView,SLOT(zoomOut()));
        connect(gpMainWindow->exportPDFAction, SIGNAL(triggered()), getCurrentTab()->mpGraphicsView,SLOT(exportToPDF()));
        connect(gpMainWindow->centerViewAction,SIGNAL(triggered()),getCurrentTab()->mpGraphicsView,SLOT(centerView()));
        connect(gpMainWindow->hideNamesAction, SIGNAL(triggered()),getCurrentSystem(),SLOT(hideNames()));
        connect(gpMainWindow->showNamesAction, SIGNAL(triggered()),getCurrentSystem(),SLOT(showNames()));
        connect(gpMainWindow->mpStartTimeLineEdit, SIGNAL(editingFinished()), getCurrentSystem(), SLOT(updateStartTime()));
        connect(gpMainWindow->mpTimeStepLineEdit, SIGNAL(editingFinished()), getCurrentSystem(), SLOT(updateTimeStep()));
        connect(gpMainWindow->mpFinishTimeLineEdit, SIGNAL(editingFinished()), getCurrentSystem(), SLOT(updateStopTime()));
        connect(gpMainWindow->disableUndoAction,SIGNAL(triggered()),getCurrentSystem(), SLOT(disableUndo()));
        connect(gpMainWindow->simulateAction, SIGNAL(triggered()), getCurrentTab(), SLOT(simulate()));
        connect(gpMainWindow->mpStartTimeLineEdit, SIGNAL(editingFinished()), getCurrentSystem(),SLOT(updateStartTime()));
        connect(gpMainWindow->mpFinishTimeLineEdit, SIGNAL(editingFinished()), getCurrentSystem(),SLOT(updateStopTime()));
        connect(gpMainWindow->mpTimeStepLineEdit, SIGNAL(editingFinished()), getCurrentSystem(),SLOT(updateTimeStep()));
        connect(gpMainWindow->saveAction, SIGNAL(triggered()), getCurrentTab(), SLOT(save()));
        connect(gpMainWindow->saveAsAction, SIGNAL(triggered()), getCurrentTab(), SLOT(saveAs()));
        connect(gpMainWindow->cutAction, SIGNAL(triggered()), getCurrentSystem(),SLOT(cutSelected()));
        connect(gpMainWindow->copyAction, SIGNAL(triggered()), getCurrentSystem(),SLOT(copySelected()));
        connect(gpMainWindow->pasteAction, SIGNAL(triggered()), getCurrentSystem(),SLOT(paste()));
        getCurrentSystem()->updateUndoStatus();
        getCurrentSystem()->updateSimulationParametersInToolBar();
    }
}
