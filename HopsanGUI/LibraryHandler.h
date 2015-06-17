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
