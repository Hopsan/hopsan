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
#include <QtNetwork/QNetworkReply>

class ProjectTabWidget;
class GraphicsView;
class QGraphicsScene;
class LibraryWidget;
class OptionsDialog;
class MessageWidget;
class TerminalConsole;
class PlotTreeWidget;
class PyDockWidget;
class SystemParametersWidget;
class WelcomeWidget;
class Configuration;
class CopyStack;
class AboutDialog;
class WelcomeDialog;
class HelpDialog;
class OptimizationDialog;
class SensitivityAnalysisDialog;
class ComponentGeneratorDialog;
class TerminalWidget;
class UndoWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();


    OptionsDialog *getOptionsDialog();
    ComponentGeneratorDialog *getComponentGeneratorDialog();
    PyDockWidget *getPythonDock();

    //Set and get methods for simulation parameters in toolbar
    void setStartTimeInToolBar(const double startTime);
    void setTimeStepInToolBar(const double timeStep);
    void setStopTimeInToolBar(const double finishTime);
    void displaySimulationTimeParameters(const QString startTime, const QString timeStep, const QString stopTime);
    double getStartTimeFromToolBar();
    double getTimeStepFromToolBar();
    double getFinishTimeFromToolBar();

    void closeEvent(QCloseEvent *event);

    void showHelpPopupMessage(QString message);
    void hideHelpPopupMessage();

    //Configuration object
    Configuration *mpConfig;    //Public so that python wrappers can access it
                                //! @todo Why can't python wrappers use gpConfig?

    //Widgets
    //! @todo These should probably not be public
    UndoWidget *mpUndoWidget;
    ProjectTabWidget *mpProjectTabs;
    WelcomeWidget *mpWelcomeWidget;
    LibraryWidget *mpLibrary;
    MessageWidget *mpMessageWidget;
    TerminalWidget *mpTerminalWidget;
    PlotTreeWidget *mpPlotWidget;
    PyDockWidget *mpPyDockWidget;
    SystemParametersWidget *mpSystemParametersWidget;
    QStatusBar *mpStatusBar;  //Not used, but gives some nice extra space at bottom :)
    QGridLayout *mpCentralGridLayout;
    HelpDialog *mpHelpDialog;   //Should not be public but accessable from other widgets somehow

    //Actions (public because other widgets connect to them)
    QAction *mpNewAction;
    QAction *mpOpenAction;
    QAction *mpSaveAction;
    QAction *mpSaveAsAction;
    QAction *mpExportModelParametersAction;
    QAction *mpImportFMUAction;
    QAction *mpExportToSimulinkAction;
    QAction *mpExportToSimulinkCoSimAction;
    QAction *mpExportToFMUAction;
    QAction *mpExportToLabviewAction;
    QAction *mpLoadModelParametersAction;
    QAction *mpCloseAction;
    QAction *mpUndoAction;
    QAction *mpRedoAction;
    QAction *mpOpenUndoAction;
    QAction *mpOpenSystemParametersAction;
    QAction *mpDisableUndoAction;
    QAction *mpCutAction;
    QAction *mpCopyAction;
    QAction *mpPasteAction;
    QAction *mpSimulateAction;
    QAction *mpOpenDebuggerAction;
    QAction *mpCoSimulationAction;
    QAction *mpOptimizeAction;
    QAction *mpSensitivityAnalysisAction;
    QAction *mpPlotAction;
    QAction *mpLoadLibsAction;
    QAction *mpPropertiesAction;
    QAction *mpOptionsAction;
    QAction *mpAnimateAction;
    QAction *mpResetZoomAction;
    QAction *mpZoomInAction;
    QAction *mpZoomOutAction;
    QAction *mpCenterViewAction;
    QAction *mpToggleNamesAction;
    QAction *mpTogglePortsAction;
    QAction *mpToggleSignalsAction;
    QAction *mpShowPortsAction;
    QAction *mpExportGfxAction;
    QAction *mpPrintAction;
    QAction *mpExportPDFAction;
    QAction *mpExportPNGAction;
    QAction *mpAlignXAction;
    QAction *mpAlignYAction;
    QAction *mpRotateLeftAction;
    QAction *mpRotateRightAction;
    QAction *mpFlipHorizontalAction;
    QAction *mpFlipVerticalAction;
    QAction *mpIssueTrackerAction;
    QAction *mpAboutAction;
    QAction *mpHelpAction;
    QAction *mpReleaseNotesAction;
    QAction *mpWebsiteAction;
    QAction *mpNewVersionsAction;
    QAction *mpShowLossesAction;
    QAction *mpMeasureSimulationTimeAction;

    QAction *mpDebug1Action;
    QAction *mpDebug2Action;

public slots:
//    void show();
    void initializeWorkspace();
    void updateToolBarsToNewTab();
    void refreshUndoWidgetList();
    void setProjectSimulationTimeParameterValues();
    void registerRecentModel(QFileInfo model);
    void updateRecentList();
    void launchAutoUpdate();
    void openContextHelp();
    void openContextHelp(QString file);

signals:
    void simulateKeyPressed();

protected:
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void keyPressEvent(QKeyEvent *);

private slots:
    void simulateKeyWasPressed();
    void openPlotWidget();
    void openUndoWidget();
    void openSystemParametersWidget();
    void openRecentModel();
    void openHopsanURL();
    void openArchiveURL();
    void openIssueTrackerDialog();
    void updatePlotActionButton(bool);
    void updateSystemParametersActionButton(bool);
    void showToolBarHelpPopup();
    void openModelByAction();
    void updateDownloadProgressBar(qint64 bytesReceived, qint64 bytesTotal);
    void commenceAutoUpdate(QNetworkReply* reply);
    void showReleaseNotes();

private:
    void createActions();
    void createMenus();
    void createToolbars();

    void buildModelActionsMenu(QMenu *pParentMenu, QDir dir);


    //Dialogs
    OptionsDialog *mpOptionsDialog;
    WelcomeDialog *mpWelcomeDialog;
    AboutDialog *mpAboutDialog;
    OptimizationDialog *mpOptimizationDialog;
    SensitivityAnalysisDialog *mpSensitivityAnalysisDialog;
    ComponentGeneratorDialog *mpComponentGeneratorDialog;

    //Simulation setup line edits
    QLineEdit *mpStartTimeLineEdit;
    QLineEdit *mpTimeStepLineEdit;
    QLineEdit *mpStopTimeLineEdit;

    //Dock area widgets
    QDockWidget *mpMessageDock;
    QDockWidget *mpTerminalDock;
    QDockWidget *mpLibDock;
    QDockWidget *mpPlotWidgetDock;
    QDockWidget *mpUndoWidgetDock;
    QDockWidget *mpSystemParametersDock;

    QWidget *mpCentralWidget;
    QGridLayout *mpTabgrid;

    //Menubar items
    QMenuBar *mpMenuBar;
    QMenu *mpFileMenu;
    QMenu *mpNewMenu;
    QMenu *mpLibsMenu;
    QMenu *mpSimulationMenu;
    QMenu *mpEditMenu;
    QMenu *mpViewMenu;
    QMenu *mpToolsMenu;
    QMenu *mpImportMenu;
    QMenu *mpExportMenu;
    QMenu *mpPlotMenu;
    QMenu *mpRecentMenu;
    QMenu *mpHelpMenu;
    QMenu *mpExamplesMenu;
    QMenu *mpTestModelsMenu;

    //Buttons
    QToolButton *mpImportButton;
    QToolButton *mpExportButton;

    //Toolbar items
    QToolBar *mpFileToolBar;
    QToolBar *mpConnectivityToolBar;
    QToolBar *mpEditToolBar;
    QToolBar *mpToolsToolBar;
    QToolBar *mpSimToolBar;
    QToolBar *mpViewToolBar;
    QLabel *mpTimeLabelDeliminator1;
    QLabel *mpTimeLabelDeliminator2;

    //Help popup
    QWidget *mpHelpPopup;
    QLabel *mpHelpPopupIcon;
    QLabel *mpHelpPopupLabel;
    QHBoxLayout *mpHelpPopupLayout;
    QGroupBox *mpHelpPopupGroupBox;
    QHBoxLayout *mpHelpPopupGroupBoxLayout;
    QTimer *mpHelpPopupTimer;

    //Auto update
    QNetworkReply *mpDownloadStatus;
    QProgressDialog *mpDownloadDialog;
};

#endif // MAINWINDOW_H
