/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

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

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

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
#include "GUIObjects/GUIModelObjectAppearance.h"

//Forward declarations

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

    SharedModelObjectAppearanceT pAppearance;
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
    const SharedModelObjectAppearanceT getModelObjectAppearancePtr(const QString &typeName, const QString &subTypeName="");

    void addReplacement(QString type1, QString type2);
    QStringList getReplacements(QString type);

public slots:
    void importFmu();
    void loadLibrary();
#ifdef EXPERIMENTAL
    void createNewCppComponent();
    void createNewModelicaComponent();
#endif //EXPERIMENTAL

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
