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

//$Id$

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
#include <QSharedPointer>

//Hopsan includes
#include "common.h"

//Forward declarations
class ModelObjectAppearance;

//! @brief Component Library metadata class
class ComponentLibrary
{
public:
    QString name;
    QString loadPath;
    QString xmlFilePath;
    QString debugExtension;
    QString lflags;
    QString cflags;
    QString libFilePath;
    QStringList cafFiles;
    QStringList sourceFiles;
    LibraryTypeEnumT type;
    QStringList guiOnlyComponents;

    QString getLibraryMainFilePath() const;
};
typedef QSharedPointer<ComponentLibrary> SharedComponentLibraryPtrT;

//! @brief Library entry class
class LibraryEntry
{
public:
    bool isNull() const;

    ModelObjectAppearance *pAppearance=nullptr;
    SharedComponentLibraryPtrT pLibrary;
    HiddenVisibleEnumT visibility=Hidden;
    QStringList path;
};

class LibraryHandler : public QObject
{
    Q_OBJECT
public:
    LibraryHandler(QObject *parent=0);

    void loadLibrary(QString loadPath, LibraryTypeEnumT type=ExternalLib, HiddenVisibleEnumT visibility=Visible);
    bool isLibraryLoaded(const QString &rLibraryXmlPath, const QString &rLibraryFilePath="") const;
    bool unloadLibraryByComponentType(QString typeName);
    bool unloadLibraryFMU(QString fmuName);
    bool isTypeNamesOkToUnload(const QStringList &typeNames);
    void recompileLibrary(SharedComponentLibraryPtrT pLib, bool showDialog=true, int solver=0, bool dontUnloadAndLoad=false);

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
    bool unloadLibrary(SharedComponentLibraryPtrT pLibrary);
    bool loadLibrary(SharedComponentLibraryPtrT pLibrary, LibraryTypeEnumT type=ExternalLib, HiddenVisibleEnumT visibility=Visible);

    YesNoToAllEnumT mUpConvertAllCAF;

    //Contents
    QList<SharedComponentLibraryPtrT> mLoadedLibraries;
    QMap<QString, LibraryEntry> mLibraryEntries;
//    QStringList mFailedComponents;
    QDir mUpdateXmlBackupDir;
    QMap<QString, QStringList> mReplacementsMap;
};

#endif // LIBRARYHANDLER_H
