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



DesktopHandler::DesktopHandler()
{
    mDefaultDataPath = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/Hopsan/";
    mDefaultDocumentsPath = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + "/Hopsan/";
}


void DesktopHandler::setupPaths()
{
    //Make sure data path exists, ask user for custom path if default cannot be created
    QDir dataDir(mDefaultDataPath);
    if (!dataDir.exists())
    {
        dataDir.mkpath(mDefaultDataPath);
    }
    QFile dummyFile1(gDesktopHandler.getDataPath()+"/HOPSANDUMMYTESTFILETHATWILLBEREMOVED");
    if(dummyFile1.exists())
    {
        dummyFile1.remove();
    }
    if (dummyFile1.open(QFile::ReadWrite))
    {
        qDebug() << "Documents path is writable!";
    }
    else
    {
        dummyFile1.close();
        qDebug() << "Documents path is NOT writable!";
        QMessageBox::information(0, "Choose documents path", "Default data directory is not writable:\n\n"+mDefaultDataPath+"\n\nPlease choose a different path.", "Okay");
        QWidget *pWidget = new QWidget();
        QFileDialog *pDialog = new QFileDialog(pWidget);
        mCustomDocumentsPath = pDialog->getExistingDirectory(pWidget, "Choose Documents Directory",
                                gExecPath,
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
                                gExecPath,
                                QFileDialog::ShowDirsOnly
                                | QFileDialog::DontResolveSymlinks);
        delete(pDialog);
        delete(pWidget);
        mUseCustomDocumentsPath = true;
    }
    dummyFile2.remove();


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
}


QString DesktopHandler::getDataPath() const
{
    if(mUseCustomDataPath)
        return mCustomDataPath;
    else
        return mDefaultDataPath;
}


QString DesktopHandler::getDocumentsPath() const
{
    if(mUseCustomDocumentsPath)
        return mCustomDocumentsPath;
    else
        return mDefaultDocumentsPath;
}


QString DesktopHandler::getBackupPath() const
{
    return getDocumentsPath()+"/Backup/";
}

QString DesktopHandler::getModelsPath() const
{
    return getDocumentsPath()+"/Models/";
}

QString DesktopHandler::getScriptsPath() const
{
    return getDocumentsPath()+"/Scripts/";
}
