//$Id$

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "plotwidget.h"
#include "MessageWidget.h"
#include "PreferenceWidget.h"
#include "OptionsWidget.h"
#include "UndoStack.h"
#include <QComboBox>



class QGridLayout;
class QHBoxLayout;
class QMenuBar;
class QMenu;
class QStatusBar;
class QAction;
class QString;
class QPlainTextEdit;
class SimulationSetupWidget;
class ProjectTabWidget;
class GraphicsView;
class GraphicsScene;
class LibraryWidget;
class PreferenceWidget;
class OptionsWidget;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QWidget *mpCentralwidget;
    QGridLayout *mpCentralgrid;
    ProjectTabWidget *mpProjectTabs;
    QGridLayout *mpTabgrid;
    LibraryWidget *mpLibrary;
    SimulationSetupWidget *mpSimulationSetupWidget;
    OptionsWidget *mpOptionsWidget;
    PreferenceWidget *mpPreferenceWidget;
    UndoWidget *mpUndoWidget;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuNew;
    QMenu *menuLibs;
    QMenu *menuSimulation;
    QMenu *menuEdit;
    QMenu *menuView;
    QMenu *menuTools;
    QMenu *menuPlot;
    MessageWidget *mpMessageWidget;
    QStatusBar *statusBar;

    QPushButton *mpBackButton;

    QAction *newAction;
    QAction *openAction;
    QAction *saveAction;
    QAction *saveAsAction;
    QAction *closeAction;
    QAction *undoAction;
    QAction *redoAction;
    QAction *openUndoAction;
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
    QAction *hideNamesAction;
    QAction *showNamesAction;
    QAction *hidePortsAction;
    QAction *showPortsAction;

    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QToolBar *simToolBar;
    QToolBar *mpSimulationToolBar;
    QToolBar *viewToolBar;

    //QComboBox *viewScaleCombo;


    //QString libDir;

    //GraphicsScene *scene;
    //GraphicsView *view;

    //VariableListDialog *variableList;

    void closeEvent(QCloseEvent *event);

private slots:
    //void addLibs(QString libDir, QString parentLib=QString());
    //void addLibs();
    void plot();
    void openPreferences();
    void openOptions();
    void openUndo();

private:
    void createActions();
    void createMenus();
    void createToolbars();
    QDockWidget *messagedock;
    QDockWidget *libdock;


};

#endif // MAINWINDOW_H
