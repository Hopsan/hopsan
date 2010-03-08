//$Id$

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "plotwidget.h"
#include "MessageWidget.h"


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
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuNew;
    QMenu *menuLibs;
    QMenu *menuSimulation;
    QMenu *menuView;
    QMenu *menuPlot;
    MessageWidget *mpMessageWidget;
    QStatusBar *statusBar;
    QAction *actionOpen;
    QAction *actionSave;
    QAction *actionClose;
    QAction *actionProject;
    QAction *actionLoadLibs;
    QAction *actionSimulate;
    QAction *actionPlot;

    //QString libDir;

    //GraphicsScene *scene;
    //GraphicsView *view;

    //VariableListDialog *variableList;

    void closeEvent(QCloseEvent *event);

private slots:
    //void addLibs(QString libDir, QString parentLib=QString());
    //void addLibs();
    void plot();



};

#endif // MAINWINDOW_H
