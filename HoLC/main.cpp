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
    pSplash->showMessage("Starting HoLC...");
    pSplash->show();
    pSplash->setAttribute(Qt::WA_DeleteOnClose, true);
    QTimer::singleShot(2000, pSplash, SLOT(close()));

    MainWindow w;
    w.showMaximized();

    return a.exec();
}
