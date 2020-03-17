/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

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

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   HopsanGUI/main.cpp
//!
//! @brief The main program for the HopsanGUI application
//!
//$Id$

#include <locale>
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
#include "CoreAccess.h"
#include "BuiltinTests.h"

// Declare global pointers
Configuration *gpConfig = 0;
DesktopHandler *gpDesktopHandler = 0;
CopyStack *gpCopyStack = 0;
GUIMessageHandler *gpMessageHandler = 0;

MainWindow* gpMainWindow = 0;
QWidget *gpMainWindowWidget = 0;
QSplashScreen *gpSplash = 0;

// Define global CoreVersion string
QString gHopsanCoreVersion = getHopsanCoreVersion();

int main(int argc, char *argv[])
{
    // Init application
    int applicationReturnCode = 0;
    QApplication app(argc, argv);

    // Ensure C locale is used
    // Forcing numeric locale to C only using QLocale::setDefault() does not seem to help
    std::setlocale(LC_NUMERIC, "C");                // Set default C locale
    std::locale::global(std::locale::classic());    // Set default C++ locale (should also set default C locale)
    std::lconv* lc = std::localeconv();
    qDebug() << "Decimal point: " << lc->decimal_point << " thousand separator: \"" << lc->thousands_sep << "\"";
    //  Force QLocale to English/USA
    qDebug() << "QLocale: " << QLocale().languageToString(QLocale().language()) << " " << QLocale().countryToString(QLocale().country()) << " Decimal point: " << QLocale().decimalPoint();
    QLocale::setDefault(QLocale("C"));
    qDebug() << "Changing QLocale to: " << QLocale().languageToString(QLocale().language()) << " " << QLocale().countryToString(QLocale().country()) << " Decimal point: " << QLocale().decimalPoint();
    //! @todo this did not help before, DomElement.setAttribute still used comma decimal point on Swedish Ubuntu, now after adding std::setLocale above it might actually work.
    lc = std::localeconv();
    qDebug() << "C Decimal point: " << lc->decimal_point << " thousand separator: \"" << lc->thousands_sep << "\"";
    qDebug() << "C++ locale name: " << std::locale().name().c_str();
    qDebug() << "QLocale is: " << QLocale().name() << " " << " Decimal point: " << QLocale().decimalPoint();

    // Regsiter special types for signal/slots
    qRegisterMetaType<GUIMessage>("GUIMessage");

    // Create gloabl objects and set global pointers
    DesktopHandler gDesktopHandler;
    gDesktopHandler.setupPaths();
    gpDesktopHandler = &gDesktopHandler;
    Configuration gConfig;
    gpConfig = &gConfig;
    CopyStack gCopyStack;
    gpCopyStack = &gCopyStack;
    GUIMessageHandler gMessageHandler;
    gpMessageHandler = &gMessageHandler;

    // Read command line arguments
    //! @todo maybe use TCLAP here
    bool runApplication = true;
    QString cmdLineHcomScript;
    QStringList args = app.arguments();
    for(QString &arg : args)
    {
        if (arg.startsWith("-h") || arg.startsWith("--help")) {
            QString msg =
R"(HopsanGUI
Arguments:
    -h, --help:          Show this help message
    -v, --version:       Show HopsanGUI version
    --test:              Run build-in test cases
    path/to/script.hcom: Execute hcom script
)";
            qInfo() << qUtf8Printable(msg);
            runApplication = false;
        }
        else if (arg == "-v" || (arg == "--version")) {
            qInfo() << qUtf8Printable(QString("HopsanCore: %1\nHopsanGUI : %2").arg(gHopsanCoreVersion).arg(HOPSANGUIVERSION));
            runApplication = false;
        }
        else if (arg == "--test") {
            applicationReturnCode = runBuiltInTests();
            runApplication = false;
        }
        else if(arg.endsWith(".hcom"))
        {
            QFileInfo fi(arg);
            if (fi.isRelative())
            {
                arg = gpDesktopHandler->getExecPath()+"/"+arg;
            }
            cmdLineHcomScript = arg;
        }
        else if (!arg.contains("/hopsangui") && !arg.contains("hopsangui.exe") )
        {
            qWarning() << qUtf8Printable(QString("Unhandled argument: %1").arg(arg));
        }
    }

    if (runApplication) {

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

        gpConfig->connect(gpConfig, SIGNAL(recentModelsListChanged()), gpMainWindow, SLOT(updateRecentList()));

        //Create contents in MainWindow
        mainwindow.createContents();

        // Clear cache folders from left over junk (if Hopsan crashed last time, or was unable to cleanup)
        gpDesktopHandler->checkLogCacheForOldFiles();

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

        // Execute application
        if (!cmdLineHcomScript.isEmpty()) {
            gpTerminalWidget->mpHandler->executeCommand("exec "+cmdLineHcomScript);
        }
        applicationReturnCode = app.exec();
    }

    return applicationReturnCode;
}


//! @brief Returns the date and time when the HopsanGUI application was built
const char* getHopsanGUIBuildTime()
{
    return __DATE__ " " __TIME__;
}
