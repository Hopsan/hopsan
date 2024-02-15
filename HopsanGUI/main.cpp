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

#if QT_VERSION < 0x050500
#include <iostream>
#endif

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
#include "LibraryHandler.h"

// Declare global pointers
Configuration *gpConfig = nullptr;
DesktopHandler *gpDesktopHandler = nullptr;
CopyStack *gpCopyStack = nullptr;
GUIMessageHandler *gpMessageHandler = nullptr;

MainWindow* gpMainWindow = nullptr;
QWidget *gpMainWindowWidget = nullptr;

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

    // Create the mainwindow
    MainWindow mainwindow;
    gpMainWindow = &mainwindow;
    gpMainWindowWidget = static_cast<QWidget*>(&mainwindow);
    mainwindow.createContents();

    // Read command line arguments
    //! @todo maybe use TCLAP here
    bool runApplication = true;
    bool runTests = false;
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
#if QT_VERSION >= 0x050500
            qInfo() << qUtf8Printable(msg);
#else
            std::cout << qPrintable(msg) << std::endl;
#endif
            runApplication = false;
            return applicationReturnCode;
        }
        else if (arg == "-v" || (arg == "--version")) {
            const auto msg = QString("HopsanCore: %1\nHopsanGUI : %2").arg(gHopsanCoreVersion).arg(HOPSANGUIVERSION);
#if QT_VERSION >= 0x050500
            qInfo() << qUtf8Printable(msg);
#else
            std::cout << qPrintable(msg) << std::endl;
#endif
            runApplication = false;
            return applicationReturnCode;
        }
        else if (arg == "--test") {
            runApplication = false;
            runTests = true;
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
        // Depending on platform and how hopsangui is launched, the first argument may be the program name
        // do not warn in this case
        else if (!arg.contains("/hopsangui") && !arg.contains("hopsangui.exe") )
        {
            const auto msg = QString("Unhandled argument: %1").arg(arg);
#if QT_VERSION >= 0x050500
            qWarning() << qUtf8Printable(msg);
#else
            std::cout << qPrintable(msg) << std::endl;
#endif
        }
    }

    // Create the splash screen
    QSplashScreen splash(&mainwindow, QPixmap(QString(GRAPHICSPATH) + "splash.png")/*, Qt::WindowStaysOnTopHint*/);
    if (runApplication) {
        QObject::connect(gpMainWindow, SIGNAL(showSplashScreenMessage(QString)), &splash, SLOT(showMessage(QString)));
        QObject::connect(gpLibraryHandler, SIGNAL(showSplashScreenMessage(QString)), &splash, SLOT(showMessage(QString)));
        QObject::connect(gpLibraryHandler, SIGNAL(closeSplashScreen()), &splash, SLOT(close()));
        splash.showMessage("Starting Hopsan...");
        splash.show();
        QTimer::singleShot(4000, &splash, SLOT(close())); // If initialize gets stuck for some reason, make sure splash screen is closed eventually
    }

    // Create contents in MainWindow
    mainwindow.initializeWorkspace();
    QTimer::singleShot(1000, &splash, SLOT(close()));

    if (runApplication) {
        gpConfig->connect(gpConfig, SIGNAL(recentModelsListChanged()), gpMainWindow, SLOT(updateRecentList()));

        // Clear cache folders from left over junk (if Hopsan crashed last time, or was unable to cleanup)
        gpDesktopHandler->checkLogCacheForOldFiles();

        mainwindow.showMaximized();

        // Process any received messages
        gpMessageHandler->startPublish();

        // Show license dialog
        if (gpConfig->getBoolSetting(cfg::showlicenseonstartup))
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
    else if (runTests) {
        applicationReturnCode = runBuiltInTests();
    }

    return applicationReturnCode;
}


//! @brief Returns the date and time when the HopsanGUI application was built
const char* getHopsanGUIBuildTime()
{
    return __DATE__ " " __TIME__;
}
