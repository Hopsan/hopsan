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
#include <QMap>

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

    double timeStep = mpSystem->mCoreSystemAccess.getDesiredTimeStep();

    mpParentProjectTabWidget->mpParentMainWindow->setTimeStepLabel(timeStep);

    mIsSaved = true;

    //mpGraphicsScene = ;
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

    connect(this,SIGNAL(tabCloseRequested(int)),SLOT(closeProjectTab(int)));
    connect(this,SIGNAL(currentChanged(int)),this, SLOT(updateSimulationSetupWidget()));
    connect(this,SIGNAL(currentChanged(int)),this, SLOT(updateUndoStatus()));
    connect(mpParentMainWindow->mpStartTimeLineEdit, SIGNAL(editingFinished()), this, SLOT(updateCurrentStartTime()));
    connect(mpParentMainWindow->mpTimeStepLineEdit, SIGNAL(editingFinished()), this, SLOT(updateCurrentTimeStep()));

    connect(mpParentMainWindow->mpFinishTimeLineEdit, SIGNAL(editingFinished()), this, SLOT(updateCurrentStopTime()));
    connect(mpParentMainWindow->hidePortsAction, SIGNAL(triggered(bool)), this,SLOT(hidePortsInCurrentTab(bool)));
    connect(mpParentMainWindow->newAction, SIGNAL(triggered()), this,SLOT(addNewProjectTab()));
    connect(mpParentMainWindow->openAction, SIGNAL(triggered()), this,SLOT(loadModel()));
    connect(mpParentMainWindow->saveAction, SIGNAL(triggered()), this,SLOT(saveProjectTab()));

    connect(mpParentMainWindow->exportPDFAction, SIGNAL(triggered()), this,SLOT(exportCurrentToPDF()));
    connect(mpParentMainWindow->saveAsAction, SIGNAL(triggered()), this,SLOT(saveProjectTabAs()));
    connect(mpParentMainWindow->simulateAction, SIGNAL(triggered()), this,SLOT(simulateCurrent()));
    connect(mpParentMainWindow->resetZoomAction, SIGNAL(triggered()),this,SLOT(resetZoom()));
    connect(mpParentMainWindow->zoomInAction, SIGNAL(triggered()),this,SLOT(zoomIn()));

    connect(mpParentMainWindow->zoomOutAction, SIGNAL(triggered()),this,SLOT(zoomOut()));
    connect(mpParentMainWindow->hideNamesAction,SIGNAL(triggered()),this, SLOT(hideNames()));
    connect(mpParentMainWindow->showNamesAction,SIGNAL(triggered()),this, SLOT(showNames()));
    connect(mpParentMainWindow->centerViewAction,SIGNAL(triggered()),this,SLOT(centerView()));
    connect(mpParentMainWindow->disableUndoAction,SIGNAL(triggered()),this, SLOT(disableUndo()));
}


//!  Tells current tab to export itself to PDF. This is needed because a direct connection to current tab would be too complicated.
void ProjectTabWidget::exportCurrentToPDF()
{
    if(this->count() != 0)
    {
        this->getCurrentTab()->mpGraphicsView->exportPDF();
    }
}


//! Slot that tells the current project tab to hide its ports.
//! @param doIt is true if ports shall be hidden, otherwise false.
void ProjectTabWidget::hidePortsInCurrentTab(bool hidePortsActionTriggered)
{
    if(this->count() != 0)
    {
        this->getCurrentTab()->mpSystem->hidePorts(hidePortsActionTriggered);
    }
}


//! Slot that tells current project tab to update its start time value.
//! @see updateCurrentTimeStep()
//! @see updateCurrentStopTime()
void ProjectTabWidget::updateCurrentStartTime()
{
    if(this->count() != 0)
    {
        this->getCurrentTab()->mpSystem->updateStartTime();
    }
}


//! Slot that tells current project tab to update its time step value.
//! @see updateCurrentStartTime()
//! @see updateCurrentStopTime()
void ProjectTabWidget::updateCurrentTimeStep()
{
    if(this->count() != 0)
    {
        this->getCurrentTab()->mpSystem->updateTimeStep();
    }
}


//! Slot that tells current project tab to update its stop time value.
//! @see updateCurrentStartTime()
//! @see updateCurrentTimeStep()
void ProjectTabWidget::updateCurrentStopTime()
{
    if(this->count() != 0)
    {
        this->getCurrentTab()->mpSystem->updateStopTime();
    }
}


//! Slot that updates the values in the simulation setup widget to display new values when current project tab is changed.
void ProjectTabWidget::updateSimulationSetupWidget()
{
    if(this->count() != 0)  //Don't do anything if there are no current tab
    {
        mpParentMainWindow->setStartTimeLabel(getCurrentSystem()->getStartTime());
        mpParentMainWindow->setTimeStepLabel(getCurrentSystem()->getTimeStep());
        mpParentMainWindow->setFinishTimeLabel(getCurrentSystem()->getStopTime());
    }
}


//! Returns a pointer to the currently active project tab - be sure to check that the number of tabs is not zero before calling this
ProjectTab *ProjectTabWidget::getCurrentTab()
{
    return qobject_cast<ProjectTab *>(currentWidget());
}


//! Returns a pointer to the currently system model - be sure to check that the number of tabs is not zero before calling this
GUISystem *ProjectTabWidget::getCurrentSystem()
{
    return getCurrentTab()->mpSystem;
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

    newTab->mpSystem->mCoreSystemAccess.setRootSystemName(tabName);

    //addTab(newTab, tabName.append(QString("*")));
    this->addTab(newTab, tabName);
    this->setCurrentWidget(newTab);

    mNumberOfUntitledTabs += 1;
}


//! Saves current project.
//! @see saveProjectTab(int index)
void ProjectTabWidget::saveProjectTab()
{
    if(this->count() != 0)
    {
        saveProjectTab(currentIndex());
    }
}

//! Saves current project to a new model file.
//! @see saveProjectTab(int index)
void ProjectTabWidget::saveProjectTabAs()
{
    if(this->count() != 0)
    {
        saveProjectTab(currentIndex(), NEWFILE);
    }
}


//! Saves project at index.
//! @param index defines which project to save.
//! @see saveProjectTab()
void ProjectTabWidget::saveProjectTab(int index, saveTarget saveAsFlag)
{
    //ProjectTab *currentTab = qobject_cast<ProjectTab *>(widget(index));
    QString tabName = tabText(index);

    if (this->getCurrentTab()->mIsSaved)
    {
        //Nothing to do
        //statusBar->showMessage(QString("Project: ").append(tabName).append(QString(" is already saved")));
    }
    else
    {
        /*Add some "saving code" in the future:
         *
         *
         *
         */
        tabName.chop(1);
        setTabText(index, tabName);
        //statusBar->showMessage(QString("Project: ").append(tabName).append(QString(" saved")));
        std::cout << "ProjectTabWidget: " << qPrintable(QString("Project: ").append(tabName).append(QString(" saved"))) << std::endl;
        this->getCurrentTab()->mIsSaved = true;
    }
    this->saveModel(saveAsFlag);
}


//! Closes current project.
//! @param index defines which project to close.
//! @return true if closing went ok. false if the user canceled the operation.
//! @see closeAllProjectTabs()
bool ProjectTabWidget::closeProjectTab(int index)
{
    if (!(this->getCurrentTab()->mIsSaved))
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
            saveProjectTab(index, EXISTINGFILE);
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


//! Simulates the model in current open tab in a separate thread, the GUI runs a progressbar parallel to the simulation.
void ProjectTabWidget::simulateCurrent()
{

        //Check if simulation is possible
    if (this->count() == 0)
    {
        mpParentMainWindow->mpMessageWidget->printGUIMessage(QString("There is no open system to simulate"));
        return;
    }

    if(!this->getCurrentTab()->mpSystem->mCoreSystemAccess.isSimulationOk())
    {
        mpParentMainWindow->mpMessageWidget->printCoreMessages();
        mpParentMainWindow->mpMessageWidget->printGUIMessage(QString("Simulation failed"));
        return;
    }


        //Setup simulation parameters
    ProjectTab *pCurrentTab = getCurrentTab();
    double startTime = pCurrentTab->mpParentProjectTabWidget->mpParentMainWindow->getStartTimeLabel();
    double finishTime = pCurrentTab->mpParentProjectTabWidget->mpParentMainWindow->getFinishTimeLabel();
    double dt = finishTime - startTime;
    size_t nSteps = dt/pCurrentTab->mpSystem->mCoreSystemAccess.getDesiredTimeStep();


        //Ask core to initialize simulation
    InitializationThread actualInitialization(&(pCurrentTab->mpSystem->mCoreSystemAccess), startTime, finishTime, this);
    actualInitialization.start();
    actualInitialization.setPriority(QThread::HighestPriority);
    ProgressBarThread progressThread(this);

    QProgressDialog progressBar(tr("Initializing simulation..."), tr("&Abort initialization"), 0, 0, this);
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
            pCurrentTab->mpSystem->mCoreSystemAccess.stop();
        }
    }
    progressBar.setValue(i);
    actualInitialization.wait(); //Make sure actualSimulation do not goes out of scope during simulation
    actualInitialization.quit();


        //Ask core to execute (and finalize) simulation
    if (!progressBar.wasCanceled())
    {
        QTime simTimer;
        simTimer.start();
        SimulationThread actualSimulation(&(pCurrentTab->mpSystem->mCoreSystemAccess), startTime, finishTime, this);
        actualSimulation.start();
        actualSimulation.setPriority(QThread::HighestPriority);
            //! @todo TimeCriticalPriority seem to work on dual core, is it a problem on single core machines only?
        //actualSimulation.setPriority(QThread::TimeCriticalPriority); //No bar appears in Windows with this prio

        progressBar.setLabelText(tr("Running simulation..."));
        progressBar.setCancelButtonText(tr("&Abort simulation"));
        progressBar.setMinimum(0);
        progressBar.setMaximum(nSteps);
        while (actualSimulation.isRunning())
        {
           progressThread.start();
           progressThread.setPriority(QThread::LowestPriority);
           progressThread.wait();
           progressBar.setValue((size_t)(getCurrentTab()->mpSystem->mCoreSystemAccess.getCurrentTime()/dt * nSteps));
           if (progressBar.wasCanceled())
           {
              pCurrentTab->mpSystem->mCoreSystemAccess.stop();
           }
        }
        progressThread.quit();
        progressBar.setValue((size_t)(getCurrentTab()->mpSystem->mCoreSystemAccess.getCurrentTime()/dt * nSteps));

        actualSimulation.wait(); //Make sure actualSimulation do not goes out of scope during simulation
        actualSimulation.quit();
        //qDebug() << "Simulation time: " << (simTimer.elapsed()) << " ms";
        QString timeString;
        timeString.setNum(simTimer.elapsed());
        mpParentMainWindow->mpMessageWidget->printGUIInfoMessage(QString("Simulation time: ").append(timeString).append(" ms"));
    }

    if (progressBar.wasCanceled())
        mpParentMainWindow->mpMessageWidget->printGUIMessage(QString(tr("Simulation of '").append(pCurrentTab->mpSystem->mCoreSystemAccess.getRootSystemName()).append(tr("' was terminated!"))));
    else
    mpParentMainWindow->mpMessageWidget->printGUIMessage(QString(tr("Simulated '").append(pCurrentTab->mpSystem->mCoreSystemAccess.getRootSystemName()).append(tr("' successfully!"))));
    emit checkMessages();
}


//! Loads a model from a file and opens it in a new project tab.
//! @see saveModel(saveTarget saveAsFlag)
void ProjectTabWidget::loadModel()
{
    QDir fileDialogOpenDir;
    QString modelFileName = QFileDialog::getOpenFileName(this, tr("Choose Model File"),
                                                         fileDialogOpenDir.currentPath() + QString("/../../Models"),
                                                         tr("Hopsan Model Files (*.hmf)"));
    if (modelFileName.isEmpty())
        return;

    QFile file(modelFileName);   //Create a QFile object
    QFileInfo fileInfo(file);

    for(int t=0; t!=this->count(); ++t)
    {
        if( (this->tabText(t) == fileInfo.fileName()) or (this->tabText(t) == (fileInfo.fileName() + "*")) )
        {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::information(this, tr("Error"), tr("Unable to load model. File is already open."));
            return;
        }
    }


    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))  //open file
    {
        qDebug() << "Failed to open file or not a text file: " + modelFileName;
        return;
    }
    QTextStream inputStream(&file);  //Create a QTextStream object to stream the content of file


    this->addProjectTab(new ProjectTab(this), fileInfo.fileName());
    ProjectTab *pCurrentTab = qobject_cast<ProjectTab *>(currentWidget());
    pCurrentTab->mpSystem->mModelFileName = modelFileName;
    pCurrentTab->mpSystem->mUndoStack->newPost();
    pCurrentTab->mIsSaved = true;

    //Read the header data, also checks version numbers
    //! @todo maybe not check the version numbers in there
    HeaderLoadData headerData = readHeader(inputStream, mpParentMainWindow->mpMessageWidget);

    //It is assumed that these data have been successfully read
    mpParentMainWindow->setStartTimeLabel(headerData.startTime);
    mpParentMainWindow->setTimeStepLabel(headerData.timeStep);
    mpParentMainWindow->setFinishTimeLabel(headerData.stopTime);

    //It is assumed that these data have been successfully read
    getCurrentTab()->mpGraphicsView->centerOn(headerData.viewport_x, headerData.viewport_y);
    getCurrentTab()->mpGraphicsView->scale(headerData.viewport_zoomfactor, headerData.viewport_zoomfactor);
    getCurrentTab()->mpGraphicsView->mZoomFactor = headerData.viewport_zoomfactor;
    getCurrentTab()->mpGraphicsView->resetBackgroundBrush();

    //Sets the file name (exluding path and ending) as the model name
    getCurrentTab()->mpSystem->mCoreSystemAccess.setRootSystemName(fileInfo.baseName());

    while ( !inputStream.atEnd() )
    {
        //Extract first word on line
        QString inputWord;
        inputStream >> inputWord;

        if ( (inputWord == "SUBSYSTEM") or (inputWord == "BEGINSUBSYSTEM") )
        {
            SubsystemLoadData subsysData;
            subsysData.read(inputStream);
            loadSubsystemGUIObject(subsysData, mpParentMainWindow->mpLibrary, pCurrentTab->mpSystem, NOUNDO);
            //! @todo convenience function
        }

        if ( (inputWord == "COMPONENT") || (inputWord == "SYSTEMPORT") )
        {
            loadGUIObject(inputStream, mpParentMainWindow->mpLibrary, pCurrentTab->mpSystem, NOUNDO);
        }


        if ( inputWord == "PARAMETER" )
        {
            loadParameterValues(inputStream, pCurrentTab->mpSystem, NOUNDO);
        }


        if ( inputWord == "CONNECT" )
        {
            loadConnector(inputStream, pCurrentTab->mpSystem, &(pCurrentTab->mpSystem->mCoreSystemAccess), NOUNDO);
        }
    }
    //Deselect all components
   //pCurrentTab->mpGraphicsView->deselectAllGUIObjects();

    pCurrentTab->mpSystem->deselectAll();
    this->centerView();
    pCurrentTab->mpSystem->mUndoStack->clear();
    pCurrentTab->mpGraphicsView->resetBackgroundBrush();

    emit checkMessages();
}


//! Saves the model in the active project tab to a model file.
//! @param saveAsFlag tells whether or not an already existing file name shall be used
//! @see saveProjectTab()
//! @see loadModel()
void ProjectTabWidget::saveModel(saveTarget saveAsFlag)
{
    ProjectTab *pCurrentTab = qobject_cast<ProjectTab *>(currentWidget());
    GraphicsView *pCurrentView = pCurrentTab->mpGraphicsView;

    QString modelFileName;
    if((pCurrentTab->mpSystem->mModelFileName.isEmpty()) | (saveAsFlag == NEWFILE))
    {
        QDir fileDialogSaveDir;
        modelFileName = QFileDialog::getSaveFileName(this, tr("Save Model File"),
                                                             fileDialogSaveDir.currentPath() + QString("/../../Models"),
                                                             tr("Hopsan Model Files (*.hmf)"));
        pCurrentTab->mpSystem->mModelFileName = modelFileName;
    }
    else
    {
        modelFileName = pCurrentTab->mpSystem->mModelFileName;
    }

    QFile file(modelFileName);   //Create a QFile object
    QFileInfo fileInfo(file);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
    {
        qDebug() << "Failed to open file for writing: " + modelFileName;
        return;
    }

    //Sets the model name (must set this name before saving or else systemports wont know the real name of their rootsystem parent)
    pCurrentTab->mpSystem->mCoreSystemAccess.setRootSystemName(fileInfo.baseName());
    this->setTabText(this->currentIndex(), fileInfo.fileName());

    //QLineF line(QPointF(sceneCenterPointF.x(), sceneCenterPointF.y()), QPointF(groupPortPoint.x(), groupPortPoint.y()));

    QTextStream modelFile(&file);  //Create a QTextStream object to stream the content of file

    writeHeader(modelFile);

    modelFile << "SIMULATIONTIME " << mpParentMainWindow->getStartTimeLabel() << " " << mpParentMainWindow->getTimeStepLabel() << " " <<  mpParentMainWindow->getFinishTimeLabel() << "\n";
    modelFile << "VIEWPORT " << (getCurrentTab()->mpGraphicsView->horizontalScrollBar()->value() + getCurrentTab()->mpGraphicsView->width()/2 - getCurrentTab()->mpGraphicsView->pos().x()) / getCurrentTab()->mpGraphicsView->mZoomFactor << " " <<
                                (getCurrentTab()->mpGraphicsView->verticalScrollBar()->value() + getCurrentTab()->mpGraphicsView->height()/2 - getCurrentTab()->mpGraphicsView->pos().x()) / getCurrentTab()->mpGraphicsView->mZoomFactor << " " <<
                                getCurrentTab()->mpGraphicsView->mZoomFactor << "\n";
    modelFile << "--------------------------------------------------------------\n";
    modelFile << "USERICON " << addQuotes(getCurrentSystem()->getUserIconPath()) << "\n";
    modelFile << "ISOICON " << addQuotes(getCurrentSystem()->getIsoIconPath()) << "\n";

    //Calculate the position of the subsystem ports:
    QMap<QString, GUIObject*>::iterator it;
    QLineF line;
    double angle, x, y;

    double xMax = pCurrentTab->mpSystem->mGUIObjectMap.begin().value()->x()+pCurrentTab->mpSystem->mGUIObjectMap.begin().value()->rect().width()/2.0;
    double xMin = pCurrentTab->mpSystem->mGUIObjectMap.begin().value()->x()+pCurrentTab->mpSystem->mGUIObjectMap.begin().value()->rect().width()/2.0;
    double yMax = pCurrentTab->mpSystem->mGUIObjectMap.begin().value()->y()+pCurrentTab->mpSystem->mGUIObjectMap.begin().value()->rect().height()/2.0;
    double yMin = pCurrentTab->mpSystem->mGUIObjectMap.begin().value()->y()+pCurrentTab->mpSystem->mGUIObjectMap.begin().value()->rect().height()/2.0;

    for(it = pCurrentTab->mpSystem->mGUIObjectMap.begin(); it!=pCurrentTab->mpSystem->mGUIObjectMap.end(); ++it)
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
    //getCurrentTab()->mpGraphicsScene->addRect(xMin, yMin, w, h); //debug-grej

    for(it = pCurrentTab->mpSystem->mGUIObjectMap.begin(); it!=pCurrentTab->mpSystem->mGUIObjectMap.end(); ++it)
    {
        if(it.value()->getTypeName() == "SystemPort")
        {
            line = QLineF(center.x(), center.y(), it.value()->x()+it.value()->rect().width()/2, it.value()->y()+it.value()->rect().height()/2);
            //getCurrentTab()->mpGraphicsScene->addLine(line); //debug-grej
            angle = line.angle()*3.141592/180.0;
            calcSubsystemPortPosition(w, h, angle, x, y);
            x = (x/w+1)/2; //Change coordinate system
            y = (-y/h+1)/2; //Change coordinate system
            modelFile << "PORT " << addQuotes(it.value()->getName()) <<" " << x << " " << y << " " << angle << "\n";
        }
    }
        modelFile << "--------------------------------------------------------------\n";

    //QMap<QString, GUIObject*>::iterator it;
    for(it = pCurrentTab->mpSystem->mGUIObjectMap.begin(); it!=pCurrentTab->mpSystem->mGUIObjectMap.end(); ++it)
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

    for(int i = 0; i != pCurrentTab->mpSystem->mConnectorVector.size(); ++i)
    {
        pCurrentTab->mpSystem->mConnectorVector[i]->saveToTextStream(modelFile, "CONNECT");
    }
    modelFile << "--------------------------------------------------------------\n";
}


//! Tells the current tab to change to or from ISO graphics.
//! @param value is true if ISO should be activated and false if it should be deactivated.
//! @todo Break out the guiconnector appearance stuff into a separate general function
void ProjectTabWidget::setIsoGraphics(graphicsType gfxType)
{
    if(this->count() != 0)
    {
        this->getCurrentSystem()->mGfxType = gfxType;
        mpParentMainWindow->mpLibrary->setGfxType(gfxType);
        ProjectTab *pCurrentTab = getCurrentTab();
       // GraphicsView *pCurrentView = pCurrentTab->mpGraphicsView;

        for(int i = 0; i!=pCurrentTab->mpSystem->mConnectorVector.size(); ++i)
        {
            pCurrentTab->mpSystem->mConnectorVector[i]->setIsoStyle(gfxType);
        }

        QMap<QString, GUIObject*>::iterator it2;
        for(it2 = pCurrentTab->mpSystem->mGUIObjectMap.begin(); it2!=pCurrentTab->mpSystem->mGUIObjectMap.end(); ++it2)
        {
            it2.value()->setIcon(gfxType);
        }
    }
}

//! Tells the current tab to reset zoom to 100%.
//! @see zoomIn()
//! @see zoomOut()
void ProjectTabWidget::resetZoom()
{
    if(this->count() != 0)
    {
        this->getCurrentTab()->mpGraphicsView->resetZoom();
    }
}


//! Tells the current tab to increase its zoom factor.
//! @see resetZoom()
//! @see zoomOut()
void ProjectTabWidget::zoomIn()
{
    if(this->count() != 0)
    {
        this->getCurrentTab()->mpGraphicsView->zoomIn();
    }
}


//! Tells the current tab to decrease its zoom factor.
//! @see resetZoom()
//! @see zoomIn()
void ProjectTabWidget::zoomOut()
{
    if(this->count() != 0)
    {
        this->getCurrentTab()->mpGraphicsView->zoomOut();
    }
}


//! Tells the current tab to hide all component names.
//! @see showNames()
void ProjectTabWidget::hideNames()
{
    if(this->count() != 0)
    {
        this->getCurrentTab()->mpSystem->hideNames();
    }
}


//! Tells the current tab to show all component names.
//! @see hideNames()
void ProjectTabWidget::showNames()
{
    if(this->count() != 0)
    {
        this->getCurrentTab()->mpSystem->showNames();
    }
}


//! Tells the current tab to center the viewport
void ProjectTabWidget::centerView()
{
    if(this->count() != 0)
    {
        this->getCurrentTab()->mpGraphicsView->centerOn(getCurrentTab()->mpGraphicsView->sceneRect().center());
    }
}


//! Disables the undo function for the current model
void ProjectTabWidget::disableUndo()
{
    if(this->count() != 0)
    {
        if(!getCurrentTab()->mpSystem->mUndoDisabled)
        {
            QMessageBox disableUndoWarningBox(QMessageBox::Warning, tr("Warning"),tr("Disabling undo history will clear all undo history for this model. Do you want to continue?"), 0, this);
            disableUndoWarningBox.addButton(tr("&Yes"), QMessageBox::AcceptRole);
            disableUndoWarningBox.addButton(tr("&No"), QMessageBox::RejectRole);

            if (disableUndoWarningBox.exec() == QMessageBox::AcceptRole)
            {
                getCurrentTab()->mpSystem->clearUndo();
                getCurrentTab()->mpSystem->mUndoDisabled = true;
                mpParentMainWindow->undoAction->setDisabled(true);
                mpParentMainWindow->redoAction->setDisabled(true);
            }
            else
            {
                return;
            }
        }
        else
        {
            getCurrentTab()->mpSystem->mUndoDisabled = false;
            mpParentMainWindow->undoAction->setDisabled(false);
            mpParentMainWindow->redoAction->setDisabled(false);
        }
    }
}


//! Enables or disables the undo buttons depending on whether or not undo is disabled in current tab
void ProjectTabWidget::updateUndoStatus()
{
    if(this->count() != 0)
    {
        if(getCurrentTab()->mpSystem->mUndoDisabled)
        {
            mpParentMainWindow->undoAction->setDisabled(true);
            mpParentMainWindow->redoAction->setDisabled(true);
        }
        else
        {
            mpParentMainWindow->undoAction->setDisabled(false);
            mpParentMainWindow->redoAction->setDisabled(false);
        }
    }
}
