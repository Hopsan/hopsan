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
#include <QtGui>

#include "mainwindow.h"
#include "treewidget.h"
#include "treewidgetitem.h"
#include "listwidget.h"
#include "ProjectTabWidget.h"
#include "LibraryWidget.h"
#include "SimulationSetupWidget.h"
#include "version.h"
#include "UndoStack.h"


//! Constructor
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    //Set the name and size of the main window
    this->setObjectName("MainWindow");
    this->resize(1024,768);
    this->setWindowTitle("HOPSAN NG");
    this->setWindowIcon(QIcon("../../HopsanGUI/icons/hopsan.png"));


    mpOptionsWidget = new OptionsWidget(this);

    //Create a centralwidget for the main window
    mpCentralwidget = new QWidget(this);
    mpCentralwidget->setObjectName("centralwidget");

    //Create a grid on the centralwidget
    mpCentralgrid = new QGridLayout(mpCentralwidget);
    mpCentralgrid->setSpacing(10);

    //Create a dock for the MessageWidget
    messagedock = new QDockWidget(tr("Messages"), this);
    messagedock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    mpMessageWidget = new MessageWidget(this);
    mpMessageWidget->setReadOnly(true);
    messagedock->setWidget(mpMessageWidget);
    addDockWidget(Qt::BottomDockWidgetArea, messagedock);
    mpMessageWidget->printGUIMessage("HopsanGUI, Version: " + QString(HOPSANGUIVERSION));

    //Create a dock for the componentslibrary
    libdock = new QDockWidget(tr("Components"), this);
    libdock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mpLibrary = new LibraryWidget(this);
    libdock->setWidget(mpLibrary);
    addDockWidget(Qt::LeftDockWidgetArea, libdock);

    //Set dock widget corner owner
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    //setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    //Create a SimulationSetupWidget
    mpSimulationSetupWidget = new SimulationSetupWidget(tr("Simulation Setup"), this);

    this->createActions();
    this->createToolbars();
    this->createMenus();

    //Create the main tab container, need at least one tab
    mpProjectTabs = new ProjectTabWidget(this);
    mpProjectTabs->setObjectName("projectTabs");

    //mpCentralgrid->addWidget(mpSimulationSetupWidget,0,0);
    mpBackButton = new QPushButton("Back");
    mpCentralgrid->addWidget(mpBackButton,0,0);
    mpCentralgrid->addWidget(mpProjectTabs,1,0);
    mpBackButton->hide();

    mpCentralwidget->setLayout(mpCentralgrid);

    //Set the centralwidget
    this->setCentralWidget(mpCentralwidget);



    //Create the Statusbar
    statusBar = new QStatusBar();
    statusBar->setObjectName("statusBar");
    this->setStatusBar(statusBar);

    mpUndoWidget = new UndoWidget(this);
    mpProjectTabs->addNewProjectTab();

    mpPreferenceWidget = new PreferenceWidget(this);

        //Load default libraries
    mpLibrary->addEmptyLibrary("User defined libraries");

    mpLibrary->addLibrary("../../HopsanGUI/componentData/Subsystem");

    mpLibrary->addEmptyLibrary("Signal");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/signal/Sources","Signal");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/signal/Sinks","Signal");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/signal/Arithmetics","Signal");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/signal/Non-Linearities","Signal");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/signal/Filters","Signal");

    mpLibrary->addEmptyLibrary("Mechanic");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/mechanic/Transformers","Mechanic");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/mechanic/Mass Loads","Mechanic");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/mechanic/Springs & Dampers","Mechanic");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/mechanic/Sensors","Mechanic");

    mpLibrary->addEmptyLibrary("Hydraulic");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/hydraulic/sources","Hydraulic");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/hydraulic/sensors","Hydraulic");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/hydraulic/restrictors","Hydraulic");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/hydraulic/volumes","Hydraulic");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/hydraulic/actuators","Hydraulic");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/hydraulic/valves","Hydraulic");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/hydraulic/pumps","Hydraulic");

    QMetaObject::connectSlotsByName(this);

    connect(mpSimulationSetupWidget->mpSimulateButton, SIGNAL(released()), mpProjectTabs, SLOT(simulateCurrent()));
    connect(mpProjectTabs, SIGNAL(currentChanged(int)), this, SLOT(updateToolBarsToNewTab()));
}


//! Destructor
MainWindow::~MainWindow()
{
    delete mpProjectTabs;
    delete menubar;
    delete statusBar;
}


//! Opens the plot widget.
void MainWindow::plot()
{
    QDockWidget *varPlotDock = new QDockWidget(tr("Plot Variables"), this);
    varPlotDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    VariableListDialog *variableList = new VariableListDialog(this);
    varPlotDock->setWidget(variableList);
    //variableList->show();
    addDockWidget(Qt::RightDockWidgetArea, varPlotDock);

}


//! Event triggered re-implemented method that closes the main window.
//! First all tabs (models) are closed, if the user do not push Cancel
//! (closeAllProjectTabs then returns 'false') the event is accepted and
//! the main window is closed.
//! @param event contains information of the closing operation.
void MainWindow::closeEvent(QCloseEvent *event)
{
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
    connect(this->closeAction,SIGNAL(triggered()),SLOT(close()));

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

    plotAction = new QAction(QIcon("../../HopsanGUI/icons/Hopsan-Plot.png"), tr("&Plot"), this);
    plotAction->setShortcut(tr("Plot"));
    plotAction->setStatusTip(tr("Plot Something"));
    connect(plotAction, SIGNAL(triggered()), this,SLOT(plot()));

    loadLibsAction = new QAction(this);
    loadLibsAction->setText("Load Libraries");
    connect(loadLibsAction,SIGNAL(triggered()),mpLibrary,SLOT(addLibrary()));

    preferencesAction = new QAction(QIcon("../../HopsanGUI/icons/Hopsan-Configure.png"), tr("&Model Preferences"), this);
    preferencesAction->setText("Model Preferences");
    preferencesAction->setShortcut(QKeySequence("Ctrl+Alt+p"));
    connect(preferencesAction,SIGNAL(triggered()),this,SLOT(openPreferences()));

    optionsAction = new QAction(QIcon("../../HopsanGUI/icons/Hopsan-Options.png"), tr("&Options"), this);
    optionsAction->setText("Options");
    connect(optionsAction,SIGNAL(triggered()),this,SLOT(openOptions()));

    resetZoomAction = new QAction(QIcon("../../HopsanGUI/icons/Hopsan-Zoom100.png"), tr("&Reset Zoom"), this);
    resetZoomAction->setText("Reset Zoom");

    zoomInAction = new QAction(QIcon("../../HopsanGUI/icons/Hopsan-ZoomIn.png"), tr("&Zoom In"), this);
    zoomInAction->setText("Zoom In");

    zoomOutAction = new QAction(QIcon("../../HopsanGUI/icons/Hopsan-ZoomOut.png"), tr("&Zoom Out"), this);
    zoomOutAction->setText("Zoom Out");

    hideNamesAction = new QAction(QIcon("../../HopsanGUI/icons/Hopsan-HideNames.png"), tr("&Hide All Component Names"), this);
    hideNamesAction->setText("Hide All Component Names");

    showNamesAction = new QAction(QIcon("../../HopsanGUI/icons/Hopsan-ShowNames.png"), tr("&Show All Component Names"), this);
    showNamesAction->setText("Show All Component Names");

    QIcon hidePortsIcon;
    hidePortsIcon.addFile("../../HopsanGUI/icons/Hopsan-HidePorts.png", QSize(), QIcon::Normal, QIcon::On);
    hidePortsAction = new QAction(hidePortsIcon, tr("&Hide All Ports"), this);
    hidePortsAction->setText("Hide All Ports");
    hidePortsAction->setCheckable(true);
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

    menuPlot = new QMenu(menubar);
    menuPlot->setTitle("&Plot");

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
    menuEdit->addSeparator();
    menuEdit->addAction(copyAction);
    menuEdit->addAction(cutAction);
    menuEdit->addAction(pasteAction);

    menuView->addAction(libdock->toggleViewAction());
    menuView->addAction(messagedock->toggleViewAction());
    menuView->addAction(fileToolBar->toggleViewAction());
    menuView->addAction(editToolBar->toggleViewAction());
    menuView->addAction(simToolBar->toggleViewAction());

    menuTools->addAction(optionsAction);

    menuPlot->addAction(plotAction);

    menubar->addAction(menuFile->menuAction());
    menubar->addAction(menuEdit->menuAction());
    menubar->addAction(menuTools->menuAction());
    menubar->addAction(menuSimulation->menuAction());
    menubar->addAction(menuPlot->menuAction());
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

    fileToolBar = addToolBar(tr("File Toolbar"));
    fileToolBar->setAllowedAreas(Qt::TopToolBarArea);
    fileToolBar->addAction(newAction);
    fileToolBar->addAction(openAction);
    fileToolBar->addAction(saveAction);
    fileToolBar->addAction(saveAsAction);

    editToolBar = addToolBar(tr("Clipboard Toolbar"));
    editToolBar->setAllowedAreas(Qt::TopToolBarArea);
    editToolBar->addAction(cutAction);
    editToolBar->addAction(copyAction);
    editToolBar->addAction(pasteAction);
    editToolBar->addAction(undoAction);
    editToolBar->addAction(redoAction);
    editToolBar->addAction(optionsAction);

    simToolBar = addToolBar(tr("Simulation Toolbar"));
    simToolBar->setAllowedAreas(Qt::TopToolBarArea);
    //simToolBar->addWidget(viewScaleCombo);
    simToolBar->addAction(preferencesAction);
    simToolBar->addAction(simulateAction);
    simToolBar->addAction(plotAction);

    viewToolBar = addToolBar(tr("View Toolbar"));
    viewToolBar->setAllowedAreas(Qt::TopToolBarArea);
    viewToolBar->addAction(resetZoomAction);
    viewToolBar->addAction(zoomInAction);
    viewToolBar->addAction(zoomOutAction);
    viewToolBar->addAction(hideNamesAction);
    viewToolBar->addAction(showNamesAction);
    viewToolBar->addAction(hidePortsAction);

    //addToolBar(tr("Simulation"));
    //mpSimulationToolBar->setAllowedAreas(Qt::TopToolBarArea);
    //mpSimulationToolBar->addWidget(mpSimulationSetupWidget);
    mpCentralgrid->addWidget(mpSimulationSetupWidget);


}


//! Opens the model preference widget.
void MainWindow::openPreferences()
{
    this->mpPreferenceWidget->show();
}


//! Opens the options widget.
void MainWindow::openOptions()
{
    this->mpOptionsWidget->show();
}


//! Opens the undo widget.
void MainWindow::openUndo()
{
    //this->mpUndoWidget->show();

    QDockWidget *undoDock = new QDockWidget(tr("Undo History"), this);
    undoDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    //VariableListDialog *variableList = new VariableListDialog(varPlotDock);

    undoDock->setWidget(mpUndoWidget);
    //variableList->show();
    addDockWidget(Qt::RightDockWidgetArea, undoDock);

    mpUndoWidget->refreshList();
}


void MainWindow::updateToolBarsToNewTab()
{
    if(mpProjectTabs->count() > 0)
    {
        hidePortsAction->setChecked(mpProjectTabs->getCurrentTab()->mpGraphicsView->mPortsHidden);
    }
}
