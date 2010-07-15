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
#include "UndoStack.h"
#include "LibraryWidget.h"
#include "GUIObject.h"
#include "GUIConnector.h"
#include "GraphicsScene.h"
#include "GraphicsView.h"


#include "version.h"
#include "GUIUtilities.h"


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

    mStartTime = 0;
    mTimeStep = 0.01;
    mStopTime = 5;

    MainWindow *pMainWindow = mpParentProjectTabWidget->mpParentMainWindow;
    connect(this, SIGNAL(checkMessages()), pMainWindow->mpMessageWidget, SLOT(checkMessages()));

    mGUIRootSystem.setDesiredTimeStep(.001);
    mGUIRootSystem.setRootTypeCQS("S");

    emit checkMessages();

    double timeStep = mGUIRootSystem.getDesiredTimeStep();

    mpParentProjectTabWidget->mpParentMainWindow->setTimeStepLabel(timeStep);

    mIsSaved = true;
    mModelFileName.clear();

    mpGraphicsScene = new GraphicsScene(this);

    mpGraphicsView  = new GraphicsView(this);

    mpGraphicsView->setScene(mpGraphicsScene);

    QVBoxLayout *tabLayout = new QVBoxLayout;

    tabLayout->addWidget(mpGraphicsView);

    //    addStretch(1);
    //    setWindowModified(true);

    setLayout(tabLayout);

    this->useIsoGraphics = false;

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


//! Slot that updates start time value of the current project to the one in the simulation setup widget.
//! @see updateTimeStep()
//! @see updateStopTime()
void ProjectTab::updateStartTime()
{
    mStartTime = mpParentProjectTabWidget->mpParentMainWindow->getStartTimeLabel();
}


//! Slot that updates time step value of the current project to the one in the simulation setup widget.
//! @see updateStartTime()
//! @see updateStopTime()
void ProjectTab::updateTimeStep()
{
    mTimeStep = mpParentProjectTabWidget->mpParentMainWindow->getTimeStepLabel();
}


//! Slot that updates stop time value of the current project to the one in the simulation setup widget.
//! @see updateStartTime()
//! @see updateTimeStep()
void ProjectTab::updateStopTime()
{
    mStopTime = mpParentProjectTabWidget->mpParentMainWindow->getFinishTimeLabel();
}


//! Returns the start time value of the current project.
//! @see getTimeStep()
//! @see getStopTime()
double ProjectTab::getStartTime()
{
    return mStartTime;
}


//! Returns the time step value of the current project.
//! @see getStartTime()
//! @see getStopTime()
double ProjectTab::getTimeStep()
{
    return mTimeStep;
}


//! Returns the stop time value of the current project.
//! @see getStartTime()
//! @see getTimeStep()
double ProjectTab::getStopTime()
{
    return mStopTime;
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
}


//!  Tells current tab to export itself to PDF. This is needed because a direct connection to current tab would be too complicated.
void ProjectTabWidget::exportCurrentToPDF()
{
    getCurrentTab()->mpGraphicsView->exportPDF();
}


//! Slot that tells the current project tab to hide its ports.
//! @param doIt is true if ports shall be hidden, otherwise false.
void ProjectTabWidget::hidePortsInCurrentTab(bool doIt)
{
    this->getCurrentTab()->mpGraphicsView->hidePorts(doIt);
}


//! Slot that tells current project tab to update its start time value.
//! @see updateCurrentTimeStep()
//! @see updateCurrentStopTime()
void ProjectTabWidget::updateCurrentStartTime()
{
    getCurrentTab()->updateStartTime();
}


//! Slot that tells current project tab to update its time step value.
//! @see updateCurrentStartTime()
//! @see updateCurrentStopTime()
void ProjectTabWidget::updateCurrentTimeStep()
{
    getCurrentTab()->updateTimeStep();
}


//! Slot that tells current project tab to update its stop time value.
//! @see updateCurrentStartTime()
//! @see updateCurrentTimeStep()
void ProjectTabWidget::updateCurrentStopTime()
{
    getCurrentTab()->updateStopTime();
}


//! Slot that updates the values in the simulation setup widget to display new values when current project tab is changed.
void ProjectTabWidget::updateSimulationSetupWidget()
{
    if(this->count() != 0)  //Don't do anything if there are no current tab
    {
        mpParentMainWindow->setStartTimeLabel(getCurrentTab()->getStartTime());
        mpParentMainWindow->setTimeStepLabel(getCurrentTab()->getTimeStep());
        mpParentMainWindow->setFinishTimeLabel(getCurrentTab()->getStopTime());
    }
}


//! Returns a pointer to the currently active project tab
ProjectTab *ProjectTabWidget::getCurrentTab()
{
    return qobject_cast<ProjectTab *>(currentWidget());
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

    newTab->mGUIRootSystem.setRootSystemName(tabName);

    //addTab(newTab, tabName.append(QString("*")));
    addTab(newTab, tabName);
    setCurrentWidget(newTab);

    mNumberOfUntitledTabs += 1;
}


//! Saves current project.
//! @see saveProjectTab(int index)
void ProjectTabWidget::saveProjectTab()
{
    saveProjectTab(currentIndex(), false);
}

//! Saves current project to a new model file.
//! @see saveProjectTab(int index)
void ProjectTabWidget::saveProjectTabAs()
{
    saveProjectTab(currentIndex(), true);
}


//! Saves project at index.
//! @param index defines which project to save.
//! @see saveProjectTab()
void ProjectTabWidget::saveProjectTab(int index, bool saveAs)
{
    ProjectTab *currentTab = qobject_cast<ProjectTab *>(widget(index));
    QString tabName = tabText(index);

    if (currentTab->mIsSaved)
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
        currentTab->mIsSaved = true;
    }
    this->saveModel(saveAs);
}


//! Closes current project.
//! @param index defines which project to close.
//! @return true if closing went ok. false if the user canceled the operation.
//! @see closeAllProjectTabs()
bool ProjectTabWidget::closeProjectTab(int index)
{
    if (!(qobject_cast<ProjectTab *>(widget(index))->mIsSaved))
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
            saveProjectTab(index, false);
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
    if (!currentWidget())
    {
        mpParentMainWindow->mpMessageWidget->printGUIMessage(QString("There is no open system to simulate"));
        return;
    }

    if(!this->getCurrentTab()->mGUIRootSystem.isSimulationOk())
    {
        mpParentMainWindow->mpMessageWidget->printCoreMessages();
        mpParentMainWindow->mpMessageWidget->printGUIMessage(QString("Simulation failed"));
        return;
    }


    ProjectTab *pCurrentTab = getCurrentTab();

    double startTime = pCurrentTab->mpParentProjectTabWidget->mpParentMainWindow->getStartTimeLabel();
    double finishTime = pCurrentTab->mpParentProjectTabWidget->mpParentMainWindow->getFinishTimeLabel();
    
    QString timeTxt;
    double dt = finishTime - startTime;
    size_t nSteps = dt/pCurrentTab->mGUIRootSystem.getDesiredTimeStep();

    QProgressDialog progressBar(tr("Initialize simulation..."), tr("&Abort initialization"), 0, 0, this);
    std::cout << progressBar.geometry().width() << " " << progressBar.geometry().height() << std::endl;
    progressBar.setWindowModality(Qt::WindowModal);
    progressBar.setWindowTitle(tr("Simulate!"));

    InitializationThread actualInitialization(&(pCurrentTab->mGUIRootSystem), startTime, finishTime, this);

    size_t i=0;
    actualInitialization.start();
//    actualInitialization.setPriority(QThread::TimeCriticalPriority); //No bar appears in Windows with this prio
    actualInitialization.setPriority(QThread::HighestPriority);
    while (actualInitialization.isRunning())
    {
        progressBar.setValue(i++);
        if (progressBar.wasCanceled())
        {
            pCurrentTab->mGUIRootSystem.stop();
        }
    }
    progressBar.setValue(i);
    actualInitialization.wait(); //Make sure actualSimulation do not goes out of scope during simulation

    if (!progressBar.wasCanceled())
    {
        SimulationThread actualSimulation(&(pCurrentTab->mGUIRootSystem), startTime, finishTime, this);
        actualSimulation.start();
//        actualSimulation.setPriority(QThread::TimeCriticalPriority); //No bar appears in Windows with this prio
        actualSimulation.setPriority(QThread::HighestPriority);
        progressBar.setLabelText(tr("Running simulation..."));
        progressBar.setCancelButtonText(tr("&Abort simulation"));
        progressBar.setMinimum(0);
        progressBar.setMaximum(nSteps);
        while (actualSimulation.isRunning())
        {
            progressBar.setValue((size_t)(getCurrentTab()->mGUIRootSystem.getCurrentTime()/dt * nSteps));
            if (progressBar.wasCanceled())
            {
                pCurrentTab->mGUIRootSystem.stop();
            }
        }
        progressBar.setValue((size_t)(getCurrentTab()->mGUIRootSystem.getCurrentTime()/dt * nSteps));
        actualSimulation.wait(); //Make sure actualSimulation do not goes out of scope during simulation
    }

    if (progressBar.wasCanceled())
        mpParentMainWindow->mpMessageWidget->printGUIMessage(QString(tr("Simulation of '").append(pCurrentTab->mGUIRootSystem.getName()).append(tr("' was terminated!"))));
    else
        mpParentMainWindow->mpMessageWidget->printGUIMessage(QString(tr("Simulated '").append(pCurrentTab->mGUIRootSystem.getName()).append(tr("' successfully!"))));

    emit checkMessages();
}

//! Loads a model from a file and opens it in a new project tab.
//! @see saveModel(bool saveAs)
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
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))  //open file
    {
        qDebug() << "Failed to open file or not a text file: " + modelFileName;
        return;
    }
    QTextStream inputStream(&file);  //Create a QTextStream object to stream the content of file


    this->addProjectTab(new ProjectTab(this), fileInfo.fileName());
    ProjectTab *pCurrentTab = qobject_cast<ProjectTab *>(currentWidget());
    pCurrentTab->mModelFileName = modelFileName;
    pCurrentTab->mpGraphicsView->undoStack->newPost();
    pCurrentTab->mIsSaved = true;

        //Necessary declarations
    QString inputWord, componentType, componentName, startComponentName, endComponentName, parameterName, startPortName, endPortName, tempString;
    //int length, heigth;
    qreal posX, posY;
    int nameTextPos;
    qreal rotation;
    double parameterValue;

    while ( !inputStream.atEnd() )
    {
        //Extract first word on line
        inputStream >> inputWord;

        //----------- Create New SubSystem -----------//

        if ( inputWord == "HOPSANGUIVERSION")
        {
            inputStream >> tempString;
            if(tempString > QString(HOPSANGUIVERSION))
            {
                mpParentMainWindow->mpMessageWidget->printGUIWarningMessage(QString("Warning: File was saved in newer version of Hopsan"));
            }
            else if(tempString < QString(HOPSANGUIVERSION))
            {
                mpParentMainWindow->mpMessageWidget->printGUIWarningMessage(QString("Warning: File was saved in older version of Hopsan"));
            }
        }
        else if ( inputWord == "HOPSANGUIMODELFILEVERSION")
        {
            inputStream >> tempString;
            if(tempString > QString(HOPSANGUIMODELFILEVERSION))
            {
                mpParentMainWindow->mpMessageWidget->printGUIWarningMessage(QString("Warning: File was saved in newer version of Hopsan"));
            }
            else if(tempString < QString(HOPSANGUIMODELFILEVERSION))
            {
                mpParentMainWindow->mpMessageWidget->printGUIWarningMessage(QString("Warning: File was saved in older version of Hopsan"));
            }
        }
        else if ( inputWord == "HOPSANGUICOMPONENTDESCRIPTIONFILEVERSION")
        {
            inputStream >> tempString;
            qDebug() << inputWord << " " << tempString;
            if(tempString > QString(HOPSANGUICOMPONENTDESCRIPTIONFILEVERSION))
            {
                mpParentMainWindow->mpMessageWidget->printGUIWarningMessage(QString("Warning: File was saved in newer version of Hopsan"));
            }
            else if(tempString < QString(HOPSANGUICOMPONENTDESCRIPTIONFILEVERSION))
            {
                mpParentMainWindow->mpMessageWidget->printGUIWarningMessage(QString("Warning: File was saved in older version of Hopsan"));
            }
        }

        if ( inputWord == "STARTTIME" )
        {
            double startTime;
            inputStream >> startTime;
            mpParentMainWindow->setStartTimeLabel(startTime);
        }

        if ( inputWord == "TIMESTEP" )
        {
            double timeStep;
            inputStream >> timeStep;
            mpParentMainWindow->setTimeStepLabel(timeStep);
        }

        if ( inputWord == "FINISHTIME" )
        {
            double finishTime;
            inputStream >> finishTime;
            mpParentMainWindow->setFinishTimeLabel(finishTime);
        }

        if ( inputWord == "COMPONENT" )
        {
            componentType = readName(inputStream);
            componentName = readName(inputStream);  //Now read the name, assume that the name is contained within quotes signs, "name"
            inputStream >> posX;
            inputStream >> posY;
            inputStream >> rotation;
            inputStream >> nameTextPos;

            //! @todo This component need to be loaded in the library, or maybe we should auto load it if possible if missing (probably dfficult)
            //qDebug() << "componentType: " << componentType;
            AppearanceData appearanceData = *mpParentMainWindow->mpLibrary->getAppearanceData(componentType);
            appearanceData.setName(componentName);
            pCurrentTab->mpGraphicsView->addGUIObject(appearanceData, QPoint(posX, posY), 0);
            pCurrentTab->mpGraphicsView->getGUIObject(componentName)->setNameTextPos(nameTextPos);
            while(pCurrentTab->mpGraphicsView->getGUIObject(componentName)->rotation() != rotation)
            {
                pCurrentTab->mpGraphicsView->getGUIObject(componentName)->rotate();
            }
        }


        if ( inputWord == "PARAMETER" )
        {
            componentName = readName(inputStream);
            inputStream >> parameterName;
            inputStream >> parameterValue;

            //qDebug() << "Parameter: " << componentName << " " << parameterName << " " << parameterValue;
            pCurrentTab->mpGraphicsView->mGUIObjectMap.find(componentName).value()->setParameterValue(parameterName, parameterValue);
        }


        if ( inputWord == "CONNECT" )
        {

            GraphicsView *pCurrentView = pCurrentTab->mpGraphicsView;
            startComponentName = readName(inputStream);
            startPortName = readName(inputStream);
            endComponentName = readName(inputStream);
            endPortName = readName(inputStream);
            GUIPort *startPort = pCurrentView->getGUIObject(startComponentName)->getPort(startPortName);
            GUIPort *endPort = pCurrentView->getGUIObject(endComponentName)->getPort(endPortName);

            bool success = pCurrentTab->mGUIRootSystem.connect(startComponentName, startPortName, endComponentName, endPortName);
            if (!success)
            {
                qDebug() << "Unsuccessful connection try" << endl;
            }
            else
            {
                QVector<QPointF> tempPointVector;
                qreal tempX, tempY;

                QString restOfLineString = inputStream.readLine();
                QTextStream restOfLineStream(&restOfLineString);
                while( !restOfLineStream.atEnd() )
                {
                    restOfLineStream >> tempX;
                    restOfLineStream >> tempY;
                    tempPointVector.push_back(QPointF(tempX, tempY));
                }

                //! @todo: Store useIso bool in model file and pick the correct line styles when loading
                GUIConnectorAppearance *pConnApp = new GUIConnectorAppearance(startPort->getPortType(), pCurrentTab->useIsoGraphics);
                GUIConnector *pTempConnector = new GUIConnector(startPort, endPort, tempPointVector, pConnApp, pCurrentView);

                pCurrentView->scene()->addItem(pTempConnector);

                //Hide connected ports
                startPort->hide();
                endPort->hide();

                connect(startPort->getGuiObject(),SIGNAL(componentDeleted()),pTempConnector,SLOT(deleteMeWithNoUndo()));
                connect(endPort->getGuiObject(),SIGNAL(componentDeleted()),pTempConnector,SLOT(deleteMeWithNoUndo()));

                pCurrentView->mConnectorVector.append(pTempConnector);
            }
        }
    }
    //Deselect all comonents
    GraphicsView *pCurrentView = pCurrentTab->mpGraphicsView;
    QMap<QString, GUIObject *>::iterator it;
    for(it = pCurrentView->mGUIObjectMap.begin(); it!=pCurrentView->mGUIObjectMap.end(); ++it)
    {
        it.value()->setSelected(false);
    }

    //Sets the file name as model name
    getCurrentTab()->mGUIRootSystem.setRootSystemName(fileInfo.fileName());

    pCurrentView->undoStack->clear();
    pCurrentView->resetBackgroundBrush();

    emit checkMessages();
}


//! Saves the model in the active project tab to a model file.
//! @param saveAs tells whether or not an already existing file name shall be used
//! @see saveProjectTab()
//! @see loadModel()
void ProjectTabWidget::saveModel(bool saveAs)
{
    ProjectTab *pCurrentTab = qobject_cast<ProjectTab *>(currentWidget());
    GraphicsView *pCurrentView = pCurrentTab->mpGraphicsView;

    QString modelFileName;
    if(pCurrentTab->mModelFileName.isEmpty() | saveAs)
    {
        QDir fileDialogSaveDir;
        modelFileName = QFileDialog::getSaveFileName(this, tr("Save Model File"),
                                                             fileDialogSaveDir.currentPath() + QString("/../../Models"),
                                                             tr("Hopsan Model Files (*.hmf)"));
        pCurrentTab->mModelFileName = modelFileName;
    }
    else
    {
        modelFileName = pCurrentTab->mModelFileName;
    }

    QFile file(modelFileName);   //Create a QFile object
    QFileInfo fileInfo(file);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
    {
        qDebug() << "Failed to open file for writing: " + modelFileName;
        return;
    }
    QTextStream modelFile(&file);  //Create a QTextStream object to stream the content of file

    modelFile << "--------------------------------------------------------------\n";
    modelFile << "-------------------  HOPSAN NG MODEL FILE  -------------------\n";
    modelFile << "--------------------------------------------------------------\n";
    modelFile << "HOPSANGUIVERSION " << HOPSANGUIVERSION << "\n";
    modelFile << "HOPSANGUIMODELFILEVERSION " << HOPSANGUIMODELFILEVERSION << "\n";
    modelFile << "HOPSANGUICOMPONENTDESCRIPTIONFILEVERSION " << HOPSANGUICOMPONENTDESCRIPTIONFILEVERSION << "\n";
    modelFile << "--------------------------------------------------------------\n";

    modelFile << "STARTTIME " << mpParentMainWindow->getStartTimeLabel() << "\n";
    modelFile << "TIMESTEP " << mpParentMainWindow->getTimeStepLabel() << "\n";
    modelFile << "FINISHTIME " << mpParentMainWindow->getFinishTimeLabel() << "\n";
    modelFile << "--------------------------------------------------------------\n";

    QMap<QString, GUIObject*>::iterator it;
    for(it = pCurrentView->mGUIObjectMap.begin(); it!=pCurrentView->mGUIObjectMap.end(); ++it)
    {
        it.value()->saveToTextStream(modelFile);
    }

    modelFile << "--------------------------------------------------------------\n";

   // QMap<QString, GUIConnector *>::iterator it2;
    for(int i = 0; i != pCurrentView->mConnectorVector.size(); ++i)
    {
//        QString startPortName  = pCurrentView->mConnectorVector[i]->getStartPort()->getName();
//        QString endPortName = pCurrentView->mConnectorVector[i]->getEndPort()->getName();
//        modelFile << "CONNECT " << QString(addQuotes(pCurrentView->mConnectorVector[i]->getStartPort()->getGuiObject()->getName()) + " " + addQuotes(startPortName) + " " +
//                                           addQuotes(pCurrentView->mConnectorVector[i]->getEndPort()->getGuiObject()->getName()) + " " + addQuotes(endPortName));
//        for(size_t j = 0; j != pCurrentView->mConnectorVector[i]->getPointsVector().size(); ++j)
//        {
//            modelFile << " " << pCurrentView->mConnectorVector[i]->getPointsVector()[j].x() << " " << pCurrentView->mConnectorVector[i]->getPointsVector()[j].y();
//        }
//        modelFile << "\n";
        pCurrentView->mConnectorVector[i]->saveToTextStream(modelFile);
    }
    modelFile << "--------------------------------------------------------------\n";

    //Sets the model name
    pCurrentTab->mGUIRootSystem.setRootSystemName(fileInfo.fileName());
    this->setTabText(this->currentIndex(), fileInfo.fileName());
}


//! Tells the current tab to change to or from ISO graphics.
//! @param value is true if ISO should be activated and false if it should be deactivated.
//! @todo Break out the guiconnector appearance stuff into a separate general function
void ProjectTabWidget::setIsoGraphics(bool useISO)
{
    this->getCurrentTab()->useIsoGraphics = useISO;

    mpParentMainWindow->mpLibrary->useIsoGraphics(useISO);

    ProjectTab *pCurrentTab = getCurrentTab();
    GraphicsView *pCurrentView = pCurrentTab->mpGraphicsView;
    //QMap<QString, GUIConnector *>::iterator it;

    for(int i = 0; i!=pCurrentView->mConnectorVector.size(); ++i)
    {
        pCurrentView->mConnectorVector[i]->setIsoStyle(useISO);
    }

    QMap<QString, GUIObject*>::iterator it2;
    for(it2 = pCurrentView->mGUIObjectMap.begin(); it2!=pCurrentView->mGUIObjectMap.end(); ++it2)
    {
        it2.value()->setIcon(useISO);
    }
}


//! Tells the current tab to reset zoom to 100%.
//! @see zoomIn()
//! @see zoomOut()
void ProjectTabWidget::resetZoom()
{
    this->getCurrentTab()->mpGraphicsView->resetZoom();
}


//! Tells the current tab to increase its zoom factor.
//! @see resetZoom()
//! @see zoomOut()
void ProjectTabWidget::zoomIn()
{
    this->getCurrentTab()->mpGraphicsView->zoomIn();
}


//! Tells the current tab to decrease its zoom factor.
//! @see resetZoom()
//! @see zoomIn()
void ProjectTabWidget::zoomOut()
{
    this->getCurrentTab()->mpGraphicsView->zoomOut();
}


//! Tells the current tab to hide all component names.
//! @see showNames()
void ProjectTabWidget::hideNames()
{
    this->getCurrentTab()->mpGraphicsView->hideNames();
}


//! Tells the current tab to show all component names.
//! @see hideNames()
void ProjectTabWidget::showNames()
{
    this->getCurrentTab()->mpGraphicsView->showNames();
}
