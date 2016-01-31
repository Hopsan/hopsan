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
    const QString &getConfigPath() const;
    const QString &getTempPath() const;
    const QString &getDocumentsPath() const;
    const QString getGeneratedComponentsPath() const;
    const QString &getBackupPath() const;
    const QString &getModelsPath() const;
    const QString &getScriptsPath() const;
    const QString &getMainPath() const;
    const QString &getHelpPath() const;
    const QString &getComponentsPath() const;
    const QString &getAutoLibsPath() const;
    const QString &getCoreIncludePath() const;
    const QString &getMSVC2008X86Path() const;
    const QString &getMSVC2010X86Path() const;
    const QString &getMSVC2008X64Path() const;
    const QString &getMSVC2010X64Path() const;
    const QString &getFMUPath() const;
    const QString &getLogDataPath() const;
    const QString &getResourcesPath() const;
    QString getIncludedCompilerPath(int expectedArch=-1) const;

private:
    QString mExecPath;

    bool mUseCustomDataPath;
    QString mDefaultConfigPath;
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
    QString mAutoLibsPath;
    QString mCoreIncludePath;
    QString mMSVC2008X86Path;
    QString mMSVC2010X86Path;
    QString mMSVC2008X64Path;
    QString mMSVC2010X64Path;
    QString mFMUPath;
    QString mLogDataPath;
    QString mResourcesPath; // Primarilly used for handling the Resources dir in a Apple App bundle
};

#endif // DESKTOPHANDLER_H

