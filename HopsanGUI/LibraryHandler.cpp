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
#define XML_LIBRARY_LIB "lib"
#define XML_LIBRARY_LIB_DBGEXT "debug_ext"
#define XML_LIBRARY_CAF "caf"
#define XML_LIBRARY_SOURCE "source"

#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>

//Hopsan includes
#include "global.h"
#include "Configuration.h"
#include "CoreAccess.h"
#include "DesktopHandler.h"
#include "GUIObjects/GUIModelObjectAppearance.h"
#include "LibraryHandler.h"
#include "ModelHandler.h"
#include "version_gui.h"
#include "MessageHandler.h"
#include "Dialogs/EditComponentDialog.h"
#include "GUIObjects/GUISystem.h"

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
        gpMessageHandler->addWarningMessage("Library: "+libraryLoadPathInfo.absoluteFilePath()+" does not exist");
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
                    QDomElement xmlRoot = domDocument.documentElement();
                    if(xmlRoot.tagName() == QString(XML_LIBRARY))
                    {
                        foundLibraryXmlFiles.append(fileInfo.absoluteFilePath());
                    }
                }
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
                    QDomElement xmlRoot = domDocument.documentElement();
                    if(xmlRoot.tagName() == QString(XML_LIBRARY))
                    {
                        foundLibraryXmlFiles.append(fileInfo.absoluteFilePath());
                    }
                }
            }
            file.close();
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
    // Fall back, load dll/so/dynlibs
    else
    {
        gpMessageHandler->addWarningMessage("Did not find any libary xml files, falling back to " TO_STR(DLL_EXT) " loading for: "+loadPath);
        libraryLoadPathRootDir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDot | QDir::NoDotDot);
        libraryLoadPathRootDir.setNameFilters(QStringList() << "*"+QString(LIBEXT));
        QDirIterator itd(libraryLoadPathRootDir, QDirIterator::Subdirectories);
        while(itd.hasNext())
        {
            QString filePath = itd.next();
            SharedComponentLibraryPtrT pLib(new ComponentLibrary);
            pLib->loadPath = loadPath;
            pLib->name = filePath.section("/",-1,-1);
            pLib->libFilePath = filePath;
            pLib->type = type;
            if (loadLibrary(pLib, type, visibility))
            {
                loadedSomething=true;
            }
        }
    }

    if (!loadedSomething)
    {
        gpConfig->removeUserLib(loadPath);
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


//! @brief Unloads library by component type name
//! @param typeName Type name of any component in the library
bool LibraryHandler::unloadLibraryByComponentType(QString typeName)
{
    // Find the library that the component belongs to
    LibraryEntry selectedEntry = getEntry(typeName);
    if(selectedEntry.isNull())
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
    LibraryEntry fmuEntry = getFMUEntry(fmuName);
    if(fmuEntry.isNull())
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
    bool isXmlLib=false, isDllLib=false;
    // Decide the library root dir and if this is an xml library or if it is a "fall back" dll library
    if (!pLibrary->xmlFilePath.isEmpty())
    {
        isXmlLib = true;
        libraryMainFileInfo.setFile(pLibrary->xmlFilePath);
    }
    else if (!pLibrary->libFilePath.isEmpty())
    {
        gpMessageHandler->addWarningMessage("Fallback loading " TO_STR(DLL_EXT));
        isDllLib=true;
        libraryMainFileInfo.setFile(pLibrary->libFilePath);
    }
    QFileInfo libraryDLLFileInfo(pLibrary->libFilePath);
    QDir libraryRootDir = libraryMainFileInfo.absoluteDir();

    // If this lib has already been loaded then skip it
    if ( isLibraryLoaded(libraryMainFileInfo.canonicalFilePath(), libraryDLLFileInfo.canonicalFilePath()) )
    {
        gpMessageHandler->addWarningMessage("Library: "+libraryMainFileInfo.canonicalFilePath()+" is already loaded");
        return true; // We return true anywawy, since the library is actually loaded
    }

    // Load the xml libarry file
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
                    // Set name of library
                    if(xmlRoot.hasAttribute(QString(XML_LIBRARY_NAME)))
                    {
                        pLibrary->name = xmlRoot.attribute(QString(XML_LIBRARY_NAME));
                    }
                    else
                    {
                        pLibrary->name = libraryMainFileInfo.fileName().section(".", 0,0);  //Use filename in case no lib name is provided
                    }

                    // Set library DLL file
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

                    // Set build flags
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

                    // Set source files
                    QDomElement sourceElement = xmlRoot.firstChildElement(QString(XML_LIBRARY_SOURCE));
                    while(!sourceElement.isNull())
                    {
                        pLibrary->sourceFiles.append(QFileInfo(file).absolutePath()+"/"+sourceElement.text());
                        sourceElement = sourceElement.nextSiblingElement(QString(XML_LIBRARY_SOURCE));
                    }

                    // Remember library (we do this here even if no DLL/SO files are loaded as we might load internal or "gui only" components
                    mLoadedLibraries.append(pLibrary);

                    // Try to load specified library file (if available)
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
                                recompileLibrary(pLibrary,true,0,true);
                                gpMessageHandler->collectHopsanCoreMessages();

                                // Try to load again
                                if(!coreAccess.loadComponentLib(pLibrary->libFilePath))
                                {
                                    // Still no success, recompilation failed. Ignore and go on.
                                    gpMessageHandler->addErrorMessage("Recompilation failed.");
                                    mLoadedLibraries.pop_back(); //Discard library
                                }
                                else
                                {
                                    // Successful loading after recompilation
                                    gpMessageHandler->addInfoMessage("Recompilation successful!");
                                    loadedSomething = true;
                                }
                            }
                            else
                            {
                                gpMessageHandler->addWarningMessage("No compiler path set, cant recompile");
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
            }
        }
        file.close();
    }
    // Fallback loading
    else if (isDllLib)
    {
        if(coreAccess.loadComponentLib(libraryDLLFileInfo.canonicalFilePath()))
        {
            mLoadedLibraries.append(pLibrary);
            loadedSomething = true;
        }
        else
        {
            gpMessageHandler->collectHopsanCoreMessages();
            gpMessageHandler->addErrorMessage("Failed to load library: "+libraryMainFileInfo.filePath());
            gpMessageHandler->collectHopsanCoreMessages();
        }
    }


    // Determine where to store any backups of updated appearance xml files
    mUpdateXmlBackupDir.setPath(gpDesktopHandler->getBackupPath() + "updateXML_" + QDate::currentDate().toString("yyMMdd")  + "_" + QTime::currentTime().toString("HHmm"));

    // Recurse sub directories and find all xml caf files
    libraryRootDir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDot | QDir::NoDotDot);
    libraryRootDir.setNameFilters(QStringList() << "*.xml");
    QDirIterator it(libraryRootDir, QDirIterator::Subdirectories);
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
                    //Read appearance data from the caf xml file, begin with the first
                    QDomElement xmlModelObjectAppearance = cafRoot.firstChildElement(CAF_MODELOBJECT); //! @todo extend this code to be able to read many appearance objects from same file
                    ModelObjectAppearance *pAppearanceData = new ModelObjectAppearance;
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
                    if(!((pAppearanceData->getTypeName()==HOPSANGUISYSTEMTYPENAME) || (pAppearanceData->getTypeName()==HOPSANGUICONDITIONALSYSTEMTYPENAME) || (pAppearanceData->getTypeName()==HOPSANGUICONTAINERPORTTYPENAME)) ) //Do not check if it is Subsystem or SystemPort
                    {
                        //! @todo maybe they should be reserved in hopsan core instead
                        success = coreAccess.hasComponent(pAppearanceData->getTypeName()) || !pAppearanceData->getHmfFile().isEmpty(); //Check so that there is such a component available in the Core
                        if(!success)
                        {
                            gpMessageHandler->addWarningMessage("Failed to load component of type: "+pAppearanceData->getFullTypeName(), "failedtoloadcomp");
                            continue;
                        }
                    }

                    //Success, add component to library
                    if (success)
                    {
                        LibraryEntry entry;
                        entry.pLibrary = mLoadedLibraries.last();

                        //Store appearance data
                        entry.pAppearance = pAppearanceData;
                        QString subTypeName = pAppearanceData->getSubTypeName();
                        QString fullTypeName = makeFullTypeString(entry.pAppearance->getTypeName(), subTypeName);

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
                            entry.pLibrary->guiOnlyComponents.append(fullTypeName);
                        }

                        //Store caf file
                        entry.pLibrary->cafFiles.append(cafFileInfo.canonicalFilePath());

                        //Calculate path to show in library
                        QString relDir = QDir(libraryMainFileInfo.canonicalPath()).relativeFilePath(cafFileInfo.canonicalFilePath());
                        entry.path = relDir.split("/");
                        entry.path.removeLast();
                        if(type == ExternalLib)
                        {
                            entry.path.prepend(libraryRootDir.dirName());
                            entry.path.prepend(QString(EXTLIBSTR));
                        }
                        else if(type == FmuLib)
                        {
                            entry.path.prepend(libraryRootDir.dirName());
                            entry.path.prepend(QString(FMULIBSTR));
                        }


                        //Store visibility
                        entry.visibility = visibility;

                        //Store new entry, but only if it does not already exist
                        if(!mLibraryEntries.contains(fullTypeName))
                        {
                            mLibraryEntries.insert(fullTypeName, entry);
                            loadedSomething = true;
                            if(gpSplash)
                            {
                                gpSplash->showMessage("Loaded component: " + pAppearanceData->getTypeName());
                            }
                        }
                        else
                        {
                            gpMessageHandler->addWarningMessage("Component with full type name \""+fullTypeName+"\" is already registered in library handler. Ignored.");
                        }
                    }
                }
            }
        }
        cafFile.close();
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
QStringList LibraryHandler::getLoadedTypeNames()
{
    return mLibraryEntries.keys();
}


//! @brief Returns a component entry in the library
//! @param typeName Type name of component
//! @param subTypeName of component (optional)
LibraryEntry LibraryHandler::getEntry(const QString &typeName, const QString &subTypeName)
{
    QString fullTypeString = makeFullTypeString(typeName, subTypeName);
    return mLibraryEntries.value(fullTypeString, LibraryEntry() );
}

//! @brief Returns an FMU component entry in the library
//! @param rFmuName The FMU name
//! @returns The library entery for given fmu name or an invalid library entery if fmu name not found
LibraryEntry LibraryHandler::getFMUEntry(const QString &rFmuName)
{
    //! @todo I dont think we can have multiple component in the same FMU so this should be safe (for now)
    //QString fullTypeString = makeFullTypeString(typeName, subTypeName);
    foreach (const LibraryEntry &le, mLibraryEntries.values())
    {
        if (le.pLibrary && le.pLibrary->type == FmuLib)
        {
            // Here it is assumed that the load code prepends FMULIBSTR before the actual path
            if (le.path.last() == rFmuName)
            {
                return le;
            }
        }
    }
    return LibraryEntry();
}


ModelObjectAppearance *LibraryHandler::getModelObjectAppearancePtr(const QString &typeName, const QString &subTypeName)
{
    QMap<QString, LibraryEntry>::iterator it = mLibraryEntries.find((makeFullTypeString(typeName, subTypeName)));
    if(it != mLibraryEntries.end())
    {
        return it.value().pAppearance;
    }
    else
    {
        return 0;
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

    CoreGeneratorAccess *pCoreAccess = new CoreGeneratorAccess();
    pCoreAccess->generateFromFmu(filePath);
    delete(pCoreAccess);
}


//! @brief Recompiles specified component library (safe to use with opened models)
//! @param lib Component library to recompile
//! @param solver Solver to use (for Modelica code only)
void LibraryHandler::recompileLibrary(SharedComponentLibraryPtrT pLib, bool showDialog, int solver, bool dontUnloadAndLoad)
{
    CoreLibraryAccess coreLibrary;
    CoreGeneratorAccess coreGenerator;

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
            coreGenerator.generateFromModelica(path+"/"+sourceFile, showDialog, solver, false);
        }
    }

    // Add extra compiler flags for FMU libraries (to make sure we compile with the correct fmi library version shiped with Hopsan)
    // Note we only add the paths here, in case the paths in the FMU wrapper library xml are incorrect (old export)
    QString extraCFlags, extraLFlags;
    if(pLib->type == FmuLib)
    {
#ifdef _WIN64
    QString fmiLibDir="/Dependencies/FMILibrary-2.0.1_x64/";
#else
    QString fmiLibDir="/Dependencies/FMILibrary-2.0.1/";
#endif
        extraCFlags = QString("-I%1").arg(gpDesktopHandler->getMainPath()+fmiLibDir+"install/include");
        extraLFlags = QString("-L%1 -l").arg(gpDesktopHandler->getMainPath()+fmiLibDir+"install/lib");
#ifdef _WIN32
    extraLFlags += "libfmilib_shared";
#else
    extraLFlags += "fmilib_shared";  //Remove extra "lib" prefix in Linux
#endif
    }

    //Call compile utility
    coreGenerator.compileComponentLibrary(QFileInfo(pLib->xmlFilePath).absoluteFilePath(), extraCFlags, extraLFlags, showDialog);

    if(!dontUnloadAndLoad)
    {
        //Load library again
        coreLibrary.loadComponentLib(pLib->libFilePath);

        //Restore GUI state
        gpModelHandler->restoreState();
    }
}

bool LibraryEntry::isNull() const
{
    return ((pLibrary==0) && (pAppearance==0) && (path.isEmpty())) ;
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
