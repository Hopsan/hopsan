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



DesktopHandler::DesktopHandler()
{
    mExecPath = qApp->applicationDirPath().append('/');
    mUseCustomDataPath = false;
    mUseCustomTempPath = false;
    mUseCustomDocumentsPath = false;
    mDefaultDataPath = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/Hopsan/";
    mDefaultTempPath = QDesktopServices::storageLocation(QDesktopServices::TempLocation) + "/Hopsan/";
    mDefaultDocumentsPath = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + "/Hopsan/";
    mBackupPath = mDefaultDocumentsPath+"/Backup/";
    mModelsPath = mDefaultDocumentsPath+"/Models/";
    mScriptsPath = mDefaultDocumentsPath+"Scripts";
    mMainPath = mExecPath+"../";
    mHelpPath = mExecPath+"../doc/user/html/";
    mComponentsPath = mExecPath+"../componentLibraries/defaultLibrary/";
    mCoreIncludePath = mExecPath+"../HopsanCore/include/";
    mMSVC2008X86Path = mExecPath+"MSVC2008_x86/";
    mMSVC2010X86Path = mExecPath+"MSVC2010_x86/";
    mMSVC2008X64Path = mExecPath+"MSVC2008_x64/";
    mMSVC2010X64Path = mExecPath+"MSVC2010_x64/";
    mFMUPath = mExecPath+"../import/FMU/";
    mLogDataPath = mDefaultTempPath + "/LogData";
}


void DesktopHandler::setupPaths()
{
    //Make sure data path exists, ask user for custom path if default cannot be created
    QDir dataDir(mDefaultDataPath);
    if (!dataDir.exists())
    {
        dataDir.mkpath(mDefaultDataPath);
    }
    QFile dummyFile1(mDefaultDataPath+"/HOPSANDUMMYTESTFILETHATWILLBEREMOVED");
    if(dummyFile1.exists())
    {
        dummyFile1.remove();
    }
    if (dummyFile1.open(QFile::ReadWrite))
    {
        qDebug() << "Data path is writable!";
    }
    else
    {
        dummyFile1.close();
        qDebug() << "Data path is NOT writable!";
        QMessageBox::information(0, "Choose data path", "Default data directory is not writable:\n\n"+mDefaultDataPath+"\n\nPlease choose a different path.", "Okay");
        QWidget *pWidget = new QWidget();
        QFileDialog *pDialog = new QFileDialog(pWidget);
        mCustomDocumentsPath = pDialog->getExistingDirectory(pWidget, "Choose Data Directory",
                                mExecPath,
                                QFileDialog::ShowDirsOnly
                                | QFileDialog::DontResolveSymlinks);
        delete(pDialog);
        delete(pWidget);
        mUseCustomDocumentsPath = true;
    }
    dummyFile1.remove();


    //Make sure documents path exists, ask user for custom path if default cannot be created
    QDir documentsDir(mDefaultDocumentsPath);
    if (!documentsDir.exists())
    {
        documentsDir.mkpath(mDefaultDocumentsPath);
    }
    QFile dummyFile2(mDefaultDocumentsPath+"/HOPSANDUMMYTESTFILETHATWILLBEREMOVED");
    if(dummyFile2.exists())
    {
        dummyFile2.remove();
    }
    if (dummyFile2.open(QFile::ReadWrite))
    {
        qDebug() << "Documents path is writable!";
    }
    else
    {
        dummyFile2.close();
        qDebug() << "Documents path is NOT writable!";
        QMessageBox::information(0, "Choose documents path", "Default documents directory is not writable:\n\n"+mDefaultDocumentsPath+"\n\nPlease choose a different path.", "Okay");
        QWidget *pWidget = new QWidget();
        QFileDialog *pDialog = new QFileDialog(pWidget);
        mCustomDocumentsPath = pDialog->getExistingDirectory(pWidget, "Choose Documents Directory",
                                mExecPath,
                                QFileDialog::ShowDirsOnly
                                | QFileDialog::DontResolveSymlinks);
        delete(pDialog);
        delete(pWidget);
        mUseCustomDocumentsPath = true;
    }
    dummyFile2.remove();


    //Update paths depending on data, temp and documents paths
    mBackupPath = getDocumentsPath()+"/Backup/";
    mModelsPath = getDocumentsPath()+"/Models/";
    mScriptsPath = getDocumentsPath()+"Scripts/";
    mLogDataPath = getTempPath() + "/LogData/";


     // Make sure backup folder exists, create it if not
    if (!QDir().exists(getBackupPath()))
    {
        QDir().mkpath(getBackupPath());
    }

    // Make sure model folder exists, create it if not, if create not sucessfull use dev dir
    if (!QDir().exists(getModelsPath()))
    {
        QDir().mkpath(getModelsPath());
    }

    // Select which scripts path to use, create it if not, if create not sucessfull use dev dir
    //! @todo problem in linux if scripts must be changed, as they  are not installed to user home
    if (!QDir().exists(getScriptsPath()))
    {
        QDir().mkpath(getScriptsPath());
    }

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
    return getDocumentsPath()+"/Generated Components/";
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
