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
    QAction *mpSaveAndRunAction;
    QAction *mpNewScriptAction;
    QAction *mpOpenTextFileAction;
    QAction *mpExportSimulationStateAction;
    QAction *mpExportModelParametersActionToSsv;
    QAction *mpExportModelParametersActionToHpf;
    QAction *mpImportFMUAction;
    QAction *mpImportDataFileAction;
    QAction *mpExportToSimulinkAction;
    QAction *mpExportToFMU1_32Action;
    QAction *mpExportToFMU1_64Action;
    QAction *mpExportToFMU2_32Action;
    QAction *mpExportToFMU2_64Action;
    QAction *mpExportToFMU3_32Action;
    QAction *mpExportToFMU3_64Action;
    QAction *mpExportToLabviewAction;
    QAction *mpExportToExe_32Action;
    QAction *mpExportToExe_64Action;
    QAction *mpLoadModelParametersFromSsvAction;
    QAction *mpLoadModelParametersFromHpfAction;
    QAction *mpCloseAction;
    QAction *mpUndoAction;
    QAction *mpRedoAction;
    QAction *mpOpenUndoAction;
    QAction *mpOpenSystemParametersAction;
    QAction *mpOpenHvcWidgetAction;
    QAction *mpOpenDataExplorerAction;
    QAction *mpOpenFindWidgetAction;
    QAction *mpRevertModelAction;
    QAction *mpNumHopAction;
    QAction *mpEnableUndoAction;
    QAction *mpCutAction;
    QAction *mpCopyAction;
    QAction *mpPasteAction;
    QAction *mpOpenDebuggerAction;
    QAction *mpToggleRemoteCoreSimAction;
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
    QAction *mpLicenseAction;
    QAction *mpHelpAction;
    QAction *mpReleaseNotesAction;
    QAction *mpWebsiteAction;
    QAction *mpTutorialsAction;
    QAction *mpNewVersionsAction;
    QAction *mpShowLossesAction;
    QAction *mpMeasureSimulationTimeAction;
    QAction *mpToggleHideAllDockAreasAction;
    QAction *mpNewDcpModelAction;
    QAction *mpStartDcpMasterAction;
    QAction *mpStartDcpServerAction;

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
    void showSplashScreenMessage(QString);

protected:
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void keyPressEvent(QKeyEvent *);

private slots:
    void simulateKeyWasPressed();
    void openHVCWidget();
    void openDataExplorerWidget();
    void openFindWidget();
    void revertModel();
    void openNumHopDialog();
    void toggleVisiblePlotWidget();
    void openUndoWidget();
    void openSystemParametersWidget();
    void openRecentModel();
    void openHopsanURL();
    void openHopsanTutorialURL();
    void openArchiveURL();
    void openIssueTrackerDialog();
    void updatePlotActionButton(bool);
    void updateSystemParametersActionButton(bool);
    void showToolBarHelpPopup();
    void openModelByAction();
    void showReleaseNotes();
    void toggleHideShowDockAreas(bool show);
    void openLicenseDialog();

private:
    void createActions();
    void createMenus();
    void createToolbars();

    void buildModelActionsMenu(QMenu *pParentMenu, QDir dir);

    // Private Actions
    QAction *mpSimulateAction;

    QToolButton *mpExportToFMUMenuButton;
    QToolButton *mpExportToExeMenuButton;
    QToolButton *mpExportModelParametersMenuButton;
    QToolButton *mpImportModelParametersMenuButton;

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
    QMenu *mpExportToExeMenu;
    QMenu *mpPlotMenu;
    QMenu *mpRecentMenu;
    QMenu *mpHelpMenu;
    QMenu *mpExamplesMenu;
    QMenu *mpTestModelsMenu;
    QMenu *mpExportModelParametersMenu;
    QMenu *mpImportModelParametersMenu;

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
    QToolBar *mpDcpToolBar;
    QLabel *mpTimeLabelDeliminator1;
    QLabel *mpTimeLabelDeliminator2;

    // Help popup
    HelpPopUpWidget *mpHelpPopup;
    QMap<QObject*, QString> mHelpPopupTextMap;
};

#endif // MAINWINDOW_H
