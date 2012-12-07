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
//! @file   MainWindow.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the HopsanGUI MainWindow class
//!
//$Id$

#include <QDebug>
#include <QFontDatabase>
#include <QtNetwork/QNetworkReply>
#include "MainWindow.h"
#include "version_gui.h"
#include "common.h"

#include "Widgets/PlotWidget.h"
#include "Widgets/MessageWidget.h"
#include "Widgets/HcomWidget.h"
#include "Widgets/LibraryWidget.h"
#include "Widgets/ProjectTabWidget.h"
#include "Widgets/PyDockWidget.h"
#include "Widgets/SystemParametersWidget.h"
#include "Widgets/UndoWidget.h"
#include "Widgets/WelcomeWidget.h"

#include "Dialogs/OptionsDialog.h"
#include "Dialogs/AboutDialog.h"
#include "Dialogs/HelpDialog.h"
#include "Dialogs/WelcomeDialog.h"
#include "Dialogs/OptimizationDialog.h"
#include "Dialogs/SensitivityAnalysisDialog.h"
#include "Dialogs/ComponentGeneratorDialog.h"

#include "UndoStack.h"
#include "Configuration.h"
#include "CopyStack.h"
#include "Utilities/GUIUtilities.h"

//! @todo maybe we can make sure that we dont need to include these here
#include "GraphicsView.h"
#include "GUIObjects/GUISystem.h"

//Declaration of global variables
Configuration gConfig;
CopyStack gCopyStack;

//! @brief Constructor for main window
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->hide();
    gpMainWindow = this;        //!< @todo It would be nice to not declare this pointer here, but in main.cpp instead if possible
                                //! @note This is however not possible, because the gpMainWindow pointer is needed by the MainWindow constructor code.

    //Set main window options
    this->setDockOptions(QMainWindow::ForceTabbedDocks);
    this->setMouseTracking(true);

    mpConfig = &gConfig;

    //Create the message widget and its dock (must be done before everything that uses it!)
    mpMessageDock = new QDockWidget(tr("Messages"), this);
    mpMessageDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    mpMessageWidget = new MessageWidget(this);
    mpMessageDock->setWidget(mpMessageWidget);
    mpMessageDock->setFeatures(QDockWidget::DockWidgetVerticalTitleBar | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    addDockWidget(Qt::BottomDockWidgetArea, mpMessageDock);
    mpMessageDock->hide();
    //mpMessageWidget->checkMessages();
    //mpMessageWidget->printGUIInfoMessage(tr("HopsanGUI, Version: ") + QString(HOPSANGUIVERSION));

    //Create the terminal widget
    mpTerminalDock = new QDockWidget(tr("Terminal"), this);
    mpTerminalDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    mpTerminalWidget = new TerminalWidget(this);
    mpTerminalWidget->mpConsole->printFirstInfo();
    mpTerminalDock->setWidget(mpTerminalWidget);
    mpTerminalDock->setFeatures(QDockWidget::DockWidgetVerticalTitleBar | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    addDockWidget(Qt::BottomDockWidgetArea, mpTerminalDock);

    mpTerminalWidget->checkMessages();
    mpTerminalWidget->mpConsole->printInfoMessage("HopsanGUI, Version: " + QString(HOPSANGUIVERSION));

    //Load configuration from settings file
    gConfig.loadFromXml();      //!< @todo This does not really belong in main window constructor, but it depends on main window so keep it for now

    //Update style sheet setting
    if(!gConfig.getUseNativeStyleSheet())
    {
        setStyleSheet(gConfig.getStyleSheet());
        setPalette(gConfig.getPalette());
    }
    qApp->setFont(gConfig.getFont());

    //Set name and icon of main window
#ifdef DEVELOPMENT
    this->setWindowTitle(tr("Hopsan (development version)"));
#else
    this->setWindowTitle(tr("Hopsan");
#endif
    this->setWindowIcon(QIcon(QString(QString(ICONPATH) + tr("hopsan.png"))));

    //Set dock widget corner owner
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);

    //Create dialogs
    mpAboutDialog = new AboutDialog(this);
    mpOptimizationDialog = new OptimizationDialog(this);
    mpSensitivityAnalysisDialog = new SensitivityAnalysisDialog(this);
    mpComponentGeneratorDialog = new ComponentGeneratorDialog(this);
    mpHelpDialog = new HelpDialog(this);

    //Create the Python widget
    mpPyDockWidget = new PyDockWidget(this, this);
    mpPyDockWidget->setFeatures(QDockWidget::DockWidgetVerticalTitleBar | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    addDockWidget(Qt::BottomDockWidgetArea, mpPyDockWidget);

    QTime time;
    time.start();

    //Create the component library widget and its dock
    mpLibDock = new QDockWidget(tr("Component Library"), this);
    mpLibDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mpLibrary = new LibraryWidget(this);
    mpLibDock->setWidget(mpLibrary);
    addDockWidget(Qt::LeftDockWidgetArea, mpLibDock);

    qDebug() << "Time for creating library: " << time.elapsed();

    //Create the statusbar widget
    mpStatusBar = new QStatusBar();
    mpStatusBar->setObjectName("statusBar");
    this->setStatusBar(mpStatusBar);

    //Create the undo widget and the options dialog
    mpUndoWidget = new UndoWidget(this);
    mpOptionsDialog = new OptionsDialog(this);

    //Create the central widget for the main window
    mpCentralWidget = new QWidget(this);
    mpCentralWidget->setObjectName("centralwidget");
    mpCentralWidget->setMouseTracking(true);
    this->setCentralWidget(mpCentralWidget);

    //Create the grid layout for the centralwidget
    mpCentralGridLayout = new QGridLayout(mpCentralWidget);
    mpCentralGridLayout->setContentsMargins(4,4,4,4);

    //Create the main tab container, need at least one tab
    mpProjectTabs = new ProjectTabWidget(this);
    mpProjectTabs->setObjectName("projectTabs");
    mpProjectTabs->setMouseTracking(true);
    mpCentralGridLayout->addWidget(mpProjectTabs,0,0,4,4);

    //Create the system parameter widget and hide it
    mpSystemParametersWidget = new SystemParametersWidget(this);
    mpSystemParametersWidget->setVisible(false);

    //Create the plot dock widget and hide it
    mpPlotWidgetDock = new QDockWidget(tr("Plot Variables"), this);
    mpPlotWidgetDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mpPlotWidgetDock->hide();
    addDockWidget(Qt::RightDockWidgetArea, mpPlotWidgetDock);

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

    tabifyDockWidget(mpTerminalDock, mpPyDockWidget);
    tabifyDockWidget(mpPyDockWidget, mpMessageDock);
    tabifyDockWidget(mpMessageDock, mpTerminalDock);


    //Initialize the help message popup
    mpHelpPopup = new QWidget(this);
    mpHelpPopup->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    mpHelpPopupIcon = new QLabel();
    mpHelpPopupIcon->setPixmap(QPixmap(QString(ICONPATH) + "Hopsan-Info.png"));
    mpHelpPopupLabel = new QLabel();
    mpHelpPopupGroupBoxLayout = new QHBoxLayout(mpHelpPopup);
    mpHelpPopupGroupBoxLayout->addWidget(mpHelpPopupIcon);
    mpHelpPopupGroupBoxLayout->addWidget(mpHelpPopupLabel);
    mpHelpPopupGroupBoxLayout->setContentsMargins(3,3,3,3);
    mpHelpPopupGroupBox = new QGroupBox(mpHelpPopup);
    mpHelpPopupGroupBox->setLayout(mpHelpPopupGroupBoxLayout);
    mpHelpPopupLayout = new QHBoxLayout(mpHelpPopup);
    mpHelpPopupLayout->addWidget(mpHelpPopupGroupBox);
    mpHelpPopup->setLayout(mpHelpPopupLayout);
    mpHelpPopup->setBaseSize(100,30);
    mpHelpPopup->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    mpHelpPopup->setStyleSheet("QGroupBox { background-color : rgba(255,255,224,255); } QLabel { margin : 0px; } ");
    mpHelpPopup->hide();
    mpHelpPopupTimer = new QTimer(this);
    connect(mpHelpPopupTimer, SIGNAL(timeout()), mpHelpPopup, SLOT(hide()));

    //Set the correct position of the help popup message in the central widget
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

   // connect(mpCopyAction, SIGNAL(triggered()), mpMessageWidget, SLOT(copy()));
    connect(mpCopyAction, SIGNAL(triggered()), mpTerminalWidget->mpConsole, SLOT(copy()));

    mpTerminalWidget->loadConfig();

    mpWelcomeWidget = 0;

    initializeWorkspace();


    mpWelcomeWidget = new WelcomeWidget(this);
    mpCentralGridLayout->addWidget(mpWelcomeWidget,0,0,4,4);
    mpCentralGridLayout->setAlignment(mpWelcomeWidget, Qt::AlignTop);

    //Set the size and position of the main window
    int sh = qApp->desktop()->screenGeometry().height();
    int sw = qApp->desktop()->screenGeometry().width();
    this->resize(sw*0.8, sh*0.8);   //Resize window to 80% of screen height and width
//    int w = this->size().width();
//    int h = this->size().height();
//    int x = (sw - w)/2;
//    int y = (sh - h)/2;
//    this->move(x, y);       //Move window to center of screen

    updateToolBarsToNewTab();
}


//! @brief Destructor
//! @todo Shouldn't all member pointers be deleted here?
MainWindow::~MainWindow()
{
    delete mpProjectTabs;
    delete mpMenuBar;
    delete mpStatusBar;
}


//! @brief Initializes the workspace.
//! All startup events that does not involve creating the main window and its widgets/dialogs belongs here.
void MainWindow::initializeWorkspace()
{
    // Load HopsanGui built in secret components
    mpLibrary->loadHiddenSecretDir(QString(BUILTINCAFPATH) + "hidden/");

    // Load default and user specified libraries
    QString componentPath = QString(COMPONENTSPATH);

    // Load built in default Library
    mpLibrary->loadLibrary(componentPath, INTERNAL);

    // Load builtIn library (Container special components)
    mpLibrary->loadLibrary(QString(BUILTINCAFPATH) + "visible/", INTERNAL);

    for(int i=0; i<gConfig.getUserLibs().size(); ++i)
    {
        mpLibrary->loadAndRememberExternalLibrary(gConfig.getUserLibs().at(i), gConfig.getUserLibFolders().at(i));
    }

    // Create the plot widget, only once! :)
    mpPlotWidget = new PlotTreeWidget(this);
    mpPlotWidget->hide();

    // File association - ignore everything else and open the specified file if there is a hmf file in the argument list
    for(int i=0; i<qApp->arguments().size(); ++i)
    {
        if(qApp->arguments().at(i).endsWith(".hmf"))
        {
            mpProjectTabs->closeAllProjectTabs();
            mpProjectTabs->loadModel(qApp->arguments().at(i));
            return;
        }
    }

    if(gConfig.getAlwaysLoadLastSession())
    {
        if(!gConfig.getLastSessionModels().empty())
        {
            for(int i=0; i<gConfig.getLastSessionModels().size(); ++i)
            {
                mpProjectTabs->loadModel(gConfig.getLastSessionModels().at(i));
            }
        }
        else
        {
            updateToolBarsToNewTab();       //This will disable the buttons if last session did not contain any models
        }
    }
}




////! @brief Overloaded function for showing the mainwindow. This is to make sure the view is centered when the program starts.
////! @todo This function is supposed to do something, but doesn't do anything?!
//void MainWindow::show()
//{
//    QMainWindow::show();
//    //! @todo this should not be done here should happen when a new tab is created, OK! MainWindow must be shown before center works, maybe we can go through projecttabwidget instead, leaveing it for now
//}


//! @brief Opens the plot widget.
void MainWindow::openPlotWidget()
{
    if(mpProjectTabs->count() != 0)
    {
        if(!mpPlotWidgetDock->isVisible())
        {
            mpPlotWidgetDock->setWidget(mpPlotWidget);

            mpPlotWidgetDock->show();
            mpPlotWidgetDock->raise();
            mpPlotAction->setChecked(true);
            connect(mpPlotWidgetDock, SIGNAL(visibilityChanged(bool)), this, SLOT(updatePlotActionButton(bool)));
        }
        else
        {
            mpPlotWidgetDock->hide();
            mpPlotAction->setChecked(false);
        }
    }
}


//! @brief Event triggered re-implemented method that closes the main window.
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

    mpTerminalWidget->saveConfig();

    //this->saveSettings();
    gConfig.saveToXml();
}


//! @brief Shows the help popup message for 5 seconds with specified message.
//! Any message already being shown will be replaced. Messages can be hidden in advance by calling mpHelpPopup->hide().
//! @param message String with text so show in message
void MainWindow::showHelpPopupMessage(QString message)
{
    if(gConfig.getShowPopupHelp())
    {
        mpHelpPopupLabel->setText(message);
        mpHelpPopup->show();
        mpHelpPopupTimer->stop();
        mpHelpPopupTimer->start(5000);
    }
}


//! @brief Hides the help popup message
void MainWindow::hideHelpPopupMessage()
{
    mpHelpPopup->hide();
}


//! @brief Returns a pointer to the python scripting dock widget.
PyDockWidget *MainWindow::getPythonDock()
{
    return mpPyDockWidget;
}



//! @brief Defines the actions used by the toolbars
void MainWindow::createActions()
{
    mpNewAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-New.png"), tr("&New"), this);
    mpNewAction->setShortcut(tr("New"));
    mpNewAction->setToolTip(tr("Create New Project"));
    connect(mpNewAction, SIGNAL(triggered()), mpProjectTabs, SLOT(addNewProjectTab()));

    mpOpenAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Open.png"), tr("&Open"), this);
    mpOpenAction->setShortcut(QKeySequence("Ctrl+o"));
    mpOpenAction->setToolTip(tr("Load Model File (Ctrl+O)"));
    connect(mpOpenAction, SIGNAL(triggered()), mpProjectTabs, SLOT(loadModel()));

    mpSaveAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Save.png"), tr("&Save"), this);
    mpSaveAction->setShortcut(QKeySequence("Ctrl+s"));
    mpSaveAction->setToolTip(tr("Save Model File (Ctrl+S)"));

    mpSaveAsAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-SaveAs.png"), tr("&Save As"), this);
    mpSaveAsAction->setShortcut(QKeySequence("Ctrl+Alt+s"));
    mpSaveAsAction->setToolTip(tr("Save Model File As (Ctrl+Alt+S)"));

    mpExportModelAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-SaveAs.png"), tr("&Export"), this);
    mpExportModelAction->setShortcut(QKeySequence("Ctrl+Alt+E"));
    mpExportModelAction->setToolTip(tr("Export Model Parameters (Ctrl+Alt+P)"));

    mpCloseAction = new QAction(this);
    mpCloseAction->setText("Close");
    mpCloseAction->setShortcut(QKeySequence("Ctrl+q"));
    connect(mpCloseAction,SIGNAL(triggered()),this,SLOT(close()));

    mpUndoAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Undo.png"), tr("&Undo"), this);
    mpUndoAction->setText("Undo");
    mpUndoAction->setShortcut(QKeySequence(tr("Ctrl+z")));
    mpUndoAction->setToolTip(tr("Undo One Step (Ctrl+Z)"));

    mpRedoAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Redo.png"), tr("&Redo"), this);
    mpRedoAction->setText("Redo");
    mpRedoAction->setShortcut(QKeySequence(tr("Ctrl+y")));
    mpRedoAction->setToolTip(tr("Redo One Step (Ctrl+Y)"));

    mpOpenUndoAction = new QAction(tr("&Undo History"), this);
    mpOpenUndoAction->setToolTip("Undo History (Ctrl+Shift+U)");
    connect(mpOpenUndoAction,SIGNAL(triggered()),this,SLOT(openUndoWidget()));
    mpOpenUndoAction->setShortcut(QKeySequence("Ctrl+Shift+u"));

    mpDisableUndoAction = new QAction(tr("&Disable Undo"), this);
    mpDisableUndoAction->setText("Disable Undo");
    mpDisableUndoAction->setCheckable(true);
    mpDisableUndoAction->setChecked(false);

    mpOpenSystemParametersAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-SystemParameter.png"), tr("&System Parameters"), this);
    mpOpenSystemParametersAction->setToolTip("System Parameters (Ctrl+Shift+Y)");
    mpOpenSystemParametersAction->setShortcut(tr("Ctrl+Shift+y"));
    mpOpenSystemParametersAction->setCheckable(true);
    connect(mpOpenSystemParametersAction,SIGNAL(triggered()),this,SLOT(openSystemParametersWidget()));
    connect(mpOpenSystemParametersAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));


    mpCutAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Cut.png"), tr("&Cut"), this);
    mpCutAction->setShortcut(tr("Ctrl+x"));
    mpCutAction->setToolTip(tr("Cut (Ctrl+X)"));

    mpCopyAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Copy.png"), tr("&Copy"), this);
    mpCopyAction->setShortcut(tr("Ctrl+c"));
    mpCopyAction->setToolTip("Copy (Ctrl+C)");

    mpPasteAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Paste.png"), tr("&Paste"), this);
    mpPasteAction->setShortcut(tr("Ctrl+v"));
    mpPasteAction->setToolTip(tr("Paste (Ctrl+V)"));

    mpSimulateAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Simulate.png"), tr("&Simulate"), this);
    mpSimulateAction->setToolTip(tr("Simulate Current Project (Ctrl+Shift+S)"));
    mpSimulateAction->setShortcut(QKeySequence("Ctrl+Shift+s"));
    connect(mpSimulateAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    connect(mpSimulateAction, SIGNAL(triggered()), this, SLOT(simulateKeyWasPressed()));


    mpOptimizeAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Optimize.png"), tr("&Optimize"), this);
    mpOptimizeAction->setToolTip(tr("Open Optimization Dialog (Ctrl+Shift+Z)"));
    mpOptimizeAction->setShortcut(QKeySequence("Ctrl+Shift+z"));
    connect(mpOptimizeAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    connect(mpOptimizeAction, SIGNAL(triggered()), mpOptimizationDialog, SLOT(open()));

    mpSensitivityAnalysisAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-SensitivityAnalysis.png"), tr("&Sensitivity Analysis"), this);
    mpSensitivityAnalysisAction->setToolTip(tr("Open Sensitivity Analysis Dialog (Ctrl+Shift+A)"));
    mpSensitivityAnalysisAction->setShortcut(QKeySequence("Ctrl+Shift+A"));
    connect(mpSensitivityAnalysisAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    connect(mpSensitivityAnalysisAction, SIGNAL(triggered()), mpSensitivityAnalysisDialog, SLOT(open()));

    mpMeasureSimulationTimeAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-MeasureSimulationTime.png"), tr("&Measure Simulation Times"), this);
    mpMeasureSimulationTimeAction->setToolTip(tr("Measure Simulation Times"));
    connect(mpMeasureSimulationTimeAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    connect(mpMeasureSimulationTimeAction, SIGNAL(triggered()), mpProjectTabs, SLOT(measureSimulationTime()));

    mpPlotAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Plot.png"), tr("&Plot Variables"), this);
    mpPlotAction->setToolTip(tr("Plot Variables (Ctrl+Shift+P)"));
    mpPlotAction->setCheckable(true);
    mpPlotAction->setShortcut(QKeySequence("Ctrl+Shift+p"));
    connect(mpPlotAction, SIGNAL(triggered()),this,SLOT(openPlotWidget()));
    connect(mpPlotAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpLoadLibsAction = new QAction(this);
    mpLoadLibsAction->setText("Load Libraries");
    connect(mpLoadLibsAction,SIGNAL(triggered()),mpLibrary,SLOT(addExternalLibrary()));

    mpPropertiesAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Configure.png"), tr("&Model Properties"), this);
    mpPropertiesAction->setToolTip("Model Properties (Ctrl+Shift+M)");
    mpPropertiesAction->setShortcut(QKeySequence("Ctrl+Shift+m"));
    connect(mpPropertiesAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpOptionsAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Options.png"), tr("&Options"), this);
    mpOptionsAction->setToolTip("Options (Ctrl+Shift+O)");
    mpOptionsAction->setShortcut(QKeySequence("Ctrl+Shift+o"));
    connect(mpOptionsAction, SIGNAL(triggered()), mpOptionsDialog, SLOT(show()));

    mpAnimateAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Animation.png"), tr("&Animate"), this);
    mpAnimateAction->setToolTip("Animate");
    connect(mpAnimateAction, SIGNAL(triggered()),mpProjectTabs, SLOT(openAnimation()));

    mpAlignXAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-AlignX.png"), tr("&Align Vertical (by last selected)"), this);
    mpAlignXAction->setText("Align Vertical");

    mpAlignYAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-AlignY.png"), tr("&Align Horizontal (by last selected)"), this);
    mpAlignYAction->setText("Align Horizontal");

    mpRotateLeftAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-RotateLeft.png"), tr("&Rotate Left (Ctrl+E)"), this);
    mpRotateLeftAction->setText("Rotate Left (Ctrl+E)");
    mpRotateLeftAction->setShortcut(QKeySequence("Ctrl+E"));

    mpRotateRightAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-RotateRight.png"), tr("&Rotate Right (Ctrl+R)"), this);
    mpRotateRightAction->setText("Rotate Right (Ctrl+R)");
    mpRotateRightAction->setShortcut(QKeySequence("Ctrl+R"));

    mpFlipHorizontalAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-FlipHorizontal.png"), tr("&Flip Horizontal"), this);
    mpFlipHorizontalAction->setText("Flip Horizontal (Ctrl+F)");
    mpFlipHorizontalAction->setShortcut(QKeySequence("Ctrl+F"));

    mpFlipVerticalAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-FlipVertical.png"), tr("&Flip Vertical"), this);
    mpFlipVerticalAction->setText("Flip Vertical (Ctrl+D");
    mpFlipVerticalAction->setShortcut(QKeySequence("Ctrl+D"));

    mpResetZoomAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Zoom100.png"), tr("&Reset Zoom (Ctrl+0)"), this);
    mpResetZoomAction->setText("Reset Zoom (Ctrl+0)");
    mpResetZoomAction->setShortcut(QKeySequence("Ctrl+0"));

    mpZoomInAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-ZoomIn.png"), tr("&Zoom In (Ctrl+Plus)"), this);
    mpZoomInAction->setText("Zoom In (Ctrl+Plus)");
    mpZoomInAction->setShortcut(QKeySequence("Ctrl++"));

    mpZoomOutAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-ZoomOut.png"), tr("&Zoom Out (Ctrl+Minus)"), this);
    mpZoomOutAction->setText("Zoom Out (Ctrl+Minus)");
    mpZoomOutAction->setShortcut(QKeySequence("Ctrl+-"));

    mpCenterViewAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-CenterView.png"), tr("&Center View (Ctrl+Space)"), this);
    mpCenterViewAction->setText("Center View (Ctrl+Space)");
    mpCenterViewAction->setShortcut(QKeySequence("Ctrl+Space"));

    QIcon toggleNamesIcon;
    toggleNamesIcon.addFile(QString(ICONPATH) + "Hopsan-ToggleNames.png", QSize(), QIcon::Normal, QIcon::On);
    mpToggleNamesAction = new QAction(toggleNamesIcon, tr("&Show Component Names (Ctrl+N)"), this);
    mpToggleNamesAction->setText("Show Component Names (Ctrl+N)");
    mpToggleNamesAction->setCheckable(true);
    mpToggleNamesAction->setChecked(gConfig.getToggleNamesButtonCheckedLastSession());
    mpToggleNamesAction->setShortcut(QKeySequence("Ctrl+n"));

    mpExportPDFAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-ExportPdf.png"), tr("&Export To PDF"), this);
    mpExportPDFAction->setText("Export Model to PDF");

    mpImportFMUAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-ImportFmu.png"), tr("Import Functional Mock-up Unit (FMU)"), this);
    mpExportToSimulinkAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-ExportSimulink.png"), tr("Export to Simulink S-function Source Files"), this);
    mpExportToFMUAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-ExportFmu.png"), tr("Export to Functional Mock-up Unit (FMU)"), this);

    mpAboutAction = new QAction(this);
    mpAboutAction->setText("About");
    connect(mpAboutAction, SIGNAL(triggered()), mpAboutDialog, SLOT(open()));
    connect(mpAboutDialog->timer, SIGNAL(timeout()), mpAboutDialog, SLOT(update()));

    mpIssueTrackerAction = new QAction(this);
    mpIssueTrackerAction->setText("Issue Tracker");
    connect(mpIssueTrackerAction, SIGNAL(triggered()), this, SLOT(openIssueTrackerDialog()));

    mpHelpAction = new QAction(this);
    mpHelpAction->setText("User Guide");
    connect(mpHelpAction, SIGNAL(triggered()), mpHelpDialog, SLOT(open()));

    mpNewVersionsAction = new QAction(this);
    mpNewVersionsAction->setText("Check For New Versions");
    connect(mpNewVersionsAction, SIGNAL(triggered()), this, SLOT(openArchiveURL()));
    mpWebsiteAction = new QAction(this);
    mpWebsiteAction->setText("Open Hopsan Website");
    connect(mpWebsiteAction, SIGNAL(triggered()), this, SLOT(openHopsanURL()));

    //! @todo Check for new version could probably work in a better way...
    mpNewVersionsAction = new QAction(this);
    mpNewVersionsAction->setText("Check For New Versions");
    connect(mpNewVersionsAction, SIGNAL(triggered()), this, SLOT(openArchiveURL()));

    QIcon togglePortsIcon;
    togglePortsIcon.addFile(QString(ICONPATH) + "Hopsan-TogglePorts.png", QSize(), QIcon::Normal, QIcon::On);
    mpTogglePortsAction = new QAction(togglePortsIcon, tr("&Show Unconnected Ports (Ctrl+T)"), this);
    mpTogglePortsAction->setText("Show Unconnected Ports (Ctrl+T)");
    mpTogglePortsAction->setCheckable(true);
    mpTogglePortsAction->setChecked(gConfig.getTogglePortsButtonCheckedLastSession());
    mpTogglePortsAction->setShortcut(QKeySequence("Ctrl+t"));

    QIcon toggleSignalsIcon;
    toggleSignalsIcon.addFile(QString(ICONPATH) + "Hopsan-ToggleSignal.png", QSize(), QIcon::Normal, QIcon::On);
    mpToggleSignalsAction = new QAction(toggleSignalsIcon, tr("&Show Signal Components"), this);
    mpToggleSignalsAction->setText("Show Signal Components");
    mpToggleSignalsAction->setCheckable(true);
    mpToggleSignalsAction->setChecked(true);      //! @todo Shall depend on gConfig setting

    mpSaveToWrappedCodeAction = new QAction(this);
    mpSaveToWrappedCodeAction->setShortcut(QKeySequence("Ctrl+Shift+Alt+W"));
    this->addAction(mpSaveToWrappedCodeAction);
    connect(mpSaveToWrappedCodeAction, SIGNAL(triggered()), mpProjectTabs, SLOT(saveCurrentModelToWrappedCode()));

    mpDebug1Action = new QAction(this);
    mpDebug1Action->setShortcut(QKeySequence("Ctrl+D+1"));
    this->addAction(mpDebug1Action);
    //connect(mpDebug1Action, SIGNAL(triggered()), mpProjectTabs, SLOT(simulateAllOpenModelsWithSplit()));

    mpDebug2Action = new QAction(this);
    mpDebug2Action->setShortcut(QKeySequence("Ctrl+D+2"));
    this->addAction(mpDebug2Action);
    //connect(mpDebug2Action, SIGNAL(triggered()), mpProjectTabs, SLOT(simulateAllOpenModelsWithoutSplit()));

    mpCreateSimulinkWrapperAction = new QAction(this);
    //mpCreateSimulinkWrapperAction->setShortcut(QKeySequence("Ctrl+Shift+Alt+S"));
    this->addAction(mpCreateSimulinkWrapperAction);
    connect(mpCreateSimulinkWrapperAction, SIGNAL(triggered()), mpProjectTabs, SLOT(createSimulinkWrapperFromCurrentModel()));

    mpShowLossesAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Losses.png"), tr("Calculate Losses"), this);
    mpShowLossesAction->setShortcut(QKeySequence("Ctrl+L"));
    mpShowLossesAction->setCheckable(true);
    this->addAction(mpShowLossesAction);
    connect(mpShowLossesAction, SIGNAL(triggered(bool)), mpProjectTabs, SLOT(showLosses(bool)));

    mpStartTimeLineEdit = new QLineEdit("0.0");
    mpStartTimeLineEdit->setMaximumWidth(70);
    mpStartTimeLineEdit->setAlignment(Qt::AlignVCenter | Qt::AlignCenter);
    mpStartTimeLineEdit->setValidator(new QDoubleValidator(-1e4, 1e6, 10, mpStartTimeLineEdit));
    mpTimeStepLineEdit = new QLineEdit("0.001");
    mpTimeStepLineEdit->setMaximumWidth(70);
    mpTimeStepLineEdit->setAlignment(Qt::AlignVCenter | Qt::AlignCenter);
    mpTimeStepLineEdit->setValidator(new QDoubleValidator(0.0, 1e3, 10, mpTimeStepLineEdit));
    mpStopTimeLineEdit = new QLineEdit("10.0");
    mpStopTimeLineEdit->setValidator(new QDoubleValidator(-1e4, 1e6, 10, mpStopTimeLineEdit));
    mpStopTimeLineEdit->setMaximumWidth(70);
    mpStopTimeLineEdit->setAlignment(Qt::AlignVCenter | Qt::AlignCenter);
    mpTimeLabelDeliminator1 = new QLabel(tr(" :: "));
    mpTimeLabelDeliminator2 = new QLabel(tr(" :: "));

    connect(mpStartTimeLineEdit, SIGNAL(editingFinished()), SLOT(setProjectSimulationTimeParameterValues()), Qt::UniqueConnection);
    connect(mpTimeStepLineEdit, SIGNAL(editingFinished()), SLOT(setProjectSimulationTimeParameterValues()), Qt::UniqueConnection);
    connect(mpStopTimeLineEdit, SIGNAL(editingFinished()), SLOT(setProjectSimulationTimeParameterValues()), Qt::UniqueConnection);
}


//! @brief Creates the menus
void MainWindow::createMenus()
{
    //Create the menubar
    mpMenuBar = new QMenuBar();
    mpMenuBar->setGeometry(QRect(0,0,800,25));

    //Create the menues
    mpFileMenu = new QMenu(mpMenuBar);
    mpFileMenu->setTitle("&File");

    mpRecentMenu = new QMenu(this);
    mpRecentMenu->setTitle("&Recent Models");

    mpNewMenu = new QMenu(mpMenuBar);
    mpNewMenu->setTitle("&New");

    mpSimulationMenu = new QMenu(mpMenuBar);
    mpSimulationMenu->setTitle("&Simulation");

    mpEditMenu = new QMenu(mpMenuBar);
    mpEditMenu->setTitle("&Edit");

    mpViewMenu = new QMenu(mpMenuBar);
    mpViewMenu->setTitle("&View");

    mpToolsMenu = new QMenu(mpMenuBar);
    mpToolsMenu->setTitle("&Tools");

    mpHelpMenu = new QMenu(mpMenuBar);
    mpHelpMenu->setTitle("&Help");

    this->setMenuBar(mpMenuBar);

    //Add the actionbuttons to the menues
    mpNewAction->setText("Project");
    mpNewMenu->addAction(mpNewAction);

    mpFileMenu->addAction(mpNewMenu->menuAction());
    mpFileMenu->addAction(mpOpenAction);
    mpFileMenu->addAction(mpSaveAction);
    mpFileMenu->addAction(mpSaveAsAction);
    mpFileMenu->addAction(mpExportModelAction);
    mpFileMenu->addMenu(mpRecentMenu);
    //mpFileMenu->addSeparator();
    //mpFileMenu->addMenu(mpImportMenu);
    //mpFileMenu->addMenu(mpExportMenu);
    mpFileMenu->addSeparator();
    mpFileMenu->addAction(mpLoadLibsAction);
    mpFileMenu->addSeparator();
    mpFileMenu->addAction(mpPropertiesAction);
    mpFileMenu->addAction(mpOpenSystemParametersAction);
    mpFileMenu->addSeparator();
    mpFileMenu->addAction(mpCloseAction);

    this->updateRecentList();

    mpSimulationMenu->addAction(mpSimulateAction);
    mpSimulationMenu->addAction(mpAnimateAction);
    mpSimulationMenu->addAction(mpMeasureSimulationTimeAction);
    mpSimulationMenu->addAction(mpOptimizeAction);
    mpSimulationMenu->addAction(mpSensitivityAnalysisAction);
    mpSimulationMenu->addAction(mpPlotAction);
    mpSimulationMenu->addAction(mpShowLossesAction);

    mpEditMenu->addAction(mpUndoAction);
    mpEditMenu->addAction(mpRedoAction);
    mpEditMenu->addAction(mpOpenUndoAction);
    mpEditMenu->addAction(mpDisableUndoAction);
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
    mpViewMenu->addAction(mpPyDockWidget->toggleViewAction());
    mpViewMenu->addAction(mpSimToolBar->toggleViewAction());

    mpToolsMenu->addAction(mpOptionsAction);
    mpToolsMenu->addAction(mpOpenSystemParametersAction);

    mpHelpMenu->addAction(mpHelpAction);
    mpHelpMenu->addMenu(mpExamplesMenu);
    mpHelpMenu->addAction(mpIssueTrackerAction);
    mpHelpMenu->addAction(mpWebsiteAction);
    mpHelpMenu->addAction(mpNewVersionsAction);
    mpHelpMenu->addAction(mpAboutAction);

    mpMenuBar->addAction(mpFileMenu->menuAction());
    mpMenuBar->addAction(mpEditMenu->menuAction());
    mpMenuBar->addAction(mpToolsMenu->menuAction());
    mpMenuBar->addAction(mpSimulationMenu->menuAction());
    mpMenuBar->addAction(mpViewMenu->menuAction());
    mpMenuBar->addAction(mpHelpMenu->menuAction());
}

//! @brief Creates the toolbars
void MainWindow::createToolbars()
{
    //File toolbar, contains all file handling stuff (open, save etc)
    mpFileToolBar = addToolBar(tr("File Toolbar"));
    mpFileToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::LeftToolBarArea | Qt::RightToolBarArea);
    mpFileToolBar->addAction(mpNewAction);
    mpFileToolBar->addAction(mpOpenAction);
    mpFileToolBar->addAction(mpSaveAction);
    mpFileToolBar->addAction(mpExportModelAction);
    mpFileToolBar->addAction(mpSaveAsAction);

    mpConnectivityToolBar = addToolBar(tr("Import/Export Toolbar)"));
    mpConnectivityToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::LeftToolBarArea | Qt::RightToolBarArea);
    mpConnectivityToolBar->addAction(mpExportPDFAction);
    mpConnectivityToolBar->addAction(mpExportToSimulinkAction);
    mpConnectivityToolBar->addAction(mpExportToFMUAction);
    mpConnectivityToolBar->addAction(mpImportFMUAction);


    //! @note Action and menu shouldn't be here, but it doesn't work otherwise because the menus are created after the toolbars
    //mpImportMenu = new QMenu("Import");
    //mpImportMenu->addAction(mpImportFMUAction);
//    mpImportButton = new QToolButton(mpFileToolBar);
//    mpImportButton->setToolTip("Import");
//    mpImportButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Import.png"));
//    mpImportButton->setMenu(mpImportMenu);
//    mpImportButton->setPopupMode(QToolButton::InstantPopup);
    //mpFileToolBar->addWidget(mpImportButton);

//    mpExportMenu = new QMenu("Export Model");
//    mpExportMenu->addAction(mpExportToSimulinkAction);
//    mpExportMenu->addAction(mpExportToFMUAction);
//    mpExportButton = new QToolButton(mpFileToolBar);
//    mpExportButton->setToolTip("Export");
//    mpExportButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Export.png"));
//    mpExportButton->setMenu(mpExportMenu);
//    mpExportButton->setPopupMode(QToolButton::InstantPopup);
//    mpFileToolBar->addWidget(mpExportButton);

    //Simulation toolbar, contains tools for simulationg, plotting and model preferences
    mpSimToolBar = addToolBar(tr("Simulation Toolbar"));
    mpSimToolBar->setAllowedAreas(Qt::TopToolBarArea);
    mpSimToolBar->setAttribute(Qt::WA_MouseTracking);
    mpSimToolBar->addWidget(mpStartTimeLineEdit);
    mpSimToolBar->addWidget(mpTimeLabelDeliminator1);

    mpSimToolBar->addWidget(mpTimeStepLineEdit);
    mpSimToolBar->addWidget(mpTimeLabelDeliminator2);
    mpSimToolBar->addWidget(mpStopTimeLineEdit);
    mpSimToolBar->addAction(mpSimulateAction);
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
    mpViewToolBar->addAction(mpCenterViewAction);
    mpViewToolBar->addAction(mpResetZoomAction);
    mpViewToolBar->addAction(mpZoomInAction);
    mpViewToolBar->addAction(mpZoomOutAction);
    mpViewToolBar->addAction(mpToggleNamesAction);
    mpViewToolBar->addAction(mpTogglePortsAction);
    mpViewToolBar->addAction(mpToggleSignalsAction);

    //Tools toolbar, contains all tools used to modify the model
    mpToolsToolBar = new QToolBar(tr("Tools Toolbar"));
    addToolBar(Qt::LeftToolBarArea, mpToolsToolBar);
    mpToolsToolBar->addAction(mpAlignXAction);
    mpToolsToolBar->addAction(mpAlignYAction);
    mpToolsToolBar->addAction(mpRotateRightAction);
    mpToolsToolBar->addAction(mpRotateLeftAction);
    mpToolsToolBar->addAction(mpFlipHorizontalAction);
    mpToolsToolBar->addAction(mpFlipVerticalAction);

    mpExamplesMenu = new QMenu("Example Models");
    QAction *pTempAction;
    QStringList exampleModels;

    QDir exampleModelsDir(QString(MODELS_DEV_PATH+"Example Models/"));
    QStringList filters;
    filters << "*.hmf";
    exampleModelsDir.setNameFilters(filters);
    exampleModels = exampleModelsDir.entryList();
    for (int i = 0; i < exampleModels.size(); ++i)
    {
        exampleModels[i].chop(4);
    }

    for(int i=0; i<exampleModels.size(); ++i)
    {
        pTempAction = new QAction(exampleModels.at(i), this);
        pTempAction->setIcon(QIcon(QString(ICONPATH) + "hmf.ico"));
        mpExamplesMenu->addAction(pTempAction);
        connect(pTempAction, SIGNAL(triggered()), this, SLOT(openExampleModel()));
    }

    connect(mpImportFMUAction, SIGNAL(triggered()), mpLibrary, SLOT(importFmu()));
    connect(mpExportToSimulinkAction, SIGNAL(triggered()), mpProjectTabs, SLOT(createSimulinkWrapperFromCurrentModel()));
    connect(mpExportToFMUAction, SIGNAL(triggered()), mpProjectTabs, SLOT(createFMUFromCurrentModel()));
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
        mpProjectTabs->loadModel(action->text());
    }
}


void MainWindow::openHopsanURL()
{
    QDesktopServices::openUrl(QUrl(QString(HOPSANLINK)));
}


void MainWindow::openArchiveURL()
{
    QDesktopServices::openUrl(QUrl(QString(DOWNLOADLINK)));
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


//! @brief Shows help popup for the toolbar icon that is currently hovered by the mouse pointer
void MainWindow::showToolBarHelpPopup()
{
    QCursor cursor;
    QAction *pHoveredAction = mpSimToolBar->actionAt(mpSimToolBar->mapFromGlobal(cursor.pos()));
    if(pHoveredAction == mpSimulateAction)
    {
        showHelpPopupMessage("Starts a new simulation of current model.");
    }
    else if(pHoveredAction == mpOptimizeAction)
    {
        showHelpPopupMessage("Open optimization dialog to initialize numerical optimization of current model.");
    }
    else if(pHoveredAction == mpSensitivityAnalysisAction)
    {
        showHelpPopupMessage("Perform sensitivity analysis of current model.");
    }
    else if(pHoveredAction == mpMeasureSimulationTimeAction)
    {
        showHelpPopupMessage("Measure simulation time for each component in current model.");
    }
    else if(pHoveredAction == mpPlotAction)
    {
        showHelpPopupMessage("Opens the list with all available plot variables from current model.");
    }
    else if(pHoveredAction == mpOpenSystemParametersAction)
    {
        showHelpPopupMessage("Opens the list of system parameters in current model.");
    }
    else if(pHoveredAction == mpPropertiesAction)
    {
        showHelpPopupMessage("Opens a dialog with settings for the current model.");
    }
}


//! @brief Slot that loads an example model, based on the name of the calling action
void MainWindow::openExampleModel()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
    {
        QString modelPath = QString(MODELS_DEV_PATH+"Example Models/") +action->text() + ".hmf";
        qDebug() << "Trying to open " << modelPath;
        mpProjectTabs->loadModel(modelPath);
    }
}


//! @brief This will attempt to download the latest installer and (if successful) launch it in silent mode and close Hopsan
//! @todo Disable this in Linux/Mac releases, or make it work for those platforms as well.
void MainWindow::launchAutoUpdate()
{
    qDebug() << "HewrdPewrn";
    QNetworkAccessManager *pNetworkManager = new QNetworkAccessManager();
    connect(pNetworkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(commenceAutoUpdate(QNetworkReply*)));

    qDebug() << "FewrSpeil";
    //QString path = QString(AUTOUPDATELINK);
    //QString path = "http://tiny.cc/hopsanupdate";
    QUrl url = QUrl(mpWelcomeWidget->getUpdateLink());

    qDebug() << "Downloading: " << mpWelcomeWidget->getUpdateLink();

    mpDownloadDialog = new QProgressDialog("Downloading new version...", "Cancel",0, 100, this);
    mpDownloadDialog->setWindowTitle("Hopsan Auto Updater");
    mpDownloadDialog->setWindowModality(Qt::WindowModal);
    mpDownloadDialog->setMinimumWidth(300);
    mpDownloadDialog->setValue(0);

    mpDownloadStatus = pNetworkManager->get(QNetworkRequest(url));
    connect(mpDownloadStatus, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(updateDownloadProgressBar(qint64, qint64)));
    qDebug() << "FewFighers";
}


void MainWindow::openContextHelp()
{
    QAction *action = qobject_cast<QAction *>(sender());

    if(action->parent() == mpOptimizationDialog)
    {
        mpHelpDialog->open("userOptimization.html");
    }
    else if(action->parent() == mpSensitivityAnalysisDialog)
    {
        mpHelpDialog->open("userSensitivityAnalysis.html");
    }
    else if(action->parent() == mpComponentGeneratorDialog)
    {
        mpHelpDialog->open("component-generator.html");
    }
    else if(action->parent() == mpProjectTabs->getCurrentContainer())
    {
        mpHelpDialog->open("userEnergyLosses.html");
    }
    else if(action->parent() == mpLibrary)
    {
        mpHelpDialog->open("userCustomComponents.html");
    }
    else
    {
        mpHelpDialog->open();
    }
    mpHelpDialog->centerOnScreen();
}


void MainWindow::openContextHelp(QString file)
{
    mpHelpDialog->open(file);
    mpHelpDialog->centerOnScreen();
}


//! @brief Private slot that updates the progress bar during auto update downloads
//! @param bytesReceived Number of bytes downloaded
//! @param bytesTotal Total number of bytes to download
void MainWindow::updateDownloadProgressBar(qint64 bytesReceived, qint64 bytesTotal)
{
    qDebug() << "Dewnlewding!";
    int progress = 100*bytesReceived/bytesTotal;
    mpDownloadDialog->setValue(progress);
}


//! @brief Private slot that saves the downloaded installer file, launches it and quit Hopsan
//! @param reply Contains information about the downloaded installation executable
void MainWindow::commenceAutoUpdate(QNetworkReply* reply)
{
    qDebug() << "SkewHewrn";
    QUrl url = reply->url();
    if (reply->error())
    {
        mpTerminalWidget->mpConsole->printErrorMessage("Download of " + QString(url.toEncoded().constData()) + "failed: "+reply->errorString()+"\n");
        qDebug() << "Dewnlewd Prewblem";
        return;
    }
    else
    {
        QFile file(QString(DATAPATH)+"/update.exe");
        if (!file.open(QIODevice::WriteOnly)) {
            mpTerminalWidget->mpConsole->printErrorMessage("Could not open update.exe for writing.");
            qDebug() << "Feil Prewblem";
            return;
        }
        file.write(reply->readAll());
        file.close();
    }
    qDebug() << "KewHewrn";
    reply->deleteLater();

    QProcess *pProcess = new QProcess();
    pProcess->start(QString(DATAPATH)+"/update.exe", QStringList() << "/silent");
    pProcess->waitForStarted();
    this->close();
}


//! @brief Updates the toolbar values that are tab specific when a new tab is activated
void MainWindow::updateToolBarsToNewTab()
{
    if(mpProjectTabs->count() > 0)
    {
        mpTogglePortsAction->setChecked(!mpProjectTabs->getCurrentTab()->getTopLevelSystem()->areSubComponentPortsHidden());
    }

    bool noTabs = !(mpProjectTabs->count() > 0);
    mpSaveAction->setEnabled(!noTabs);
    mpSaveAsAction->setEnabled(!noTabs);
    mpExportModelAction->setEnabled(!noTabs);
    mpCutAction->setEnabled(!noTabs);
    mpCopyAction->setEnabled(!noTabs);
    mpPasteAction->setEnabled(!noTabs);
    mpUndoAction->setEnabled(!noTabs);
    mpRedoAction->setEnabled(!noTabs);
    mpCenterViewAction->setEnabled(!noTabs);
    mpResetZoomAction->setEnabled(!noTabs);
    mpZoomInAction->setEnabled(!noTabs);
    mpZoomOutAction->setEnabled(!noTabs);
    mpToggleNamesAction->setEnabled(!noTabs);
    mpToggleSignalsAction->setEnabled(!noTabs);
    mpTogglePortsAction->setEnabled(!noTabs);
    mpTogglePortsAction->setEnabled(!noTabs);
    mpExportPDFAction->setEnabled(!noTabs);
    mpAlignXAction->setEnabled(!noTabs);
    mpAlignYAction->setEnabled(!noTabs);
    mpRotateLeftAction->setEnabled(!noTabs);
    mpRotateRightAction->setEnabled(!noTabs);
    mpFlipHorizontalAction->setEnabled(!noTabs);
    mpFlipVerticalAction->setEnabled(!noTabs);
    mpStartTimeLineEdit->setEnabled(!noTabs);
    mpTimeStepLineEdit->setEnabled(!noTabs);
    mpStopTimeLineEdit->setEnabled(!noTabs);
    mpSimulateAction->setEnabled(!noTabs);
    mpOptimizeAction->setEnabled(!noTabs);
    mpSensitivityAnalysisAction->setEnabled(!noTabs);
    mpMeasureSimulationTimeAction->setEnabled(!noTabs);
    mpPlotAction->setEnabled(!noTabs);
    mpShowLossesAction->setEnabled(!noTabs);
    mpAnimateAction->setEnabled(!noTabs);
    mpPropertiesAction->setEnabled(!noTabs);
    mpOpenSystemParametersAction->setEnabled(!noTabs);
    mpExportToFMUAction->setEnabled(!noTabs);
    mpExportToSimulinkAction->setEnabled(!noTabs);

    if(mpWelcomeWidget)
    {
        mpWelcomeWidget->setVisible(noTabs);
    }
}


//! @brief Slot that calls refresh list function in undo widget. Used because undo widget cannot have slots.
void MainWindow::refreshUndoWidgetList()
{
    mpUndoWidget->refreshList();
}


//! @brief Registers a recently opened model file in the "Recent Models" list
void MainWindow::registerRecentModel(QFileInfo model)
{
    if(model.fileName() == "")
        return;

    gConfig.addRecentModel(model.filePath());
    updateRecentList();
}


//! @brief Updates the "Recent Models" list
void MainWindow::updateRecentList()
{
    mpRecentMenu->clear();

    mpRecentMenu->setEnabled(!gConfig.getRecentModels().empty());
    if(!gConfig.getRecentModels().empty())
    {
        for(int i=0; i<gConfig.getRecentModels().size(); ++i)
        {
            if(gConfig.getRecentModels().at(i) != "")
            {
                QAction *tempAction;
                tempAction = mpRecentMenu->addAction(gConfig.getRecentModels().at(i));
                tempAction->setIcon(QIcon(QString(ICONPATH) + "hmf.ico"));
                disconnect(mpRecentMenu, SIGNAL(triggered(QAction *)), mpProjectTabs, SLOT(loadModel(QAction *)));    //Ugly hack to make sure connecetions are not made twice (then program would try to open model more than once...)
                connect(tempAction, SIGNAL(triggered()), this, SLOT(openRecentModel()));
            }
        }
    }
}


void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    hideHelpPopupMessage();

    if(!mpLibrary->underMouse())
    {
        mpLibrary->mpComponentNameField->setText(QString());
    }

    QMainWindow::mouseMoveEvent(event);
}


void MainWindow::keyPressEvent(QKeyEvent *event)
{
    qDebug() << "Mainwindow caught keypress: " << event->key();

    QMainWindow::keyPressEvent(event);
}


OptionsDialog *MainWindow::getOptionsDialog()
{
    return mpOptionsDialog;
}


ComponentGeneratorDialog *MainWindow::getComponentGeneratorDialog()
{
    return mpComponentGeneratorDialog;
}

//! @brief Sets a new startvalue.
//! @param startTime is the new value
void MainWindow::setStartTimeInToolBar(const double startTime)
{
    QString valueTxt;
    valueTxt.setNum(startTime, 'g', 10 );
    mpStartTimeLineEdit->setText(valueTxt);
    setProjectSimulationTimeParameterValues();
}


//! @brief Sets a new timestep.
//! @param timeStep is the new value
void MainWindow::setTimeStepInToolBar(const double timeStep)
{
    QString valueTxt;
    valueTxt.setNum(timeStep, 'g', 10 );
    mpTimeStepLineEdit->setText(valueTxt);
    setProjectSimulationTimeParameterValues();
}


//! @brief Sets a new finish value.
//! @param finishTime is the new value
void MainWindow::setStopTimeInToolBar(const double finishTime)
{
    QString valueTxt;
    valueTxt.setNum(finishTime, 'g', 10 );
    mpStopTimeLineEdit->setText(valueTxt);
    setProjectSimulationTimeParameterValues();
}

//! @brief Special function to set start step and stop time as QStrings
//! @note ONLY! call this function to write back changes to toolbar, it will not try to sent the changes back to teh project tab as that would risk endles loop in some cases
void MainWindow::displaySimulationTimeParameters(const QString startTime, const QString timeStep, const QString stopTime)
{
    mpStartTimeLineEdit->setText(startTime);
    mpTimeStepLineEdit->setText(timeStep);
    mpStopTimeLineEdit->setText(stopTime);
}


//! @brief Access function to the starttimelabel value.
//! @returns the starttime value
double MainWindow::getStartTimeFromToolBar()
{
    return mpStartTimeLineEdit->text().toDouble();
}


//! @brief Access function to the timesteplabel value.
//! @returns the timestep value
double MainWindow::getTimeStepFromToolBar()
{
    return mpTimeStepLineEdit->text().toDouble();
}


//! @brief Access function to the finishlabel value.
//! @returns the finish value
double MainWindow::getFinishTimeFromToolBar()
{
    return mpStopTimeLineEdit->text().toDouble();
}


void MainWindow::setProjectSimulationTimeParameterValues()
{
    mpProjectTabs->setCurrentTopLevelSimulationTimeParameters(mpStartTimeLineEdit->text(), mpTimeStepLineEdit->text(), mpStopTimeLineEdit->text() );
}

void MainWindow::simulateKeyWasPressed()
{
    if (mpStartTimeLineEdit->hasFocus())
    {
        mpStartTimeLineEdit->clearFocus();
    }
    if (mpStopTimeLineEdit->hasFocus())
    {
        mpStopTimeLineEdit->clearFocus();
    }
    if (mpTimeStepLineEdit->hasFocus())
    {
        mpTimeStepLineEdit->clearFocus();
    }
    emit simulateKeyPressed();
}
