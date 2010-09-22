/*
 * This file is part of OpenModelica.
 *
 * Copyright (c) 1998-CurrentYear, Linköping University,
 * Department of Computer and Information Science,
 * SE-58183 Linköping, Sweden.
 *
 * All rights reserved.
 *
 * THIS PROGRAM IS PROVIDED UNDER THE TERMS OF GPL VERSION 3 
 * AND THIS OSMC PUBLIC LICENSE (OSMC-PL). 
 * ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS PROGRAM CONSTITUTES RECIPIENT'S  
 * ACCEPTANCE OF THE OSMC PUBLIC LICENSE.
 *
 * The OpenModelica software and the Open Source Modelica
 * Consortium (OSMC) Public License (OSMC-PL) are obtained
 * from Linköping University, either from the above address,
 * from the URLs: http://www.ida.liu.se/projects/OpenModelica or  
 * http://www.openmodelica.org, and in the OpenModelica distribution. 
 * GNU version 3 is obtained from: http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without
 * even the implied warranty of  MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE, EXCEPT AS EXPRESSLY SET FORTH
 * IN THE BY RECIPIENT SELECTED SUBSIDIARY LICENSE CONDITIONS
 * OF OSMC-PL.
 *
 * See the full OSMC Public License conditions for more details.
 *
 */

/*
 * HopsanGUI
 * Fluid and Mechatronic Systems, Department of Management and Engineering, Linköping University
 * Main Authors 2009-2010:  Robert Braun, Björn Eriksson, Peter Nordin
 * Contributors 2009-2010:  Mikael Axin, Alessandro Dell'Amico, Karl Pettersson, Ingo Staack
 */

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
#include "GUIObject.h"
#include "GUIConnector.h"
#include "GraphicsScene.h"
#include "GraphicsView.h"
#include "GUISystem.h"

#include "version.h"
#include "GUIUtilities.h"
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

    MainWindow *pMainWindow = mpParentProjectTabWidget->mpParentMainWindow;
    connect(this, SIGNAL(checkMessages()), pMainWindow->mpMessageWidget, SLOT(checkMessages()));

    emit checkMessages();

    double timeStep = mpSystem->mpCoreSystemAccess->getDesiredTimeStep();

    mpParentProjectTabWidget->mpParentMainWindow->setTimeStepInToolBar(timeStep);

    mIsSaved = true;

    mpGraphicsView  = new GraphicsView(this);
    mpGraphicsView->setScene(mpSystem->mpScene);

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

    MessageWidget *pMessageWidget = mpParentProjectTabWidget->mpParentMainWindow->mpMessageWidget;

    mpSystem->updateStartTime();
    mpSystem->updateStopTime();
    mpSystem->updateTimeStep();

        //Setup simulation parameters
    double startTime = mpSystem->getStartTime();
    double finishTime = mpSystem->getStopTime();
    double dt = finishTime - startTime;
    size_t nSteps = dt/mpSystem->mpCoreSystemAccess->getDesiredTimeStep();

    qDebug() << "Initializing simulation: " << startTime << nSteps << finishTime;

        //Ask core to initialize simulation
    InitializationThread actualInitialization(mpSystem->mpCoreSystemAccess, startTime, finishTime, this);
    actualInitialization.start();
    actualInitialization.setPriority(QThread::HighestPriority);

    ProgressBarThread progressThread(this);
    QProgressDialog progressBar(tr("Initializing simulation..."), tr("&Abort initialization"), 0, 0, this);
    if(mpParentProjectTabWidget->mpParentMainWindow->mEnableProgressBar)
    {
        progressBar.setWindowModality(Qt::WindowModal);
        progressBar.setWindowTitle(tr("Simulate!"));
        size_t i=0;
        while (actualInitialization.isRunning())
        {
            progressThread.start();
            progressThread.setPriority(QThread::LowestPriority);
            progressThread.wait();
            progressBar.setValue(i++);
            if (progressBar.wasCanceled())
            {
                mpSystem->mpCoreSystemAccess->stop();
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
        SimulationThread actualSimulation(mpSystem->mpCoreSystemAccess, startTime, finishTime, this);
        actualSimulation.start();
        actualSimulation.setPriority(QThread::HighestPriority);
            //! @todo TimeCriticalPriority seem to work on dual core, is it a problem on single core machines only?
        //actualSimulation.setPriority(QThread::TimeCriticalPriority); //No bar appears in Windows with this prio

        if(mpParentProjectTabWidget->mpParentMainWindow->mEnableProgressBar)
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
               progressBar.setValue((size_t)(mpSystem->mpCoreSystemAccess->getCurrentTime()/dt * nSteps));
               if (progressBar.wasCanceled())
               {
                  mpSystem->mpCoreSystemAccess->stop();
               }
            }
            progressThread.quit();
            progressBar.setValue((size_t)(mpSystem->mpCoreSystemAccess->getCurrentTime()/dt * nSteps));
        }

        actualSimulation.wait(); //Make sure actualSimulation do not goes out of scope during simulation
        actualSimulation.quit();
        QString timeString;
        timeString.setNum(simTimer.elapsed());
        pMessageWidget->printGUIInfoMessage(QString("Simulation time: ").append(timeString).append(" ms"));
    }

    if (progressBar.wasCanceled())
        pMessageWidget->printGUIMessage(QString(tr("Simulation of '").append(mpSystem->mpCoreSystemAccess->getRootSystemName()).append(tr("' was terminated!"))));
    else
        pMessageWidget->printGUIMessage(QString(tr("Simulated '").append(mpSystem->mpCoreSystemAccess->getRootSystemName()).append(tr("' successfully!"))));
    emit checkMessages();

    return progressBar.wasCanceled();
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

    MainWindow *pMainWindow = mpParentProjectTabWidget->mpParentMainWindow;


    if((mpSystem->mModelFileInfo.filePath().isEmpty()) | (saveAsFlag == NEWFILE))
    {
        QDir fileDialogSaveDir;
        QString modelFilePath;
        modelFilePath = QFileDialog::getSaveFileName(this, tr("Save Model File"),
                                                             fileDialogSaveDir.currentPath() + QString(MODELPATH),
                                                             tr("Hopsan Model Files (*.hmf)"));
        mpSystem->mModelFileInfo.setFile(modelFilePath);
    }

    QFile file(mpSystem->mModelFileInfo.filePath());   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
    {
        qDebug() << "Failed to open file for writing: " + mpSystem->mModelFileInfo.filePath();
        return;
    }

    //Sets the model name (must set this name before saving or else systemports wont know the real name of their rootsystem parent)
    mpSystem->mpCoreSystemAccess->setRootSystemName(mpSystem->mModelFileInfo.baseName());
    mpParentProjectTabWidget->setTabText(mpParentProjectTabWidget->currentIndex(), mpSystem->mModelFileInfo.fileName());

    QTextStream modelFile(&file);  //Create a QTextStream object to stream the content of file

    writeHeader(modelFile);

    modelFile << "SIMULATIONTIME " << pMainWindow->getStartTimeFromToolBar() << " " << pMainWindow->getTimeStepFromToolBar() << " " <<  pMainWindow->getFinishTimeFromToolBar() << "\n";
    modelFile << "VIEWPORT " << (mpGraphicsView->horizontalScrollBar()->value() + mpGraphicsView->width()/2 - mpGraphicsView->pos().x()) / mpGraphicsView->mZoomFactor << " " <<
                                (mpGraphicsView->verticalScrollBar()->value() + mpGraphicsView->height()/2 - mpGraphicsView->pos().x()) / mpGraphicsView->mZoomFactor << " " <<
                                mpGraphicsView->mZoomFactor << "\n";
    modelFile << "--------------------------------------------------------------\n";
    modelFile << "USERICON " << addQuotes(mpSystem->getUserIconPath()) << "\n";
    modelFile << "ISOICON " << addQuotes(mpSystem->getIsoIconPath()) << "\n";

    //Calculate the position of the subsystem ports:
    QHash<QString, GUIObject*>::iterator it;
    QLineF line;
    double angle, x, y;

    double xMax = mpSystem->mGUIObjectMap.begin().value()->x() + mpSystem->mGUIObjectMap.begin().value()->rect().width()/2.0;
    double xMin = mpSystem->mGUIObjectMap.begin().value()->x() + mpSystem->mGUIObjectMap.begin().value()->rect().width()/2.0;
    double yMax = mpSystem->mGUIObjectMap.begin().value()->y() + mpSystem->mGUIObjectMap.begin().value()->rect().height()/2.0;
    double yMin = mpSystem->mGUIObjectMap.begin().value()->y() + mpSystem->mGUIObjectMap.begin().value()->rect().height()/2.0;

    for(it = mpSystem->mGUIObjectMap.begin(); it!=mpSystem->mGUIObjectMap.end(); ++it)
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

    for(it = mpSystem->mGUIObjectMap.begin(); it!=mpSystem->mGUIObjectMap.end(); ++it)
    {
        if(it.value()->getTypeName() == "SystemPort")
        {
            line = QLineF(center.x(), center.y(), it.value()->x()+it.value()->rect().width()/2, it.value()->y()+it.value()->rect().height()/2);
            //getCurrentTab()->mpGraphicsScene->addLine(line); //debug-grej
            angle = line.angle()*3.141592/180.0;
            calcSubsystemPortPosition(w, h, angle, x, y);
            x = (x/w+1)/2; //Change coordinate system
            y = (-y/h+1)/2; //Change coordinate system
            modelFile << "PORT " << addQuotes(it.value()->getName()) <<" " << x << " " << y << " " << it.value()->rotation() << "\n";
        }
    }
        modelFile << "--------------------------------------------------------------\n";

    //QHash<QString, GUIObject*>::iterator it;
    for(it = mpSystem->mGUIObjectMap.begin(); it!=mpSystem->mGUIObjectMap.end(); ++it)
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
    mpParentMainWindow = parent;
    //MainWindow *pMainWindow = (qobject_cast<MainWindow *>(parent)); //Ugly!!!

    connect(this, SIGNAL(checkMessages()), mpParentMainWindow->mpMessageWidget, SLOT(checkMessages()));

    setTabsClosable(true);
    mNumberOfUntitledTabs = 0;


    connect(this,SIGNAL(currentChanged(int)),SLOT(tabChanged()));
    connect(this,SIGNAL(tabCloseRequested(int)),SLOT(closeProjectTab(int)));
    connect(this,SIGNAL(tabCloseRequested(int)),SLOT(tabChanged()));

    connect(mpParentMainWindow->newAction, SIGNAL(triggered()), this, SLOT(addNewProjectTab()));
    connect(mpParentMainWindow->openAction, SIGNAL(triggered()), this, SLOT(loadModel()));
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
    disconnect(mpParentMainWindow->resetZoomAction, SIGNAL(triggered()),getTab(index)->mpGraphicsView,SLOT(resetZoom()));
    disconnect(mpParentMainWindow->zoomInAction, SIGNAL(triggered()),getTab(index)->mpGraphicsView,SLOT(zoomIn()));
    disconnect(mpParentMainWindow->zoomOutAction, SIGNAL(triggered()),getTab(index)->mpGraphicsView,SLOT(zoomOut()));
    disconnect(mpParentMainWindow->exportPDFAction, SIGNAL(triggered()), getTab(index)->mpGraphicsView,SLOT(exportToPDF()));
    disconnect(mpParentMainWindow->centerViewAction,SIGNAL(triggered()),getTab(index)->mpGraphicsView,SLOT(centerView()));
    disconnect(mpParentMainWindow->hideNamesAction, SIGNAL(triggered()),getSystem(index),SLOT(hideNames()));
    disconnect(mpParentMainWindow->showNamesAction, SIGNAL(triggered()),getSystem(index),SLOT(showNames()));
    disconnect(mpParentMainWindow->mpStartTimeLineEdit, SIGNAL(editingFinished()), getSystem(index), SLOT(updateStartTime()));
    disconnect(mpParentMainWindow->mpTimeStepLineEdit, SIGNAL(editingFinished()), getSystem(index), SLOT(updateTimeStep()));
    disconnect(mpParentMainWindow->mpFinishTimeLineEdit, SIGNAL(editingFinished()), getSystem(index), SLOT(updateStopTime()));
    disconnect(mpParentMainWindow->disableUndoAction,SIGNAL(triggered()),getSystem(index), SLOT(disableUndo()));
    disconnect(mpParentMainWindow->simulateAction, SIGNAL(triggered()), getTab(index), SLOT(simulate()));
    disconnect(mpParentMainWindow->mpStartTimeLineEdit, SIGNAL(editingFinished()), getSystem(index),SLOT(updateStartTime()));
    disconnect(mpParentMainWindow->mpFinishTimeLineEdit, SIGNAL(editingFinished()), getSystem(index),SLOT(updateStopTime()));
    disconnect(mpParentMainWindow->mpTimeStepLineEdit, SIGNAL(editingFinished()), getSystem(index),SLOT(updateTimeStep()));
    disconnect(mpParentMainWindow->saveAction, SIGNAL(triggered()), getTab(index), SLOT(save()));
    disconnect(mpParentMainWindow->saveAsAction, SIGNAL(triggered()), getTab(index), SLOT(saveAs()));

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
    while(count() > 0)
    {
        setCurrentIndex(count()-1);
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
                                                         tr("Hopsan Model Files (*.hmf)"));
    loadModel(modelFileName);
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


//    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))  //open file
//    {
//        qDebug() << "Failed to open file or not a text file: " + modelFileName;
//        return;
//    }
//    QTextStream inputStream(&file);  //Create a QTextStream object to stream the content of file


    this->addProjectTab(new ProjectTab(this), fileInfo.fileName());
    ProjectTab *pCurrentTab = qobject_cast<ProjectTab *>(currentWidget());
//    pCurrentTab->mpSystem->mModelFileName = modelFileName;
//    pCurrentTab->mpSystem->mUndoStack->newPost();
    pCurrentTab->setSaved(true);

    pCurrentTab->mpSystem->loadFromHMF(modelFileName);

//    //Read the header data, also checks version numbers
//    //! @todo maybe not check the version numbers in there
//    HeaderLoadData headerData = readHeader(inputStream, mpParentMainWindow->mpMessageWidget);

//    //It is assumed that these data have been successfully read
//    mpParentMainWindow->setStartTimeInToolBar(headerData.startTime);
//    mpParentMainWindow->setTimeStepInToolBar(headerData.timeStep);
//    mpParentMainWindow->setFinishTimeInToolBar(headerData.stopTime);

//    //It is assumed that these data have been successfully read
//    getCurrentTab()->mpGraphicsView->centerOn(headerData.viewport_x, headerData.viewport_y);
//    getCurrentTab()->mpGraphicsView->scale(headerData.viewport_zoomfactor, headerData.viewport_zoomfactor);
//    getCurrentTab()->mpGraphicsView->mZoomFactor = headerData.viewport_zoomfactor;
//    getCurrentTab()->mpGraphicsView->updateViewPort();

//    //Sets the file name (exluding path and ending) as the model name
//    getCurrentTab()->mpSystem->mpCoreSystemAccess->setRootSystemName(fileInfo.baseName());

//    while ( !inputStream.atEnd() )
//    {
//        //Extract first word on line
//        QString inputWord;
//        inputStream >> inputWord;

//        if ( (inputWord == "SUBSYSTEM") or (inputWord == "BEGINSUBSYSTEM") )
//        {
//            SubsystemLoadData subsysData;
//            subsysData.read(inputStream);
//            loadSubsystemGUIObject(subsysData, mpParentMainWindow->mpLibrary, pCurrentTab->mpSystem, NOUNDO);
//            //! @todo convenience function
//        }

//        if ( (inputWord == "COMPONENT") || (inputWord == "SYSTEMPORT") )
//        {
//            loadGUIObject(inputStream, mpParentMainWindow->mpLibrary, pCurrentTab->mpSystem, NOUNDO);
//        }


//        if ( inputWord == "PARAMETER" )
//        {
//            loadParameterValues(inputStream, pCurrentTab->mpSystem, NOUNDO);
//        }


//        if ( inputWord == "CONNECT" )
//        {
//            loadConnector(inputStream, pCurrentTab->mpSystem, NOUNDO);
//        }
//    }
//    //Deselect all components
//   //pCurrentTab->mpGraphicsView->deselectAllGUIObjects();

//    pCurrentTab->mpSystem->deselectAll();
//    this->centerView();
//    pCurrentTab->mpSystem->mUndoStack->clear();
//    pCurrentTab->mpGraphicsView->updateViewPort();

//    emit checkMessages();
}


void ProjectTabWidget::tabChanged()
{
    for(size_t i=0; i<count(); ++i)
    {
            //If you add a disconnect here, remember to also add it to the close tab function!
        disconnect(mpParentMainWindow->resetZoomAction, SIGNAL(triggered()),getTab(i)->mpGraphicsView,SLOT(resetZoom()));
        disconnect(mpParentMainWindow->zoomInAction, SIGNAL(triggered()),getTab(i)->mpGraphicsView,SLOT(zoomIn()));
        disconnect(mpParentMainWindow->zoomOutAction, SIGNAL(triggered()),getTab(i)->mpGraphicsView,SLOT(zoomOut()));
        disconnect(mpParentMainWindow->exportPDFAction, SIGNAL(triggered()), getTab(i)->mpGraphicsView,SLOT(exportToPDF()));
        disconnect(mpParentMainWindow->centerViewAction,SIGNAL(triggered()),getTab(i)->mpGraphicsView,SLOT(centerView()));
        disconnect(mpParentMainWindow->hideNamesAction, SIGNAL(triggered()),getSystem(i),SLOT(hideNames()));
        disconnect(mpParentMainWindow->showNamesAction, SIGNAL(triggered()),getSystem(i),SLOT(showNames()));
        disconnect(mpParentMainWindow->mpStartTimeLineEdit, SIGNAL(editingFinished()), getSystem(i), SLOT(updateStartTime()));
        disconnect(mpParentMainWindow->mpTimeStepLineEdit, SIGNAL(editingFinished()), getSystem(i), SLOT(updateTimeStep()));
        disconnect(mpParentMainWindow->mpFinishTimeLineEdit, SIGNAL(editingFinished()), getSystem(i), SLOT(updateStopTime()));
        disconnect(mpParentMainWindow->disableUndoAction,SIGNAL(triggered()),getSystem(i), SLOT(disableUndo()));
        disconnect(mpParentMainWindow->simulateAction, SIGNAL(triggered()), getTab(i), SLOT(simulate()));
        disconnect(mpParentMainWindow->mpStartTimeLineEdit, SIGNAL(editingFinished()), getSystem(i),SLOT(updateStartTime()));
        disconnect(mpParentMainWindow->mpFinishTimeLineEdit, SIGNAL(editingFinished()), getSystem(i),SLOT(updateStopTime()));
        disconnect(mpParentMainWindow->mpTimeStepLineEdit, SIGNAL(editingFinished()), getSystem(i),SLOT(updateTimeStep()));
        disconnect(mpParentMainWindow->saveAction, SIGNAL(triggered()), getTab(i), SLOT(save()));
        disconnect(mpParentMainWindow->saveAsAction, SIGNAL(triggered()), getTab(i), SLOT(saveAs()));
    }
    if(this->count() != 0)
    {
        connect(mpParentMainWindow->resetZoomAction, SIGNAL(triggered()),getCurrentTab()->mpGraphicsView,SLOT(resetZoom()));
        connect(mpParentMainWindow->zoomInAction, SIGNAL(triggered()),getCurrentTab()->mpGraphicsView,SLOT(zoomIn()));
        connect(mpParentMainWindow->zoomOutAction, SIGNAL(triggered()),getCurrentTab()->mpGraphicsView,SLOT(zoomOut()));
        connect(mpParentMainWindow->exportPDFAction, SIGNAL(triggered()), getCurrentTab()->mpGraphicsView,SLOT(exportToPDF()));
        connect(mpParentMainWindow->centerViewAction,SIGNAL(triggered()),getCurrentTab()->mpGraphicsView,SLOT(centerView()));
        connect(mpParentMainWindow->hideNamesAction, SIGNAL(triggered()),getCurrentSystem(),SLOT(hideNames()));
        connect(mpParentMainWindow->showNamesAction, SIGNAL(triggered()),getCurrentSystem(),SLOT(showNames()));
        connect(mpParentMainWindow->mpStartTimeLineEdit, SIGNAL(editingFinished()), getCurrentSystem(), SLOT(updateStartTime()));
        connect(mpParentMainWindow->mpTimeStepLineEdit, SIGNAL(editingFinished()), getCurrentSystem(), SLOT(updateTimeStep()));
        connect(mpParentMainWindow->mpFinishTimeLineEdit, SIGNAL(editingFinished()), getCurrentSystem(), SLOT(updateStopTime()));
        connect(mpParentMainWindow->disableUndoAction,SIGNAL(triggered()),getCurrentSystem(), SLOT(disableUndo()));
        connect(mpParentMainWindow->simulateAction, SIGNAL(triggered()), getCurrentTab(), SLOT(simulate()));
        connect(mpParentMainWindow->mpStartTimeLineEdit, SIGNAL(editingFinished()), getCurrentSystem(),SLOT(updateStartTime()));
        connect(mpParentMainWindow->mpFinishTimeLineEdit, SIGNAL(editingFinished()), getCurrentSystem(),SLOT(updateStopTime()));
        connect(mpParentMainWindow->mpTimeStepLineEdit, SIGNAL(editingFinished()), getCurrentSystem(),SLOT(updateTimeStep()));
        connect(mpParentMainWindow->saveAction, SIGNAL(triggered()), getCurrentTab(), SLOT(save()));
        connect(mpParentMainWindow->saveAsAction, SIGNAL(triggered()), getCurrentTab(), SLOT(saveAs()));
        getCurrentSystem()->updateUndoStatus();
        getCurrentSystem()->updateSimulationParametersInToolBar();
    }
}
