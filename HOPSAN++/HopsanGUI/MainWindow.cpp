//$Id$

#include <iostream>
#include <QDebug>

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
#include "PyDock.h"
#include "GlobalParametersWidget.h"

#include "loadObjects.h"

//! Constructor
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{

    //QString(MAINPATH) = "../../";
    //mQString(ICONPATH) = QString(MAINPATH) + "HopsanGUI/icons/";
    //mComponentPath = QString(MAINPATH) + "HopsanGUI/componentData/";

    //Set the name and size of the main window
    this->setObjectName("MainWindow");
    this->resize(1024,768);
    this->setFont(QFont("Comic Sans"));
    this->setWindowTitle("HOPSAN NG");
    this->setWindowIcon(QIcon(QString(QString(ICONPATH) + "hopsan.png")));
    this->setDockOptions(QMainWindow::ForceTabbedDocks);

    mpPlotWidget = 0;
    mpGlobalParametersWidget = 0;

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
    mpClearMessageWidgetButton = new QPushButton("Clear Messages");
    QFont tempFont = mpClearMessageWidgetButton->font();
    tempFont.setBold(true);
    mpClearMessageWidgetButton->setFont(tempFont);
    QGridLayout *pTempLayout = new QGridLayout(mpMessageDock);
    pTempLayout->addWidget(mpMessageWidget,0,0,1,5);
    pTempLayout->addWidget(mpClearMessageWidgetButton,1,0,1,1);
    QWidget *pTempWidget = new QWidget(this);
    pTempWidget->setLayout(pTempLayout);
    mpMessageDock->setWidget(pTempWidget);
    addDockWidget(Qt::BottomDockWidgetArea, mpMessageDock);
    mpMessageWidget->printGUIMessage("HopsanGUI, Version: " + QString(HOPSANGUIVERSION));
    connect(mpClearMessageWidgetButton, SIGNAL(pressed()),mpMessageWidget,SLOT(clear()));

    this->loadSettings();

    //Create a dock for the componentslibrary
    mpLibDock = new QDockWidget(tr("Component Library"), this);
    mpLibDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mpLibrary = new LibraryWidget(this);
    mpLibDock->setWidget(mpLibrary);
    addDockWidget(Qt::LeftDockWidgetArea, mpLibDock);

    mpPyDock = new PyDock(this, this);
    addDockWidget(Qt::BottomDockWidgetArea, mpPyDock);

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

    mpLibrary->addLibrary(QString(COMPONENTPATH) + "Subsystem");

    mpLibrary->addEmptyLibrary("Signal");
    mpLibrary->addLibrary(QString(COMPONENTPATH) + "signal/Sources & Sinks","Signal");
    mpLibrary->addLibrary(QString(COMPONENTPATH) + "signal/Arithmetics","Signal");
    mpLibrary->addLibrary(QString(COMPONENTPATH) + "signal/Non-Linearities","Signal");
    mpLibrary->addLibrary(QString(COMPONENTPATH) + "signal/Filters","Signal");
    mpLibrary->addLibrary(QString(COMPONENTPATH) + "signal/Logic","Signal");
    mpLibrary->addLibrary(QString(COMPONENTPATH) + "signal/Simulation Control","Signal");

    mpLibrary->addEmptyLibrary("Mechanic");
    mpLibrary->addLibrary(QString(COMPONENTPATH) + "mechanic/Transformers","Mechanic");
    mpLibrary->addLibrary(QString(COMPONENTPATH) + "mechanic/Mass Loads","Mechanic");
    mpLibrary->addLibrary(QString(COMPONENTPATH) + "mechanic/Springs & Dampers","Mechanic");
    mpLibrary->addLibrary(QString(COMPONENTPATH) + "mechanic/Sensors","Mechanic");

    mpLibrary->addEmptyLibrary("Hydraulic");
    mpLibrary->addLibrary(QString(COMPONENTPATH) + "hydraulic/Sources & Sinks","Hydraulic");
    mpLibrary->addLibrary(QString(COMPONENTPATH) + "hydraulic/sensors","Hydraulic");
    mpLibrary->addLibrary(QString(COMPONENTPATH) + "hydraulic/restrictors","Hydraulic");
    mpLibrary->addLibrary(QString(COMPONENTPATH) + "hydraulic/volumes","Hydraulic");
    mpLibrary->addLibrary(QString(COMPONENTPATH) + "hydraulic/actuators","Hydraulic");
    mpLibrary->addLibrary(QString(COMPONENTPATH) + "hydraulic/valves","Hydraulic");
    mpLibrary->addLibrary(QString(COMPONENTPATH) + "hydraulic/pumps","Hydraulic");

    mpLibrary->addLibrary(QString(COMPONENTPATH) + "_Optimized");

        //Create the plot dock widget and hide it
    mpPlotWidgetDock = new QDockWidget(tr("Plot Variables"), this);
    mpPlotWidgetDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mpPlotWidgetDock->hide();
    addDockWidget(Qt::RightDockWidgetArea, mpPlotWidgetDock);

        //Create the global parameters dock widget and hide it
    mpGlobalParametersDock = new QDockWidget(tr("Global Parameters"), this);
    mpGlobalParametersDock->setAllowedAreas((Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea));
    addDockWidget(Qt::RightDockWidgetArea, mpGlobalParametersDock);
    mpGlobalParametersDock->hide();

        //Create the undo dock widget and hide it
    mpUndoWidgetDock = new QDockWidget(tr("Undo History"), this);
    mpUndoWidgetDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mpUndoWidgetDock->hide();
    addDockWidget(Qt::RightDockWidgetArea, mpUndoWidgetDock);

        //Make dock widgets that share same dock area tabified, instead of stacking them above each other
    tabifyDockWidget(mpPlotWidgetDock, mpGlobalParametersDock);
    tabifyDockWidget(mpGlobalParametersDock, mpUndoWidgetDock);
    tabifyDockWidget(mpUndoWidgetDock, mpPlotWidgetDock);

    tabifyDockWidget(mpMessageDock, mpPyDock);

    connect(mpProjectTabs, SIGNAL(currentChanged(int)), this, SLOT(updateToolBarsToNewTab()));
    connect(mpProjectTabs, SIGNAL(currentChanged(int)), this, SLOT(refreshUndoWidgetList()));

    if(!mLastSessionModels.empty())
    {
        mpProjectTabs->closeProjectTab(0);
        for(size_t i=0; i<mLastSessionModels.size(); ++i)
        {
            //mpProjectTabs->loadModel(mLastSessionModels.at(i));
            mpProjectTabs->loadModel(mLastSessionModels.at(i));

        }
    }

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
void MainWindow::openPlotWidget()
{
    if(mpProjectTabs->count() != 0)
    {
        if(!mpPlotWidgetDock->isVisible())
        {
            if(mpPlotWidget == 0)
            {
                mpPlotWidget = new PlotWidget(this);
            }
            mpPlotWidgetDock->setWidget(mpPlotWidget);

            mpPlotWidgetDock->show();
            mpPlotWidgetDock->raise();
        }
        else
        {
            mpPlotWidgetDock->hide();

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
    if (mpProjectTabs->closeAllProjectTabs())
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }

    this->saveSettings();
}


//! Defines the actions used by the toolbars
void MainWindow::createActions()
{
    newAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-New.png"), tr("&New"), this);
    newAction->setShortcut(tr("New"));
    newAction->setStatusTip(tr("Create New Project"));

    openAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Open.png"), tr("&Open"), this);
    openAction->setShortcut(QKeySequence("Ctrl+o"));
    openAction->setStatusTip(tr("Load Model File"));

    saveAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Save.png"), tr("&Save"), this);
    saveAction->setShortcut(QKeySequence("Ctrl+s"));
    saveAction->setStatusTip(tr("Save Model File"));

    saveAsAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-SaveAs.png"), tr("&Save As"), this);
    saveAsAction->setShortcut(QKeySequence("Ctrl+Alt+s"));
    saveAsAction->setStatusTip(tr("Save Model File As"));

    closeAction = new QAction(this);
    closeAction->setText("Close");
    closeAction->setShortcut(QKeySequence("Ctrl+q"));
    connect(closeAction,SIGNAL(triggered()),this,SLOT(close()));

    undoAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Undo.png"), tr("&Undo"), this);
    undoAction->setText("Undo");
    undoAction->setShortcut(QKeySequence(tr("Ctrl+z")));
    undoAction->setStatusTip(tr("Undo One Step"));

    redoAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Redo.png"), tr("&Redo"), this);
    redoAction->setText("Redo");
    redoAction->setShortcut(QKeySequence(tr("Ctrl+y")));
    redoAction->setStatusTip(tr("Redo One Step"));

    openUndoAction = new QAction(tr("&Undo History"), this);
    openUndoAction->setText("Undo History");
    connect(openUndoAction,SIGNAL(triggered()),this,SLOT(openUndoWidget()));

    disableUndoAction = new QAction(tr("&Disable Undo"), this);
    disableUndoAction->setText("Disable Undo");
    disableUndoAction->setCheckable(true);
    disableUndoAction->setChecked(false);

    openGlobalParametersAction = new QAction(tr("&Global Parameters"), this);
    openGlobalParametersAction->setText("Global Parameters");
    connect(openGlobalParametersAction,SIGNAL(triggered()),this,SLOT(openGlobalParametersWidget()));

    cutAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Cut.png"), tr("&Cut"), this);
    cutAction->setShortcut(tr("Ctrl+x"));
    cutAction->setStatusTip(tr("Cut Selection"));

    copyAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Copy.png"), tr("&Copy"), this);
    copyAction->setShortcut(tr("Ctrl+c"));
    copyAction->setStatusTip(tr("Copy Selection"));

    pasteAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Paste.png"), tr("&Paste"), this);
    pasteAction->setShortcut(tr("Ctrl+v"));
    pasteAction->setStatusTip(tr("Paste Selection"));

    simulateAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Simulate.png"), tr("&Simulate"), this);
    simulateAction->setShortcut(tr("Simulate"));
    simulateAction->setStatusTip(tr("Simulate Current Project"));

    plotAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Plot.png"), tr("&Plot Variables"), this);
    plotAction->setShortcut(tr("Plot"));
    plotAction->setStatusTip(tr("Plot Variables"));
    connect(plotAction, SIGNAL(triggered()),this,SLOT(openPlotWidget()));

    loadLibsAction = new QAction(this);
    loadLibsAction->setText("Load Libraries");
    connect(loadLibsAction,SIGNAL(triggered()),mpLibrary,SLOT(addLibrary()));

    preferencesAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Configure.png"), tr("&Model Preferences"), this);
    preferencesAction->setText("Model Preferences");
    preferencesAction->setShortcut(QKeySequence("Ctrl+Alt+p"));

    optionsAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Options.png"), tr("&Options"), this);
    optionsAction->setText("Options");

    resetZoomAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Zoom100.png"), tr("&Reset Zoom"), this);
    resetZoomAction->setText("Reset Zoom");

    zoomInAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-ZoomIn.png"), tr("&Zoom In"), this);
    zoomInAction->setText("Zoom In");

    zoomOutAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-ZoomOut.png"), tr("&Zoom Out"), this);
    zoomOutAction->setText("Zoom Out");

    centerViewAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-CenterView.png"), tr("&Center View"), this);
    centerViewAction->setText("Center View");

    hideNamesAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-HideNames.png"), tr("&Hide All Component Names"), this);
    hideNamesAction->setText("Hide All Component Names");

    showNamesAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-ShowNames.png"), tr("&Show All Component Names"), this);
    showNamesAction->setText("Show All Component Names");

    exportPDFAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-SaveToPDF.png"), tr("&Export To PDF"), this);
    exportPDFAction->setText("Export Model to PDF");

    QIcon hidePortsIcon;
    hidePortsIcon.addFile(QString(ICONPATH) + "Hopsan-HidePorts.png", QSize(), QIcon::Normal, QIcon::On);
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

    recentMenu = new QMenu(this);
    recentMenu->setTitle("Recent Models");

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
    menuFile->addMenu(recentMenu);
    menuFile->addSeparator();
    menuFile->addAction(loadLibsAction);
    menuFile->addSeparator();
    menuFile->addAction(preferencesAction);
    menuFile->addAction(openGlobalParametersAction);
    menuFile->addSeparator();
    menuFile->addAction(closeAction);

    this->updateRecentList();

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
    menuView->addAction(mpPyDock->toggleViewAction());

    menuTools->addAction(optionsAction);

    menuSimulation->addAction(plotAction);

    menubar->addAction(menuFile->menuAction());
    menubar->addAction(menuEdit->menuAction());
    menubar->addAction(menuTools->menuAction());
    menubar->addAction(menuSimulation->menuAction());
    menubar->addAction(menuView->menuAction());
}

//! Creates the toolbars
void MainWindow::createToolbars()
{
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
}


//! Opens the undo widget.
void MainWindow::openUndoWidget()
{
    if(!mpUndoWidgetDock->isVisible())
    {
        mpUndoWidgetDock->setWidget(mpUndoWidget);
        mpUndoWidgetDock->show();
        mpUndoWidgetDock->raise();
        mpUndoWidget->refreshList();
    }
}


//! Opens the undo widget.
void MainWindow::openGlobalParametersWidget()
{
    if(!mpGlobalParametersDock->isVisible())
    {
        if(mpGlobalParametersWidget == 0)
        {
            mpGlobalParametersWidget = new GlobalParametersWidget(this);
        }
        mpGlobalParametersDock->setWidget(mpGlobalParametersWidget);

//        if( (dockWidgetArea(mpGlobalParametersDock) == dockWidgetArea(mpPlotWidgetDock)) ||
//             dockWidgetArea(mpGlobalParametersDock) == dockWidgetArea(mpUndoWidgetDock) )
//        {
//            tabifyDockWidget(mpGlobalParametersDock, mpUndoWidgetDock);
//        }

        mpGlobalParametersDock->show();
        mpGlobalParametersDock->raise();
    }
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
    mUseMulticore = false;
    mEnableProgressBar = true;
    mProgressBarStep = 50;
    mSnapping = true;
    mBackgroundColor = QColor("white");
    mAntiAliasing = true;
    mLastSessionModels.clear();
    mRecentModels.clear();

    mDefaultUnits.insert("Pressure", "Pa");
    mDefaultUnits.insert("Flow", "m^3/s");
    mDefaultUnits.insert("Position", "m");
    mDefaultUnits.insert("Velocity", "m/s");
    mDefaultUnits.insert("Acceleration", "m/s^2");
    mDefaultUnits.insert("Force", "N");

        //Definition of dalternative units
    QMap<QString, double> PressureUnitMap;
    PressureUnitMap.insert("Pa", 1);
    PressureUnitMap.insert("Bar", 1e-5);
    PressureUnitMap.insert("MPa", 1e-6);
    PressureUnitMap.insert("psi", 1.450326e-4);
    QMap<QString, double> FlowUnitMap;
    FlowUnitMap.insert("m^3/s", 1);
    FlowUnitMap.insert("l/min", 60000);
    QMap<QString, double> ForceUnitMap;
    ForceUnitMap.insert("N", 1);
    ForceUnitMap.insert("kN", 1e-3);
    QMap<QString, double> PositionUnitMap;
    PositionUnitMap.insert("m", 1);
    PositionUnitMap.insert("mm", 1000);
    PositionUnitMap.insert("cm", 100);
    PositionUnitMap.insert("inch", 39.3700787);
    PositionUnitMap.insert("ft", 3.2808);
    QMap<QString, double> VelocityUnitMap;
    VelocityUnitMap.insert("m/s", 1);
    QMap<QString, double> AccelerationUnitMap;
    AccelerationUnitMap.insert("m/s^2", 1);
    mAlternativeUnits.insert("Pressure", PressureUnitMap);
    mAlternativeUnits.insert("Flow", FlowUnitMap);
    mAlternativeUnits.insert("Force", ForceUnitMap);
    mAlternativeUnits.insert("Position", PositionUnitMap);
    mAlternativeUnits.insert("Velocity", VelocityUnitMap);
    mAlternativeUnits.insert("Acceleration", AccelerationUnitMap);


        //Read from hopsanconfig.xml
    QFile file(QString(MAINPATH) + "hopsanconfig.xml");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        mpMessageWidget->printGUIErrorMessage("Unable to read settings file. Using default settings.");
        return;
    }
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
        QDomElement configRoot = domDocument.documentElement();
        if (configRoot.tagName() != "hopsanconfig")
        {
            QMessageBox::information(window(), tr("Hopsan GUI"),
                                     "The file is not an Hopsan Configuration File. Incorrect hmf root tag name: "
                                     + configRoot.tagName() + " != hopsanconfig");
        }
        else
        {
            QDomElement settingsElement = configRoot.firstChildElement("settings");

            mBackgroundColor.setNamedColor(settingsElement.firstChildElement("backgroundcolor").text());
            mAntiAliasing = parseDomBooleanNode(settingsElement.firstChildElement("antialiasing"));
            mInvertWheel = parseDomBooleanNode(settingsElement.firstChildElement("invertwheel"));
            mSnapping = parseDomBooleanNode(settingsElement.firstChildElement("snapping"));
            mEnableProgressBar = parseDomBooleanNode(settingsElement.firstChildElement("progressbar"));
            mProgressBarStep = parseDomValueNode(settingsElement.firstChildElement("progressbar_step"));
            mUseMulticore = parseDomBooleanNode(settingsElement.firstChildElement("multicore"));

            QDomElement modelsElement = configRoot.firstChildElement("models");
            QDomElement lastSessionElement = modelsElement.firstChildElement("lastsessionmodel");
            while (!lastSessionElement.isNull())
            {
                mLastSessionModels.prepend(lastSessionElement.text());
                lastSessionElement = lastSessionElement.nextSiblingElement("lastsessionmodel");
            }
            QDomElement recentModelElement = modelsElement.firstChildElement("recentmodel");
            while (!recentModelElement.isNull())
            {
                mRecentModels.prepend(QFileInfo(readName(recentModelElement.text())));
                recentModelElement = recentModelElement.nextSiblingElement("recentmodel");
            }

            QDomElement unitsElement = configRoot.firstChildElement("units");
            QDomElement defaultUnitElement = unitsElement.firstChildElement("defaultunit");
            while (!defaultUnitElement.isNull())
            {
                mDefaultUnits.insert(defaultUnitElement.firstChildElement("dataname").text(),
                                     defaultUnitElement.firstChildElement("unitname").text());
                defaultUnitElement = defaultUnitElement.nextSiblingElement("defaultunit");
            }
            QDomElement alternativeUnitElement = unitsElement.firstChildElement("customunit");
            while (!alternativeUnitElement.isNull())
            {
                QString physicalQuantity = alternativeUnitElement.firstChildElement("dataname").text();
                QString unitName = alternativeUnitElement.firstChildElement("unitname").text();
                double unitScale = parseDomValueNode(alternativeUnitElement.firstChildElement("scale"));

                if(!mAlternativeUnits.find(physicalQuantity).value().contains(unitName))
                {
                    mAlternativeUnits.find(physicalQuantity).value().insert(unitName, unitScale);
                }
                alternativeUnitElement = alternativeUnitElement.nextSiblingElement("customunit");
            }
        }
    }
    file.close();
}


//! Saves global settings to a text file
void MainWindow::saveSettings()
{
        //Write to hopsanconfig.xml
    QDomDocument domDocument;
    QDomElement configRoot = domDocument.createElement("hopsanconfig");
    domDocument.appendChild(configRoot);

    QDomElement settings = appendDomElement(configRoot,"settings");
    appendDomTextNode(settings, "backgroundcolor", mBackgroundColor.name());
    appendDomBooleanNode(settings, "antialiasing", mAntiAliasing);
    appendDomBooleanNode(settings, "invertwheel", mInvertWheel);
    appendDomBooleanNode(settings, "snapping", mSnapping);
    appendDomBooleanNode(settings, "progressbar", mEnableProgressBar);
    appendDomValueNode(settings, "progressbar_step", mProgressBarStep);
    appendDomBooleanNode(settings, "multicore", mUseMulticore);

    QDomElement libs = appendDomElement(configRoot, "libs");
    for(size_t i=0; i<mUserLibs.size(); ++i)
    {
        appendDomTextNode(libs, "userlib", mUserLibs.at(i));
    }

    QDomElement models = appendDomElement(configRoot, "models");
    for(size_t i=0; i<mLastSessionModels.size(); ++i)
    {
        if(mLastSessionModels.at(i) != "")
        {
            appendDomTextNode(models, "lastsessionmodel", mLastSessionModels.at(i));
        }
    }
    for(size_t i = 0; i<mRecentModels.size(); ++i)
    {
        if(mRecentModels.at(i).filePath() != "")
            appendDomTextNode(models, "recentmodel", mRecentModels.at(i).filePath());
    }

    QDomElement units = appendDomElement(configRoot, "units");
    QMap<QString, QString>::iterator itdu;
    for(itdu = mDefaultUnits.begin(); itdu != mDefaultUnits.end(); ++itdu)
    {
        QDomElement tempElement = appendDomElement(units, "defaultunit");
        appendDomTextNode(tempElement, "dataname", itdu.key());
        appendDomTextNode(tempElement, "unitname", itdu.value());
    }
    QMap<QString, QMap<QString, double> >::iterator itpcu;
    QMap<QString, double>::iterator itcu;
    for(itpcu = mAlternativeUnits.begin(); itpcu != mAlternativeUnits.end(); ++itpcu)
    {
        for(itcu = itpcu.value().begin(); itcu != itpcu.value().end(); ++itcu)
        {
            QDomElement tempElement = appendDomElement(units, "customunit");
            appendDomTextNode(tempElement, "dataname", itpcu.key());
            appendDomTextNode(tempElement, "unitname", itcu.key());
            appendDomValueNode(tempElement, "scale", itcu.value());
        }
    }

    appendRootXMLProcessingInstruction(domDocument);

    //Save to file
    const int IndentSize = 4;
    QFile xmlsettings(QString(MAINPATH) + "hopsanconfig.xml");
    if (!xmlsettings.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
    {
        qDebug() << "Failed to open file for writing: " << QString(MAINPATH) << "settings.xml";
        return;
    }
    QTextStream out(&xmlsettings);
    domDocument.save(out, IndentSize);

}







//! Make sure the values make sens.
//! @see fixTimeStep()
void MainWindow::fixSimulationParameterValues()
{
    fixFinishTime();
    fixTimeStep();
}


void MainWindow::registerRecentModel(QFileInfo model)
{
    if(model.fileName() == "")
        return;

    mRecentModels.removeAll(model);
    mRecentModels.prepend(model);
    while(mRecentModels.size() > 10)
    {
        mRecentModels.pop_back();
    }
    updateRecentList();
}


void MainWindow::updateRecentList()
{
    recentMenu->clear();
    if(mRecentModels.empty())
    {
        recentMenu->setDisabled(true);
    }
    else
    {
        recentMenu->setEnabled(true);
        for(size_t i=0; i<mRecentModels.size(); ++i)
        {
            if(mRecentModels.at(i).fileName() != "")
            {
                QAction *tempAction;
                tempAction = recentMenu->addAction(mRecentModels.at(i).fileName());
            }
        }
    }
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
