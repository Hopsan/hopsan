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

namespace componentlibrary {
namespace roots {
constexpr auto externalLibraries = "External Libraries";
constexpr auto fmus = "FMUs";
}
}

//Qt includes
#include <QObject>
#include <QStringList>
#include <QDir>
#include <QMap>
#include <QSharedPointer>
#include <QTableWidget>
#include <QDialog>
#include <QToolButton>
#include <QComboBox>

//Hopsan includes
#include "common.h"
#include "GUIObjects/GUIModelObjectAppearance.h"

//! @brief Component Library metadata class
class GUIComponentLibrary
{
public:
    QString id;
    QString version;
    bool recompilable;
    QString name;
    QString loadPath;
    QString xmlFilePath;
    QString debugExtension;
    QString lflags;
    QString cflags;
    QStringList includePaths;   //Include paths
    QStringList linkPaths;      //Link paths
    QStringList linkLibraries;           //External libraries to link against
    QString libFilePath;
    QStringList cafFiles;
    QStringList sourceFiles;
    LibraryTypeEnumT type;
    QStringList guiOnlyComponents;

    QString getLibraryMainFilePath() const;
};
typedef QSharedPointer<GUIComponentLibrary> SharedComponentLibraryPtrT;

class ComponentSpecification
{
public:
    QString typeName;
    QString displayName;
    QString cqsType;
    QStringList constantNames;
    QStringList constantDescriptions;
    QStringList constantUnits;
    QStringList constantInits;
    QStringList inputNames;
    QStringList inputDescriptions;
    QStringList inputUnits;
    QStringList inputInits;
    QStringList outputNames;
    QStringList outputDescriptions;
    QStringList outputUnits;
    QStringList outputInits;
    QStringList portNames;
    QStringList portDescriptions;
    QStringList portTypes;
    QList<bool> portsRequired;
    bool modelica;
    QString transform;
};

//! @brief Represents an object (most likely a component) that has been loaded
class ComponentLibraryEntry
{
public:
    //! @brief Check if the object entry is valid
    bool isValid() const;

    SharedModelObjectAppearanceT pAppearance;   //!< The appearance data for the object
    SharedComponentLibraryPtrT pLibrary;        //!< What library the object belongs to
    HiddenVisibleEnumT visibility=Hidden;       //!< Visible or hidden in library widget
    QStringList displayPath;                    //!< The display path in the library widget
    EnabledDisabledEnumT state=Enabled;      //!< Is component enabled or disabled?
};

class NewComponentDialog : public QDialog
{
    Q_OBJECT
public:
    enum NewComponentLanguage { Cpp, Modelica };

    NewComponentDialog(QWidget *parent);

    ComponentSpecification getSpecification();

private slots:
    void validate();

    void addConstantRow();
    void addInputVariableRow();
    void addOutputVariableRow();
    void addPortRow();

    void removeConstantRow();
    void removeInputVariableRow();
    void removeOutputVariableRow();
    void removePortRow();

    void updateIntegrationMethodComboBoxVisibility();
private:
    void addLabelItem(QTableWidget *pTable, int r, int c, QString text);
    void addInputItem(QTableWidget *pTable, int r, int c, QString defaultValue=QString());
    void adjustTableSize(QTableWidget *pTable);

    QTableWidget *mpGeneralTable;
    QTableWidget *mpConstantsTable;
    QTableWidget *mpInputVariablesTable;
    QTableWidget *mpOutputVariablesTable;
    QTableWidget *mpPortsTable;

    QComboBox *mpCqsTypeComboBox, *mpLanguageComboBox, *mpIntegrationMethodComboBox;;

    QVector<QToolButton*> mRemoveConstantToolButtons;
    QVector<QToolButton*> mRemoveInputVariableToolButtons;
    QVector<QToolButton*> mRemoveOutputVariableToolButtons;
    QVector<QToolButton*> mRemovePortToolButtons;
};

class LibraryHandler : public QObject
{
    Q_OBJECT
public:
    LibraryHandler(QObject *parent=nullptr);

    void loadLibrary(QString loadPath, LibraryTypeEnumT type=ExternalLib, HiddenVisibleEnumT visibility=Visible, RecompileEnumT recompile=Recompile);
    bool unloadLibraryByComponentType(QString typeName);
    bool unloadLibraryFMU(QString fmuName);
    bool unloadLibrary(SharedComponentLibraryPtrT pLibrary);
    bool isTypeNamesOkToUnload(const QStringList &typeNames);
    bool isLibraryLoaded(const QString &rLibraryXmlPath, const QString &rLibraryFilePath="") const;
    QStringList getLoadedLibraryNames() const;
    const SharedComponentLibraryPtrT getLibrary(const QString& id) const;
    const QVector<SharedComponentLibraryPtrT> getLibraries(const QStringList& ids, const LibraryTypeEnumT type=LibraryTypeEnumT::AnyLib) const;
    const QVector<SharedComponentLibraryPtrT> getLibraries(const LibraryTypeEnumT type=LibraryTypeEnumT::AnyLib) const;

    void openCreateComponentDialog();
    void createNewLibrary();
    void addComponentToLibrary(SharedComponentLibraryPtrT pLibrary, SaveTargetEnumT newOrExisting, QStringList folders=QStringList());
    void removeComponentFromLibrary(const QString &typeName, SharedComponentLibraryPtrT pLibrary, DeleteOrKeepFilesEnumT deleteOrKeepFiles);
    void generateCafFile(const QString &target, const QString &typeName, const QString &displayName, const QString &srcFile);
    void generateMainSource(SharedComponentLibraryPtrT pLibrary);

    void recompileLibrary(SharedComponentLibraryPtrT pLib, bool dontUnloadAndLoad=false);

    QStringList getLoadedTypeNames() const;
    ComponentLibraryEntry getEntry(const QString &typeName, const QString &subTypeName="") const;
    ComponentLibraryEntry getFMUEntry(const QString &rFmuName) const;
    const SharedModelObjectAppearanceT getModelObjectAppearancePtr(const QString &typeName, const QString &subTypeName="") const;

    void addReplacement(QString type1, QString type2);
    QStringList getReplacements(QString type);

public slots:
    void importFmu();
    void loadLibrary();

signals:
    void contentsChanged();
    void showSplashScreenMessage(QString);
    void closeSplashScreen();

private:

    bool loadLibrary(SharedComponentLibraryPtrT pLibrary, LibraryTypeEnumT type=ExternalLib, HiddenVisibleEnumT visibility=Visible, RecompileEnumT recompile=Recompile);

    // Library contents
    QList<SharedComponentLibraryPtrT> mLoadedLibraries;
    QMap<QString, ComponentLibraryEntry> mLibraryEntries;
    QMap<QString, QStringList> mReplacementsMap;

    // Auto update appearance files
    YesNoToAllEnumT mUpConvertAllCAF;
    QDir mUpdateXmlBackupDir;

    NewComponentDialog *mpDialog;
};

#endif // LIBRARYHANDLER_H
