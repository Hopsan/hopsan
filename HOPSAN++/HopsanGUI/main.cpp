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
#include "MainWindow.h"
#include "Configuration.h"
#include "CopyStack.h"
#include "Dialogs/WelcomeDialog.h"

//Global stuff
MainWindow* gpMainWindow = 0;
QString gExecPath;
QString gModelsPath;
QString gScriptsPath;

void loadApplicationFonts();

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //Force locale to English/USA
    qDebug() << QLocale().languageToString(QLocale().language()) << " " << QLocale().countryToString(QLocale().country()) << " Decimal point: " << QLocale().decimalPoint();
    //! @todo this does not seem to help, DomElement.setAttribute still use comma decimal point on swedish ubuntu, maybe a Qt bug
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
    qDebug() << "Changing to: " << QLocale().languageToString(QLocale().language()) << " " << QLocale().countryToString(QLocale().country()) << " Decimal point: " << QLocale().decimalPoint();

    // Make sure backup folder exists, create it if not
    QDir backupDir(BACKUPPATH);
    if (!backupDir.exists())
    {
        backupDir.mkpath(BACKUPPATH);
    }

    //Create global variables
    gExecPath = qApp->applicationDirPath().append('/');
    gConfig = Configuration();
    gCopyStack = CopyStack();


//    // Use development place for models and scripts
//    gModelsPath = MODELS_DEV_PATH;
//    gScriptsPath = SCRIPTS_DEV_PATH;

    // Make sure model folder exists, create it if not
    QDir modelsDir(MODELS_REL_PATH);
    if (!modelsDir.exists())
    {
        modelsDir.mkpath(MODELS_REL_PATH);
        if(modelsDir.exists())
        {
            gModelsPath = MODELS_REL_PATH;
        }
        else
        {
            gModelsPath = MODELS_DEV_PATH;
        }

    }

    // Select which scripts path to use
    //! @todo problem in linux if scripts must be changed, as they  are not installed to user home
    QDir scriptsDir(SCRIPTS_REL_PATH);
    if (!scriptsDir.exists())
    {
        scriptsDir.mkpath(SCRIPTS_REL_PATH);
        if(scriptsDir.exists())
        {
            gScriptsPath = SCRIPTS_REL_PATH;
        }
        else
        {
            gScriptsPath = SCRIPTS_DEV_PATH;
        }
    }


    //Load settings
    loadApplicationFonts();

    //Create the splash screen
    QPixmap pixmap(QString(GRAPHICSPATH) + "splash.png");
    QSplashScreen splash(pixmap);
    splash.show();

    //Create the mainwindow
    MainWindow mainwindow;

    //Show splash screen, show main window and initialize workspace
    QTimer::singleShot(1000, &splash, SLOT(close()));
    QTimer::singleShot(20, &mainwindow, SLOT(showMaximized()));
    //mainwindow.initializeWorkspace();

    return a.exec();
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
