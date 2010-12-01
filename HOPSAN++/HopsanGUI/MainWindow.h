//$Id$

//!
//! \mainpage
//! Library used in Hopsan NG is 'qwt-5'
//!

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
//class UndoWidget;
class MessageWidget;
class PlotWidget;
class PyDockWidget;
class SystemParametersWidget;
class Configuration;
class CopyStack;
class AboutDialog;

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
    //ContainerPropertiesDialog *mpSystemPropertiesDialog;
    MessageWidget *mpMessageWidget;

    AboutDialog *mpAboutDialog;

    QStatusBar *mpStatusBar;
    PlotWidget *mpPlotWidget;
    SystemParametersWidget *mpSystemParametersWidget;

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

    //Toolbar items
    QToolBar *mpFileToolBar;
    QToolBar *mpEditToolBar;
    QToolBar *mpSimToolBar;
    QToolBar *mpSimulationToolBar;
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
    QAction *hideNamesAction;
    QAction *showNamesAction;
    QAction *hidePortsAction;
    QAction *showPortsAction;
    QAction *exportPDFAction;
    QAction *aboutAction;

    //Set and get methods for simulation parameters in toolbar
    void setStartTimeInToolBar(double startTime);
    void setTimeStepInToolBar(double timeStep);
    void setFinishTimeInToolBar(double finishTime);
    double getStartTimeFromToolBar();
    double getTimeStepFromToolBar();
    double getFinishTimeFromToolBar();
    void closeEvent(QCloseEvent *event);

    PyDockWidget *getPythonDock();

public slots:
    void show();
    void updateToolBarsToNewTab();
    void refreshUndoWidgetList();
    void fixSimulationParameterValues();
    void registerRecentModel(QFileInfo model);
    void updateRecentList();
    void makeSurePlotWidgetIsCreated();

private slots:
    void openPlotWidget();
    void openUndoWidget();
    void openSystemParametersWidget();

private:
    //Dock area widgets
    QDockWidget *mpMessageDock;
    QDockWidget *mpLibDock;
    QDockWidget *mpPlotWidgetDock;
    QDockWidget *mpUndoWidgetDock;
    PyDockWidget *mpPyDockWidget;
    QDockWidget *mpSystemParametersDock;

    //Methods that adjusts simulation parameters if they are illegal
    void fixFinishTime();
    void fixTimeStep();

    void createActions();
    void createMenus();
    void createToolbars();
};

#endif // MAINWINDOW_H
