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

//$Id$

#include <iostream>

#include "MainWindow.h"
#include "version.h"
#include "common.h"

#include "PlotWidget.h"
#include "MessageWidget.h"
#include "PreferenceWidget.h"
#include "OptionsWidget.h"
#include "UndoStack.h"
#include "LibraryWidget.h"
#include "ProjectTabWidget.h"
#include "GraphicsView.h"
#include "GraphicsScene.h"
#include "GUISystem.h"
#include "GUIUtilities.h"


//! Constructor
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    //Set the name and size of the main window
    this->setObjectName("MainWindow");
    this->resize(1024,768);
    this->setFont(QFont("Comic Sans"));
    this->setWindowTitle("HOPSAN NG");
    this->setWindowIcon(QIcon("../../HopsanGUI/icons/hopsan.png"));
    this->setDockOptions(QMainWindow::ForceTabbedDocks);
    mPlotVariableListOpen = false;

    QMetaObject::connectSlotsByName(this);

    //Create a centralwidget for the main window
    mpCentralwidget = new QWidget(this);
    mpCentralwidget->setObjectName("centralwidget");

    //Create a grid on the centralwidget
    mpCentralgrid = new QGridLayout(mpCentralwidget);
    mpCentralgrid->setSpacing(10);

    //Create a dock for the MessageWidget
    mpMessageDock = new QDockWidget(tr("Messages"), this);
    mpMessageDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    mpMessageWidget = new MessageWidget(this);
    mpMessageWidget->setReadOnly(true);
    mpMessageDock->setWidget(mpMessageWidget);
    addDockWidget(Qt::BottomDockWidgetArea, mpMessageDock);
    mpMessageWidget->printGUIMessage("HopsanGUI, Version: " + QString(HOPSANGUIVERSION));

    this->loadSettings();

    //Create a dock for the componentslibrary
    mpLibDock = new QDockWidget(tr("Component Library"), this);
    mpLibDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mpLibrary = new LibraryWidget(this);
    mpLibDock->setWidget(mpLibrary);
    addDockWidget(Qt::LeftDockWidgetArea, mpLibDock);

    //Set dock widget corner owner
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    //setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    this->createActions();
    this->createToolbars();
    this->createMenus();

    //Create the main tab container, need at least one tab
    mpProjectTabs = new ProjectTabWidget(this);
    mpProjectTabs->setObjectName("projectTabs");

    mpBackButton = new QPushButton("Back");
    mpCentralgrid->addWidget(mpBackButton,0,0);
    mpCentralgrid->addWidget(mpProjectTabs,1,0);
    mpBackButton->hide();

    mpCentralwidget->setLayout(mpCentralgrid);

    //Set the centralwidget
    this->setCentralWidget(mpCentralwidget);



    //Create the Statusbar
    mpStatusBar = new QStatusBar();
    mpStatusBar->setObjectName("statusBar");
    this->setStatusBar(mpStatusBar);

    mpUndoWidget = new UndoWidget(this);

    mpProjectTabs->addNewProjectTab();

    mpPreferenceWidget = new PreferenceWidget(this);
    mpOptionsWidget = new OptionsWidget(this);

            //Load default libraries
    mpLibrary->addEmptyLibrary("User defined libraries");

    for(size_t i=0; i<mUserLibs.size(); ++i)
    {
        qDebug() << "Adding: " << mUserLibs.at(i);
        mpLibrary->addExternalLibrary(mUserLibs.at(i));
    }

    mpLibrary->addLibrary("../../HopsanGUI/componentData/Subsystem");

    mpLibrary->addEmptyLibrary("Signal");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/signal/Sources & Sinks","Signal");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/signal/Arithmetics","Signal");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/signal/Non-Linearities","Signal");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/signal/Filters","Signal");

    mpLibrary->addEmptyLibrary("Mechanic");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/mechanic/Transformers","Mechanic");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/mechanic/Mass Loads","Mechanic");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/mechanic/Springs & Dampers","Mechanic");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/mechanic/Sensors","Mechanic");

    mpLibrary->addEmptyLibrary("Hydraulic");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/hydraulic/Sources & Sinks","Hydraulic");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/hydraulic/sensors","Hydraulic");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/hydraulic/restrictors","Hydraulic");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/hydraulic/volumes","Hydraulic");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/hydraulic/actuators","Hydraulic");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/hydraulic/valves","Hydraulic");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/hydraulic/pumps","Hydraulic");

        //Create the plot widget and hide it
    mpPlotVariablesDock = new QDockWidget(tr("Plot Variables"), this);
    mpPlotVariablesDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mpPlotVariablesDock->hide();
    addDockWidget(Qt::RightDockWidgetArea, mpPlotVariablesDock);
    mPlotVariableListOpen=false;

    connect(mpProjectTabs, SIGNAL(currentChanged(int)), this, SLOT(updateToolBarsToNewTab()));
    connect(mpProjectTabs, SIGNAL(currentChanged(int)), this, SLOT(refreshUndoWidgetList()));
}


//! Destructor
MainWindow::~MainWindow()
{
    delete mpProjectTabs;
    delete menubar;
    delete mpStatusBar;
}


//! Overloaded function for showing the mainwindow. This is to make sure the view is centered when the program starts.
void MainWindow::show()
{
    QMainWindow::show();
    mpProjectTabs->getCurrentTab()->mpGraphicsView->centerView();
}


//! Opens the plot widget.
void MainWindow::plot()
{
    if(mpProjectTabs->count() != 0)
    {
        if(!mPlotVariableListOpen)
        {
    //        mpPlotVariablesDock = new QDockWidget(tr("Plot Variables"), this);
    //        mpPlotVariablesDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    //        VariableListDialog *variableList = new VariableListDialog(this);
    //        mpPlotVariablesDock->setWidget(variableList);
    //        //variableList->show();
    //        addDockWidget(Qt::RightDockWidgetArea, mpPlotVariablesDock);

            VariableListDialog *variableList = new VariableListDialog(this);
            mpPlotVariablesDock->setWidget(variableList);

            mpPlotVariablesDock->show();
            mPlotVariableListOpen = true;
        }
        else
        {
            mpPlotVariablesDock->hide();
            //this->removeDockWidget(mpPlotVariablesDock);
            mPlotVariableListOpen = false;
        }
    }
}


//! Event triggered re-implemented method that closes the main window.
//! First all tabs (models) are closed, if the user do not push Cancel
//! (closeAllProjectTabs then returns 'false') the event is accepted and
//! the main window is closed.
//! @param event contains information of the closing operation.
void MainWindow::closeEvent(QCloseEvent *event)
{
    this->saveSettings();

    if (mpProjectTabs->closeAllProjectTabs())
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}


//! Defines the actions used by the toolbars
void MainWindow::createActions()
{
    newAction = new QAction(QIcon("../../HopsanGUI/icons/Hopsan-New.png"), tr("&New"), this);
    newAction->setShortcut(tr("New"));
    newAction->setStatusTip(tr("Create New Project"));

    openAction = new QAction(QIcon("../../HopsanGUI/icons/Hopsan-Open.png"), tr("&Open"), this);
    openAction->setShortcut(QKeySequence("Ctrl+o"));
    openAction->setStatusTip(tr("Load Model File"));

    saveAction = new QAction(QIcon("../../HopsanGUI/icons/Hopsan-Save.png"), tr("&Save"), this);
    saveAction->setShortcut(QKeySequence("Ctrl+s"));
    saveAction->setStatusTip(tr("Save Model File"));

    saveAsAction = new QAction(QIcon("../../HopsanGUI/icons/Hopsan-SaveAs.png"), tr("&Save As"), this);
    saveAction->setShortcut(QKeySequence("Ctrl+Alt+s"));
    saveAsAction->setStatusTip(tr("Save Model File As"));

    closeAction = new QAction(this);
    closeAction->setText("Close");
    closeAction->setShortcut(QKeySequence("Ctrl+q"));
    connect(closeAction,SIGNAL(triggered()),this,SLOT(close()));

    undoAction = new QAction(QIcon("../../HopsanGUI/icons/Hopsan-Undo.png"), tr("&Undo"), this);
    undoAction->setText("Undo");
    undoAction->setShortcut(QKeySequence(tr("Ctrl+z")));
    undoAction->setStatusTip(tr("Undo One Step"));

    redoAction = new QAction(QIcon("../../HopsanGUI/icons/Hopsan-Redo.png"), tr("&Redo"), this);
    redoAction->setText("Redo");
    redoAction->setShortcut(QKeySequence(tr("Ctrl+y")));
    redoAction->setStatusTip(tr("Redo One Step"));

    openUndoAction = new QAction(tr("&Undo History"), this);
    openUndoAction->setText("Undo History");
    connect(openUndoAction,SIGNAL(triggered()),this,SLOT(openUndo()));

    disableUndoAction = new QAction(tr("&Disable Undo"), this);
    disableUndoAction->setText("Disable Undo");
    disableUndoAction->setCheckable(true);
    disableUndoAction->setChecked(false);

    cutAction = new QAction(QIcon("../../HopsanGUI/icons/Hopsan-Cut.png"), tr("&Cut"), this);
    cutAction->setShortcut(tr("Ctrl+x"));
    cutAction->setStatusTip(tr("Cut Selection"));

    copyAction = new QAction(QIcon("../../HopsanGUI/icons/Hopsan-Copy.png"), tr("&Copy"), this);
    copyAction->setShortcut(tr("Ctrl+c"));
    copyAction->setStatusTip(tr("Copy Selection"));

    pasteAction = new QAction(QIcon("../../HopsanGUI/icons/Hopsan-Paste.png"), tr("&Paste"), this);
    pasteAction->setShortcut(tr("Ctrl+v"));
    pasteAction->setStatusTip(tr("Paste Selection"));

    simulateAction = new QAction(QIcon("../../HopsanGUI/icons/Hopsan-Simulate.png"), tr("&Simulate"), this);
    simulateAction->setShortcut(tr("Simulate"));
    simulateAction->setStatusTip(tr("Simulate Current Project"));

    plotAction = new QAction(QIcon("../../HopsanGUI/icons/Hopsan-Plot.png"), tr("&Plot Variables"), this);
    plotAction->setShortcut(tr("Plot"));
    plotAction->setStatusTip(tr("Plot Variables"));
    connect(plotAction, SIGNAL(triggered()),this,SLOT(plot()));

    loadLibsAction = new QAction(this);
    loadLibsAction->setText("Load Libraries");
    connect(loadLibsAction,SIGNAL(triggered()),mpLibrary,SLOT(addLibrary()));

    preferencesAction = new QAction(QIcon("../../HopsanGUI/icons/Hopsan-Configure.png"), tr("&Model Preferences"), this);
    preferencesAction->setText("Model Preferences");
    preferencesAction->setShortcut(QKeySequence("Ctrl+Alt+p"));

    optionsAction = new QAction(QIcon("../../HopsanGUI/icons/Hopsan-Options.png"), tr("&Options"), this);
    optionsAction->setText("Options");

    resetZoomAction = new QAction(QIcon("../../HopsanGUI/icons/Hopsan-Zoom100.png"), tr("&Reset Zoom"), this);
    resetZoomAction->setText("Reset Zoom");

    zoomInAction = new QAction(QIcon("../../HopsanGUI/icons/Hopsan-ZoomIn.png"), tr("&Zoom In"), this);
    zoomInAction->setText("Zoom In");

    zoomOutAction = new QAction(QIcon("../../HopsanGUI/icons/Hopsan-ZoomOut.png"), tr("&Zoom Out"), this);
    zoomOutAction->setText("Zoom Out");

    centerViewAction = new QAction(QIcon("../../HopsanGUI/icons/Hopsan-CenterView.png"), tr("&Center View"), this);
    centerViewAction->setText("Center View");

    hideNamesAction = new QAction(QIcon("../../HopsanGUI/icons/Hopsan-HideNames.png"), tr("&Hide All Component Names"), this);
    hideNamesAction->setText("Hide All Component Names");

    showNamesAction = new QAction(QIcon("../../HopsanGUI/icons/Hopsan-ShowNames.png"), tr("&Show All Component Names"), this);
    showNamesAction->setText("Show All Component Names");

    exportPDFAction = new QAction(QIcon("../../HopsanGUI/icons/Hopsan-SaveToPDF.png"), tr("&Export To PDF"), this);
    exportPDFAction->setText("Export Model to PDF");

    QIcon hidePortsIcon;
    hidePortsIcon.addFile("../../HopsanGUI/icons/Hopsan-HidePorts.png", QSize(), QIcon::Normal, QIcon::On);
    hidePortsAction = new QAction(hidePortsIcon, tr("&Hide All Ports"), this);
    hidePortsAction->setText("Hide All Ports");
    hidePortsAction->setCheckable(true);

    mpStartTimeLineEdit = new QLineEdit("0.0");
    mpStartTimeLineEdit->setMaximumWidth(100);
    mpStartTimeLineEdit->setAlignment(Qt::AlignVCenter | Qt::AlignCenter);
    mpStartTimeLineEdit->setValidator(new QDoubleValidator(-999.0, 999.0, 6, mpStartTimeLineEdit));
    mpTimeStepLineEdit = new QLineEdit("0.001");
    mpTimeStepLineEdit->setMaximumWidth(100);
    mpTimeStepLineEdit->setAlignment(Qt::AlignVCenter | Qt::AlignCenter);
    mpTimeStepLineEdit->setValidator(new QDoubleValidator(0.0, 999.0, 6, mpStartTimeLineEdit));
    mpFinishTimeLineEdit = new QLineEdit("10.0");
    mpFinishTimeLineEdit->setValidator(new QDoubleValidator(-999.0, 999.0, 6, mpFinishTimeLineEdit));
    mpFinishTimeLineEdit->setMaximumWidth(100);
    mpFinishTimeLineEdit->setAlignment(Qt::AlignVCenter | Qt::AlignCenter);
    mpTimeLabelDeliminator1 = new QLabel(tr(" :: "));
    mpTimeLabelDeliminator2 = new QLabel(tr(" :: "));

    connect(mpStartTimeLineEdit, SIGNAL(editingFinished()), SLOT(fixSimulationParameterValues()));
    connect(mpTimeStepLineEdit, SIGNAL(editingFinished()), SLOT(fixSimulationParameterValues()));
    connect(mpFinishTimeLineEdit, SIGNAL(editingFinished()), SLOT(fixSimulationParameterValues()));
}


//! Creates the menus
void MainWindow::createMenus()
{
    //Create the menubar
    menubar = new QMenuBar();
    menubar->setGeometry(QRect(0,0,800,25));
    menubar->setObjectName("menubar");

    //Create the menues
    menuFile = new QMenu(menubar);
    menuFile->setObjectName("menuFile");
    menuFile->setTitle("&File");

    menuNew = new QMenu(menubar);
    menuNew->setObjectName("menuNew");
    menuNew->setTitle("New");

    menuSimulation = new QMenu(menubar);
    menuSimulation->setObjectName("menuSimulation");
    menuSimulation->setTitle("&Simulation");

    menuEdit = new QMenu(menubar);
    menuEdit->setTitle("&Edit");

    menuView = new QMenu(menubar);
    menuView->setTitle("&View");

    menuTools = new QMenu(menubar);
    menuTools->setTitle("&Tools");

    //menuPlot = new QMenu(menubar);
    //menuPlot->setTitle("&Plot");

    this->setMenuBar(menubar);

    //Add the actionbuttons to the menues
    newAction->setText("Project");
    menuNew->addAction(newAction);

    menuFile->addAction(menuNew->menuAction());
    menuFile->addAction(openAction);
    menuFile->addAction(saveAction);
    menuFile->addAction(saveAsAction);
    menuFile->addSeparator();
    menuFile->addAction(loadLibsAction);
    menuFile->addSeparator();
    menuFile->addAction(preferencesAction);
    menuFile->addSeparator();
    menuFile->addAction(closeAction);

    menuSimulation->addAction(simulateAction);

    menuEdit->addAction(undoAction);
    menuEdit->addAction(redoAction);
    menuEdit->addAction(openUndoAction);
    menuEdit->addAction(disableUndoAction);
    menuEdit->addSeparator();
    menuEdit->addAction(copyAction);
    menuEdit->addAction(cutAction);
    menuEdit->addAction(pasteAction);

    menuView->addAction(mpLibDock->toggleViewAction());
    menuView->addAction(mpMessageDock->toggleViewAction());
    menuView->addAction(mpFileToolBar->toggleViewAction());
    menuView->addAction(mpEditToolBar->toggleViewAction());
    menuView->addAction(mpSimToolBar->toggleViewAction());

    menuTools->addAction(optionsAction);

    menuSimulation->addAction(plotAction);

    menubar->addAction(menuFile->menuAction());
    menubar->addAction(menuEdit->menuAction());
    menubar->addAction(menuTools->menuAction());
    menubar->addAction(menuSimulation->menuAction());
    //menubar->addAction(menuPlot->menuAction());
    menubar->addAction(menuView->menuAction());
}

//! Creates the toolbars
void MainWindow::createToolbars()
{
    //viewScaleCombo = new QComboBox;
    //QStringList scales;
    //scales << tr("50%") << tr("75%") << tr("100%") << tr("125%") << tr("150%");
    //viewScaleCombo->addItems(scales);
    //viewScaleCombo->setCurrentIndex(2);

    mpFileToolBar = addToolBar(tr("File Toolbar"));
    mpFileToolBar->setAllowedAreas(Qt::TopToolBarArea);
    mpFileToolBar->addAction(newAction);
    mpFileToolBar->addAction(openAction);
    mpFileToolBar->addAction(saveAction);
    mpFileToolBar->addAction(saveAsAction);

    mpEditToolBar = addToolBar(tr("Edit Toolbar"));
    mpEditToolBar->setAllowedAreas(Qt::TopToolBarArea);
    mpEditToolBar->addAction(cutAction);
    mpEditToolBar->addAction(copyAction);
    mpEditToolBar->addAction(pasteAction);
    mpEditToolBar->addAction(undoAction);
    mpEditToolBar->addAction(redoAction);
    mpEditToolBar->addAction(optionsAction);

    mpViewToolBar = addToolBar(tr("View Toolbar"));
    mpViewToolBar->setAllowedAreas(Qt::TopToolBarArea);
    mpViewToolBar->addAction(centerViewAction);
    mpViewToolBar->addAction(resetZoomAction);
    mpViewToolBar->addAction(zoomInAction);
    mpViewToolBar->addAction(zoomOutAction);
    mpViewToolBar->addAction(hideNamesAction);
    mpViewToolBar->addAction(showNamesAction);
    mpViewToolBar->addAction(hidePortsAction);
    mpViewToolBar->addAction(exportPDFAction);

    mpSimToolBar = addToolBar(tr("Simulation Toolbar"));
    mpSimToolBar->setAllowedAreas(Qt::TopToolBarArea);
    mpSimToolBar->addWidget(mpStartTimeLineEdit);
    mpSimToolBar->addWidget(mpTimeLabelDeliminator1);
    mpSimToolBar->addWidget(mpTimeStepLineEdit);
    mpSimToolBar->addWidget(mpTimeLabelDeliminator2);
    mpSimToolBar->addWidget(mpFinishTimeLineEdit);
    mpSimToolBar->addAction(simulateAction);
    mpSimToolBar->addAction(plotAction);
    mpSimToolBar->addAction(preferencesAction);

    //mpSimulationToolBar = addToolBar(tr("Simulation"));
    //mpSimulationToolBar->setAllowedAreas(Qt::TopToolBarArea);
}


//! Opens the undo widget.
void MainWindow::openUndo()
{
    //mpUndoWidget->show();

    mpUndoDock = new QDockWidget(tr("Undo History"), this);
    mpUndoDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    //VariableListDialog *variableList = new VariableListDialog(varPlotDock);

    mpUndoDock->setWidget(mpUndoWidget);
    //variableList->show();

    addDockWidget(Qt::RightDockWidgetArea, mpUndoDock);

    if(dockWidgetArea(mpPlotVariablesDock) == dockWidgetArea(mpUndoDock))
    {
        tabifyDockWidget(mpUndoDock, mpPlotVariablesDock);
    }

    mpUndoWidget->activateWindow();
    mpUndoWidget->raise();
    mpUndoWidget->refreshList();
}


//! Updates the toolbar values that are tab specific when a new tab is activated
void MainWindow::updateToolBarsToNewTab()
{
    if(mpProjectTabs->count() > 0)
    {
        hidePortsAction->setChecked(mpProjectTabs->getCurrentTab()->mpSystem->mPortsHidden);
    }

    bool noTabs = !(mpProjectTabs->count() > 0);
    saveAction->setEnabled(!noTabs);
    saveAsAction->setEnabled(!noTabs);
    cutAction->setEnabled(!noTabs);
    copyAction->setEnabled(!noTabs);
    pasteAction->setEnabled(!noTabs);
    undoAction->setEnabled(!noTabs);
    redoAction->setEnabled(!noTabs);
    centerViewAction->setEnabled(!noTabs);
    resetZoomAction->setEnabled(!noTabs);
    zoomInAction->setEnabled(!noTabs);
    zoomOutAction->setEnabled(!noTabs);
    hideNamesAction->setEnabled(!noTabs);
    showNamesAction->setEnabled(!noTabs);
    hidePortsAction->setEnabled(!noTabs);
    exportPDFAction->setEnabled(!noTabs);
    mpStartTimeLineEdit->setEnabled(!noTabs);
    mpTimeStepLineEdit->setEnabled(!noTabs);
    mpFinishTimeLineEdit->setEnabled(!noTabs);
    simulateAction->setEnabled(!noTabs);
    plotAction->setEnabled(!noTabs);
    preferencesAction->setEnabled(!noTabs);

}


//! Slot that calls refresh list function in undo widget. Used because undo widget cannot have slots.
void MainWindow::refreshUndoWidgetList()
{
    mpUndoWidget->refreshList();
}

//! Loads global settings from a text file
void MainWindow::loadSettings()
{
        //Apply default values
    mInvertWheel = false;
    mUseMulticore = true;
    mEnableProgressBar = true;
    mProgressBarStep = 10;
    mBackgroundColor = QColor("white");
    mAntiAliasing = false;

    QFile file("../../settings.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        mpMessageWidget->printGUIErrorMessage("Unable to read settings file. Using default settings.");
        return;
    }

    QString inputWord;
    QTextStream inputStream(&file);
    while ( !inputStream.atEnd() )
    {
        inputStream >> inputWord;
        if(inputWord == "INVERTWHEEL")
        {
            inputStream >> inputWord;
            mInvertWheel = (inputWord == "TRUE");
        }
        if(inputWord == "ENABLEPROGRESSBAR")
        {
            inputStream >> inputWord;
            mEnableProgressBar = (inputWord == "TRUE");
        }
        if(inputWord == "PROGRESSBARSTEP")
        {
            inputStream >> mProgressBarStep;
        }
        if(inputWord == "USEMULTICORE")
        {
            inputStream >> inputWord;
            mUseMulticore = (inputWord == "TRUE");
        }
        if(inputWord == "BACKGROUNDCOLOR")
        {
            inputStream >> inputWord;
            QColor tempColor;
            tempColor.setNamedColor(inputWord);
            mBackgroundColor = tempColor;
        }
        if(inputWord == "ANTIALIASING")
        {
            inputStream >> inputWord;
            mAntiAliasing = (inputWord == "TRUE");
        }
        if(inputWord == "USERLIB")
        {
            //qDebug() << "Appending:  readName(" << inputWord << ")";
            mUserLibs.append(readName(inputStream));
        }
    }
    file.close();
}


//! Saves global settings to a text file
void MainWindow::saveSettings()
{
    QFile file("../../settings.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
    {
        mpMessageWidget->printGUIErrorMessage("Error writing to settings file. Default values will be used next session");
        return;
    }
    QTextStream settingsFile(&file);  //Create a QTextStream object to stream the content of file

    settingsFile << "INVERTWHEEL ";
    if(mInvertWheel)
    {
        settingsFile << "TRUE\n";
    }
    else
    {
        settingsFile << "FALSE\n";
    }

    settingsFile << "ENABLEPROGRESSBAR ";
    if(mEnableProgressBar)
    {
        settingsFile << "TRUE\n";
    }
    else
    {
        settingsFile << "FALSE\n";
    }

    settingsFile << "PROGRESSBARSTEP ";
    settingsFile << mProgressBarStep << "\n";

    settingsFile << "USEMULTICORE ";
    if(mUseMulticore)
    {
        settingsFile << "TRUE\n";
    }
    else
    {
        settingsFile << "FALSE\n";
    }

    settingsFile << "BACKGROUNDCOLOR ";
    settingsFile << mBackgroundColor.name() << "\n";

    settingsFile << "ANTIALIASING ";
    if(mAntiAliasing)
    {
        settingsFile << "TRUE\n";
    }
    else
    {
        settingsFile << "FALSE\n";
    }

    for(size_t i=0; i<mUserLibs.size(); ++i)
    {
        settingsFile << "USERLIB " << addQuotes(mUserLibs.at(i)) << "\n";
    }

    file.close();
}







//! Make sure the values make sens.
//! @see fixTimeStep()
void MainWindow::fixSimulationParameterValues()
{
    fixFinishTime();
    fixTimeStep();
}


//! Make sure that the finishs time of the simulation is not smaller than start time.
//! @see fixTimeStep()
//! @see fixLabelValues()
void MainWindow::fixFinishTime()
{
    if (getFinishTimeFromToolBar() < getStartTimeFromToolBar())
        setFinishTimeInToolBar(getStartTimeFromToolBar());

}


//! Make sure that the timestep is in the right range i.e. not larger than the simulation time.
//! @see fixFinishTime()
//! @see fixLabelValues()
void MainWindow::fixTimeStep()
{
    //! @todo Maybe more checks, i.e. the time step should be even divided into the simulation time.
    if (getTimeStepFromToolBar() > (getFinishTimeFromToolBar() - getStartTimeFromToolBar()))
        setTimeStepInToolBar(getFinishTimeFromToolBar() - getStartTimeFromToolBar());

    if (mpProjectTabs->getCurrentTab()) //crashes if not if statement if no tabs are there...
    {
        mpProjectTabs->getCurrentTab()->mpSystem->mpCoreSystemAccess->setDesiredTimeStep(getTimeStepFromToolBar());
    }
}


//! Sets a new startvalue.
//! @param startTime is the new value
void MainWindow::setStartTimeInToolBar(double startTime)
{
    QString valueTxt;
    valueTxt.setNum(startTime, 'g', 6 );
    mpStartTimeLineEdit->setText(valueTxt);
    fixTimeStep();
    fixFinishTime();
}


//! Sets a new timestep.
//! @param timeStep is the new value
void MainWindow::setTimeStepInToolBar(double timeStep)
{
    QString valueTxt;
    valueTxt.setNum(timeStep, 'g', 6 );
    mpTimeStepLineEdit->setText(valueTxt);
    fixTimeStep();
    fixFinishTime();
}


//! Sets a new finish value.
//! @param finishTime is the new value
void MainWindow::setFinishTimeInToolBar(double finishTime)
{
    QString valueTxt;
    valueTxt.setNum(finishTime, 'g', 6 );
    mpFinishTimeLineEdit->setText(valueTxt);
    fixTimeStep();
    fixFinishTime();
}


//! Acess function to the starttimelabel value.
//! @returns the starttime value
double MainWindow::getStartTimeFromToolBar()
{
    return mpStartTimeLineEdit->text().toDouble();
}


//! Acess function to the timesteplabel value.
//! @returns the timestep value
double MainWindow::getTimeStepFromToolBar()
{
    return mpTimeStepLineEdit->text().toDouble();
}


//! Acess function to the finishlabel value.
//! @returns the finish value
double MainWindow::getFinishTimeFromToolBar()
{
    return mpFinishTimeLineEdit->text().toDouble();
}
