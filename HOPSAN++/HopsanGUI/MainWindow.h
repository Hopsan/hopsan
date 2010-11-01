//$Id$

//!
//! \mainpage
//! Library used in Hopsan NG is 'qwt-5'
//!

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>

//#include <QComboBox>
//#include <QLabel>
//class QGridLayout;
//class QHBoxLayout;
//class QMenuBar;
//class QMenu;
//class QmpStatusBar;
//class QAction;
//class QString;
//class QPlainTextEdit;

class ProjectTabWidget;
class GraphicsView;
class GraphicsScene;
class LibraryWidget;
class PreferenceWidget;
class OptionsWidget;
class UndoWidget;
class MessageWidget;
class PlotWidget;
class PyDock;
class GlobalParametersWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QWidget *mpCentralwidget;
    QGridLayout *mpCentralgrid;
    QGridLayout *mpTabgrid;

    //Widgets that can be displayed in main window
    UndoWidget *mpUndoWidget;
    ProjectTabWidget *mpProjectTabs;
    LibraryWidget *mpLibrary;
    OptionsWidget *mpOptionsWidget;
    PreferenceWidget *mpPreferenceWidget;
    MessageWidget *mpMessageWidget;
    QPushButton *mpClearMessageWidgetButton;

    QStatusBar *mpStatusBar;
    PlotWidget *mpPlotWidget;
    GlobalParametersWidget *mpGlobalParametersWidget;

    //Settings variable - stored in and loaded from settings.txt
    bool mInvertWheel;
    bool mUseMulticore;
    int mProgressBarStep;
    bool mEnableProgressBar;
    QColor mBackgroundColor;
    bool mAntiAliasing;
    QStringList mUserLibs;
    bool mSnapping;
    QList<QFileInfo> mRecentModels;
    QStringList mLastSessionModels;
    QMap<QString, QString> mDefaultUnits;
    QMap< QString, QMap<QString, double> > mAlternativeUnits;

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
    QAction *openGlobalParametersAction;
    QAction *disableUndoAction;
    QAction *cutAction;
    QAction *copyAction;
    QAction *pasteAction;
    QAction *simulateAction;
    QAction *plotAction;
    QAction *loadLibsAction;
    QAction *preferencesAction;
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

    //Set and get methods for simulation parameters in toolbar
    void setStartTimeInToolBar(double startTime);
    void setTimeStepInToolBar(double timeStep);
    void setFinishTimeInToolBar(double finishTime);
    double getStartTimeFromToolBar();
    double getTimeStepFromToolBar();
    double getFinishTimeFromToolBar();

    QPushButton *mpBackButton;

    //QString libDir;

    void closeEvent(QCloseEvent *event);

public slots:
    void show();
    void updateToolBarsToNewTab();
    void refreshUndoWidgetList();
    void fixSimulationParameterValues();
    void registerRecentModel(QFileInfo model);
    void updateRecentList();
    void saveSettings();


private slots:
    void openPlotWidget();
    void openUndoWidget();
    void openGlobalParametersWidget();
    void loadSettings();

private:
    //Dock area widgets
    QDockWidget *mpMessageDock;
    QDockWidget *mpLibDock;
    QDockWidget *mpPlotWidgetDock;
    QDockWidget *mpUndoWidgetDock;
    PyDock      *mpPyDock;
    QDockWidget *mpGlobalParametersDock;

    //Methods that adjusts simulation parameters if they are illegal
    void fixFinishTime();
    void fixTimeStep();

    void createActions();
    void createMenus();
    void createToolbars();
};

#endif // MAINWINDOW_H
