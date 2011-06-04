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
//! @file   MainWindow.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the HopsanGUI MainWindow class
//!
//$Id$

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>

#include "Widgets/UndoWidget.h"

class ProjectTabWidget;
class GraphicsView;
class QGraphicsScene;
class LibraryWidget;
class OptionsDialog;
class MessageWidget;
class PlotWidget;
class PyDockWidget;
class SystemParametersWidget;
class Configuration;
class CopyStack;
class AboutDialog;
class WelcomeDialog;
class HelpDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QWidget *mpCentralWidget;
    QGridLayout *mpCentralGridLayout;
    QGridLayout *mpTabgrid;

    //Widgets that can be displayed in main window
    UndoWidget *mpUndoWidget;
    ProjectTabWidget *mpProjectTabs;
    LibraryWidget *mpLibrary;
    OptionsDialog *mpOptionsDialog;
    MessageWidget *mpMessageWidget;
    WelcomeDialog *mpWelcomeDialog;
    Configuration *mpConfig;

    AboutDialog *mpAboutDialog;
    HelpDialog *mpHelpDialog;

    QStatusBar *mpStatusBar;
    PlotWidget *mpPlotWidget;
    PyDockWidget *mpPyDockWidget;
    SystemParametersWidget *mpSystemParametersWidget;

    //Help popup
    QWidget *mpHelpPopup;
    QLabel *mpHelpPopupIcon;
    QLabel *mpHelpPopupLabel;
    QHBoxLayout *mpHelpPopupLayout;
    QGroupBox *mpHelpPopupGroupBox;
    QHBoxLayout *mpHelpPopupGroupBoxLayout;
    QTimer *mpHelpPopupTimer;

    //Menubar items
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuNew;
    QMenu *menuLibs;
    QMenu *menuSimulation;
    QMenu *menuEdit;
    QMenu *menuView;
    QMenu *menuTools;
    QMenu *menuPlot;
    QMenu *recentMenu;
    QMenu *menuHelp;
    QMenu *menuExport;

    //Buttons
    QToolButton *mpExportButton;

    //Toolbar items
    QToolBar *mpFileToolBar;
    QToolBar *mpEditToolBar;
    QToolBar *mpToolsToolBar;
    QToolBar *mpSimToolBar;
    QToolBar *mpViewToolBar;
    QLineEdit *mpStartTimeLineEdit;
    QLineEdit *mpTimeStepLineEdit;
    QLineEdit *mpFinishTimeLineEdit;
    QLabel *mpTimeLabelDeliminator1;
    QLabel *mpTimeLabelDeliminator2;

    //Actions used in menubar and toolbar
    QAction *newAction;
    QAction *openAction;
    QAction *saveAction;
    QAction *saveAsAction;
    QAction *exportToSimulinkAction;
    QAction *closeAction;
    QAction *undoAction;
    QAction *redoAction;
    QAction *openUndoAction;
    QAction *openSystemParametersAction;
    QAction *disableUndoAction;
    QAction *cutAction;
    QAction *copyAction;
    QAction *pasteAction;
    QAction *simulateAction;
    QAction *plotAction;
    QAction *loadLibsAction;
    QAction *propertiesAction;
    QAction *optionsAction;
    QAction *resetZoomAction;
    QAction *zoomInAction;
    QAction *zoomOutAction;
    QAction *centerViewAction;
    QAction *toggleNamesAction;
    QAction *togglePortsAction;
    QAction *toggleSignalsAction;
    QAction *showPortsAction;
    QAction *exportPDFAction;
    QAction *alignXAction;
    QAction *alignYAction;
    QAction *rotateLeftAction;
    QAction *rotateRightAction;
    QAction *flipHorizontalAction;
    QAction *flipVerticalAction;
    QAction *aboutAction;
    QAction *helpAction;
    QAction *newVersionsAction;
    QAction *saveToWrappedCodeAction;
    QAction *createSimulinkWrapperAction;

    //Set and get methods for simulation parameters in toolbar
    void setStartTimeInToolBar(double startTime);
    void setTimeStepInToolBar(double timeStep);
    void setFinishTimeInToolBar(double finishTime);
    double getStartTimeFromToolBar();
    double getTimeStepFromToolBar();
    double getFinishTimeFromToolBar();
    void closeEvent(QCloseEvent *event);

    void showHelpPopupMessage(QString message);

    PyDockWidget *getPythonDock();

public slots:
    void show();
    void initializeWorkspace();
    void updateToolBarsToNewTab();
    void refreshUndoWidgetList();
    void fixSimulationParameterValues();
    void registerRecentModel(QFileInfo model);
    void updateRecentList();

private slots:
    void openPlotWidget();
    void openUndoWidget();
    void openSystemParametersWidget();
    void openRecentModel();
    void openArchiveURL();
    void updatePlotActionButton(bool);
    void updateSystemParametersActionButton(bool);

private:
    //Dock area widgets
    QDockWidget *mpMessageDock;
    QDockWidget *mpLibDock;
    QDockWidget *mpPlotWidgetDock;
    QDockWidget *mpUndoWidgetDock;
    QDockWidget *mpSystemParametersDock;

    //Methods that adjusts simulation parameters if they are illegal
    void fixFinishTime();
    void fixTimeStep();

    void createActions();
    void createMenus();
    void createToolbars();
};

#endif // MAINWINDOW_H
