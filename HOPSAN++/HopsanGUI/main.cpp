/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

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
#include <QLocale>

#include "common.h"
#include "global.h"
#include "MainWindow.h"
#include "Configuration.h"
#include "CopyStack.h"
#include "DesktopHandler.h"

void loadApplicationFonts();

// Declare global pointers
MainWindow* gpMainWindow;
QWidget *gpMainWindowWidget;
Configuration *gpConfig;
DesktopHandler *gpDesktopHandler;
CopyStack *gpCopyStack;
QSplashScreen *gpSplash;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#ifdef USETBB
    qDebug() << "TBB is used!";
#endif

    //  Force locale to English/USA
    qDebug() << QLocale().languageToString(QLocale().language()) << " " << QLocale().countryToString(QLocale().country()) << " Decimal point: " << QLocale().decimalPoint();
    //! @todo this does not seem to help, DomElement.setAttribute still use comma decimal point on swedish ubuntu, maybe a Qt bug
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
    qDebug() << "Changing to: " << QLocale().languageToString(QLocale().language()) << " " << QLocale().countryToString(QLocale().country()) << " Decimal point: " << QLocale().decimalPoint();

    // Create/set global objects
    gpConfig = new Configuration();
    gpDesktopHandler = new DesktopHandler();
    gpDesktopHandler->setupPaths();
    gpCopyStack = new CopyStack();

    //Load settings
    loadApplicationFonts();

    //Create the splash screen
    QPixmap pixmap(QString(GRAPHICSPATH) + "splash.png");
    gpSplash = new QSplashScreen(pixmap, Qt::WindowStaysOnTopHint);
    //! @todo We need to delete it somehow, but still be able to check if it has been deleted or not (perhaps a QPointer will work)
    //gpSplash->setAttribute(Qt::WA_DeleteOnClose);
    gpSplash->showMessage("Starting Hopsan...");
    gpSplash->show();

    // Create the mainwindow
    MainWindow mainwindow;
    gpMainWindow = &mainwindow;
    gpMainWindowWidget = static_cast<QWidget*>(&mainwindow);
    mainwindow.createContents();

    //Show main window and initialize workspace
    QTimer::singleShot(20, &mainwindow, SLOT(showMaximized()));
    mainwindow.initializeWorkspace();

    // Execute application
    int rc = a.exec();

    // Deltete global objects
    delete gpCopyStack;
    delete gpDesktopHandler;
    delete gpConfig;

    // Return application return code
    return rc;
}


//! @todo Not sure if this shall be here or in main window, it has nothing to do with a main window and must be loaded before main window is loaded.
//! @todo This error checking may slow down startup, and it is probably never needed. Remove when this functionality is tested and verified.
void loadApplicationFonts()
{
    bool error=false;
    int i;
    i = QFontDatabase::addApplicationFont(QString(GRAPHICSPATH)+"uifonts/calibri.ttf");
    if(i == -1) error=true;
    i = QFontDatabase::addApplicationFont(QString(GRAPHICSPATH)+"uifonts/calibrib.ttf");
    if(i == -1) error=true;
    i = QFontDatabase::addApplicationFont(QString(GRAPHICSPATH)+"uifonts/calibrii.ttf");
    if(i == -1) error=true;
    i = QFontDatabase::addApplicationFont(QString(GRAPHICSPATH)+"uifonts/calibriz.ttf");
    if(i == -1) error=true;
    i = QFontDatabase::addApplicationFont(QString(GRAPHICSPATH)+"uifonts/consola.ttf");
    if(i == -1) error=true;
    i = QFontDatabase::addApplicationFont(QString(GRAPHICSPATH)+"uifonts/consolab.ttf");
    if(i == -1) error=true;
    i = QFontDatabase::addApplicationFont(QString(GRAPHICSPATH)+"uifonts/consolai.ttf");
    if(i == -1) error=true;
    i = QFontDatabase::addApplicationFont(QString(GRAPHICSPATH)+"uifonts/consolaz.ttf");
    if(i == -1) error=true;

    if(error)
        qDebug() << "Error loading fonts!";
    else
        qDebug() << "Successfully loaded fonts!";
}
