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

