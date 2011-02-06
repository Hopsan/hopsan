//!
//! @file   main.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief The main program for the HopsanGUI application
//!
//$Id$

#include <QtGui/QApplication>
#include <QSplashScreen>
#include <QTimer>

#include "common.h"
#include "MainWindow.h"
#include "Dialogs/WelcomeDialog.h"

//Global stuff
MainWindow* gpMainWindow = 0;
QString gExecPath;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    gExecPath = qApp->arguments().first();
#ifdef WIN32
    gExecPath.chop(13);//removes HOPSANGUI.exe
#else
    gExecPath.chop(9);//removes HOPSANGUI
#endif

    QPixmap pixmap(QString(GRAPHICSPATH) + "splash.png");
    QSplashScreen splash(pixmap);
    splash.show();
    //splash.raise();

    MainWindow mainwindow;

    QTimer::singleShot(750, &splash, SLOT(close()));
    QTimer::singleShot(400, &mainwindow, SLOT(show()));
    QTimer::singleShot(751, &mainwindow, SLOT(initializeWorkspace()));

    //splash.finish(&mainwindow);

    return a.exec();
}
