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
//! @file   DesktopHandler.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2013-03-13
//!
//! @brief Contains a utility class for handling desktop interaction, especially paths
//!
//$Id$

#include "DesktopHandler.h"
#include "common.h"

#include <QDesktopServices>
#include <QString>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QApplication>
#include "Utilities/GUIUtilities.h"

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
    mHelpPath = mResourcesPath+"/doc/user/html/";
    mComponentsPath = mExecPath+"../Frameworks/componentLibraries/defaultLibrary/";
    mAutoLibsPath = mExecPath+"../Frameworks/componentLibraries/autoLibs/";
    mCoreIncludePath = mResourcesPath+"HopsanCore/include/";
    mMSVC2008X86Path = mResourcesPath+"MSVC2008_x86/";
    mMSVC2010X86Path = mResourcesPath+"MSVC2010_x86/";
    mMSVC2008X64Path = mResourcesPath+"MSVC2008_x64/";
    mMSVC2010X64Path = mResourcesPath+" MSVC2010_x64/";
#else
    mMainPath = mExecPath+"../";
    mHelpPath = mExecPath+"../doc/user/html/";
    mComponentsPath = mExecPath+"../componentLibraries/defaultLibrary/";
    mAutoLibsPath = mExecPath+"../componentLibraries/autoLibs/";
    mCoreIncludePath = mExecPath+"../HopsanCore/include/";
    mMSVC2008X86Path = mExecPath+"MSVC2008_x86/";
    mMSVC2010X86Path = mExecPath+"MSVC2010_x86/";
    mMSVC2008X64Path = mExecPath+"MSVC2008_x64/";
    mMSVC2010X64Path = mExecPath+"MSVC2010_x64/";
#endif
    mFMUPath = mDefaultDocumentsPath+"import/FMU/";
    mLogDataPath = mDefaultTempPath + "LogData/";
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
            QMessageBox::information(0, "Choose data path", "Default data directory is not writable:\n\n"+mDefaultDataPath+"\n\nPlease choose a different path.", "Okay");
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
            QMessageBox::information(0, "Choose documents path", "Default documents directory is not writable:\n\n"+mDefaultDocumentsPath+"\n\nPlease choose a different path.", "Okay");
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
            QMessageBox::information(0, "Choose temp path", "Default temp directory is not writable:\n\n"+mDefaultTempPath+"\n\nPlease choose a different path.", "Okay");
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
    mLogDataPath = getTempPath()+"LogData/";

     // Make sure backup folder exists, create it if not
    mkpath(getBackupPath());

    // Make sure model folder exists, create it if not, if create not sucessful use dev dir
    mkpath(getModelsPath());

    // Select which scripts path to use, create it if not, if create not sucessful use dev dir
    //! @todo problem in linux if scripts must be changed, as they  are not installed to user home
    mkpath(getScriptsPath());

    // Clear cache folders from left over junk (if Hopsan crashed last time, or was unable to cleanup)
    qDebug() << "LogdataCache: " << getLogDataPath();
    removeDir(getLogDataPath());
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

const QString &DesktopHandler::getLogDataPath() const
{
    return mLogDataPath;
}

const QString &DesktopHandler::getResourcesPath() const
{
    return mResourcesPath;
}
