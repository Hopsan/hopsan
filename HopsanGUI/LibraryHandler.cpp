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

//!
//! @file   LibraryHandler.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2013-10-23
//!
//! @brief Contains a class for handling component libraries
//!
//$Id$

//Defines
#define XML_LIBRARY "hopsancomponentlibrary"
#define XML_LIBRARY_NAME "name"
#define XML_LIBRARY_ID "id"
#define XML_LIBRARY_LIB "lib"
#define XML_LIBRARY_LIB_DBGEXT "debug_ext"
#define XML_LIBRARY_CAF "caf"
#define XML_LIBRARY_SOURCE "source"

#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>

//Hopsan includes
#include "LibraryHandler.h"
#include "global.h"
#include "Configuration.h"
#include "CoreAccess.h"
#include "DesktopHandler.h"
#include "GUIObjects/GUIModelObjectAppearance.h"
#include "ModelHandler.h"
#include "version_gui.h"
#include "MessageHandler.h"
#include "Dialogs/EditComponentDialog.h"
#include "GUIObjects/GUISystem.h"
#include "GeneratorUtils.h"

//! @brief Helpfunction to create full typename from type and subtype
//! @returns The full typename type|subtype, or type is subtype was empty
QString makeFullTypeString(const QString &rType, const QString &rSubType)
{
    if (rSubType.isEmpty())
    {
        return rType;
    }
    else
    {
       return rType+"|"+rSubType;
    }
}


//! @brief Constructor for library handler
LibraryHandler::LibraryHandler(QObject *parent)
    : QObject(parent)
{
    mUpConvertAllCAF = UndecidedToAll;


}

void LibraryHandler::loadLibrary()
{
    QString libDir = QFileDialog::getExistingDirectory(gpMainWindowWidget, tr("Choose Library Directory"),
                                                   gpConfig->getStringSetting(CFG_EXTERNALLIBDIR),
                                                   QFileDialog::ShowDirsOnly
                                                   | QFileDialog::DontResolveSymlinks);
    if(libDir.isEmpty())
    {
        return;
    }
    else
    {
        gpConfig->setStringSetting(CFG_EXTERNALLIBDIR,libDir);

        loadLibrary(libDir);

//        // Check so that lib is not already loaded
//        if(!gpConfig->hasUserLib(libDir))
//        {
//            loadLibrary(libDir);
//        }
//        else
//        {
//            gpMessageHandler->addErrorMessage("Library " + libDir + " is already loaded!");
//        }
    }
}

#ifdef EXPERIMENTAL
void LibraryHandler::createNewCppComponent()
{
    EditComponentDialog *pEditDialog = new EditComponentDialog("", EditComponentDialog::Cpp, gpMainWindowWidget);
    pEditDialog->exec();
    if(pEditDialog->result() == QDialog::Accepted)
    {
        CoreGeneratorAccess coreAccess;
        QString typeName = pEditDialog->getCode().section("class ", 1, 1).section(" ",0,0);

        QString dummy = gpDesktopHandler->getGeneratedComponentsPath();
        QString libPath = dummy+typeName+"/";
        QDir().mkpath(libPath);

        QFile hppFile(libPath+typeName+".hpp");
        hppFile.open(QFile::WriteOnly | QFile::Truncate);
        hppFile.write(pEditDialog->getCode().toUtf8());
        hppFile.close();

        coreAccess.generateFromCpp(libPath+typeName+".hpp", true);
        loadLibrary(libPath+typeName+"_lib.xml");
    }
    delete(pEditDialog);
    return;
}

void LibraryHandler::createNewModelicaComponent()
{
    EditComponentDialog *pEditDialog = new EditComponentDialog("", EditComponentDialog::Modelica, gpMainWindowWidget);
    pEditDialog->exec();
    if(pEditDialog->result() == QDialog::Accepted)
    {
        CoreGeneratorAccess coreAccess;
        QString typeName = pEditDialog->getCode().section("model ", 1, 1).section(" ",0,0).section("\n",0,0);
        QString dummy = gpDesktopHandler->getGeneratedComponentsPath();
        QString libPath = dummy+typeName+"/";
        QDir().mkpath(libPath);
        int solver = pEditDialog->getSolver();

        QFile moFile(libPath+typeName+".mo");
        moFile.open(QFile::WriteOnly | QFile::Truncate);
        moFile.write(pEditDialog->getCode().toUtf8());
        moFile.close();

        coreAccess.generateFromModelica(libPath+typeName+".mo", true, solver, true);
        loadLibrary(libPath+typeName+"_lib.xml");
    }
    delete(pEditDialog);
    return;
}
#endif //EXPERIMENTAL


//! @brief Loads a component library from either XML or folder (deprecated, for backwards compatibility)
//! @param loadPath Path to .xml file (or folder)
//! @param type Specifies whether library is internal or external
//! @param visibility Specifies whether library is visible or invisible
void LibraryHandler::loadLibrary(QString loadPath, LibraryTypeEnumT type, HiddenVisibleEnumT visibility)
{
    QFileInfo libraryLoadPathInfo(loadPath);
    QDir libraryLoadPathRootDir;
    QStringList foundLibraryXmlFiles;

    if (!libraryLoadPathInfo.exists())
    {
        gpMessageHandler->addWarningMessage("Library: "+libraryLoadPathInfo.absoluteFilePath()+" does not exist. The library was not loaded!");
        gpConfig->removeUserLib(loadPath);
        return;
    }

    // If loadpath is a directory then search for library xml files
    if(libraryLoadPathInfo.isDir())
    {
        // Remember root dir path (we use file path, to get all of the path)
        libraryLoadPathRootDir.setPath(libraryLoadPathInfo.absoluteFilePath());

        // Iterate over all xml files in folder and subfolders
        libraryLoadPathRootDir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDot | QDir::NoDotDot);
        libraryLoadPathRootDir.setNameFilters(QStringList() << "*.xml");
        QDirIterator it(libraryLoadPathRootDir, QDirIterator::Subdirectories);
        while(it.hasNext())
        {
            // Read from the xml file
            QFile file(it.next());
            QFileInfo fileInfo(file);

            if(file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                QDomDocument domDocument;
                QString errorStr;
                int errorLine, errorColumn;
                if(domDocument.setContent(&file, false, &errorStr, &errorLine, &errorColumn))
                {
                    // Here only library xml files are interesting, other xml files are ignored
                    QDomElement xmlRoot = domDocument.documentElement();
                    if(xmlRoot.tagName() == QString(XML_LIBRARY))
                    {
                        foundLibraryXmlFiles.append(fileInfo.canonicalFilePath());
                    }
                }
                else
                {
                    gpMessageHandler->addWarningMessage(QString("When looking for Library XML files. Could not parse file: %1, Error: %2, Line: %3, Column: %4")
                                                        .arg(fileInfo.canonicalFilePath()).arg(errorStr).arg(errorLine).arg(errorColumn));
                }
            }
            else
            {
                gpMessageHandler->addWarningMessage(QString("When looking for Library XML files. Could not open file: %1").arg(fileInfo.canonicalFilePath()));
            }
            file.close();
        }
    }
    else if (libraryLoadPathInfo.isFile())
    {
        // Remember root dir path
        libraryLoadPathRootDir.setPath(libraryLoadPathInfo.absolutePath());

        // Read from the file
        QFile file(libraryLoadPathInfo.absoluteFilePath());
        QFileInfo fileInfo(file);
        // IF this is an xml file, then it must be a library file
        if (fileInfo.suffix() == "xml")
        {
            if(file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                QDomDocument domDocument;
                QString errorStr;
                int errorLine, errorColumn;
                if(domDocument.setContent(&file, false, &errorStr, &errorLine, &errorColumn))
                {
                    // The document must have library root tag to be valid
                    QDomElement xmlRoot = domDocument.documentElement();
                    if(xmlRoot.tagName() == QString(XML_LIBRARY))
                    {
                        foundLibraryXmlFiles.append(fileInfo.canonicalFilePath());
                    }
                    else
                    {
                        gpMessageHandler->addErrorMessage(QString("The specified XML file does not have Hopsan library root element. Expected: %1, Found: %2, In: %3")
                                                          .arg(XML_LIBRARY).arg(xmlRoot.tagName()).arg(fileInfo.canonicalFilePath()));
                    }
                }
                else
                {
                    gpMessageHandler->addErrorMessage(QString("Could not parse File: %1, Error: %2, Line: %3, Column: %4. Is it a Library XML file?")
                                                      .arg(fileInfo.canonicalFilePath()).arg(errorStr).arg(errorLine).arg(errorColumn));
                }
            }
            else
            {
                gpMessageHandler->addErrorMessage(QString("Could not open (read) Library XML file: %1").arg(fileInfo.canonicalFilePath()));
            }
            file.close();
        }
        else
        {
            gpMessageHandler->addErrorMessage(QString("The Library XML file must have file suffix .xml, File: %1").arg(fileInfo.canonicalFilePath()));
        }
    }

    bool loadedSomething=false;
    if (!foundLibraryXmlFiles.isEmpty())
    {
        for (QString &xmlFile : foundLibraryXmlFiles)
        {
            SharedComponentLibraryPtrT pLib(new ComponentLibrary);
            pLib->loadPath = loadPath;
            pLib->xmlFilePath = xmlFile;
            pLib->type = type;
            if (loadLibrary(pLib, type, visibility))
            {
                loadedSomething = true;
            }
        }
        return;
    }
    else
    {
        gpMessageHandler->addErrorMessage("Did not find any libary xml files, fall-back " TO_STR(DLL_EXT) " loading is no longer possible");
    }

    if (!loadedSomething)
    {
        gpConfig->removeUserLib(loadPath);
        gpMessageHandler->addWarningMessage("Could not find any component libraries to load!");
    }
}

bool LibraryHandler::isLibraryLoaded(const QString &rLibraryXmlPath, const QString &rLibraryFilePath) const
{
    for (const SharedComponentLibraryPtrT &pLib : mLoadedLibraries)
    {
        if (!rLibraryXmlPath.isEmpty() && (pLib->xmlFilePath == rLibraryXmlPath))
        {
            return true;
        }
        if (!rLibraryFilePath.isEmpty() && (pLib->libFilePath == rLibraryFilePath))
        {
            return true;
        }
    }
    return false;
}

QStringList LibraryHandler::getLoadedLibraryNames() const
{
    QStringList libraries;
    for (const auto& pLibrary : mLoadedLibraries) {
        libraries.append(pLibrary->name);
    }
    return libraries;
}

const SharedComponentLibraryPtrT LibraryHandler::getLibrary(const QString &id) const
{
    for (const auto& pLibrary : mLoadedLibraries) {
        if (pLibrary->id == id) {
            return pLibrary;
        }
    }
    return {};
}


//! @brief Unloads library by component type name
//! @param typeName Type name of any component in the library
bool LibraryHandler::unloadLibraryByComponentType(QString typeName)
{
    // Find the library that the component belongs to
    ComponentLibraryEntry selectedEntry = getEntry(typeName);
    if(!selectedEntry.isValid())
    {
        qDebug() << "Component: " << typeName << " not found.";
        return false; //No component found, probably already unloaded
    }
    if(!selectedEntry.pLibrary)
    {
        qDebug() << "Library with component: " << typeName << " not found.";
        return false; //No library found, ignore (should normally never happen)
    }
    qDebug() << "Unloading component: " << typeName << ".";
    return unloadLibrary(selectedEntry.pLibrary);
}

//! @brief Unloads fmu library by fmu name
//! @param fmuName Name of the fmu to unload
bool LibraryHandler::unloadLibraryFMU(QString fmuName)
{
    // Find the library entery that has fmuName (and is an fmu)
    ComponentLibraryEntry fmuEntry = getFMUEntry(fmuName);
    if(!fmuEntry.isValid())
    {
        qDebug() << "fmuEntry: " << fmuName << " not found.";
        return false;
    }
    return unloadLibrary(fmuEntry.pLibrary);
}

bool LibraryHandler::unloadLibrary(SharedComponentLibraryPtrT pLibrary)
{
    if(pLibrary)
    {
        CoreLibraryAccess core;
        QStringList components, nodes;  //Components and nodes to remove

        components.append(pLibrary->guiOnlyComponents);

        //Generate list of all components and nodes in library
        core.getLibraryContents(pLibrary->libFilePath, components, nodes);

        //Unload the library from HopsanCore
        core.unLoadComponentLib(pLibrary->libFilePath);

        //Remove all unloaded components from library
        for(int c=0; c<components.size(); ++c)
        {
            mLibraryEntries.remove(components[c]);
        }

        // Forget library
        gpConfig->removeUserLib(pLibrary->getLibraryMainFilePath());

        //Remove library from list of loaded libraries
        for(int l=0; l<mLoadedLibraries.size(); ++l)
        {

            if (mLoadedLibraries[l]->getLibraryMainFilePath() == pLibrary->getLibraryMainFilePath())
            {
                mLoadedLibraries.removeAt(l);
                --l;
            }
        }

        gpMessageHandler->collectHopsanCoreMessages();
        emit contentsChanged();

        return true;
    }
    return false;
}

bool LibraryHandler::loadLibrary(SharedComponentLibraryPtrT pLibrary, LibraryTypeEnumT type, HiddenVisibleEnumT visibility)
{
    CoreLibraryAccess coreAccess;
    bool loadedSomething=false;

    QFileInfo libraryMainFileInfo;
    bool isXmlLib=false;
    // Decide the library root dir and if this is an xml library or if it is a "fall-back" dll library
    if (!pLibrary->xmlFilePath.isEmpty())
    {
        isXmlLib = true;
        libraryMainFileInfo.setFile(pLibrary->xmlFilePath);
    }
    else if (!pLibrary->libFilePath.isEmpty())
    {
        gpMessageHandler->addErrorMessage("Fall-back " TO_STR(DLL_EXT) " loading is no longer possible, your library must have a library xml file");
        return false;
    }
    QFileInfo libraryDLLFileInfo(pLibrary->libFilePath);
    QDir libraryRootDir = libraryMainFileInfo.absoluteDir();

    // If this lib has already been loaded then skip it
    if ( isLibraryLoaded(libraryMainFileInfo.canonicalFilePath(), libraryDLLFileInfo.canonicalFilePath()) )
    {
        gpMessageHandler->addWarningMessage("Library: "+libraryMainFileInfo.canonicalFilePath()+" is already loaded");
        return true; // We return true anyway, since the library is actually loaded
    }

    // Load the xml library file
    if (isXmlLib)
    {
        QFile file(libraryMainFileInfo.canonicalFilePath());
        if(file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QDomDocument domDocument;
            QString errorStr;
            int errorLine, errorColumn;
            if(domDocument.setContent(&file, false, &errorStr, &errorLine, &errorColumn))
            {
                QDomElement xmlRoot = domDocument.documentElement();
                if(xmlRoot.tagName() == QString(XML_LIBRARY))
                {
                    // Read name of library
                    pLibrary->name = xmlRoot.firstChildElement(XML_LIBRARY_NAME).text();
                    if (pLibrary->name.isEmpty()) {
                        // Try fall-back loading deprecated name attribute
                        if(xmlRoot.hasAttribute(XML_LIBRARY_NAME)) {
                            pLibrary->name = xmlRoot.attribute(XML_LIBRARY_NAME);
                        }
                    }

                    // A name is required so aborting if it is not present
                    if (pLibrary->name.isEmpty()) {
                        gpMessageHandler->addErrorMessage(QString("Library: %1 is missing the <name> element, or name is empty.")
                                                          .arg(libraryMainFileInfo.canonicalFilePath()));
                        return false;
                    }

                    // Read id of library
                    pLibrary->id = xmlRoot.firstChildElement(XML_LIBRARY_ID).text();
                    if (pLibrary->id.isEmpty()) {
                        gpMessageHandler->addWarningMessage(QString("Library: %1 is missing the <id> element, or id is empty. Using name '%2' as fall-back.")
                                                            .arg(libraryMainFileInfo.canonicalFilePath()).arg(pLibrary->name));
                        pLibrary->id = pLibrary->name;
                    }

                    // Abort loading if a library with the samme ID is already loaded
                    auto alreadyLoadedLibrary = getLibrary(pLibrary->id);
                    if (!alreadyLoadedLibrary.isNull()) {
                        gpMessageHandler->addErrorMessage(QString("A library with ID: %1 is already loaded. Its name is '%2'. If the library you are trying to"
                                                                  " load: '%3' is a copy, it must have a new unique id given in its xml file.")
                                                          .arg(alreadyLoadedLibrary->id).arg(alreadyLoadedLibrary->name).arg(pLibrary->name));
                        return false;
                    }

                    // Read library share lib file
                    QDomElement libElement = xmlRoot.firstChildElement(XML_LIBRARY_LIB);
                    if(!libElement.isNull())
                    {
                        pLibrary->debugExtension = libElement.attribute(XML_LIBRARY_LIB_DBGEXT,"");
                        pLibrary->libFilePath = libraryMainFileInfo.canonicalPath()+"/"+QString(LIBPREFIX)+libElement.text();
#ifdef DEBUGCOMPILING
                        pLibrary->libFilePath += pLibrary->debugExtension;
#endif
                        pLibrary->libFilePath += QString(LIBEXT);
                    }

                    // Read build flags
                    QDomElement bfElement = xmlRoot.firstChildElement("buildflags").firstChildElement();
                    while (!bfElement.isNull())
                    {
                        if (bfElement.tagName() == "cflags")
                        {
                            pLibrary->cflags.append(" "+bfElement.text());
                        }
                        else if (bfElement.tagName() == "lflags")
                        {
                            pLibrary->lflags.append(" "+bfElement.text());
                        }
                        //! @todo handle other elements such as includepath libpath libflag defineflag and such
                        bfElement = bfElement.nextSiblingElement();
                    }

                    // Read source files
                    QDomElement sourceElement = xmlRoot.firstChildElement(QString(XML_LIBRARY_SOURCE));
                    while(!sourceElement.isNull())
                    {
                        pLibrary->sourceFiles.append(QFileInfo(file).canonicalPath()+"/"+sourceElement.text());
                        sourceElement = sourceElement.nextSiblingElement(QString(XML_LIBRARY_SOURCE));
                    }

                    // Remember library (we do this here even if no DLL/SO files are loaded as we might load internal or "gui only" components
                    mLoadedLibraries.append(pLibrary);

                    // Try to load specified compiled library "plugin" file (if available)
                    if(!pLibrary->libFilePath.isEmpty())
                    {
                        if(!coreAccess.loadComponentLib(pLibrary->libFilePath))
                        {
                            // Failed to load
                            gpMessageHandler->collectHopsanCoreMessages();
                            gpMessageHandler->addErrorMessage("Failed to load library: "+pLibrary->libFilePath);
                            // Attempt recompilation if we have a compiler set
                            if (!gpConfig->getGCCPath().isEmpty())
                            {
                                gpMessageHandler->addInfoMessage("Attempting to recompile library: "+pLibrary->name+"...");
                                recompileLibrary(pLibrary,0,true);
                                gpMessageHandler->collectHopsanCoreMessages();

                                // Try to load again
                                if(!coreAccess.loadComponentLib(pLibrary->libFilePath))
                                {
                                    // Still no success, recompilation failed. Ignore and go on.
                                    gpMessageHandler->addErrorMessage("Failed to load recompiled library!");
                                    mLoadedLibraries.pop_back(); //Discard library
                                }
                                else
                                {
                                    // Successful loading after recompilation
                                    gpMessageHandler->addInfoMessage("Success loading recompiled library!");
                                    loadedSomething = true;
                                }
                            }
                            else
                            {
                                gpMessageHandler->addWarningMessage("No compiler path set, will not try to recompile the library!");
                                mLoadedLibraries.pop_back(); //Discard library
                            }
                        }
                        else
                        {
                            // Successful loading
                            loadedSomething = true;
                        }
                    }
                }
                else
                {
                    gpMessageHandler->addErrorMessage(QString("The specified XML file does not have Hopsan library root element. Expected: %1, Found: %2, In: %3")
                                                      .arg(XML_LIBRARY).arg(xmlRoot.tagName()).arg(libraryMainFileInfo.canonicalFilePath()));
                }
            }
            else
            {
                gpMessageHandler->addErrorMessage(QString("Could not parse File: %1, Error: %2, Line: %3, Column: %4. Is it a Library XML file?")
                                                  .arg(libraryMainFileInfo.canonicalFilePath()).arg(errorStr).arg(errorLine).arg(errorColumn));
            }
        }
        else
        {
            gpMessageHandler->addErrorMessage(QString("Could not open (read) Library XML file: %1").arg(libraryMainFileInfo.canonicalFilePath()));
        }
        file.close();
    }

    // Determine where to store any backups of updated appearance xml files
    mUpdateXmlBackupDir.setPath(gpDesktopHandler->getBackupPath() + "updateXML_" + QDate::currentDate().toString("yyMMdd")  + "_" + QTime::currentTime().toString("HHmm"));

    // Recurse sub directories and find all xml caf files
    libraryRootDir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDot | QDir::NoDotDot);
    libraryRootDir.setNameFilters(QStringList() << "*.xml");
    QDirIterator it(libraryRootDir, QDirIterator::Subdirectories);
    bool foundCafFiles = false;
    while(it.hasNext())
    {
        //Read from the xml file
        QFile cafFile(it.next());
        QFileInfo cafFileInfo(cafFile);

        //Iterate over all xml files in folder and subfolders
        if(cafFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QDomDocument domDocument;
            QString errorStr;
            int errorLine, errorColumn;
            if(domDocument.setContent(&cafFile, false, &errorStr, &errorLine, &errorColumn))
            {
                QDomElement cafRoot = domDocument.documentElement();
                if(cafRoot.tagName() == QString(CAF_ROOT))
                {
                    foundCafFiles = true;

                    //Read appearance data from the caf xml file, begin with the first
                    QDomElement xmlModelObjectAppearance = cafRoot.firstChildElement(CAF_MODELOBJECT); //! @todo extend this code to be able to read many appearance objects from same file
                    SharedModelObjectAppearanceT pAppearanceData = SharedModelObjectAppearanceT(new ModelObjectAppearance);
                    pAppearanceData->setBasePath(QFileInfo(cafFile).absolutePath()+"/");
                    pAppearanceData->readFromDomElement(xmlModelObjectAppearance);
                    pAppearanceData->cacheIcons();

                    // Check CAF version, and ask user if they want to update to latest version
                    QString caf_version = cafRoot.attribute(CAF_VERSION);

                    if (caf_version < CAF_VERSIONNUM)
                    {
                        bool doSave=false;
                        if (mUpConvertAllCAF==UndecidedToAll)
                        {
                            QMessageBox questionBox(gpMainWindowWidget);
                            QString text;
                            QTextStream ts(&text);
                            ts << cafFile.fileName() << "\n"
                               << "Your file format is older than the newest version! " << caf_version << "<" << CAF_VERSIONNUM << " Do you want to auto update to the latest format?\n\n"
                               << "NOTE! Your old files will be copied to the hopsan Backup folder, but you should make sure that you have a backup in case something goes wrong.\n"
                               << "NOTE! All non-standard Hopsan contents will be lost\n"
                               << "NOTE! Attributes may change order within a tag (but the order is not important)\n\n"
                               << "If you want to update manually, see the documentation about the latest format version.";
                            questionBox.setWindowTitle("New appearance data format available");
                            questionBox.setText(text);
                            QPushButton* pYes = questionBox.addButton(QMessageBox::Yes);
                            questionBox.addButton(QMessageBox::No);
                            QPushButton* pYesToAll = questionBox.addButton(QMessageBox::YesToAll);
                            QPushButton* pNoToAll = questionBox.addButton(QMessageBox::NoToAll);
                            questionBox.setDefaultButton(QMessageBox::No);
                            if(gpSplash)
                            {
                                gpSplash->close();
                            }
                            questionBox.exec();
                            QAbstractButton* pClickedButton = questionBox.clickedButton();

                            if ( (pClickedButton == pYes) || (pClickedButton == pYesToAll) )
                            {
                                doSave = true;
                            }

                            if (pClickedButton == pYesToAll)
                            {
                                mUpConvertAllCAF = YesToAll;
                            }
                            else if (pClickedButton == pNoToAll)
                            {
                                mUpConvertAllCAF = NoToAll;
                            }
                        }
                        else if (mUpConvertAllCAF==YesToAll)
                        {
                            doSave = true;
                        }

                        if (doSave)
                        {
                            // Make backup of original file
                            QFileInfo newBakFile(mUpdateXmlBackupDir.absolutePath());
                            QDir dir;
                            dir.mkpath(newBakFile.absoluteDir().absolutePath());
                            cafFile.copy(newBakFile.absoluteFilePath());

                            // Save (overwrite original file)
                            pAppearanceData->saveToXMLFile(cafFile.fileName());
                        }
                    }

                    // Verify appearance data loaded from caf file
                    bool success = true;
                    const QString typeName = pAppearanceData->getTypeName();
                    // Do not check in case it is a Subsystem or SystemPort
                    if( !((typeName==HOPSANGUISYSTEMTYPENAME) || (typeName==HOPSANGUICONDITIONALSYSTEMTYPENAME) || (typeName==HOPSANGUICONTAINERPORTTYPENAME)) )
                    {
                        //! @todo maybe they should be reserved in hopsan core instead, then we could aske the core if the exist
                        // Check so that there is such a component available in the Core, or if the component points to an external model file
                        success = coreAccess.hasComponent(typeName) || !pAppearanceData->getHmfFile().isEmpty();
                        if(!success)
                        {
                            gpMessageHandler->addWarningMessage("Failed to load component of type: "+pAppearanceData->getFullTypeName(), "failedtoloadcomp");
                            continue;
                        }
                    }

                    // Success, add component to library
                    if (success)
                    {
                        auto pLibraryBeingLoaded = mLoadedLibraries.last();

                        ComponentLibraryEntry newEntry;
                        newEntry.pLibrary = pLibraryBeingLoaded;

                        // Store appearance data
                        newEntry.pAppearance = pAppearanceData;
                        QString subTypeName = pAppearanceData->getSubTypeName();
                        QString fullTypeName = makeFullTypeString(newEntry.pAppearance->getTypeName(), subTypeName);

                        if (!subTypeName.isEmpty())
                        {
                            // Find what library this component depend on (main component type) and make sure that library knows bout this dependency
                            QString libPath;
                            coreAccess.getLibPathForComponent(pAppearanceData->getTypeName(), libPath);
                            if( !libPath.isEmpty() )
                            {
                                for(int l=0; l<mLoadedLibraries.size(); ++l)
                                {
                                    if( mLoadedLibraries[l]->libFilePath == libPath )
                                    {
                                        mLoadedLibraries[l]->guiOnlyComponents.append(fullTypeName);
                                        break;
                                    }
                                }
                            }

                            // Make this library remember this entry as a gui only component
                            pLibraryBeingLoaded->guiOnlyComponents.append(fullTypeName);
                        }

                        // Make this library remember this entrys appearance file path
                        pLibraryBeingLoaded->cafFiles.append(cafFileInfo.canonicalFilePath());

                        // Calculate path to show in library
                        QString relDir = QDir(libraryMainFileInfo.canonicalPath()).relativeFilePath(cafFileInfo.canonicalFilePath());
                        newEntry.displayPath = relDir.split("/");
                        newEntry.displayPath.removeLast();
                        QString libName = newEntry.pLibrary->name;
                        if (libName.isEmpty()) {
                            libName = libraryRootDir.dirName();
                        }
                        if(type == ExternalLib)
                        {
                            newEntry.displayPath.prepend(libName);
                            newEntry.displayPath.prepend(componentlibrary::roots::externalLibraries);
                        }
                        else if(type == FmuLib)
                        {
                            newEntry.displayPath.prepend(libName);
                            newEntry.displayPath.prepend(componentlibrary::roots::fmus);
                        }

                        // Store visibility
                        newEntry.visibility = visibility;

                        // Store new entry, but only if it does not already exist
                        if(!mLibraryEntries.contains(fullTypeName))
                        {
                            mLibraryEntries.insert(fullTypeName, newEntry);
                            loadedSomething = true;
                            if(gpSplash)
                            {
                                gpSplash->showMessage("Loaded component: " + pAppearanceData->getTypeName());
                            }
                        }
                        else
                        {
                            const auto& existingEntery = mLibraryEntries[fullTypeName];
                            gpMessageHandler->addWarningMessage(QString("A component with type name '%1' was already registered by library '%2'. Ignoring version in library '%3'.")
                                                                .arg(fullTypeName)
                                                                .arg(existingEntery.pLibrary ? existingEntery.pLibrary->name : "Unknown")
                                                                .arg(newEntry.pLibrary ? newEntry.pLibrary->name : "Unknown"));
                        }
                    }
                }
            }
            else
            {
                gpMessageHandler->addErrorMessage(QString("When loading component appearance files. Could not parse file: %1, Error: %2, Line: %3, Column: %4. Is it a component XML file?")
                                                  .arg(cafFileInfo.canonicalFilePath()).arg(errorStr).arg(errorLine).arg(errorColumn));
            }
        }
        else
        {
            gpMessageHandler->addErrorMessage(QString("When loading component appearance files. Could not open (read) file: %1").arg(cafFileInfo.canonicalFilePath()));
        }
        cafFile.close();
    }

    if (!foundCafFiles)
    {
        gpMessageHandler->addWarningMessage(QString("Did not find any component XML files when loading library: %1").arg(pLibrary->getLibraryMainFilePath()));
    }

    gpMessageHandler->collectHopsanCoreMessages();

    if(loadedSomething)
    {
        if(type != InternalLib)
        {
            gpConfig->addUserLib(pLibrary->getLibraryMainFilePath(), type);
        }
        emit contentsChanged();
    }
    else
    {
        gpConfig->removeUserLib(pLibrary->getLibraryMainFilePath());
    }
    return loadedSomething;
}

bool LibraryHandler::isTypeNamesOkToUnload(const QStringList &typeNames)
{
    QStringList models;
    for(int m=0; m<gpModelHandler->count(); ++m)
    {
        bool hasUnloadingComponent=false;
        SystemContainer *pSystem = gpModelHandler->getTopLevelSystem(m);
        Q_FOREACH(const QString &comp, pSystem->getModelObjectNames())
        {
            if(typeNames.contains(pSystem->getModelObject(comp)->getTypeName()))
            {
                hasUnloadingComponent = true;
            }
        }
        if(hasUnloadingComponent)
        {
            models.append(pSystem->getName());
        }
    }

    if(!models.isEmpty())
    {
        QString msg = "The following models are using components from the library:\n\n";
        Q_FOREACH(const QString &model, models)
        {
            msg.append(model+"\n");
        }
        msg.append("\nThey must be closed before unloading.");
        QMessageBox::critical(gpMainWindowWidget, "Unload failed", msg);
        return false;
    }

    return true;
}


//! @brief Returns a list of all loaded component type names
QStringList LibraryHandler::getLoadedTypeNames() const
{
    return mLibraryEntries.keys();
}


//! @brief Returns a component entry in the library
//! @param typeName Type name of component
//! @param subTypeName of component (optional)
ComponentLibraryEntry LibraryHandler::getEntry(const QString &typeName, const QString &subTypeName) const
{
    QString fullTypeString = makeFullTypeString(typeName, subTypeName);
    return mLibraryEntries.value(fullTypeString, ComponentLibraryEntry() );
}

//! @brief Returns an FMU component entry in the library
//! @param rFmuName The FMU name
//! @returns The library entery for given fmu name or an invalid library entery if fmu name not found
ComponentLibraryEntry LibraryHandler::getFMUEntry(const QString &rFmuName) const
{
    //! @todo I dont think we can have multiple component in the same FMU so this should be safe (for now)
    //QString fullTypeString = makeFullTypeString(typeName, subTypeName);
    for (const ComponentLibraryEntry &le : mLibraryEntries.values())
    {
        if (le.pLibrary && le.pLibrary->type == FmuLib)
        {
            // Here it is assumed that the load code prepends FMULIBSTR before the actual path
            if (le.displayPath.last() == rFmuName)
            {
                return le;
            }
        }
    }
    return ComponentLibraryEntry();
}


const SharedModelObjectAppearanceT LibraryHandler::getModelObjectAppearancePtr(const QString &typeName, const QString &subTypeName) const
{
    auto it = mLibraryEntries.find((makeFullTypeString(typeName, subTypeName)));
    if(it != mLibraryEntries.end())
    {
        return it.value().pAppearance;
    }
    else
    {
        return SharedModelObjectAppearanceT();
    }
}

void LibraryHandler::addReplacement(QString type1, QString type2)
{
    //qDebug() << "Adding replacement: " << type1 << ", " << type2;

    if(mReplacementsMap.contains(type1))
    {
        if(!mReplacementsMap.find(type1).value().contains(type2))
        {
            mReplacementsMap.find(type1).value().append(type2);
        }
    }
    else
    {
        mReplacementsMap.insert(type1, QStringList() << type2);
    }

    if(mReplacementsMap.contains(type2))
    {
        if(!mReplacementsMap.find(type2).value().contains(type1))
        {
            mReplacementsMap.find(type2).value().append(type1);
        }
    }
    else
    {
        mReplacementsMap.insert(type2, QStringList() << type1);
    }
}


//! @brief Returns a list of all replacements for specified component
//! @param type Type name to look for
QStringList LibraryHandler::getReplacements(QString type)
{
    if(mReplacementsMap.contains(type))
    {
        return mReplacementsMap.find(type).value();
    }
    return QStringList();
}


//! @brief Imports a functional mock-up unit from a file dialog
void LibraryHandler::importFmu()
{
    //Load .fmu file and create paths
    QString filePath = QFileDialog::getOpenFileName(gpMainWindowWidget, tr("Import Functional Mockup Unit (FMU)"),
                                                    gpConfig->getStringSetting(CFG_FMUIMPORTDIR),
                                                    tr("Functional Mockup Unit (*.fmu)"));
    if(filePath.isEmpty())      //Cancelled by user
        return;

    QFileInfo fmuFileInfo = QFileInfo(filePath);
    if(!fmuFileInfo.exists())
    {
        gpMessageHandler->addErrorMessage("File not found: "+filePath);
        return;
    }
    gpConfig->setStringSetting(CFG_FMUIMPORTDIR, fmuFileInfo.absolutePath());

    importFMU(fmuFileInfo.absoluteFilePath());
}


//! @brief Recompiles specified component library (safe to use with opened models)
//! @param lib Component library to recompile
//! @param solver Solver to use (for Modelica code only)
void LibraryHandler::recompileLibrary(SharedComponentLibraryPtrT pLib, int solver, bool dontUnloadAndLoad)
{
    CoreLibraryAccess coreLibrary;
    auto spGenerator = createDefaultImportGenerator();

    if(!dontUnloadAndLoad)
    {
        //Save GUI state
        gpModelHandler->saveState();

        //Unload library from core
        coreLibrary.unLoadComponentLib(pLib->libFilePath);
    }

    //Generate C++ code from Modelica if source files are Modelica code
    for(const QString &caf : pLib->cafFiles)
    {
        QFile cafFile(caf);
        cafFile.open(QFile::ReadOnly);
        QString code = cafFile.readAll();
        cafFile.close();
        QString path;
        QString sourceFile = code.section("sourcecode=\"",1,1).section("\"",0,0);
        path = QFileInfo(cafFile).path();
        qDebug() << "PATH: " << path;
        if(sourceFile.endsWith(".mo"))
        {
            qDebug() << "GENERATING: " << path+"/"+sourceFile;
            if (!spGenerator->generateFromModelica(path+"/"+sourceFile, solver,
                                                   HopsanGeneratorGUI::CompileT::DoNotCompile))
            {
                gpMessageHandler->addErrorMessage("Failed to import Modelica");
            }
        }
    }

    // Add extra compiler flags for FMU libraries (to make sure we compile with the correct fmi library version shiped with Hopsan)
    // Note we only add the paths here, in case the paths in the FMU wrapper library xml are incorrect (old export)
    QString extraCFlags, extraLFlags;
    if(pLib->type == FmuLib)
    {
        const QString fmiLibDir="/Dependencies/FMILibrary";
        extraCFlags = QString("-I\"%1\"").arg(gpDesktopHandler->getMainPath()+fmiLibDir+"/include");
        extraLFlags = QString("-L\"%1\" -l").arg(gpDesktopHandler->getMainPath()+fmiLibDir+"/lib");
#ifdef _WIN32
    extraLFlags += "libfmilib_shared";
#else
    extraLFlags += "fmilib_shared";  //Remove extra "lib" prefix in Linux
#endif
    }

    //Call compile utility
    QString libfile = QFileInfo(pLib->xmlFilePath).absoluteFilePath();
    if (!spGenerator->compileComponentLibrary(libfile, extraCFlags, extraLFlags))
    {
        gpMessageHandler->addErrorMessage(QString("Failed to compile component library: %1").arg(libfile));
    }

    if(!dontUnloadAndLoad)
    {
        //Load library again
        coreLibrary.loadComponentLib(pLib->libFilePath);

        //Restore GUI state
        gpModelHandler->restoreState();
    }
}

bool ComponentLibraryEntry::isValid() const
{
    return (pLibrary!=nullptr) && (pAppearance!=nullptr) && (!displayPath.isEmpty()) ;
}


QString ComponentLibrary::getLibraryMainFilePath() const
{
    if (xmlFilePath.isEmpty())
    {
        if (!libFilePath.isEmpty())
        {
            return libFilePath;
        }
    }
    else
    {
        return xmlFilePath;
    }
    return "";
}
