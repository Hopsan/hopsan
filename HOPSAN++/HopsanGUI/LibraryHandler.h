#ifndef LIBRARYHANDLER_H
#define LIBRARYHANDLER_H

#define EXTLIBSTR "External Libraries"
#define FMULIBSTR "FMU"

//Qt includes
#include <QObject>
#include <QStringList>
#include <QDir>
#include <QMap>

//Hopsan includes
#include <common.h>

//Forward declarations
class ModelObjectAppearance;
class ComponentLibrary;
class LibraryEntry;

class LibraryHandler : public QObject
{
    Q_OBJECT
public:
    LibraryHandler(QObject *parent=0);

    void loadLibrary(QString xmlPath, LibraryTypeEnumT type=External, HiddenVisibleEnumT visibility=Visible);
    void unloadLibrary(QString typeName);
    void recompileLibrary(ComponentLibrary lib, bool showDialog=true, int solver=0);

    QStringList getLoadedTypeNames();
    LibraryEntry getEntry(const QString &typeName, const QString &subTypeName="");

    void addReplacement(QString type1, QString type2);
    QStringList getReplacements(QString type);

public slots:
    void importFmu();

signals:
    void contentsChanged();

private:
    YesNoToAllEnumT mUpConvertAllCAF;

    //Contents
    QList<ComponentLibrary> mLoadedLibraries;
    QMap<QString, LibraryEntry> mLibraryEntries;
    QStringList mFailedComponents;
    QDir mUpdateXmlBackupDir;
    QStringList mLastLoadedLibFiles;
    QMap<QString, QStringList> mReplacementsMap;
};


//! @brief Library entry class
class ComponentLibrary
{
public:
    QString name;
    QString xmlFilePath;
    QString libFilePath;
    QStringList cafFiles;
    QStringList sourceFiles;
    LibraryTypeEnumT type;
};


//! @brief Library entry class
class LibraryEntry
{
public:
    ModelObjectAppearance *pAppearance;
    QStringList path;
    ComponentLibrary *pLibrary;
    HiddenVisibleEnumT visibility;
};


#endif // LIBRARYHANDLER_H
