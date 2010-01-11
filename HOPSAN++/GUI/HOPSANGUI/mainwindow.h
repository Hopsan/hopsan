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

#include "treewidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QWidget *centralwidget;
    QGridLayout *centralgrid;
    QTabWidget *projectTabs;
    QWidget *tab;
    QGridLayout *tabgrid;
    TreeWidget *componentsTree;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuNew;
    QMenu *menuLibs;
    QMenu *menuSimulation;
    QStatusBar *statusBar;
    QAction *actionOpen;
    QAction *actionClose;
    QAction *actionProject;
    QAction *actionLoadLibs;
    QAction *actionSimulate;

    QString fileName;
    QString libDir;
    //QDir libDirObject;

private slots:
    void addProject();
    void addLibs();



};

#endif // MAINWINDOW_H
