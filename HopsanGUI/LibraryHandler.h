#ifndef LIBRARYHANDLER_H
#define LIBRARYHANDLER_H

#define EXTLIBSTR "External Libraries"
#define MODELICALIBSTR "Modelica Components"
#define FMULIBSTR "FMU"
#define MODELICATYPENAME "ModelicaComponent"


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

    void loadLibrary(QString xmlPath, LibraryTypeEnumT type=ExternalLib, HiddenVisibleEnumT visibility=Visible);
    bool unloadLibraryByComponentType(QString typeName);
    bool unloadLibraryFMU(QString fmuName);
    bool isTypeNamesOkToUnload(const QStringList &typeNames);
    void recompileLibrary(ComponentLibrary lib, bool showDialog=true, int solver=0, bool dontUnloadAndLoad=false);

    QStringList getLoadedTypeNames();
    LibraryEntry getEntry(const QString &typeName, const QString &subTypeName="");
    LibraryEntry getFMUEntry(const QString &rFmuName);
    ModelObjectAppearance *getModelObjectAppearancePtr(const QString &typeName, const QString &subTypeName="");

    void addReplacement(QString type1, QString type2);
    QStringList getReplacements(QString type);

public slots:
    void importFmu();
    void loadLibrary();
    void createNewCppComponent();
    void createNewModelicaComponent();

signals:
    void contentsChanged();

private:
    bool unloadLibrary(ComponentLibrary *pLibrary);

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
    QString debugExtension;
    QString libFilePath;
    QStringList cafFiles;
    QStringList sourceFiles;
    LibraryTypeEnumT type;
    QStringList guiOnlyComponents;
};


//! @brief Library entry class
class LibraryEntry
{
public:
    LibraryEntry();
    bool isNull() const;

    ModelObjectAppearance *pAppearance;
    ComponentLibrary *pLibrary;
    HiddenVisibleEnumT visibility;
    QStringList path;
};


#endif // LIBRARYHANDLER_H
