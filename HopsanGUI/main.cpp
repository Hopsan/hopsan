/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

//!
//! @file   HopsanGUI/main.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief The main program for the HopsanGUI application
//!
//$Id$

#include <clocale>

#include <QApplication>
#include <QSplashScreen>
#include <QTimer>
#include <QLocale>
#include <QDebug>
#include <QFontDatabase>

#include "common.h"
#include "global.h"
#include "version_gui.h"
#include "MainWindow.h"
#include "Configuration.h"
#include "CopyStack.h"
#include "DesktopHandler.h"
#include "MessageHandler.h"
#include "CoreUtilities/HmfLoader.h"
#include "Widgets/HcomWidget.h"
#include "HcomHandler.h"
#include "Dialogs/LicenseDialog.h"

// Declare global pointers
MainWindow* gpMainWindow = 0;
QWidget *gpMainWindowWidget = 0;
Configuration *gpConfig = 0;
DesktopHandler *gpDesktopHandler = 0;
CopyStack *gpCopyStack = 0;
QSplashScreen *gpSplash = 0;
GUIMessageHandler *gpMessageHandler = 0;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<GUIMessage>("GUIMessage");

#ifdef USETBB
    qDebug() << "TBB is used!";
#endif

    // Forcing numeric locale to C only using QLocale::setDefault() does not seem to help
    std::setlocale(LC_NUMERIC, "C");
    std::lconv* lc = std::localeconv();
    qDebug() << "Decimal point: " << lc->decimal_point << " thousand separator: \"" << lc->thousands_sep << "\"";
    //  Force QLocale to English/USA
    qDebug() << "QLocale: " << QLocale().languageToString(QLocale().language()) << " " << QLocale().countryToString(QLocale().country()) << " Decimal point: " << QLocale().decimalPoint();
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
    qDebug() << "Changing QLocale to: " << QLocale().languageToString(QLocale().language()) << " " << QLocale().countryToString(QLocale().country()) << " Decimal point: " << QLocale().decimalPoint();
    //! @todo this did not help before, DomElement.setAttribute still used comma decimal point on Swedish Ubuntu, now after adding std::setLocale above it might actually work.
    lc = std::localeconv();
    qDebug() << "Decimal point: " << lc->decimal_point << " thousand separator: \"" << lc->thousands_sep << "\"";

    // Create the mainwindow
    MainWindow mainwindow;
    gpMainWindow = &mainwindow;
    gpMainWindowWidget = static_cast<QWidget*>(&mainwindow);

    // Create the splash screen
    QPixmap pixmap(QString(GRAPHICSPATH) + "splash.png");
    gpSplash = new QSplashScreen(&mainwindow, pixmap/*, Qt::WindowStaysOnTopHint*/);
    //! @todo We need to delete it somehow, but still be able to check if it has been deleted or not (perhaps a QPointer will work)
    //gpSplash->setAttribute(Qt::WA_DeleteOnClose);
    gpSplash->showMessage("Starting Hopsan...");
    gpSplash->show();

    // Create/set global objects
    gpDesktopHandler = new DesktopHandler();
    gpDesktopHandler->setupPaths();
    gpConfig = new Configuration();
    gpCopyStack = new CopyStack();
    gpMessageHandler = new GUIMessageHandler();

    gpConfig->connect(gpConfig, SIGNAL(recentModelsListChanged()), gpMainWindow, SLOT(updateRecentList()));

    //Create contents in MainWindow
    mainwindow.createContents();

    // Show main window and initialize workspace
    //QTimer::singleShot(20, &mainwindow, SLOT(showMaximized()));
    mainwindow.initializeWorkspace();
    mainwindow.showMaximized();

    // Process any received messages
    gpMessageHandler->startPublish();

    // Show license dialog
    if (gpConfig->getBoolSetting(CFG_SHOWLICENSEONSTARTUP))
    {
        (new LicenseDialog(gpMainWindowWidget))->show();
        // Note! it will delete on close automatically
    }

    // Read command line arguments, search for hcom scripts, ignore everything else
    //! @todo maybe use TCLAP here
    QStringList args = a.arguments();
    for(QString &arg : args)
    {
        if(arg.endsWith(".hcom"))
        {
            QFileInfo fi(arg);
            if (fi.isRelative())
            {
                arg = gpDesktopHandler->getExecPath()+"/"+arg;
            }
            gpTerminalWidget->mpHandler->executeCommand("exec "+arg);
        }
    }

    // Execute application
    int rc = a.exec();

    // Delete global objects after program execution is finished
    delete gpCopyStack;
    delete gpDesktopHandler;
    delete gpConfig;

    // Return application return code
    return rc;
}


//! @brief Returns the date and time when the HopsanGUI application was built
const char* getHopsanGUIBuildTime()
{
    return __DATE__ " " __TIME__;
}


bool isHospanGUIVersionHigherThan(const char *version)
{
    return hopsan::isVersionGreaterThan(QString(version).toStdString().c_str(), QString(HOPSANGUIVERSION).toStdString().c_str());
}
