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
#include <QLineEdit>
#include <QGridLayout>
#include <QFileInfo>
#include <QDir>
#include <QLabel>
#include <QToolButton>

class CentralTabWidget;
class GraphicsView;
class QGraphicsScene;
class LibraryWidget;
class OptionsDialog;
class MessageWidget;
class TerminalConsole;
class PlotWidget2;
class PyDockWidget;
class SystemParametersWidget;
class WelcomeWidget;
class Configuration;
class CopyStack;
class AboutDialog;
class HelpDialog;
class OptimizationDialog;
class SensitivityAnalysisDialog;
class TerminalWidget;
class UndoWidget;
class HVCWidget;
class ModelHandler;
class MainWindowLineEdit;
class DataExplorer;
class FindWidget;
class HelpPopUpWidget;


class SimulationTimeEdit : public QWidget
{
    Q_OBJECT
public:
    SimulationTimeEdit(QWidget *pParent);

    void getSimulationTime(QString &rStartTime, QString &rTimeStep, QString &rStopTime) const;
    QString getStartTime() const;
    QString getTimeStep() const;
    QString getStopTime() const;

    void displayStartTime(const QString startTime);
    void displayTimeStep(const QString timeStep);
    void dispalyStopTime(const QString stopTime);

    void clearFocus();

public slots:
    void displaySimulationTime(const QString startTime, const QString timeStep, const QString stopTime);

signals:
    void simulationTimeChanged(QString start, QString ts, QString stop);
    void mouseEnterEvent();

protected:
    bool eventFilter(QObject *object, QEvent *event);

private slots:
    void emitSimTime();

private:
    QLineEdit *mpStartTimeLineEdit;
    QLineEdit *mpTimeStepLineEdit;
    QLineEdit *mpStopTimeLineEdit;
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void createContents();

    OptionsDialog *getOptionsDialog();

    // Get methods for simulation parameters in toolbar
    double getStartTimeFromToolBar();
    double getTimeStepFromToolBar();
    double getFinishTimeFromToolBar();

    void closeEvent(QCloseEvent *event);

    void showHelpPopupMessage(const QString &rMessage);
    void hideHelpPopupMessage();

    //Widgets
    //! @todo These should probably not be public
    UndoWidget *mpUndoWidget;
    CentralTabWidget *mpCentralTabs;
    ModelHandler *mpModelHandler;
    WelcomeWidget *mpWelcomeWidget;
    LibraryWidget *mpLibraryWidget;
    SystemParametersWidget *mpSystemParametersWidget;
    QStatusBar *mpStatusBar;  //Not used, but gives some nice extra space at bottom :)
    QGridLayout *mpCentralGridLayout;

    HVCWidget *mpHVCWidget;
    DataExplorer *mpDataExplorer;
    OptimizationDialog *mpOptimizationDialog;
    SimulationTimeEdit *mpSimulationTimeEdit;

    //Actions (public because other widgets connect to them)
    QAction *mpNewAction;
    QAction *mpOpenAction;
    QAction *mpSaveAction;
    QAction *mpSaveAsAction;
    QAction *mpExportModelParametersAction;
    QAction *mpImportFMUAction;
    QAction *mpImportDataFileAction;
    QAction *mpExportToSimulinkAction;
    QAction *mpExportToSimulinkCoSimAction;
    QAction *mpExportToFMU1_32Action;
    QAction *mpExportToFMU1_64Action;
    QAction *mpExportToFMU2_32Action;
    QAction *mpExportToFMU2_64Action;
    QAction *mpExportToLabviewAction;
    QAction *mpLoadModelParametersAction;
    QAction *mpCloseAction;
    QAction *mpUndoAction;
    QAction *mpRedoAction;
    QAction *mpOpenUndoAction;
    QAction *mpOpenSystemParametersAction;
    QAction *mpOpenHvcWidgetAction;
    QAction *mpOpenDataExplorerAction;
    QAction *mpOpenFindWidgetAction;
    QAction *mpDisableUndoAction;
    QAction *mpCutAction;
    QAction *mpCopyAction;
    QAction *mpPasteAction;
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
    QAction *mpExportGfxAction;
    QAction *mpPrintAction;
    QAction *mpExportPDFAction;
    QAction *mpExportPNGAction;
    QAction *mpAlignXAction;
    QAction *mpAlignYAction;
    QAction *mpDistributeXAction;
    QAction *mpDistributeYAction;
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
    QAction *mpToggleHideAllDockAreasAction;

    QAction *mpDebug1Action;
    QAction *mpDebug2Action;

    QDockWidget *mpTerminalDock;

public slots:
//    void show();
    void initializeWorkspace();
    void updateToolBarsToNewTab();
    void refreshUndoWidgetList();
    void updateRecentList();

signals:
    void simulateKeyPressed();

protected:
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void keyPressEvent(QKeyEvent *);

private slots:
    void simulateKeyWasPressed();
    void openHVCWidget();
    void openDataExplorerWidget();
    void openFindWidget();
    void toggleVisiblePlotWidget();
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
    void showReleaseNotes();
    void toggleHideShowDockAreas(bool show);

private:
    void createActions();
    void createMenus();
    void createToolbars();

    void buildModelActionsMenu(QMenu *pParentMenu, QDir dir);

    // Private Actions
    QAction *mpSimulateAction;

    QToolButton *mpExportToFMUMenuButton;

    // Dialogs
    OptionsDialog *mpOptionsDialog;
    AboutDialog *mpAboutDialog;
    SensitivityAnalysisDialog *mpSensitivityAnalysisDialog;

    // Dock area widgets
    QDockWidget *mpMessageDock;
    QDockWidget *mpLibDock;
    QDockWidget *mpPlotWidgetDock;
    QDockWidget *mpUndoWidgetDock;
    QDockWidget *mpSystemParametersDock;
    QDockWidget *mpPyDockWidget;

    QWidget *mpCentralWidget;
    QGridLayout *mpTabgrid;

    // Widgets
    MessageWidget *mpMessageWidget;
    HelpDialog *mpHelpDialog;

    // Menubar items
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
    QMenu *mpExportToFMUMenu;
    QMenu *mpPlotMenu;
    QMenu *mpRecentMenu;
    QMenu *mpHelpMenu;
    QMenu *mpExamplesMenu;
    QMenu *mpTestModelsMenu;

    // Buttons
    QToolButton *mpImportButton;
    QToolButton *mpExportButton;

    // Toolbar items
    QToolBar *mpFileToolBar;
    QToolBar *mpConnectivityToolBar;
    QToolBar *mpEditToolBar;
    QToolBar *mpToolsToolBar;
    QToolBar *mpSimToolBar;
    QToolBar *mpViewToolBar;
    QLabel *mpTimeLabelDeliminator1;
    QLabel *mpTimeLabelDeliminator2;

    // Help popup
    HelpPopUpWidget *mpHelpPopup;
    QMap<QAction*, QString> mHelpPopupTextMap;
};

#endif // MAINWINDOW_H
