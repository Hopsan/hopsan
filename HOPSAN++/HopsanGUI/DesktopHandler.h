#ifndef DESKTOPHANDLER_H
#define DESKTOPHANDLER_H

#include <QString>

class QString;

class DesktopHandler
{
public:
    DesktopHandler();

    void setupPaths();

    const QString &getDataPath() const;
    const QString &getDocumentsPath() const;
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

private:
    bool mUseCustomDocumentsPath;
    bool mUseCustomDataPath;

    QString mDefaultDataPath;
    QString mCustomDataPath;

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
};

#endif // DESKTOPHANDLER_H

