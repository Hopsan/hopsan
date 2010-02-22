#include <QtGui/QApplication>
#include <QSplashScreen>
#include <QTimer>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QPixmap pixmap("../../HopsanGUI/splash.png");
    QSplashScreen splash(pixmap);
    splash.show();

    splash.raise();

    MainWindow mainwindow;

    QTimer::singleShot(2500, &splash, SLOT(close()));
    //QTimer::singleShot(2600, &mainwindow, SLOT(show()));

    mainwindow.show();
    splash.finish(&mainwindow);
    return a.exec();




/*    QApplication a(argc, argv);
    MainWindow mainwindow;
    mainwindow.show();
    return a.exec();*/
}
