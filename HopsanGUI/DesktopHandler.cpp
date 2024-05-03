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
//! @file   DesktopHandler.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2013-03-13
//!
//! @brief Contains a utility class for handling desktop interaction, especially paths
//!
//$Id$

#include "DesktopHandler.h"
#include "common.h"
#include "compiler_info.h"

#include <QDesktopServices>
#include <QString>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QApplication>
#include <QTimer>
#include <QProcess>
#include "Utilities/GUIUtilities.h"
#include "MessageHandler.h"
#include "global.h"

namespace {

#if QT_VERSION >= 0x050000
#include <QStandardPaths>
QString getStandardLocation(QStandardPaths::StandardLocation type)
{
    QString location;
    QStringList locations = QStandardPaths::standardLocations(type);
    if (!locations.isEmpty())
    {
        // Take first reported location
        location = locations.first();
        // Append '/' to end if not already present
        if (location[location.size()-1] != '/')
        {
            location.append('/');
        }
    }
    return location;
}
#endif

bool testIfDirectoryIsWritable(QDir &rDir)
{
    if (rDir.exists())
    {
        bool rc = true;
        QFile dummyFile(rDir.absoluteFilePath("HOPSANDUMMYTESTFILETHATWILLBEREMOVED"));
        // Remove file if it already exists
        if(dummyFile.exists())
        {
            rc = dummyFile.remove();
        }
        // If we could remove it (and or if we can open a new one (writeable) then all is good
        if (rc && dummyFile.open(QFile::ReadWrite))
        {
            qDebug() << rDir.absolutePath() << " is writable!";
        }
        // Else it is not so good
        else
        {
            qDebug() << rDir.absolutePath() << " is NOT writable!";
            rc = false;
        }
        dummyFile.close();
        dummyFile.remove();
        return rc;
    }
    return false;
}

bool mkpath(const QDir &rDir)
{
    // No need to check if it exists first, mkpath does that internally
    return rDir.mkpath(rDir.absolutePath());
}

bool mkpath(const QString &rPath)
{
    return mkpath(QDir(rPath));
}

} // End anon namespace

DesktopHandler::DesktopHandler()
{
    mExecPath = qApp->applicationDirPath().append('/');
    mUseCustomDataPath = false;
    mUseCustomTempPath = false;
    mUseCustomDocumentsPath = false;
#if QT_VERSION >= 0x050000
#ifdef Q_OS_OSX
    mDefaultConfigPath = getStandardLocation(QStandardPaths::ConfigLocation) + "Hopsan/";
    mDefaultDataPath = getStandardLocation(QStandardPaths::DataLocation); // in the future use AppDataLocation instead
    mDefaultTempPath = getStandardLocation(QStandardPaths::TempLocation) + "Hopsan/";
    mResourcesPath = mExecPath + "../Resources/";
#else
    mDefaultConfigPath = getStandardLocation(QStandardPaths::ConfigLocation);
    mDefaultDataPath = getStandardLocation(QStandardPaths::DataLocation);
    mDefaultTempPath = getStandardLocation(QStandardPaths::TempLocation) + "Hopsan/";
    mResourcesPath = mDefaultConfigPath;
#endif
    mDefaultDocumentsPath = getStandardLocation(QStandardPaths::DocumentsLocation) + "Hopsan/";
#else
#ifdef Q_OS_OSX
#error "QT<5.0 not supported on Apple platform (magse)"
#else
    mDefaultDataPath = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/Hopsan/";
    mDefaultConfigPath = mDefaultDataPath;
    mDefaultTempPath = QDesktopServices::storageLocation(QDesktopServices::TempLocation) + "/Hopsan/";
    mDefaultDocumentsPath = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + "/Hopsan/";
    mDefaultDocumentsPath.replace("\\", "/");
#endif
#endif
    mBackupPath = mDefaultDocumentsPath+"Backup/";
    mModelsPath = mDefaultDocumentsPath+"Models/";
    mScriptsPath = mDefaultDocumentsPath+"Scripts/";
#ifdef Q_OS_OSX
    mMainPath = getStandardLocation(QStandardPaths::HomeLocation);
    mHelpPath = mResourcesPath+"/doc/html/";
    mComponentsPath = mExecPath+"../Frameworks/componentLibraries/defaultLibrary/";
    mAutoLibsPath = mExecPath+"../Frameworks/componentLibraries/autoLibs/";
    mCoreIncludePath = mResourcesPath+"HopsanCore/include/";
    mMSVC2008X86Path = mResourcesPath+"MSVC2008_x86/";
    mMSVC2010X86Path = mResourcesPath+"MSVC2010_x86/";
    mMSVC2008X64Path = mResourcesPath+"MSVC2008_x64/";
    mMSVC2010X64Path = mResourcesPath+" MSVC2010_x64/";
#else
    mMainPath = mExecPath+"../";
    mHelpPath = mExecPath+"../doc/html/";
    mComponentsPath = mExecPath+"../componentLibraries/defaultLibrary/";
    mAutoLibsPath = mExecPath+"../componentLibraries/autoLibs/";
    mCoreIncludePath = mExecPath+"../HopsanCore/include/";
    mMSVC2008X86Path = mExecPath+"MSVC2008_x86/";
    mMSVC2010X86Path = mExecPath+"MSVC2010_x86/";
    mMSVC2008X64Path = mExecPath+"MSVC2008_x64/";
    mMSVC2010X64Path = mExecPath+"MSVC2010_x64/";
#endif
    mFMUPath = mDefaultDocumentsPath+"import/FMU/";
}


void DesktopHandler::setupPaths()
{
    // Make sure data path exists, ask user for custom path if default cannot be created
    QDir dataDir(mDefaultDataPath);
    mkpath(dataDir);
    if (!testIfDirectoryIsWritable(dataDir))
    {
        QDir customDir;
        do
        {
            QMessageBox::information(0, "Choose data path", "Default data directory is not writeable:\n\n"+mDefaultDataPath+"\n\nPlease choose a different path.", "Okay");
            QWidget *pWidget = new QWidget();
            QFileDialog *pDialog = new QFileDialog(pWidget);
            mCustomDataPath = pDialog->getExistingDirectory(pWidget, "Choose Data Directory",
                                                            mExecPath,
                                                            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
            mCustomDataPath.append("/");
            customDir.setPath(mCustomDataPath);
            mkpath(customDir);
            pDialog->deleteLater();
            pWidget->deleteLater();
        }while(!testIfDirectoryIsWritable(customDir));
        mUseCustomDataPath = true;
    }

    // Make sure documents path exists, ask user for custom path if default cannot be created
    QDir documentsDir(mDefaultDocumentsPath);
    mkpath(documentsDir);
    if(!testIfDirectoryIsWritable(documentsDir))
    {
        QDir customDir;
        do
        {
            QMessageBox::information(0, "Choose documents path", "Default documents directory is not writeable:\n\n"+mDefaultDocumentsPath+"\n\nPlease choose a different path.", "Okay");
            QWidget *pWidget = new QWidget();
            QFileDialog *pDialog = new QFileDialog(pWidget);
            mCustomDocumentsPath = pDialog->getExistingDirectory(pWidget, "Choose Documents Directory",
                                    mExecPath,
                                    QFileDialog::ShowDirsOnly
                                    | QFileDialog::DontResolveSymlinks);
            mCustomDocumentsPath.append('/');
            customDir.setPath(mCustomDocumentsPath);
            mkpath(customDir);
            pDialog->deleteLater();
            pWidget->deleteLater();
        }while(!testIfDirectoryIsWritable(customDir));
        mUseCustomDocumentsPath = true;
    }

    // Make sure temp path exists, ask user for custom path if default cannot be created
    QDir tempDir(mDefaultTempPath);
    mkpath(tempDir);
    if (!testIfDirectoryIsWritable(tempDir))
    {
        QDir customDir;
        do
        {
            QMessageBox::information(0, "Choose temp path", "Default temp directory is not writeable:\n\n"+mDefaultTempPath+"\n\nPlease choose a different path.", "Okay");
            QWidget *pWidget = new QWidget();
            QFileDialog *pDialog = new QFileDialog(pWidget);
            mCustomTempPath = pDialog->getExistingDirectory(pWidget, "Choose Temp Directory",
                                                            mExecPath,
                                                            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
            mCustomTempPath.append("/");
            customDir.setPath(mCustomTempPath);
            mkpath(customDir);
            pDialog->deleteLater();
            pWidget->deleteLater();
        }while(!testIfDirectoryIsWritable(customDir));
        mUseCustomTempPath = true;
    }

    // Update paths depending on data, temp and documents paths
    mBackupPath = getDocumentsPath()+"Backup/";
    mModelsPath = getDocumentsPath()+"Models/";
    mScriptsPath = getDocumentsPath()+"Scripts/";
    mFMUPath = getDocumentsPath()+"import/FMU/";

     // Make sure backup folder exists, create it if not
    mkpath(getBackupPath());

    // Make sure model folder exists, create it if not, if create not successful use dev dir
    mkpath(getModelsPath());

    // Select which scripts path to use, create it if not, if create not successful use dev dir
    //! @todo problem in linux if scripts must be changed, as they  are not installed to user home
    mkpath(getScriptsPath());
}


const QString &DesktopHandler::getExecPath() const
{
    return mExecPath;
}


const QString &DesktopHandler::getDataPath() const
{
    if(mUseCustomDataPath)
        return mCustomDataPath;
    else
        return mDefaultDataPath;
}

const QString &DesktopHandler::getConfigPath() const
{
    return mDefaultConfigPath;
}


const QString &DesktopHandler::getTempPath() const
{
    if(mUseCustomTempPath)
        return mCustomTempPath;
    else
        return mDefaultTempPath;
}


const QString &DesktopHandler::getDocumentsPath() const
{
    if(mUseCustomDocumentsPath)
        return mCustomDocumentsPath;
    else
        return mDefaultDocumentsPath;
}


const QString DesktopHandler::getGeneratedComponentsPath() const
{
    return getDocumentsPath()+"/GeneratedComponents/";
}


const QString &DesktopHandler::getBackupPath() const
{
    return mBackupPath;
}

const QString &DesktopHandler::getModelsPath() const
{
    return mModelsPath;
}

const QString &DesktopHandler::getScriptsPath() const
{
    return mScriptsPath;
}

const QString &DesktopHandler::getMainPath() const
{
    return mMainPath;
}

const QString &DesktopHandler::getHelpPath() const
{
    return mHelpPath;
}

const QString &DesktopHandler::getComponentsPath() const
{
    return mComponentsPath;
}

const QString &DesktopHandler::getAutoLibsPath() const
{
    return mAutoLibsPath;
}

const QString &DesktopHandler::getCoreIncludePath() const
{
    return mCoreIncludePath;
}

const QString &DesktopHandler::getMSVC2008X86Path() const
{
    return mMSVC2008X86Path;
}

const QString &DesktopHandler::getMSVC2010X86Path() const
{
    return mMSVC2010X86Path;
}

const QString &DesktopHandler::getMSVC2008X64Path() const
{
    return mMSVC2008X64Path;
}

const QString &DesktopHandler::getMSVC2010X64Path() const
{
    return mMSVC2010X64Path;
}

const QString &DesktopHandler::getFMUPath() const
{
    return mFMUPath;
}

QString DesktopHandler::getLogDataPath() const
{
    return getTempPath()+"/logdata_cache/";
}

const QString &DesktopHandler::getResourcesPath() const
{
    return mResourcesPath;
}

QString DesktopHandler::getIncludedCompilerPath(int expectedArch) const
{
    // Note! The compiler is only included on Windows (for now)
    // we look for a mingw or mingw64 directory in the Hopsan root

    if (expectedArch == -1)
    {
#ifdef HOPSANCOMPILED64BIT
        expectedArch = 64;
#else
        expectedArch = 32;
#endif
    }

    QDir compilerpath;
    // Try 32-bit mingw
    if (expectedArch == 32)
    {
        compilerpath.setPath(getMainPath()+"/mingw/bin");
    }
    // Try 64-bit mingw
    else if (expectedArch == 64)
    {
        compilerpath.setPath(getMainPath()+"/mingw64/bin");
    }
    // Fail
    else
    {
        return "";
    }

    // Note! IncludedCompiler only available on Windows so we only check for gcc.exe
    //! @todo might need clang support on OSX
    if (compilerpath.exists("gcc.exe"))
    {
        return compilerpath.canonicalPath();
    }
    return "";
}

void DesktopHandler::setCustomTempPath(const QString &tempPath)
{
    mCustomTempPath = tempPath;
    mUseCustomTempPath = !mCustomTempPath.isEmpty();
}

void DesktopHandler::checkLogCacheForOldFiles()
{
    qDebug() << "LogdataCache: " << getLogDataPath();
    QDir logcache(getLogDataPath());
    if (logcache.exists())
    {
        QStringList enteries = logcache.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
        if (!enteries.isEmpty())
        {
            bool otherHopsanIsRunning;
#ifdef _WIN32
            QProcess tasklist;
            tasklist.start("tasklist", QStringList() << "/NH" << "/FO" << "CSV" << "/FI" << "IMAGENAME eq hopsangui.exe" << "/FI" << "STATUS eq RUNNING");
            tasklist.waitForFinished();
            QString output = tasklist.readAllStandardOutput();
            otherHopsanIsRunning = output.contains("hopsangui.exe"); //On Windows systems, this process has not officially started yet
#else
            QProcess ps;
            ps.start("ps", QStringList() << "-a");
            ps.waitForFinished();
            QString output = ps.readAllStandardOutput();
            otherHopsanIsRunning = output.count("hopsangui") > 1;   //On Unix systems, this process has already started
#endif

            if(otherHopsanIsRunning) {
                gpMessageHandler->addWarningMessage("I found old log data on disk and will not remove it because another instance of Hopsan is running.");
            }
            else {
                gpMessageHandler->addWarningMessage("I found old log data on disk, but I will remove it because no other instance of Hopsan is running.");
                removeDir(getLogDataPath());
            }
        }
    }
}
