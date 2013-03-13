#ifndef DESKTOPHANDLER_H
#define DESKTOPHANDLER_H

#include <QString>

class QString;

class DesktopHandler
{
public:
    DesktopHandler();

    void setupPaths();

    QString getDataPath() const;
    QString getDocumentsPath() const;
    QString getBackupPath() const;
    QString getModelsPath() const;
    QString getScriptsPath() const;

private:
    bool mUseCustomDocumentsPath;
    bool mUseCustomDataPath;

    QString mDefaultDataPath;
    QString mCustomDataPath;

    QString mDefaultDocumentsPath;
    QString mCustomDocumentsPath;
};

#endif // DESKTOPHANDLER_H

