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

#include "Widgets/UndoWidget.h"

class ProjectTabWidget;
class GraphicsView;
class QGraphicsScene;
class LibraryWidget;
class OptionsDialog;
class MessageWidget;
class PlotTreeWidget;
class PyDockWidget;
class SystemParametersWidget;
class Configuration;
class CopyStack;
class AboutDialog;
class WelcomeDialog;
class HelpDialog;
class OptimizationDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    //Set and get methods for simulation parameters in toolbar
    OptionsDialog *getOptionsDialog();
    PyDockWidget *getPythonDock();

    QLineEdit *getStartTimeLineEdit();
    QLineEdit *getTimeStepLineEdit();
    QLineEdit *getFinishTimeLineEdit();
    void setStartTimeInToolBar(double startTime);
    void setTimeStepInToolBar(double timeStep);
    void setFinishTimeInToolBar(double finishTime);
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
    LibraryWidget *mpLibrary;
    MessageWidget *mpMessageWidget;
    PlotTreeWidget *mpPlotWidget;
    PyDockWidget *mpPyDockWidget;
    SystemParametersWidget *mpSystemParametersWidget;
    QStatusBar *mpStatusBar;  //Not used, but gives some nice extra space at bottom :)

    //Actions (public because other widgets connect to them)
    QAction *mpNewAction;
    QAction *mpOpenAction;
    QAction *mpSaveAction;
    QAction *mpSaveAsAction;
    QAction *mpImportFMUAction;
    QAction *mpExportToSimulinkAction;
    QAction *mpExportToFMUAction;
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
    QAction *mpOptimizeAction;
    QAction *mpPlotAction;
    QAction *mpLoadLibsAction;
    QAction *mpPropertiesAction;
    QAction *mpOptionsAction;
    QAction *mpResetZoomAction;
    QAction *mpZoomInAction;
    QAction *mpZoomOutAction;
    QAction *mpCenterViewAction;
    QAction *mpToggleNamesAction;
    QAction *mpTogglePortsAction;
    QAction *mpToggleSignalsAction;
    QAction *mpShowPortsAction;
    QAction *mpExportPDFAction;
    QAction *mpAlignXAction;
    QAction *mpAlignYAction;
    QAction *mpRotateLeftAction;
    QAction *mpRotateRightAction;
    QAction *mpFlipHorizontalAction;
    QAction *mpFlipVerticalAction;
    QAction *mpAboutAction;
    QAction *mpHelpAction;
    QAction *mpWebsiteAction;
    QAction *mpNewVersionsAction;
    QAction *mpSaveToWrappedCodeAction;
    QAction *mpCreateSimulinkWrapperAction;
    QAction *mpShowLossesAction;

    QAction *mpDebugAction;

public slots:
    void show();
    void initializeWorkspace();
    void updateToolBarsToNewTab();
    void refreshUndoWidgetList();
    void fixSimulationParameterValues();
    void registerRecentModel(QFileInfo model);
    void updateRecentList();
    void launchAutoUpdate();

protected:
    virtual void mouseMoveEvent(QMouseEvent *);

private slots:
    void openPlotWidget();
    void openUndoWidget();
    void openSystemParametersWidget();
    void openRecentModel();
    void openHopsanURL();
    void openArchiveURL();
    void updatePlotActionButton(bool);
    void updateSystemParametersActionButton(bool);
    void showToolBarHelpPopup();
    void openExampleModel();
    void updateDownloadProgressBar(qint64 bytesReceived, qint64 bytesTotal);
    void commenceAutoUpdate(QNetworkReply* reply);

private:
    //Methods that adjusts simulation parameters if they are illegal
    void fixFinishTime();
    void fixTimeStep();

    void createActions();
    void createMenus();
    void createToolbars();

    //Dialogs
    OptionsDialog *mpOptionsDialog;
    WelcomeDialog *mpWelcomeDialog;
    AboutDialog *mpAboutDialog;
    HelpDialog *mpHelpDialog;
    OptimizationDialog *mpOptimizationDialog;

    //Simulation setup line edits
    QLineEdit *mpStartTimeLineEdit;
    QLineEdit *mpTimeStepLineEdit;
    QLineEdit *mpFinishTimeLineEdit;

    //Dock area widgets
    QDockWidget *mpMessageDock;
    QDockWidget *mpLibDock;
    QDockWidget *mpPlotWidgetDock;
    QDockWidget *mpUndoWidgetDock;
    QDockWidget *mpSystemParametersDock;

    QWidget *mpCentralWidget;
    QGridLayout *mpCentralGridLayout;
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
    QMenu *mpPlotMenu;
    QMenu *mpRecentMenu;
    QMenu *mpHelpMenu;
    QMenu *mpImportMenu;
    QMenu *mpExportMenu;
    QMenu *mpExamplesMenu;

    //Buttons
    QToolButton *mpImportButton;
    QToolButton *mpExportButton;

    //Toolbar items
    QToolBar *mpFileToolBar;
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
