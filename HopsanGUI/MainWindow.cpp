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
//! @file   MainWindow.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the HopsanGUI MainWindow class
//!
//$Id$

#define MINSTARTTIME -1000000
#define MAXSTARTTIME 1000000
#define MINTIMESTEP 0.0
#define MAXTIMESTEP 1000000
#define MINSTOPTIME -1000000
#define MAXSTOPTIME 1000000

#include <QDebug>
#include <QFontDatabase>
#include <QtNetwork>
#include <QPixmap>
#include <QDesktopWidget>
#include <QDockWidget>
#include <QStatusBar>
#include <QMenuBar>
#include <QToolBar>
#include <QMessageBox>
#include <QDesktopServices>


#include "common.h"
#include "global.h"
#include "Configuration.h"
#include "CopyStack.h"
#include "DesktopHandler.h"
#include "LibraryHandler.h"
#include "MainWindow.h"
#include "ModelHandler.h"
#include "PlotHandler.h"
#include "UndoStack.h"
#include "version_gui.h"
#include "HcomHandler.h"
#include "SimulationThreadHandler.h"
#include "CoreAccess.h"

#include "Widgets/DebuggerWidget.h"
#include "Widgets/PlotWidget2.h"
#include "Widgets/MessageWidget.h"
#include "Widgets/HcomWidget.h"
#include "Widgets/LibraryWidget.h"
#include "Widgets/ModelWidget.h"
#include "Widgets/ProjectTabWidget.h"
#include "Widgets/SystemParametersWidget.h"
#include "Widgets/UndoWidget.h"
#include "Widgets/WelcomeWidget.h"
#include "Widgets/HVCWidget.h"
#include "Widgets/DataExplorer.h"
#include "Widgets/FindWidget.h"
#include "Widgets/TextEditorWidget.h"

#include "Dialogs/OptionsDialog.h"
#include "Dialogs/AboutDialog.h"
#include "Dialogs/HelpDialog.h"
#include "Dialogs/OptimizationDialog.h"
#include "Dialogs/SensitivityAnalysisDialog.h"
#include "Dialogs/LicenseDialog.h"
#include "Dialogs/NumHopScriptDialog.h"

#include "Utilities/GUIUtilities.h"
#include "Utilities/HelpPopUpWidget.h"

//! @todo maybe we can make sure that we don't need to include these here
#include "GraphicsView.h"
#include "GUIObjects/GUIContainerObject.h"

// Declare (create) global pointers that will point to MainWindow children
PlotHandler *gpPlotHandler = 0;
LibraryWidget *gpLibraryWidget = 0;
TerminalWidget *gpTerminalWidget = 0;
ModelHandler *gpModelHandler = 0;
PlotWidget2 *gpPlotWidget = 0;
CentralTabWidget *gpCentralTabWidget = 0;
SystemParametersWidget *gpSystemParametersWidget = 0;
UndoWidget *gpUndoWidget = 0;
LibraryHandler *gpLibraryHandler = 0;
HelpPopUpWidget *gpHelpPopupWidget = 0;
SensitivityAnalysisDialog *gpSensitivityAnalysisDialog = 0;
HelpDialog *gpHelpDialog = 0;
OptimizationDialog *gpOptimizationDialog = 0;
QAction *gpToggleNamesAction = 0;
QAction *gpTogglePortsAction = 0;
OptionsDialog *gpOptionsDialog = 0;
QGridLayout *gpCentralGridLayout = 0;
FindWidget *gpFindWidget = 0;

//! @brief Constructor for main window
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->hide();

    // Set main window options
    this->setDockOptions(QMainWindow::ForceTabbedDocks);
    this->setMouseTracking(true);

    // Set name and icon of main window
#ifdef DEVELOPMENT
    this->setWindowTitle(tr("Hopsan (development version)"));
#else
    this->setWindowTitle(tr("Hopsan"));
#endif
    this->setWindowIcon(QIcon(QString(QString(GRAPHICSPATH) + "hopsan-icon.svg")));

    // Set dock widget corner owner
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);

    // Set the size and position of the main window
    int sh = qApp->desktop()->screenGeometry().height();
    int sw = qApp->desktop()->screenGeometry().width();
    this->resize(sw*0.8, sh*0.8);   //Resize window to 80% of screen height and width
//    int w = this->size().width();
//    int h = this->size().height();
//    int x = (sw - w)/2;
//    int y = (sh - h)/2;
//    this->move(x, y);       //Move window to center of screen
}


//! @brief Destructor
//! @todo Shouldn't all member pointers be deleted here?
MainWindow::~MainWindow()
{
    delete gpPlotHandler;
    delete mpCentralTabs;
    delete mpModelHandler;
    delete mpMenuBar;
    delete mpStatusBar;
}

//! @brief Creates the contents of the MainWindow, call this only once
void MainWindow::createContents()
{
    // Create plothandler as child to mainwindow but assign to global ptr
    gpPlotHandler = new PlotHandler(this);

    // Initialize the help message popup
    mpHelpPopup = new HelpPopUpWidget(this);
    gpHelpPopupWidget = mpHelpPopup;

    //Create the terminal widget
    mpTerminalDock = new QDockWidget(tr("Terminal"), this);
    mpTerminalDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    gpTerminalWidget = new TerminalWidget(this);
    gpTerminalWidget->mpConsole->printFirstInfo();
    mpTerminalDock->setWidget(gpTerminalWidget);
    mpTerminalDock->setFeatures(QDockWidget::DockWidgetVerticalTitleBar | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    addDockWidget(Qt::BottomDockWidgetArea, mpTerminalDock);
    mpTerminalDock->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(gpMessageHandler, SIGNAL(newAnyMessage(GUIMessage)), gpTerminalWidget, SLOT(printMessage(GUIMessage)));

    //Create the message widget and its dock (must be done before everything that uses it!)
    mpMessageDock = new QDockWidget(tr("Messages"), this);
    mpMessageDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    mpMessageWidget = new MessageWidget(this);
    mpMessageDock->setWidget(mpMessageWidget);
    mpMessageDock->setFeatures(QDockWidget::DockWidgetVerticalTitleBar | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    addDockWidget(Qt::BottomDockWidgetArea, mpMessageDock);
    connect(gpMessageHandler, SIGNAL(newAnyMessage(GUIMessage)), mpMessageWidget, SLOT(receiveMessage(GUIMessage)));

    // Print init messages
    gpMessageHandler->collectHopsanCoreMessages();
#ifdef HOPSANCOMPILED64BIT
    QString archtext="HopsanGUI 64-bit, ";
#else
    QString archtext="HopsanGUI 32-bit, ";
#endif
    QString debugtext;
#ifdef HOPSAN_BUILD_TYPE_DEBUG
    debugtext=" (Compiled in debug mode)";
#endif
    gpMessageHandler->addInfoMessage(archtext+"Version: "+QString(HOPSANGUIVERSION)+debugtext);

    //Load configuration from settings file
    emit showSplashScreenMessage("Loading configuration...");
    gpConfig->loadFromXml();      //!< @todo This does not really belong in main window constructor, but it depends on main window so keep it for now
    initializaHopsanCore(gpConfig->getStringSetting(cfg::paths::corelogfile));

    emit showSplashScreenMessage("Initializing GUI...");

    //Create dialogs
    mpAboutDialog = new AboutDialog(this);
    mpOptimizationDialog = new OptimizationDialog(this);
    gpOptimizationDialog = mpOptimizationDialog;
    mpHelpDialog = new HelpDialog(this);
    gpHelpDialog = mpHelpDialog;

    //Create the library handler
    gpLibraryHandler = new LibraryHandler();

    //Create the component library widget and its dock
    mpLibDock = new QDockWidget(tr("Component Library"), this);
    mpLibDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mpLibraryWidget = new LibraryWidget(this);
    gpLibraryWidget = mpLibraryWidget;
    mpLibDock->setWidget(mpLibraryWidget);
    addDockWidget(Qt::LeftDockWidgetArea, mpLibDock);

    //Create the statusbar widget
    mpStatusBar = new QStatusBar();
    mpStatusBar->setObjectName("statusBar");
    this->setStatusBar(mpStatusBar);

    //Create the undo widget and the options dialog
    mpUndoWidget = new UndoWidget(this);
    gpUndoWidget = mpUndoWidget;
    mpOptionsDialog = new OptionsDialog(this);
    gpOptionsDialog = mpOptionsDialog;

    //Create the central widget for the main window
    mpCentralWidget = new QWidget(this);
    mpCentralWidget->setObjectName("centralwidget");
    mpCentralWidget->setMouseTracking(true);
    this->setCentralWidget(mpCentralWidget);

    //Create the grid layout for the centralwidget
    mpCentralGridLayout = new QGridLayout(mpCentralWidget);
    gpCentralGridLayout = mpCentralGridLayout;
    mpCentralGridLayout->setContentsMargins(4,4,4,4);

    //Create the model handler object
    mpModelHandler = new ModelHandler(this);
    gpModelHandler = mpModelHandler;
    connect(mpModelHandler, SIGNAL(modelChanged(ModelWidget*)), gpTerminalWidget->mpHandler, SLOT(setModelPtr(ModelWidget*)));

    //Create the sensitivity analysis dialog
    mpSensitivityAnalysisDialog = new SensitivityAnalysisDialog(this);
    gpSensitivityAnalysisDialog = mpSensitivityAnalysisDialog;

    //Create the main tab container, need at least one tab
    mpCentralTabs = new CentralTabWidget(this);
    gpCentralTabWidget = mpCentralTabs;
    mpCentralTabs->setObjectName("centralTabs");
    mpCentralTabs->setMouseTracking(true);

    connect(mpCentralTabs, SIGNAL(currentChanged(int)),         this,           SLOT(updateToolBarsToNewTab()), Qt::UniqueConnection);
    connect(mpCentralTabs, SIGNAL(currentChanged(int)),         this,           SLOT(refreshUndoWidgetList()), Qt::UniqueConnection);
    connect(mpCentralTabs,   SIGNAL(currentChanged(int)),       gpModelHandler, SLOT(selectModelByTabIndex(int)), Qt::UniqueConnection);
    connect(mpCentralTabs,   SIGNAL(tabCloseRequested(int)),    gpModelHandler, SLOT(closeModelByTabIndex(int)), Qt::UniqueConnection);

    //Create the system parameter widget and hide it
    mpSystemParametersWidget = new SystemParametersWidget(this);
    gpSystemParametersWidget = mpSystemParametersWidget;
    mpSystemParametersWidget->setVisible(false);

    // Create the HVC Widget
    mpHVCWidget = new HVCWidget(this);
    mpHVCWidget->setVisible(false);

    // Create the data explorer widget
    mpDataExplorer = new DataExplorer(this);
    mpDataExplorer->hide();

    //Create the plot dock widget and hide it
    mpPlotWidgetDock = new QDockWidget(tr("Plot Variables"), this);
    mpPlotWidgetDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mpPlotWidgetDock->hide();
    addDockWidget(Qt::RightDockWidgetArea, mpPlotWidgetDock);
    connect(mpPlotWidgetDock, SIGNAL(visibilityChanged(bool)), this, SLOT(updatePlotActionButton(bool)));

    //Create the system parameters dock widget and hide it
    mpSystemParametersDock = new QDockWidget(tr("System Parameters"), this);
    mpSystemParametersDock->setAllowedAreas((Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea));
    addDockWidget(Qt::RightDockWidgetArea, mpSystemParametersDock);
    mpSystemParametersDock->hide();

    //Create the undo dock widget and hide it
    mpUndoWidgetDock = new QDockWidget(tr("Undo History"), this);
    mpUndoWidgetDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mpUndoWidgetDock->hide();
    addDockWidget(Qt::RightDockWidgetArea, mpUndoWidgetDock);

    //Make dock widgets that share same dock area tabified, instead of stacking them above each other
    tabifyDockWidget(mpPlotWidgetDock, mpSystemParametersDock);
    tabifyDockWidget(mpSystemParametersDock, mpUndoWidgetDock);
    tabifyDockWidget(mpUndoWidgetDock, mpPlotWidgetDock);

    tabifyDockWidget(mpTerminalDock, mpMessageDock);
    mpTerminalDock->raise();

    // Set the correct position of the help popup message in the central widget
    mpCentralGridLayout->addWidget(mpCentralTabs,0,0,4,4);
    mpCentralGridLayout->addWidget(mpHelpPopup, 1,1,1,1);
    mpCentralGridLayout->setColumnMinimumWidth(0,5);
    mpCentralGridLayout->setColumnStretch(0,0);
    mpCentralGridLayout->setColumnStretch(1,0);
    mpCentralGridLayout->setColumnStretch(2,0);
    mpCentralGridLayout->setColumnStretch(3,1);
    mpCentralGridLayout->setRowMinimumHeight(0,25);
    mpCentralGridLayout->setRowStretch(0,0);
    mpCentralGridLayout->setRowStretch(1,0);
    mpCentralGridLayout->setRowStretch(2,1);

    //Create actions, toolbars and menus
    this->createActions();
    this->createToolbars();
    this->createMenus();

    connect(mpCopyAction, SIGNAL(triggered()), mpMessageWidget, SLOT(copy()));
    connect(mpCopyAction, SIGNAL(triggered()), gpTerminalWidget->mpConsole, SLOT(copy()));

    gpTerminalWidget->loadConfig();

    mpWelcomeWidget = 0;

    mpWelcomeWidget = new WelcomeWidget(this);
    mpCentralTabs->addTab(mpWelcomeWidget, "Start");
    mpCentralTabs->setTabNotClosable(0);

    this->updateRecentList();

    // Update style sheet setting after all children has been created and added so that they too will be affected
    if(!gpConfig->getBoolSetting(cfg::nativestylesheet))
    {
        setStyleSheet(gpConfig->getStyleSheet());
        setPalette(gpConfig->getPalette());
        qApp->setFont(gpConfig->getFont());
    }

    updateToolBarsToNewTab();
}


//! @brief Initializes the workspace.
//! All startup events that does not involve creating the main window and its widgets/dialogs belongs here.
void MainWindow::initializeWorkspace()
{
    emit showSplashScreenMessage("Loading component libraries...");

    // Load HopsanGui built in secret components
    gpLibraryHandler->loadLibrary(QString(BUILTINCAFPATH) + "hidden/builtin_hidden.xml", InternalLib, Hidden);

    // Load default and user specified libraries
    QString componentPath = gpDesktopHandler->getComponentsPath();

    // Load built in default Library
#ifdef HOPSAN_INTERNALDEFAULTCOMPONENTS
    gpLibraryHandler->loadLibrary(componentPath, InternalLib);
#else
    gpLibraryHandler->loadLibrary(componentPath+"defaultComponentLibrary.xml", InternalLib);
#endif

    // Load libraries from autoLibs folder
    QString autoLibsPath = gpDesktopHandler->getAutoLibsPath();
    QDir autoLibsDir;
    autoLibsDir.setPath(autoLibsPath);
    if(autoLibsDir.exists())
    {
        for(const QString &libPath : autoLibsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            qDebug() << "Loading library: " << autoLibsPath+"/"+libPath;
            gpLibraryHandler->loadLibrary(autoLibsPath+"/"+libPath);
        }
    }

    // Load builtIn library (Container special components)
    gpLibraryHandler->loadLibrary(QString(BUILTINCAFPATH) + "visible/builtin_visible.xml", InternalLib);

    // Load previously loaded external libraries
    QStringList userLibs = gpConfig->getUserLibs();
    QList<LibraryTypeEnumT> userLibTypes = gpConfig->getUserLibTypes();
    for(int i=0; i<qMin(userLibs.size(), userLibTypes.size()); ++i)
    {
        emit showSplashScreenMessage("Loading library: "+userLibs[i]+"...");
        gpLibraryHandler->loadLibrary(userLibs[i], userLibTypes[i]);
    }

    // Create the plot widget, only once! :)
    gpPlotWidget = new PlotWidget2(this);
    gpPlotWidget->hide();

    // Create the find widget
    gpFindWidget = new FindWidget(this);
    mpCentralGridLayout->addWidget(gpFindWidget,5,0,1,4);
    gpFindWidget->setDisabled(true);
    gpFindWidget->hide();

    // File association - ignore everything else and open the specified file if there is a hmf file in the argument list
    for(int i=0; i<qApp->arguments().size(); ++i)
    {
        if(qApp->arguments().at(i).endsWith(".hmf"))
        {
            mpModelHandler->closeAllModels();
            mpModelHandler->loadModel(qApp->arguments().at(i));
            return;
        }
    }

    mpLibraryWidget->adjustSize();

    emit showSplashScreenMessage("Finished!");
}


//! @brief Toggles visibility of the plot widget and its dock
void MainWindow::toggleVisiblePlotWidget()
{
    // Show it if it is currently not visible
    if(mpPlotWidgetDock->isHidden())
    {
        mpPlotWidgetDock->setWidget(gpPlotWidget);
        mpPlotWidgetDock->show();
        mpPlotWidgetDock->raise();
    }
    // Else hide it
    else
    {
        mpPlotWidgetDock->hide();
    }
}


//! @brief Event triggered re-implemented method that closes the main window.
//! First all tabs (models) are closed, if the user do not push Cancel
//! (closeAllModels then returns 'false') the event is accepted and
//! the main window is closed.
//! @param event contains information of the closing operation.
void MainWindow::closeEvent(QCloseEvent *event)
{
    // Must close all open windows before closing project tabs
    //! @todo need to restore function that closes those plots belonging to a particular tab/model automatically /Peter
    gpPlotHandler->closeAllOpenWindows();

    if (mpModelHandler->closeAllModels())
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }

    gpTerminalWidget->saveConfig();
    gpConfig->saveToXml();
}


//! @brief Defines the actions used by the toolbars
void MainWindow::createActions()
{
    mpNewAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-New.svg"), tr("&New"), this);
    mpNewAction->setShortcut(tr("New"));
    mpNewAction->setToolTip(tr("Create New Project"));
    connect(mpNewAction, SIGNAL(triggered()), mpModelHandler, SLOT(addNewModel()));
    connect(mpNewAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpNewAction, "Create a new empty model.");

    mpOpenAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-Open.svg"), tr("&Open"), this);
    mpOpenAction->setShortcut(QKeySequence("Ctrl+o"));
    mpOpenAction->setToolTip(tr("Load Model File (Ctrl+O)"));
    connect(mpOpenAction, SIGNAL(triggered()), mpModelHandler, SLOT(loadModel()));
    connect(mpOpenAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpOpenAction, "Open an existing model.");

    mpNewScriptAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-Script.svg"), tr("&New HCOM Script"), this);
    mpNewScriptAction->setToolTip(tr("Create HCOM Script File"));
    connect(mpNewScriptAction, SIGNAL(triggered()), mpModelHandler, SLOT(newTextFile()));
    connect(mpNewScriptAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpNewScriptAction, "Create a new HCOM script file.");

    mpOpenTextFileAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-OpenScript.svg"), tr("&Open Text File"), this);
    mpOpenTextFileAction->setToolTip(tr("Load Text File"));
    connect(mpOpenTextFileAction, SIGNAL(triggered()), mpModelHandler, SLOT(loadTextFile()));
    connect(mpOpenTextFileAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpOpenTextFileAction, "Open a text file.");

    mpSaveAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-Save.svg"), tr("&Save"), this);
    mpSaveAction->setShortcut(QKeySequence("Ctrl+s"));
    mpSaveAction->setToolTip(tr("Save Model File (Ctrl+S)"));
    connect(mpSaveAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpSaveAction, "Save current model.");

    mpSaveAsAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-SaveAs.svg"), tr("&Save As"), this);
    mpSaveAsAction->setShortcut(QKeySequence("Ctrl+Alt+s"));
    mpSaveAsAction->setToolTip(tr("Save Model File As (Ctrl+Alt+S)"));
    connect(mpSaveAsAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpSaveAsAction, "Save current model as new file.");

    mpSaveAndRunAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-SaveAndRun.svg"), tr("&Save As"), this);
    mpSaveAndRunAction->setShortcut(QKeySequence("F5"));
    mpSaveAndRunAction->setToolTip(tr("Save And Execute Script (F5)"));
    connect(mpSaveAndRunAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpSaveAndRunAction, "Save and execute current script.");

    mpExportSimulationStateAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-ExportParameters.pvg"), tr("&Export Simulation State"), this);
    mpExportSimulationStateAction->setToolTip(tr("Export simulation state"));

    mpExportModelParametersActionToHpf = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-ExportParameters.svg"), tr("&Export Model Parameters to HPF"), this);
    mpExportModelParametersActionToHpf->setToolTip(tr("Export Model Parameters to HPF"));
    connect(mpExportModelParametersActionToHpf, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpExportModelParametersActionToHpf, "Export model parameter set to HPF.");

    mpExportModelParametersActionToSsv = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-ExportParameters.svg"), tr("&Export Model Parameters to SSV"), this);
    mpExportModelParametersActionToSsv->setShortcut(QKeySequence("Ctrl+Alt+E"));
    mpExportModelParametersActionToSsv->setToolTip(tr("Export Model Parameters to SSV (Ctrl+Alt+E)"));
    connect(mpExportModelParametersActionToSsv, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpExportModelParametersActionToSsv, "Export model parameter set to SSV.");

    mpExportModelParametersMenu = new QMenu("Export Model Parameteres");
    mpExportModelParametersMenu->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-ExportParameters.svg"));
    mpExportModelParametersMenu->addAction(mpExportModelParametersActionToSsv);
    mpExportModelParametersMenu->addAction(mpExportModelParametersActionToHpf);

    mpCloseAction = new QAction(this);
    mpCloseAction->setText("Close");
    mpCloseAction->setShortcut(QKeySequence("Ctrl+q"));
    connect(mpCloseAction,SIGNAL(triggered()),this,SLOT(close()));

    mpUndoAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-Undo.svg"), tr("&Undo"), this);
    mpUndoAction->setText("Undo");
    mpUndoAction->setShortcut(QKeySequence(tr("Ctrl+z")));
    mpUndoAction->setToolTip(tr("Undo One Step (Ctrl+Z)"));
    connect(mpUndoAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpUndoAction, "Undo last action in current model.");

    mpRedoAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-Redo.svg"), tr("&Redo"), this);
    mpRedoAction->setText("Redo");
    mpRedoAction->setShortcut(QKeySequence(tr("Ctrl+y")));
    mpRedoAction->setToolTip(tr("Redo One Step (Ctrl+Y)"));
    connect(mpRedoAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpRedoAction, "Redo last undone action in current model.");

    mpOpenUndoAction = new QAction(tr("&Undo History"), this);
    mpOpenUndoAction->setToolTip("Undo History (Ctrl+Shift+U)");
    connect(mpOpenUndoAction,SIGNAL(triggered()),this,SLOT(openUndoWidget()));
    mpOpenUndoAction->setShortcut(QKeySequence("Ctrl+Shift+u"));

    mpEnableUndoAction = new QAction(tr("&Enable Undo"), this);
    mpEnableUndoAction->setText("Enable Undo");
    mpEnableUndoAction->setCheckable(true);
    mpEnableUndoAction->setChecked(true);

    mpOpenSystemParametersAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-SystemParameters.svg"), tr("&System Parameters"), this);
    mpOpenSystemParametersAction->setToolTip("System Parameters (Ctrl+Shift+Y)");
    mpOpenSystemParametersAction->setShortcut(tr("Ctrl+Shift+y"));
    mpOpenSystemParametersAction->setCheckable(true);
    connect(mpOpenSystemParametersAction,SIGNAL(triggered()),this,SLOT(openSystemParametersWidget()));
    connect(mpOpenSystemParametersAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpOpenSystemParametersAction, "Opens the list of system parameters in current model.");

    mpOpenHvcWidgetAction = new QAction(tr("&Model Validation"), this);
    mpOpenHvcWidgetAction->setToolTip("Open the HopsanValidationConfiguration Widget");
    connect(mpOpenHvcWidgetAction, SIGNAL(triggered()), this, SLOT(openHVCWidget()));

    mpOpenDataExplorerAction = new QAction(tr("&Data Explorer"), this);
    mpOpenDataExplorerAction->setShortcut(QKeySequence("Ctrl+Shift+d"));
    mpOpenDataExplorerAction->setToolTip("Open the Data Explorer Widget (Ctrl+Shift+d)");
    connect(mpOpenDataExplorerAction, SIGNAL(triggered()), this, SLOT(openDataExplorerWidget()));

    mpOpenFindWidgetAction = new QAction(tr("&Find Widget"), this);
    mpOpenFindWidgetAction->setShortcut(QKeySequence("Ctrl+f"));
    mpOpenFindWidgetAction->setToolTip("Open the Find Widget (Ctrl+Shift+f)");
    connect(mpOpenFindWidgetAction, SIGNAL(triggered()), this, SLOT(openFindWidget()));

    mpRevertModelAction = new QAction(tr("&Reload file from disk"), this);
    mpRevertModelAction->setToolTip("Reloads current file from disk");
    connect(mpRevertModelAction, SIGNAL(triggered()), this, SLOT(revertModel()));

    mpNumHopAction = new QAction(tr("&NumHop Script"), this);
    mpNumHopAction->setShortcut(QKeySequence("Ctrl+Shift+n"));
    mpNumHopAction->setToolTip("Open NumHop Script Dialogue for this System (Ctrl+Shift+n)");
    connect(mpNumHopAction, SIGNAL(triggered()), this, SLOT(openNumHopDialog()));

    mpCutAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-Cut.svg"), tr("&Cut"), this);
    mpCutAction->setShortcut(tr("Ctrl+x"));
    mpCutAction->setToolTip(tr("Cut (Ctrl+X)"));
    connect(mpCutAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    connect(mpCutAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpCutAction, "Cut selected components.");

    mpCopyAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-Copy.svg"), tr("&Copy"), this);
    mpCopyAction->setShortcut(tr("Ctrl+c"));
    mpCopyAction->setToolTip("Copy (Ctrl+C)");
    connect(mpCopyAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpCopyAction, "Copy selected components.");

    mpPasteAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-Paste.svg"), tr("&Paste"), this);
    mpPasteAction->setShortcut(tr("Ctrl+v"));
    mpPasteAction->setToolTip(tr("Paste (Ctrl+V)"));
    connect(mpPasteAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpPasteAction, "Paste copied components in current model.");

    mpSimulateAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-Simulate.svg"), tr("&Simulate"), this);
    mpSimulateAction->setToolTip(tr("Simulate Current Project (Ctrl+Shift+S)"));
    mpSimulateAction->setShortcut(QKeySequence("Ctrl+Shift+s"));
    connect(mpSimulateAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    connect(mpSimulateAction, SIGNAL(triggered()), this, SLOT(simulateKeyWasPressed()));
    mHelpPopupTextMap.insert(mpSimulateAction, "Starts a new simulation of current model.");

    mpToggleRemoteCoreSimAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-SimulateRemote.svg"), tr("Toggle Remote HopsanCore Connection"), this);
    mpToggleRemoteCoreSimAction->setCheckable(true);
    mpToggleRemoteCoreSimAction->setChecked(false);
    mHelpPopupTextMap.insert(mpToggleRemoteCoreSimAction, "Connect or disconnect to a remote HopsanCore, this will determine if local or remote simulation is run, when calling simulate");
    connect(mpToggleRemoteCoreSimAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpNewDcpModelAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-NewDcpModel.svg"), tr("Create new DCP model"), this);
    mpNewDcpModelAction->setToolTip("Create new DCP model");
    connect(mpNewDcpModelAction, SIGNAL(triggered()), mpModelHandler, SLOT(addNewDcpModel()));

    mpStartDcpMasterAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-StartDcpManager.svg"), tr("Simulate model as DCP master"), this);
    mpStartDcpMasterAction->setToolTip("Simulate model as DCP master");

    mpStartDcpServerAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-StartDcpServer.svg"), tr("Simulate model as DCP server"), this);
    mpStartDcpServerAction->setToolTip("Simulate model as DCP server");

    mpOpenDebuggerAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-Debug.svg"), tr("&Launch Debugger"), this);
    mpOpenDebuggerAction->setToolTip(tr("Launch Debugger"));
    connect(mpOpenDebuggerAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    connect(mpOpenDebuggerAction,  SIGNAL(triggered()), mpModelHandler, SLOT(launchDebugger()));
    mHelpPopupTextMap.insert(mpOpenDebuggerAction, "Open debugger dialog to examine the current model in detail.");

    mpOptimizeAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-Optimize.svg"), tr("&Optimize"), this);
    mpOptimizeAction->setToolTip(tr("Open Optimization Dialogue (Ctrl+Shift+Z)"));
    mpOptimizeAction->setShortcut(QKeySequence("Ctrl+Shift+z"));
    connect(mpOptimizeAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    connect(mpOptimizeAction, SIGNAL(triggered()), mpOptimizationDialog, SLOT(open()));
    mHelpPopupTextMap.insert(mpOptimizeAction, "Open optimization dialog to initialize numerical optimization of current model.");

    mpSensitivityAnalysisAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-SensitivityAnalysis.svg"), tr("&Sensitivity Analysis"), this);
    mpSensitivityAnalysisAction->setToolTip(tr("Open Sensitivity Analysis Dialogue (Ctrl+Shift+A)"));
    mpSensitivityAnalysisAction->setShortcut(QKeySequence("Ctrl+Shift+A"));
    connect(mpSensitivityAnalysisAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    connect(mpSensitivityAnalysisAction, SIGNAL(triggered()), mpSensitivityAnalysisDialog, SLOT(open()));
    mHelpPopupTextMap.insert(mpSensitivityAnalysisAction, "Perform sensitivity analysis of current model.");

    mpMeasureSimulationTimeAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-MeasureSimulationTime.svg"), tr("&Measure Simulation Times"), this);
    mpMeasureSimulationTimeAction->setToolTip(tr("Measure Simulation Times"));
    connect(mpMeasureSimulationTimeAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    connect(mpMeasureSimulationTimeAction, SIGNAL(triggered()), mpModelHandler, SLOT(measureSimulationTime()));
    mHelpPopupTextMap.insert(mpMeasureSimulationTimeAction, "Measure simulation time for each component in current model.");

    mpPlotAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-Plot.svg"), tr("&Plot Variables"), this);
    mpPlotAction->setToolTip(tr("Plot Variables (Ctrl+Shift+P)"));
    mpPlotAction->setCheckable(true);
    mpPlotAction->setShortcut(QKeySequence("Ctrl+Shift+p"));
    connect(mpPlotAction, SIGNAL(triggered()),this,SLOT(toggleVisiblePlotWidget()));
    connect(mpPlotAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpPlotAction, "Opens the list with all available plot variables from current model.");

    mpLoadLibsAction = new QAction(this);
    mpLoadLibsAction->setText("Load Libraries");
    connect(mpLoadLibsAction,SIGNAL(triggered()),gpLibraryHandler,SLOT(loadLibrary()));

    mpPropertiesAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-Configure.svg"), tr("&Model Properties"), this);
    mpPropertiesAction->setToolTip("Model Properties (Ctrl+Shift+M)");
    mpPropertiesAction->setShortcut(QKeySequence("Ctrl+Shift+m"));
    connect(mpPropertiesAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpPropertiesAction, "Opens a dialog with settings for the current model.");

    mpOptionsAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-Options.svg"), tr("&Options"), this);
    mpOptionsAction->setToolTip("Options (Ctrl+Shift+O)");
    mpOptionsAction->setShortcut(QKeySequence("Ctrl+Shift+o"));
    connect(mpOptionsAction, SIGNAL(triggered()), mpOptionsDialog, SLOT(show()));
    connect(mpOptionsAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpOptionsAction, "Open options dialog to change program settings.");

    mpAnimateAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-Animation.svg"), tr("&Animate"), this);
    mpAnimateAction->setToolTip("Animate");
    connect(mpAnimateAction, SIGNAL(triggered()),mpModelHandler, SLOT(openAnimation()));
    connect(mpAnimateAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpAnimateAction, "Open current model in animation mode.");

    mpAlignXAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-AlignX.svg"), tr("&Align Vertical (by last selected)"), this);
    mpAlignXAction->setText("Align Vertical");
    mHelpPopupTextMap.insert(mpAlignXAction, "Align selected components horizontally to last selected component.");
    connect(mpAlignXAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpAlignYAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-AlignY.svg"), tr("&Align Horizontal (by last selected)"), this);
    mpAlignYAction->setText("Align Horizontal");
    mHelpPopupTextMap.insert(mpAlignYAction, "Align selected components vertically to last selected component.");
    connect(mpAlignYAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpDistributeXAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-DistributeX.svg"), tr("&Distribute Equidistantly Horizontally"), this);
    mpDistributeXAction->setText("Distribute Equidistantly Horizontally");
    mHelpPopupTextMap.insert(mpDistributeXAction, "Distributes all selected components equally (horizontally)");
    connect(mpDistributeXAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpDistributeYAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-DistributeY.svg"), tr("&Distribute Equidistantly Vertically"), this);
    mpDistributeYAction->setText("Distribute Equidistantly Vertically");
    mHelpPopupTextMap.insert(mpDistributeYAction, "Distributes all selected components equally (vertically)");
    connect(mpDistributeYAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpRotateLeftAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-RotateLeft.svg"), tr("&Rotate Left (Ctrl+E)"), this);
    mpRotateLeftAction->setText("Rotate Left (Ctrl+E)");
    mpRotateLeftAction->setShortcut(QKeySequence("Ctrl+E"));
    mHelpPopupTextMap.insert(mpRotateLeftAction, "Rotate selected components counter-clockwise.");
    connect(mpRotateLeftAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpRotateRightAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-RotateRight.svg"), tr("&Rotate Right (Ctrl+R)"), this);
    mpRotateRightAction->setText("Rotate Right (Ctrl+R)");
    mpRotateRightAction->setShortcut(QKeySequence("Ctrl+R"));
    mHelpPopupTextMap.insert(mpRotateRightAction, "Rotate selected components clockwise.");
    connect(mpRotateRightAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpFlipHorizontalAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-FlipHorizontal.svg"), tr("&Flip Horizontal"), this);
    mpFlipHorizontalAction->setText("Flip Horizontal (Ctrl+Shift+E)");
    mpFlipHorizontalAction->setShortcut(QKeySequence("Ctrl+Shift+E"));
    mHelpPopupTextMap.insert(mpFlipHorizontalAction, "Flip selected components horizontally.");
    connect(mpFlipHorizontalAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpFlipVerticalAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-FlipVertical.svg"), tr("&Flip Vertical"), this);
    mpFlipVerticalAction->setText("Flip Vertical (Ctrl+Shift+R");
    mpFlipVerticalAction->setShortcut(QKeySequence("Ctrl+Shift+R"));
    mHelpPopupTextMap.insert(mpFlipVerticalAction, "Flip selected components vertically.");
    connect(mpFlipVerticalAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpResetZoomAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-Zoom100.svg"), tr("&Reset Zoom (Ctrl+0)"), this);
    mpResetZoomAction->setText("Reset Zoom (Ctrl+0)");
    mpResetZoomAction->setShortcut(QKeySequence("Ctrl+0"));
    connect(mpResetZoomAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpResetZoomAction, "Reset zoom to 100%.");

    mpZoomInAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-ZoomIn.svg"), tr("&Zoom In (Ctrl+Plus)"), this);
    mpZoomInAction->setText("Zoom In (Ctrl+Plus)");
    mpZoomInAction->setShortcut(QKeySequence("Ctrl++"));
    connect(mpZoomInAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpZoomInAction, "Increase zoom level.");

    mpZoomOutAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-ZoomOut.svg"), tr("&Zoom Out (Ctrl+Minus)"), this);
    mpZoomOutAction->setText("Zoom Out (Ctrl+Minus)");
    mpZoomOutAction->setShortcut(QKeySequence("Ctrl+-"));
    connect(mpZoomOutAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpZoomOutAction, "Decrease zoom level.");

    mpCenterViewAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-CenterView.svg"), tr("&Center View (Ctrl+Space)"), this);
    mpCenterViewAction->setText("Center View (Ctrl+Space)");
    mpCenterViewAction->setShortcut(QKeySequence("Ctrl+Space"));
    connect(mpCenterViewAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpCenterViewAction, "Center view in current model.");

    QIcon toggleNamesIcon;
    toggleNamesIcon.addFile(QString(ICONPATH) + "svg/Hopsan-ToggleNames.svg", QSize(), QIcon::Normal, QIcon::On);
    mpToggleNamesAction = new QAction(toggleNamesIcon, tr("&Show Component Names (Ctrl+N)"), this);
    gpToggleNamesAction = mpToggleNamesAction;
    mpToggleNamesAction->setText("Show Component Names (Ctrl+N)");
    mpToggleNamesAction->setCheckable(true);
    mpToggleNamesAction->setChecked(gpConfig->getBoolSetting(cfg::togglenamesbuttonchecked));
    mpToggleNamesAction->setShortcut(QKeySequence("Ctrl+n"));
    connect(mpToggleNamesAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpToggleNamesAction, "Toggle  visibility of component names for all components.");

    mpPrintAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-Print.svg"), tr("&Print Model"), this);
    mpPrintAction->setText("Print Model");
    connect(mpPrintAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpPrintAction, "Print current model.");

    mpExportPDFAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-ExportPdf.svg"), tr("&Export To PDF"), this);
    mpExportPDFAction->setText("Export Model to PDF");
    connect(mpExportPDFAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpExportPDFAction, "Export current model to Portable Document Format (PDF).");

    mpExportPNGAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-ExportPng.svg"), tr("&Export To PNG"), this);
    mpExportPNGAction->setText("Export Model to PNG");
    connect(mpExportPNGAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpExportPNGAction, "Export current model to Portable Network Graphics (PNG).");

    mpImportFMUAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-ImportFmu.svg"), tr("Import Functional Mock-up Unit (FMU)"), this);
    mHelpPopupTextMap.insert(mpImportFMUAction, "Import Functional Mock-up Unit (FMU).");
    connect(mpImportFMUAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpImportDataFileAction = new QAction(tr("Import data file"), this);
    mHelpPopupTextMap.insert(mpImportDataFileAction, "Import (PLO or CSV) data file.");
    connect(mpImportDataFileAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    connect(mpImportDataFileAction, SIGNAL(triggered()), mpDataExplorer, SLOT(openImportDataDialog()));

    mpExportToSimulinkAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-ExportSimulink.svg"), tr("Export to Simulink S-function Source Files"), this);
    mHelpPopupTextMap.insert(mpExportToSimulinkAction, "Export model to Simulink S-function.");
    connect(mpExportToSimulinkAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpExportToExe_32Action = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-ExportExe.svg"), tr("Export to Executable Model (32-bit)"), this);
    mHelpPopupTextMap.insert(mpExportToExe_32Action, "Export to Executable Model (32-bit).");
    connect(mpExportToExe_32Action, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpExportToExe_64Action = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-ExportExe.svg"), tr("Export to Executable Model (64-bit)"), this);
    mHelpPopupTextMap.insert(mpExportToExe_64Action, "Export to Executable Model (64-bit).");
    connect(mpExportToExe_64Action, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpExportToFMU1_32Action = new QAction(tr("FMU 1.0 (32-bit)"), this);
    mHelpPopupTextMap.insert(mpExportToFMU1_32Action, "FMU 1.0 (32-bit)");
    connect(mpExportToFMU1_32Action, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpExportToFMU1_64Action = new QAction(tr("FMU 1.0 (64-bit)"), this);
    mHelpPopupTextMap.insert(mpExportToFMU1_64Action, "FMU 1.0 (64-bit)");
    connect(mpExportToFMU1_64Action, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpExportToFMU2_32Action = new QAction(tr("FMU 2.0 (32-bit)"), this);
    mHelpPopupTextMap.insert(mpExportToFMU2_32Action, "FMU 2.0 (32-bit)");
    connect(mpExportToFMU2_32Action, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpExportToFMU2_64Action = new QAction(tr("FMU 2.0 (64-bit)"), this);
    mHelpPopupTextMap.insert(mpExportToFMU2_64Action, "FMU 2.0 (64-bit)");
    connect(mpExportToFMU2_64Action, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpExportToFMU3_32Action = new QAction(tr("FMU 3.0 (32-bit)"), this);
    mHelpPopupTextMap.insert(mpExportToFMU3_32Action, "FMU 3.0 (32-bit)");
    connect(mpExportToFMU3_32Action, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpExportToFMU3_64Action = new QAction(tr("FMU 3.0 (64-bit)"), this);
    mHelpPopupTextMap.insert(mpExportToFMU3_64Action, "FMU 3.0 (64-bit)");
    connect(mpExportToFMU3_64Action, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpExportToFMUMenu = new QMenu("Export to Functional Mock-Up Interface (FMI)");
    mpExportToFMUMenu->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-ExportFmu.svg"));
#ifdef _WIN32
    mpExportToFMUMenu->addAction(mpExportToFMU1_32Action);
    mpExportToFMUMenu->addAction(mpExportToFMU2_32Action);
    mpExportToFMUMenu->addAction(mpExportToFMU3_32Action);
    mpExportToFMUMenu->addAction(mpExportToFMU1_64Action);
    mpExportToFMUMenu->addAction(mpExportToFMU2_64Action);
    mpExportToFMUMenu->addAction(mpExportToFMU3_64Action);
#elif __i386__
    mpExportToFMUMenu->addAction(mpExportToFMU1_32Action);
    mpExportToFMUMenu->addAction(mpExportToFMU2_32Action);
#elif __x86_64__
    mpExportToFMUMenu->addAction(mpExportToFMU1_64Action);
    mpExportToFMUMenu->addAction(mpExportToFMU2_64Action);
#endif

    mpExportToExeMenu = new QMenu("Export to Executable Model");
    mpExportToExeMenu->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-ExportExe.svg"));
#ifdef _WIN32
    mpExportToExeMenu->addAction(mpExportToExe_32Action);
    mpExportToExeMenu->addAction(mpExportToExe_64Action);
#elif __i386__
    mpExportToExeMenu->addAction(mpExportToExe_32Action);
#elif __x86_64__
    mpExportToExeMenu->addAction(mpExportToExe_64Action);
#endif

    mpExportToLabviewAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-ExportSIT.svg"), tr("Export to LabVIEW/SIT"), this);
    mHelpPopupTextMap.insert(mpExportToLabviewAction, "Export model to LabVIEW Veristand.");
    connect(mpExportToLabviewAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpLoadModelParametersFromSsvAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-LoadModelParameters.svg"), tr("Load Model Parameters from SSV"), this);
    mHelpPopupTextMap.insert(mpLoadModelParametersFromSsvAction, "Load model parameter set from SSV.");
    connect(mpLoadModelParametersFromSsvAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpLoadModelParametersFromHpfAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-LoadModelParameters.svg"), tr("Load Model Parameters from HPF"), this);
    mHelpPopupTextMap.insert(mpLoadModelParametersFromHpfAction, "Load model parameter set from HPF.");
    connect(mpLoadModelParametersFromHpfAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpImportModelParametersMenu = new QMenu("Import Model Parameteres");
    mpImportModelParametersMenu->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-LoadModelParameters.svg"));
    mpImportModelParametersMenu->addAction(mpLoadModelParametersFromSsvAction);
    mpImportModelParametersMenu->addAction(mpLoadModelParametersFromHpfAction);

    mpAboutAction = new QAction(this);
    mpAboutAction->setText("About");
    connect(mpAboutAction, SIGNAL(triggered()), mpAboutDialog, SLOT(open()));
    connect(mpAboutDialog->timer, SIGNAL(timeout()), mpAboutDialog, SLOT(update()));

    mpLicenseAction = new QAction(this);
    mpLicenseAction->setText("License");
    connect(mpLicenseAction, SIGNAL(triggered()), this, SLOT(openLicenseDialog()));

    mpIssueTrackerAction = new QAction(this);
    mpIssueTrackerAction->setText("Issue Tracker");
    connect(mpIssueTrackerAction, SIGNAL(triggered()), this, SLOT(openIssueTrackerDialog()));

    mpHelpAction = new QAction(this);
    mpHelpAction->setText("User Guide");
    connect(mpHelpAction, SIGNAL(triggered()), mpHelpDialog, SLOT(open()));

    mpReleaseNotesAction = new QAction(this);
    mpReleaseNotesAction->setText("Release Notes");
    connect(mpReleaseNotesAction, SIGNAL(triggered()), this, SLOT(showReleaseNotes()));

    mpNewVersionsAction = new QAction(this);
    mpNewVersionsAction->setText("Check For New Versions");
    connect(mpNewVersionsAction, SIGNAL(triggered()), this, SLOT(openArchiveURL()));
    mpWebsiteAction = new QAction(this);
    mpWebsiteAction->setText("Open Hopsan Website");
    connect(mpWebsiteAction, SIGNAL(triggered()), this, SLOT(openHopsanURL()));
    mpTutorialsAction = new QAction(this);
    mpTutorialsAction->setText("Open Tutorials Website");
    connect(mpTutorialsAction, &QAction::triggered, this, &MainWindow::openHopsanTutorialURL);

    //! @todo Check for new version could probably work in a better way...
    mpNewVersionsAction = new QAction(this);
    mpNewVersionsAction->setText("Check For New Versions");
    connect(mpNewVersionsAction, SIGNAL(triggered()), this, SLOT(openArchiveURL()));

    QIcon togglePortsIcon;
    togglePortsIcon.addFile(QString(ICONPATH) + "svg/Hopsan-TogglePorts.svg", QSize(), QIcon::Normal, QIcon::On);
    mpTogglePortsAction = new QAction(togglePortsIcon, tr("&Show Unconnected Ports (Ctrl+T)"), this);
    gpTogglePortsAction = mpTogglePortsAction;
    mpTogglePortsAction->setText("Show Unconnected Ports (Ctrl+T)");
    mpTogglePortsAction->setCheckable(true);
    mpTogglePortsAction->setChecked(gpConfig->getBoolSetting(cfg::toggleportsbuttonchecked));
    mpTogglePortsAction->setShortcut(QKeySequence("Ctrl+t"));
    connect(mpTogglePortsAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpTogglePortsAction, "Toggle visibility of unconnected ports.");

    QIcon toggleSignalsIcon;
    toggleSignalsIcon.addFile(QString(ICONPATH) + "svg/Hopsan-ToggleSignal.svg", QSize(), QIcon::Normal, QIcon::On);
    mpToggleSignalsAction = new QAction(toggleSignalsIcon, tr("&Show Signal Components"), this);
    mpToggleSignalsAction->setText("Show Signal Components");
    mpToggleSignalsAction->setCheckable(true);
    mpToggleSignalsAction->setChecked(true);      //! @todo Shall depend on gpConfig setting
    connect(mpToggleSignalsAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpToggleSignalsAction, "Toggle signal components visibility.");

    mpDebug1Action = new QAction(this);
    mpDebug1Action->setCheckable(true);
    mpDebug1Action->setShortcut(QKeySequence("Ctrl+D+1"));
    this->addAction(mpDebug1Action);

    mpDebug2Action = new QAction(this);
    mpDebug2Action->setShortcut(QKeySequence("Ctrl+D+2"));
    this->addAction(mpDebug2Action);
    //connect(mpDebug2Action, SIGNAL(triggered()), mpModelHandler, SLOT(simulateAllOpenModelsWithoutSplit()));

    mpShowLossesAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-Losses.svg"), tr("Calculate Losses"), this);
    mpShowLossesAction->setShortcut(QKeySequence("Ctrl+L"));
    mpShowLossesAction->setCheckable(true);
    this->addAction(mpShowLossesAction);
    connect(mpShowLossesAction, SIGNAL(triggered(bool)), mpModelHandler, SLOT(showLosses(bool)));
    connect(mpShowLossesAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpShowLossesAction, "Show energy or power losses from last simulation.");

    mpToggleHideAllDockAreasAction = new QAction(QIcon(QString(ICONPATH) + "svg/Hopsan-ShowHideDockAreas.svg"), tr("Hide/Show dock areas"), this);
    mpToggleHideAllDockAreasAction->setCheckable(true);
    mpToggleHideAllDockAreasAction->setChecked(true);
    connect(mpToggleHideAllDockAreasAction, SIGNAL(toggled(bool)), this, SLOT(toggleHideShowDockAreas(bool)));

    mpSimulationTimeEdit = new SimulationTimeEdit(this);
    connect(mpSimulationTimeEdit, SIGNAL(mouseEnterEvent()), this, SLOT(showToolBarHelpPopup()));
}


//! @brief Creates the menus
void MainWindow::createMenus()
{
    // Create the menubar
    mpMenuBar = new QMenuBar();
    mpMenuBar->setGeometry(QRect(0,0,800,25));

    // Create the main menus
    mpFileMenu = new QMenu(mpMenuBar);
    mpFileMenu->setTitle(tr("&File"));

    mpRecentMenu = new QMenu(this);
    mpRecentMenu->setTitle(tr("&Recent Models"));

    mpNewMenu = new QMenu(mpMenuBar);
    mpNewMenu->setTitle(tr("&New"));

    mpSimulationMenu = new QMenu(mpMenuBar);
    mpSimulationMenu->setTitle(tr("&Simulation"));

    mpEditMenu = new QMenu(mpMenuBar);
    mpEditMenu->setTitle(tr("&Edit"));

    mpViewMenu = new QMenu(mpMenuBar);
    mpViewMenu->setTitle(tr("&View"));

    mpToolsMenu = new QMenu(mpMenuBar);
    mpToolsMenu->setTitle(tr("&Tools"));

    mpImportMenu = new QMenu(mpMenuBar);
    mpImportMenu->setTitle(tr("&Import"));

    mpExportMenu = new QMenu(mpMenuBar);
    mpExportMenu->setTitle(tr("&Export"));

    mpHelpMenu = new QMenu(mpMenuBar);
    mpHelpMenu->setTitle(tr("&Help"));

    this->setMenuBar(mpMenuBar);

    // Create sub menus for the help menu
    mpExamplesMenu = new QMenu("Example Models");
    QDir exampleModelsDir(gpDesktopHandler->getMainPath()+"Models/Example Models/");
    buildModelActionsMenu(mpExamplesMenu, exampleModelsDir);

    mpTestModelsMenu = new QMenu("Test Models");
    QDir testModelsDir(gpDesktopHandler->getMainPath()+"Models/Component Test/");
    buildModelActionsMenu(mpTestModelsMenu, testModelsDir);

    // Add the action buttons to the menus
    mpNewAction->setText("Project");
    mpNewMenu->addAction(mpNewAction);

    mpFileMenu->addAction(mpNewMenu->menuAction());
    mpFileMenu->addAction(mpOpenAction);
    mpFileMenu->addAction(mpSaveAction);
    mpFileMenu->addAction(mpSaveAsAction);
    mpFileMenu->addMenu(mpRecentMenu);
    mpFileMenu->addSeparator();
    mpFileMenu->addAction(mpNewScriptAction);
    mpFileMenu->addAction(mpOpenTextFileAction);
    mpFileMenu->addAction(mpSaveAndRunAction);
    mpFileMenu->addAction(mpPrintAction);
    mpFileMenu->addSeparator();
    mpFileMenu->addAction(mpLoadLibsAction);
    mpFileMenu->addSeparator();
    mpFileMenu->addAction(mpPropertiesAction);
    mpFileMenu->addAction(mpOpenSystemParametersAction);
    mpFileMenu->addSeparator();
    mpFileMenu->addAction(mpRevertModelAction);
    mpFileMenu->addSeparator();
    mpFileMenu->addAction(mpCloseAction);

    mpSimulationMenu->addAction(mpSimulateAction);
    mpSimulationMenu->addAction(mpToggleRemoteCoreSimAction);
    mpSimulationMenu->addAction(mpOpenDebuggerAction);
    mpSimulationMenu->addAction(mpAnimateAction);
    mpSimulationMenu->addAction(mpMeasureSimulationTimeAction);
    mpSimulationMenu->addAction(mpOptimizeAction);
    mpSimulationMenu->addAction(mpSensitivityAnalysisAction);
    mpSimulationMenu->addAction(mpPlotAction);
    mpSimulationMenu->addAction(mpShowLossesAction);

    mpEditMenu->addAction(mpUndoAction);
    mpEditMenu->addAction(mpRedoAction);
    mpEditMenu->addAction(mpOpenUndoAction);
    mpEditMenu->addAction(mpEnableUndoAction);
    mpEditMenu->addSeparator();
    mpEditMenu->addAction(mpCopyAction);
    mpEditMenu->addAction(mpCutAction);
    mpEditMenu->addAction(mpPasteAction);

    //The View menu shall be alphabetically sorted!
    mpViewMenu->addAction(mpToggleNamesAction);
    mpViewMenu->addAction(mpTogglePortsAction);
    mpViewMenu->addAction(mpToggleSignalsAction);
    mpViewMenu->addSeparator();
    mpViewMenu->addAction(mpLibDock->toggleViewAction());
    mpViewMenu->addAction(mpEditToolBar->toggleViewAction());
    mpViewMenu->addAction(mpFileToolBar->toggleViewAction());
    mpViewMenu->addAction(mpTerminalDock->toggleViewAction());
    mpViewMenu->addAction(mpConnectivityToolBar->toggleViewAction());
    mpViewMenu->addAction(mpMessageDock->toggleViewAction());
    mpViewMenu->addAction(mpSimToolBar->toggleViewAction());

    mpToolsMenu->addAction(mpOptionsAction);
    mpToolsMenu->addAction(mpOpenSystemParametersAction);
    mpToolsMenu->addAction(mpOpenHvcWidgetAction);
    mpToolsMenu->addAction(mpOpenDataExplorerAction);
    mpToolsMenu->addAction(mpOpenFindWidgetAction);
    mpToolsMenu->addAction(mpNumHopAction);

    mpImportMenu->addAction(mpImportDataFileAction);
    mpImportMenu->addAction(mpLoadModelParametersFromHpfAction);
    mpImportMenu->addAction(mpLoadModelParametersFromSsvAction);
    mpImportMenu->addSeparator();
    mpImportMenu->addAction(mpImportFMUAction);

    mpExportMenu->addAction(mpExportSimulationStateAction);
    mpExportMenu->addMenu(mpExportModelParametersMenu);
    mpExportMenu->addSeparator();
    mpExportMenu->addAction(mpExportToSimulinkAction);
    mpExportMenu->addAction(mpExportToLabviewAction);
    mpExportMenu->addMenu(mpExportToFMUMenu);
    mpExportMenu->addMenu(mpExportToExeMenu);

    mpExportMenu->addSeparator();
    mpExportMenu->addAction(mpExportPDFAction);
    mpExportMenu->addAction(mpExportPNGAction);

    mpHelpMenu->addAction(mpHelpAction);
    mpHelpMenu->addAction(mpReleaseNotesAction);
    mpHelpMenu->addMenu(mpExamplesMenu);
    mpHelpMenu->addMenu(mpTestModelsMenu);
    mpHelpMenu->addAction(mpIssueTrackerAction);
    mpHelpMenu->addAction(mpWebsiteAction);
    mpHelpMenu->addAction(mpTutorialsAction);
    mpHelpMenu->addAction(mpNewVersionsAction);
    mpHelpMenu->addAction(mpLicenseAction);
    mpHelpMenu->addAction(mpAboutAction);

    mpMenuBar->addAction(mpFileMenu->menuAction());
    mpMenuBar->addAction(mpEditMenu->menuAction());
    mpMenuBar->addAction(mpToolsMenu->menuAction());
    mpMenuBar->addAction(mpSimulationMenu->menuAction());
    mpMenuBar->addAction(mpImportMenu->menuAction());
    mpMenuBar->addAction(mpExportMenu->menuAction());
    mpMenuBar->addAction(mpViewMenu->menuAction());
    mpMenuBar->addAction(mpHelpMenu->menuAction());
}

//! @brief Creates the toolbars
void MainWindow::createToolbars()
{
    //File toolbar, contains all file handling stuff (open, save etc)
    mpFileToolBar = addToolBar(tr("File Toolbar"));
    mpFileToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::LeftToolBarArea | Qt::RightToolBarArea);
    mpFileToolBar->setAttribute(Qt::WA_MouseTracking);
    mpFileToolBar->addAction(mpNewAction);
    mpFileToolBar->addAction(mpOpenAction);
    mpFileToolBar->addAction(mpSaveAction);
    mpFileToolBar->addAction(mpSaveAsAction);
    mpFileToolBar->addAction(mpNewScriptAction);
    mpFileToolBar->addAction(mpOpenTextFileAction);
    mpFileToolBar->addAction(mpSaveAndRunAction);
    mpFileToolBar->addAction(mpPrintAction);

    mpConnectivityToolBar = addToolBar(tr("Import/Export Toolbar)"));
    mpConnectivityToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::LeftToolBarArea | Qt::RightToolBarArea);
    mpConnectivityToolBar->setAttribute(Qt::WA_MouseTracking);
    mpExportModelParametersMenuButton = new QToolButton(this);
    mpExportModelParametersMenuButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-ExportParameters.svg"));
    mpExportModelParametersMenuButton->setPopupMode(QToolButton::InstantPopup);
    mpExportModelParametersMenuButton->setMouseTracking(true);
    mpExportModelParametersMenuButton->setMenu(mpExportModelParametersMenu);
    mpConnectivityToolBar->addWidget(mpExportModelParametersMenuButton);
    mpImportModelParametersMenuButton = new QToolButton(this);
    mpImportModelParametersMenuButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-LoadModelParameters.svg"));
    mpImportModelParametersMenuButton->setPopupMode(QToolButton::InstantPopup);
    mpImportModelParametersMenuButton->setMouseTracking(true);
    mpImportModelParametersMenuButton->setMenu(mpImportModelParametersMenu);
    mpConnectivityToolBar->addWidget(mpImportModelParametersMenuButton);
    mpConnectivityToolBar->addSeparator();
    mpConnectivityToolBar->addAction(mpExportPDFAction);
    mpConnectivityToolBar->addAction(mpExportPNGAction);
    mpConnectivityToolBar->addSeparator();
    mpConnectivityToolBar->addAction(mpExportToSimulinkAction);
    mpConnectivityToolBar->addAction(mpExportToLabviewAction);
    mpExportToFMUMenuButton = new QToolButton(this);
    mpExportToFMUMenuButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-ExportFmu.svg"));
    mpExportToFMUMenuButton->setPopupMode(QToolButton::InstantPopup);
    mpExportToFMUMenuButton->setMouseTracking(true);
    mpExportToFMUMenuButton->setMenu(mpExportToFMUMenu);
    mpConnectivityToolBar->addWidget(mpExportToFMUMenuButton);
    mpExportToExeMenuButton = new QToolButton(this);
    mpExportToExeMenuButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-ExportExe.svg"));
    mpExportToExeMenuButton->setPopupMode(QToolButton::InstantPopup);
    mpExportToExeMenuButton->setMouseTracking(true);
    mpExportToExeMenuButton->setMenu(mpExportToExeMenu);
    mpConnectivityToolBar->addWidget(mpExportToExeMenuButton);
    //mpConnectivityToolBar->addAction(mpExportToFMUAction);
    mpConnectivityToolBar->addAction(mpImportFMUAction);


    //! @note Action and menu shouldn't be here, but it doesn't work otherwise because the menus are created after the toolbars
    //mpImportMenu = new QMenu("Import");
    //mpImportMenu->addAction(mpImportFMUAction);
//    mpImportButton = new QToolButton(mpFileToolBar);
//    mpImportButton->setToolTip("Import");
//    mpImportButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-Import.svg"));
//    mpImportButton->setMenu(mpImportMenu);
//    mpImportButton->setPopupMode(QToolButton::InstantPopup);
    //mpFileToolBar->addWidget(mpImportButton);

//    mpExportMenu = new QMenu("Export Model");
//    mpExportMenu->addAction(mpExportToSimulinkAction);
//    mpExportMenu->addAction(mpExportToFMUAction);
//    mpExportButton = new QToolButton(mpFileToolBar);
//    mpExportButton->setToolTip("Export");
//    mpExportButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-Export.svg"));
//    mpExportButton->setMenu(mpExportMenu);
//    mpExportButton->setPopupMode(QToolButton::InstantPopup);
//    mpFileToolBar->addWidget(mpExportButton);

    //Simulation toolbar, contains tools for simulation, plotting and model preferences
    mpSimToolBar = addToolBar(tr("Simulation Toolbar"));
    mpSimToolBar->setAllowedAreas(Qt::TopToolBarArea);
    mpSimToolBar->setAttribute(Qt::WA_MouseTracking);

    mpSimToolBar->addWidget(mpSimulationTimeEdit);
    mpSimToolBar->addAction(mpSimulateAction);
    mpSimToolBar->addAction(mpToggleRemoteCoreSimAction);
    mpSimToolBar->addAction(mpOpenDebuggerAction);
    mpSimToolBar->addAction(mpOptimizeAction);
    mpSimToolBar->addAction(mpSensitivityAnalysisAction);
    mpSimToolBar->addAction(mpPlotAction);
    mpSimToolBar->addAction(mpShowLossesAction);
    mpSimToolBar->addAction(mpAnimateAction);
    mpSimToolBar->addAction(mpMeasureSimulationTimeAction);
    mpSimToolBar->addAction(mpPropertiesAction);
    mpSimToolBar->addAction(mpOpenSystemParametersAction);

    //addToolBarBreak(Qt::TopToolBarArea);

    //Edit toolbar, contains clipboard operations, undo/redo and global options
    mpEditToolBar = new QToolBar(tr("Edit Toolbar"));
    addToolBar(Qt::LeftToolBarArea, mpEditToolBar);
    mpEditToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::LeftToolBarArea | Qt::RightToolBarArea);
    mpEditToolBar->setAttribute(Qt::WA_MouseTracking);
    mpEditToolBar->addAction(mpCutAction);
    mpEditToolBar->addAction(mpCopyAction);
    mpEditToolBar->addAction(mpPasteAction);
    mpEditToolBar->addAction(mpUndoAction);
    mpEditToolBar->addAction(mpRedoAction);
    mpEditToolBar->addAction(mpOptionsAction);

    //View toolbar, contains all cosmetic and zooming tools
    mpViewToolBar = new QToolBar(tr("View Toolbar"));
    addToolBar(Qt::LeftToolBarArea, mpViewToolBar);
    mpViewToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::LeftToolBarArea | Qt::RightToolBarArea);
    mpViewToolBar->setAttribute(Qt::WA_MouseTracking);
    mpViewToolBar->addAction(mpCenterViewAction);
    mpViewToolBar->addAction(mpResetZoomAction);
    mpViewToolBar->addAction(mpZoomInAction);
    mpViewToolBar->addAction(mpZoomOutAction);
    mpViewToolBar->addAction(mpToggleNamesAction);
    mpViewToolBar->addAction(mpTogglePortsAction);
    mpViewToolBar->addAction(mpToggleSignalsAction);
    mpViewToolBar->addAction(mpToggleHideAllDockAreasAction);

    mpDcpToolBar = new QToolBar(tr("DCP Toolbar"));
    addToolBar(Qt::TopToolBarArea, mpDcpToolBar);
    mpDcpToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::LeftToolBarArea | Qt::RightToolBarArea);
    mpDcpToolBar->setAttribute(Qt::WA_MouseTracking);
    mpDcpToolBar->addAction(mpNewDcpModelAction);
    mpDcpToolBar->addAction(mpStartDcpMasterAction);
    mpDcpToolBar->addAction(mpStartDcpServerAction);

    //Tools toolbar, contains all tools used to modify the model
    mpToolsToolBar = new QToolBar(tr("Tools Toolbar"));
    addToolBar(Qt::LeftToolBarArea, mpToolsToolBar);
    mpToolsToolBar->setAttribute(Qt::WA_MouseTracking);
    mpToolsToolBar->addAction(mpAlignXAction);
    mpToolsToolBar->addAction(mpAlignYAction);
    mpToolsToolBar->addAction(mpDistributeXAction);
    mpToolsToolBar->addAction(mpDistributeYAction);
    mpToolsToolBar->addAction(mpRotateRightAction);
    mpToolsToolBar->addAction(mpRotateLeftAction);
    mpToolsToolBar->addAction(mpFlipHorizontalAction);
    mpToolsToolBar->addAction(mpFlipVerticalAction);

    connect(mpImportFMUAction,                  SIGNAL(triggered()), gpLibraryHandler,  SLOT(importFmu()));
    connect(mpExportToSimulinkAction,           SIGNAL(triggered()), mpModelHandler,    SLOT(exportCurrentModelToSimulink()));
    connect(mpExportToFMU1_32Action,            SIGNAL(triggered()), mpModelHandler,    SLOT(exportCurrentModelToFMU1_32()));
    connect(mpExportToFMU1_64Action,            SIGNAL(triggered()), mpModelHandler,    SLOT(exportCurrentModelToFMU1_64()));
    connect(mpExportToFMU2_32Action,            SIGNAL(triggered()), mpModelHandler,    SLOT(exportCurrentModelToFMU2_32()));
    connect(mpExportToFMU2_64Action,            SIGNAL(triggered()), mpModelHandler,    SLOT(exportCurrentModelToFMU2_64()));
    connect(mpExportToFMU3_32Action,            SIGNAL(triggered()), mpModelHandler,    SLOT(exportCurrentModelToFMU3_32()));
    connect(mpExportToFMU3_64Action,            SIGNAL(triggered()), mpModelHandler,    SLOT(exportCurrentModelToFMU3_64()));
    connect(mpExportToLabviewAction,            SIGNAL(triggered()), mpModelHandler,    SLOT(createLabviewWrapperFromCurrentModel()));
    connect(mpExportToExe_32Action,             SIGNAL(triggered()), mpModelHandler,    SLOT(exportCurrentModelToExe_32()));
    connect(mpExportToExe_64Action,             SIGNAL(triggered()), mpModelHandler,    SLOT(exportCurrentModelToExe_64()));
    connect(mpLoadModelParametersFromHpfAction, SIGNAL(triggered()), mpModelHandler,    SLOT(loadModelParametersFromHpf()));
    connect(mpLoadModelParametersFromSsvAction, SIGNAL(triggered()), mpModelHandler,    SLOT(loadModelParametersFromSsv()));
}

void MainWindow::buildModelActionsMenu(QMenu *pParentMenu, QDir dir)
{
    QFileInfoList entrys = dir.entryInfoList(QStringList("*.hmf"), QDir::Files | QDir::NoDotAndDotDot | QDir::Readable | QDir::AllDirs);
    for (int i=0; i<entrys.size(); ++i)
    {
        //qDebug() << entrys[i].absolutePath() << " " << entrys[i].baseName();
        if (entrys[i].isDir())
        {
            QDir newDir(entrys[i].absoluteFilePath());
            QMenu *pMenu = new QMenu(newDir.dirName());
            pParentMenu->addMenu(pMenu);
            buildModelActionsMenu(pMenu, newDir);
        }
        else
        {
            QAction *pTempAction = new QAction(entrys[i].fileName(), this);
            pTempAction->setIcon(QIcon(QString(ICONPATH) + "hmf.ico"));
            pTempAction->setData(entrys[i].absoluteFilePath());
            pParentMenu->addAction(pTempAction);
            connect(pTempAction, SIGNAL(triggered()), this, SLOT(openModelByAction()));
        }
    }
}


//! @brief Opens the undo widget.
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


//! @brief Opens the SystemParametersWidget widget.
void MainWindow::openSystemParametersWidget()
{
    if(!mpSystemParametersDock->isVisible())
    {
        mpSystemParametersDock->setWidget(mpSystemParametersWidget);
        mpSystemParametersDock->show();
        mpSystemParametersDock->raise();
        connect(mpSystemParametersDock, SIGNAL(visibilityChanged(bool)), this, SLOT(updateSystemParametersActionButton(bool)));
    }
    else
    {
        mpSystemParametersDock->hide();
    }
}


//! @brief Opens a recent model
void MainWindow::openRecentModel()
{
    QAction *action = qobject_cast<QAction *>(sender());
    qDebug() << "Trying to open " << action->text();
    if (action)
    {
        mpModelHandler->loadModel(action->text());
    }
}


void MainWindow::openHopsanURL()
{
    QDesktopServices::openUrl(QUrl(hopsanweblinks::homepage));
}

void MainWindow::openHopsanTutorialURL()
{
    QDesktopServices::openUrl(QUrl(hopsanweblinks::tutorials));
}


void MainWindow::openArchiveURL()
{
    QDesktopServices::openUrl(QUrl(hopsanweblinks::releases_archive));
}

void MainWindow::openIssueTrackerDialog()
{
    QMessageBox msgBox(this);
//    QSizePolicy pol;
//    pol.setHorizontalPolicy(QSizePolicy::Expanding);
//    msgBox.setSizePolicy(pol);
    QString msg = QString("To browse or read about currently known issues you should see the Hopsan project issue tracker. You can also support development by reporting new issues or add feature requests.\n\n")+
                  QString("See the Hopsan web page for more information about bug reporting and issue tracking.");

    msgBox.setWindowTitle("Hopsan Issue Tracking");
    msgBox.setText(msg);
    QPushButton *pToWeb = msgBox.addButton("To Hopsan webpage", QMessageBox::YesRole);
    msgBox.addButton(QMessageBox::Close);
    msgBox.exec();
    if (msgBox.clickedButton() == pToWeb)
    {
        this->openHopsanURL();
    }
}


//! @brief Changes the checked setting of plot widget button when plot widget is opened or closed
void MainWindow::updatePlotActionButton(bool)
{
    mpPlotAction->setChecked(mpPlotWidgetDock->isVisible() || !tabifiedDockWidgets(mpPlotWidgetDock).isEmpty());
}


//! @brief Changes the checked setting of system parameters button when the system parameter widget is opened or closed
void MainWindow::updateSystemParametersActionButton(bool)
{
    mpOpenSystemParametersAction->setChecked(mpSystemParametersDock->isVisible() || !tabifiedDockWidgets(mpSystemParametersDock).isEmpty());
}



//! @brief Slot that loads an example model, based on the name of the calling action
void MainWindow::openModelByAction()
{
    QAction *pAction = qobject_cast<QAction *>(sender());
    if (pAction)
    {
        QString modelPath = pAction->data().toString();
        qDebug() << "Trying to open " << modelPath;
        mpModelHandler->loadModel(modelPath);
    }
}


//! @brief Shows help popup for the toolbar icon that is currently hovered by the mouse pointer
void MainWindow::showToolBarHelpPopup()
{
    QObject* senderObject = sender();
    // See if action exists in map, or if a simulation time widget is hovered
    if(mHelpPopupTextMap.contains(senderObject))
    {
        gpHelpPopupWidget->showHelpPopupMessage(mHelpPopupTextMap.find(senderObject).value());
    }
    else if (mpSimulationTimeEdit->underMouse())
    {
        gpHelpPopupWidget->showHelpPopupMessage("Set simulation time (in seconds).");
    }
}


void MainWindow::showReleaseNotes()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(gpDesktopHandler->getExecPath()+"../Hopsan-release-notes.txt"));
}

void MainWindow::toggleHideShowDockAreas(bool show)
{
    // Show default set of docks
    //! @todo would be nice if we would remember those opened and reshow them, but there will be odd behavior if you hide and then open some manually and close them again, then triggering show will reshow them, but maybe not a big deal
    if (show)
    {
        mpLibDock->show();
        mpTerminalDock->show();
        mpMessageDock->show();
        //mpPlotWidgetDock->show();
        //mpSystemParametersDock->hide();
    }
    else
    {
        mpLibDock->hide();
        mpTerminalDock->hide();
        mpMessageDock->hide();
        mpPlotWidgetDock->hide();
        mpSystemParametersDock->hide();
    }
}

void MainWindow::openLicenseDialog()
{
    (new LicenseDialog(gpMainWindowWidget))->show();
    // Note! It will deleate on close
}


//! @brief Updates the toolbar values that are tab specific when a new tab is activated
void MainWindow::updateToolBarsToNewTab()
{
    bool modelOpen = mpModelHandler->count() > 0;

    ModelWidget *pModel = qobject_cast<ModelWidget*>(mpCentralTabs->currentWidget());
    bool modelTab = modelOpen && pModel;
    bool dcpTab = modelTab && (pModel->getModelType() == ModelWidget::DcpModel);
    bool logData = modelTab && pModel->getViewContainerObject()->getLogDataHandler();

    TextEditorWidget *pEditor = qobject_cast<TextEditorWidget*>(mpCentralTabs->currentWidget());
    bool editorTab = (pEditor != nullptr);
    bool hcomTab = editorTab && (pEditor->getFileInfo().suffix() == "hcom");

    if(modelTab)
    {
        mpTogglePortsAction->setChecked(pModel->getTopLevelSystemContainer()->areSubComponentPortsShown());
    }

    mpShowLossesAction->setEnabled(logData && !dcpTab);
    mpAnimateAction->setEnabled(modelTab && !dcpTab);
    mpSaveAction->setEnabled(modelTab || editorTab);
    mpExportToFMUMenuButton->setEnabled(modelTab && !dcpTab);
    mpExportToExeMenuButton->setEnabled(modelTab && !dcpTab);
    mpExportToExeMenu->setEnabled(modelTab && !dcpTab);
    mpSaveAsAction->setEnabled(modelTab || editorTab);
    mpSaveAndRunAction->setEnabled(hcomTab);
    mpExportSimulationStateAction->setEnabled(modelTab && !dcpTab);
    mpExportModelParametersMenu->setEnabled(modelTab);
    mpExportModelParametersMenuButton->setEnabled(modelTab);
    mpExportModelParametersActionToSsv->setEnabled(modelTab);
    mpExportModelParametersActionToHpf->setEnabled(modelTab);
    mpCutAction->setEnabled(modelTab || editorTab);
    mpCopyAction->setEnabled(modelTab || editorTab);
    mpPasteAction->setEnabled(modelTab || editorTab);
    mpUndoAction->setEnabled(modelTab || editorTab);
    mpRedoAction->setEnabled(modelTab || editorTab);
    mpCenterViewAction->setEnabled(modelTab);
    mpResetZoomAction->setEnabled(modelTab);
    mpZoomInAction->setEnabled(modelTab || editorTab);
    mpZoomOutAction->setEnabled(modelTab || editorTab);
    mpToggleNamesAction->setEnabled(modelTab);
    mpToggleSignalsAction->setEnabled(modelTab);
    mpTogglePortsAction->setEnabled(modelTab);
    mpTogglePortsAction->setEnabled(modelTab);
    mpPrintAction->setEnabled(modelTab || editorTab);
    mpExportPDFAction->setEnabled(modelTab);
    mpExportPNGAction->setEnabled(modelTab);
    mpAlignXAction->setEnabled(modelTab);
    mpAlignYAction->setEnabled(modelTab);
    mpDistributeXAction->setEnabled(modelTab);
    mpDistributeYAction->setEnabled(modelTab);
    mpRotateLeftAction->setEnabled(modelTab);
    mpRotateRightAction->setEnabled(modelTab);
    mpFlipHorizontalAction->setEnabled(modelTab);
    mpFlipVerticalAction->setEnabled(modelTab);
    mpSimulationTimeEdit->setEnabled(modelTab);
    mpSimulateAction->setEnabled(modelTab && !dcpTab);
    mpToggleRemoteCoreSimAction->setEnabled(modelTab && !dcpTab);
    mpOpenDebuggerAction->setEnabled(modelTab && !dcpTab);
    mpOptimizeAction->setEnabled(modelTab && !dcpTab);
    mpSensitivityAnalysisAction->setEnabled(modelTab && !dcpTab);
    mpMeasureSimulationTimeAction->setEnabled(modelTab && !dcpTab);
    mpPlotAction->setEnabled(logData && !dcpTab);
    mpPropertiesAction->setEnabled(modelTab);
    mpOpenSystemParametersAction->setEnabled(modelTab);
    mpExportToFMU1_32Action->setEnabled(modelTab && !dcpTab);
    mpExportToFMU2_32Action->setEnabled(modelTab && !dcpTab);
    mpExportToLabviewAction->setEnabled(modelTab && !dcpTab);
    mpExportToSimulinkAction->setEnabled(modelTab && !dcpTab);
    mpImportFMUAction->setEnabled(modelTab && !dcpTab);
    mpImportModelParametersMenuButton->setEnabled(modelTab && !dcpTab);
    mpLoadModelParametersFromHpfAction->setEnabled(modelTab && !dcpTab);
    mpLoadModelParametersFromSsvAction->setEnabled(modelTab && !dcpTab);
    mpStartDcpServerAction->setEnabled(modelTab && !dcpTab);

    mpStartDcpMasterAction->setEnabled(dcpTab);

    if(gpFindWidget) {
        gpFindWidget->setEnabled(modelTab || editorTab);
    }
}


//! @brief Slot that calls refresh list function in undo widget. Used because undo widget cannot have slots.
void MainWindow::refreshUndoWidgetList()
{
    mpUndoWidget->refreshList();
}


//! @brief Updates the "Recent Models" list
void MainWindow::updateRecentList()
{
    mpRecentMenu->clear();

    mpRecentMenu->setEnabled(!gpConfig->getRecentModels().empty());
    if(!gpConfig->getRecentModels().empty())
    {
        for(int i=0; i<gpConfig->getRecentModels().size(); ++i)
        {
            if(gpConfig->getRecentModels().at(i) != "")
            {
                QAction *tempAction;
                tempAction = mpRecentMenu->addAction(gpConfig->getRecentModels().at(i));
                tempAction->setIcon(QIcon(QString(ICONPATH) + "hmf.ico"));
                disconnect(mpRecentMenu, SIGNAL(triggered(QAction *)), mpModelHandler, SLOT(loadModel(QAction *)));    //Ugly hack to make sure connections are not made twice (then program would try to open model more than once...)
                connect(tempAction, SIGNAL(triggered()), this, SLOT(openRecentModel()));
            }
        }
    }

    mpWelcomeWidget->updateRecentList();
}


void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    gpHelpPopupWidget->hide();
    QMainWindow::mouseMoveEvent(event);
}


void MainWindow::keyPressEvent(QKeyEvent *event)
{
    //qDebug() << "Mainwindow caught keypress: " << event->key();
    QMainWindow::keyPressEvent(event);
}


OptionsDialog *MainWindow::getOptionsDialog()
{
    return mpOptionsDialog;
}


//! @brief Access function to the starttimelabel value.
//! @returns the starttime value
double MainWindow::getStartTimeFromToolBar()
{
    return mpSimulationTimeEdit->getStartTime().toDouble();
}


//! @brief Access function to the timesteplabel value.
//! @returns the timestep value
double MainWindow::getTimeStepFromToolBar()
{
    return mpSimulationTimeEdit->getTimeStep().toDouble();
}


//! @brief Access function to the finishlabel value.
//! @returns the finish value
double MainWindow::getFinishTimeFromToolBar()
{
    return mpSimulationTimeEdit->getStopTime().toDouble();
}

void MainWindow::simulateKeyWasPressed()
{
    emit simulateKeyPressed();
}

void MainWindow::openHVCWidget()
{
    mpHVCWidget->setVisible(true);
}

void MainWindow::openDataExplorerWidget()
{
    mpDataExplorer->setVisible(true);
}

void MainWindow::openFindWidget()
{
    gpFindWidget->setVisible(!gpFindWidget->isVisible());
}

void MainWindow::revertModel()
{
    gpModelHandler->revertCurrentModel();
}

void MainWindow::openNumHopDialog()
{
    SystemObject *pContainer = gpModelHandler->getCurrentViewContainerObject();
    if (pContainer)
    {
        NumHopScriptDialog *pDialog = new NumHopScriptDialog(pContainer, gpMainWindowWidget);
        pDialog->show();
        //Note! Dialog will auto destruct when closed
    }
}


SimulationTimeEdit::SimulationTimeEdit(QWidget *pParent) :
    QWidget(pParent)
{
    mpStartTimeLineEdit = new QLineEdit("0.0", this);
    mpStartTimeLineEdit->setValidator(new QDoubleValidator(MINSTARTTIME, MAXSTARTTIME, 1000));
    mpStartTimeLineEdit->setToolTip("Set start time for simulation");
    mpStartTimeLineEdit->setMaximumWidth(70);
    mpStartTimeLineEdit->setAlignment(Qt::AlignVCenter | Qt::AlignCenter);

    mpTimeStepLineEdit = new QLineEdit("0.001", this);
    mpTimeStepLineEdit->setValidator(new QDoubleValidator(MINTIMESTEP, MAXTIMESTEP, 1000));
    mpTimeStepLineEdit->setToolTip("Set step time for simulation");
    mpTimeStepLineEdit->setMaximumWidth(70);
    mpTimeStepLineEdit->setAlignment(Qt::AlignVCenter | Qt::AlignCenter);

    mpStopTimeLineEdit = new QLineEdit("10.0", this);
    mpStopTimeLineEdit->setValidator(new QDoubleValidator(MINSTOPTIME, MAXSTOPTIME, 1000));
    mpStopTimeLineEdit->setToolTip("Set stop time for simulation");
    mpStopTimeLineEdit->setMaximumWidth(70);
    mpStopTimeLineEdit->setAlignment(Qt::AlignVCenter | Qt::AlignCenter);

    QHBoxLayout *pLayout = new QHBoxLayout(this);
    pLayout->addWidget(mpStartTimeLineEdit);
    pLayout->addWidget(new QLabel(" :: "));
    pLayout->addWidget(mpTimeStepLineEdit);
    pLayout->addWidget(new QLabel(" :: "));
    pLayout->addWidget(mpStopTimeLineEdit);

    setSizePolicy(QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed));

    connect(mpStartTimeLineEdit, SIGNAL(editingFinished()), this, SLOT(emitSimTime()));
    connect(mpTimeStepLineEdit, SIGNAL(editingFinished()), this, SLOT(emitSimTime()));
    connect(mpStopTimeLineEdit, SIGNAL(editingFinished()), this, SLOT(emitSimTime()));

    installEventFilter(this);
}

void SimulationTimeEdit::getSimulationTime(QString &rStartTime, QString &rTimeStep, QString &rStopTime) const
{
    rStartTime = mpStartTimeLineEdit->text();
    rTimeStep = mpTimeStepLineEdit->text();
    rStopTime = mpStopTimeLineEdit->text();
}

QString SimulationTimeEdit::getStartTime() const
{
    return mpStartTimeLineEdit->text();
}

QString SimulationTimeEdit::getTimeStep() const
{
    return mpTimeStepLineEdit->text();
}

QString SimulationTimeEdit::getStopTime() const
{
    return mpStopTimeLineEdit->text();
}

void SimulationTimeEdit::displaySimulationTime(const QString startTime, const QString timeStep, const QString stopTime)
{
    displayStartTime(startTime);
    displayTimeStep(timeStep);
    dispalyStopTime(stopTime);
}

bool SimulationTimeEdit::eventFilter(QObject *object, QEvent *event)
{
    if( object==this && (event->type()==QEvent::Enter) )
    {
        emit mouseEnterEvent();
    }
    return QWidget::eventFilter(object, event);
}

void SimulationTimeEdit::displayStartTime(const QString startTime)
{
    mpStartTimeLineEdit->setText(startTime);
}

void SimulationTimeEdit::displayTimeStep(const QString timeStep)
{
    mpTimeStepLineEdit->setText(timeStep);
}

void SimulationTimeEdit::dispalyStopTime(const QString stopTime)
{
    mpStopTimeLineEdit->setText(stopTime);
}

void SimulationTimeEdit::emitSimTime()
{
    emit simulationTimeChanged(mpStartTimeLineEdit->text(), mpTimeStepLineEdit->text(), mpStopTimeLineEdit->text());
}
