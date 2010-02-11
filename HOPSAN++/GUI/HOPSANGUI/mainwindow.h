#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QtGui/QWidget>
#include <QtGui/QGridLayout>
#include <QtGui/QTabWidget>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QMenuBar>
#include <QtGui/QMenu>
#include <QtGui/QStatusBar>
#include <QtGui/QAction>
#include <QtCore/QMetaObject>
#include <QtCore/QString>
#include <QtCore/QDir>
#include <QtCore/QStringList>
#include <QtCore/QIODevice>
#include <QListWidgetItem>
#include <QStringList>

#include "treewidget.h"
#include "treewidgetitem.h"
//#include "graphicsview.h"
//#include "graphicsscene.h"
#include "listwidget.h"
#include "listwidgetitem.h"
#include "ProjectTabWidget.h"
#include "LibraryWidget.h"

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
    QStatusBar *statusBar;
    QAction *actionOpen;
    QAction *actionSave;
    QAction *actionClose;
    QAction *actionProject;
    QAction *actionLoadLibs;
    QAction *actionSimulate;

    QString libDir;

    GraphicsScene *scene;
    GraphicsView *view;

    void closeEvent(QCloseEvent *event);

private slots:
    void addLibs();



};

#endif // MAINWINDOW_H
