//$Id$

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
//#include <QtGui/QWidget>
//#include <QtGui/QGridLayout>
//#include <QtGui/QTabWidget>
//#include <QtGui/QTreeWidget>
//#include <QtGui/QTreeWidgetItem>
//#include <QtGui/QMenuBar>
//#include <QtGui/QMenu>
//#include <QtGui/QStatusBar>
//#include <QtGui/QAction>
//#include <QtCore/QMetaObject>
//#include <QtCore/QString>
//#include <QtCore/QDir>
//#include <QtCore/QStringList>
//#include <QtCore/QIODevice>
//#include <QListWidgetItem>
//#include <QStringList>
//#include <QDockWidget>

//#include "treewidget.h"
//#include "treewidgetitem.h"
//#include "graphicsview.h"
//#include "graphicsscene.h"
//#include "listwidget.h"
//#include "listwidgetitem.h"
//#include "ProjectTabWidget.h"
//#include "LibraryWidget.h"
#include "plotwidget.h"


class QGridLayout;
class QMenuBar;
class QMenu;
class QStatusBar;
class QAction;
class QString;
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

    QWidget *centralwidget;
    QGridLayout *centralgrid;
    ProjectTabWidget *projectTabs;
    QGridLayout *tabgrid;
    LibraryWidget *library;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuNew;
    QMenu *menuLibs;
    QMenu *menuSimulation;
    QMenu *menuView;
    QMenu *menuPlot;
    QStatusBar *statusBar;
    QAction *actionOpen;
    QAction *actionSave;
    QAction *actionClose;
    QAction *actionProject;
    QAction *actionLoadLibs;
    QAction *actionSimulate;
    QAction *actionPlot;

    QString libDir;

    GraphicsScene *scene;
    GraphicsView *view;

    //VariableListDialog *variableList;

    void closeEvent(QCloseEvent *event);

private slots:
    void addLibs(QString libDir, QString parentLib=QString());
    void addLibs();
    void plot();



};

#endif // MAINWINDOW_H
