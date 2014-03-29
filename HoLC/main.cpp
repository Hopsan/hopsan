#include <QApplication>
#include <QPixmap>
#include <QSplashScreen>
#include <QTimer>

#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QPixmap pixmap(":/gfx/graphics/splash.png");
    QSplashScreen *pSplash = new QSplashScreen(pixmap, Qt::WindowStaysOnTopHint);
    pSplash->show();
    pSplash->setAttribute(Qt::WA_DeleteOnClose, true);
    QTimer::singleShot(2000, pSplash, SLOT(close()));

    MainWindow w;
    QTimer::singleShot(200, &w, SLOT(showMaximized()));

    return a.exec();
}
