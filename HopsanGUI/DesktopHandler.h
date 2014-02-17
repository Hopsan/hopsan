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
//! @file   DesktopHandler.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2013-03-13
//!
//! @brief Contains a utility class for handling desktop interaction, especially paths
//!
//$Id$

#ifndef DESKTOPHANDLER_H
#define DESKTOPHANDLER_H

#include <QString>

class QString;

class DesktopHandler
{
public:
    DesktopHandler();

    void setupPaths();

    const QString &getExecPath() const;
    const QString &getDataPath() const;
    const QString &getTempPath() const;
    const QString &getDocumentsPath() const;
    const QString getGeneratedComponentsPath() const;
    const QString &getBackupPath() const;
    const QString &getModelsPath() const;
    const QString &getScriptsPath() const;
    const QString &getMainPath() const;
    const QString &getHelpPath() const;
    const QString &getComponentsPath() const;
    const QString &getCoreIncludePath() const;
    const QString &getMSVC2008X86Path() const;
    const QString &getMSVC2010X86Path() const;
    const QString &getMSVC2008X64Path() const;
    const QString &getMSVC2010X64Path() const;
    const QString &getFMUPath() const;
    const QString &getLogDataPath() const;
private:
    QString mExecPath;

    bool mUseCustomDataPath;
    QString mDefaultDataPath;
    QString mCustomDataPath;

    bool mUseCustomTempPath;
    QString mDefaultTempPath;
    QString mCustomTempPath;

    bool mUseCustomDocumentsPath;
    QString mDefaultDocumentsPath;
    QString mCustomDocumentsPath;

    QString mBackupPath;
    QString mModelsPath;
    QString mScriptsPath;
    QString mHelpPath;
    QString mMainPath;
    QString mComponentsPath;
    QString mCoreIncludePath;
    QString mMSVC2008X86Path;
    QString mMSVC2010X86Path;
    QString mMSVC2008X64Path;
    QString mMSVC2010X64Path;
    QString mFMUPath;
    QString mLogDataPath;
};

#endif // DESKTOPHANDLER_H

