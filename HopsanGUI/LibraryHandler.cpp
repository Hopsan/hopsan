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

        // Check so that lib is not already loaded
        if(!gpConfig->hasUserLib(libDir))
        {
            loadLibrary(libDir);
        }
        else
        {
            gpMessageHandler->addErrorMessage("Error: Library " + libDir + " is already loaded!");
        }
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
//! @param xmlPath Path to .xml file (or folder)
//! @param type Specifies whether library is internal or external
//! @param visibility Specifies whether library is visible or invisible
void LibraryHandler::loadLibrary(QString xmlPath, LibraryTypeEnumT type, HiddenVisibleEnumT visibility)
{
    QFileInfo libraryRootXmlFileInfo(xmlPath);
    QDir libraryRootDir;
    QStringList loadedSubLibraryXmls;
    QStringList cafFiles;
    QStringList loadedDllLibs;     //! @todo Used for fallback function, remove before 0.7
    CoreLibraryAccess coreAccess;
    bool loadedSomething=false;

    // If xmlPath was a directory then try to find a root library xml in that directory
    // it is best if we can load such a file instead of loading the directory (pre 0.7 style)
    if(libraryRootXmlFileInfo.isDir())
    {
        // Remember root dir path (we use file path, to get all of the path)
        libraryRootDir.setPath(libraryRootXmlFileInfo.absoluteFilePath());
    }
    // Ok a file was specified, lets set the root dir
    else
    {
        libraryRootDir.setPath(libraryRootXmlFileInfo.absolutePath());
    }


    //Recurse sub directories and find all xml files
    libraryRootDir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDot | QDir::NoDotDot);
    libraryRootDir.setNameFilters(QStringList() << "*.xml");
    QDirIterator it(libraryRootDir, QDirIterator::Subdirectories);
    while(it.hasNext())
    {
        //Read from the xml file
        QFile file(it.next());
        QFileInfo fileInfo(file);

        //Iterate over all xml files in folder and subfolders
        //qDebug() << "Reading xml from " << fileInfo.filePath();
        if(file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QDomDocument domDocument;
            QString errorStr;
            int errorLine, errorColumn;
            if(domDocument.setContent(&file, false, &errorStr, &errorLine, &errorColumn))
            {
                QDomElement xmlRoot = domDocument.documentElement();
                if(xmlRoot.tagName() == QString(CAF_ROOT))
                {
                    cafFiles.append(fileInfo.filePath());
                }
                else if(xmlRoot.tagName() == QString(XML_LIBRARY))
                {
                    ComponentLibrary tempLib;

                    //Store path to own xml file
                    //tempLib.xmlFilePath = libRootXmlFileInfo.filePath();
                    tempLib.xmlFilePath = fileInfo.filePath();
                    loadedSubLibraryXmls.append(tempLib.xmlFilePath);

                    //Store name of library
                    if(xmlRoot.hasAttribute(QString(XML_LIBRARY_NAME)))
                    {
                        tempLib.name = xmlRoot.attribute(QString(XML_LIBRARY_NAME));
                    }
                    else
                    {
                        tempLib.name = QFileInfo(xmlPath).fileName().section(".", 0,0);  //Use filename in case no lib name is provided
                    }

                    //Store library file
                    QDomElement libElement = xmlRoot.firstChildElement(XML_LIBRARY_LIB);
                    if(!libElement.isNull())
                    {
                        tempLib.debugExtension = libElement.attribute(XML_LIBRARY_LIB_DBGEXT,"");
                        tempLib.libFilePath = fileInfo.absolutePath()+"/"+QString(LIBPREFIX)+libElement.text();
#ifdef DEBUGCOMPILING
                        tempLib.libFilePath += tempLib.debugExtension;
#endif
                        tempLib.libFilePath += QString(LIBEXT);
                    }

                    //Store source files
                    QDomElement sourceElement = xmlRoot.firstChildElement(QString(XML_LIBRARY_SOURCE));
                    while(!sourceElement.isNull())
                    {
                        tempLib.sourceFiles.append(QFileInfo(file).absolutePath()+"/"+sourceElement.text());
                        sourceElement = sourceElement.nextSiblingElement(QString(XML_LIBRARY_SOURCE));
                    }

                    // Remember library (we do this here even if no DLL/SO files are loaded as we might load internal or "gui only" components
                    mLoadedLibraries.append(tempLib);

                    //Try to load specified library file
                    if(!tempLib.libFilePath.isEmpty())
                    {
                        if(!coreAccess.loadComponentLib(tempLib.libFilePath))
                        {
                            //Failed to load, attempt recompilation
                            gpMessageHandler->collectHopsanCoreMessages();
                            gpMessageHandler->addErrorMessage("Failed to load library: "+it.filePath());
                            gpMessageHandler->addInfoMessage("Attempting to recompile library: "+tempLib.name+"...");
                            recompileLibrary(tempLib,false,0,true);
                            gpMessageHandler->collectHopsanCoreMessages();

                            //Try to load again
                            if(!coreAccess.loadComponentLib(tempLib.libFilePath))
                            {
                                //Still no success, recompilation failed. Ignore and go on.
                                gpMessageHandler->addErrorMessage("Recompilation failed.");
                            }
                            else
                            {
                                //Successful loading after recompilation
                                gpMessageHandler->addInfoMessage("Recompilation successful!");
                                loadedDllLibs.append(tempLib.libFilePath);
                                loadedSomething = true;
                            }
                        }
                        else
                        {
                            //Successful loading
                            loadedDllLibs.append(tempLib.libFilePath);
                            loadedSomething = true;
                        }
                    }
                }
            }
        }
    }

    // Determine where to store any backups of updated appearance xml files
    mUpdateXmlBackupDir.setPath(gpDesktopHandler->getBackupPath() + "updateXML_" + QDate::currentDate().toString("yyMMdd")  + "_" + QTime::currentTime().toString("HHmm"));

    //Recurse sub directories, find all dll files and load them (FALLBACK for pre 0.7 load style (loading dll directly))
    if (!loadedSomething)
    {
        //! @todo Fallback, remove this before 0.7
        libraryRootDir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDot | QDir::NoDotDot);
        libraryRootDir.setNameFilters(QStringList() << "*"+QString(LIBEXT));
        QDirIterator itd(libraryRootDir, QDirIterator::Subdirectories);
        while(itd.hasNext())
        {
            itd.next();
            if(loadedDllLibs.contains(itd.filePath()))      //Ignore libraries already loaded above
            {
                continue;
            }
            if(!coreAccess.loadComponentLib(itd.filePath()))
            {

                gpMessageHandler->collectHopsanCoreMessages();
                gpMessageHandler->addErrorMessage("Failed to load library: "+it.filePath());
                gpMessageHandler->collectHopsanCoreMessages();
            }
            else
            {
                loadedSomething = true;
                ComponentLibrary tempLib;
                tempLib.name = itd.filePath().section("/",-1,-1);
                tempLib.libFilePath = itd.filePath();
                tempLib.type = type;
                tempLib.xmlFilePath.append(libraryRootDir.canonicalPath());
                bool exists=false;
                for(int l=0; l<mLoadedLibraries.size(); ++l)
                {
                    if(mLoadedLibraries[l].libFilePath == tempLib.libFilePath)
                    {
                        exists=true;
                    }
                }
                if(!exists)
                {
                    mLoadedLibraries.append(tempLib);
                }
            }
        }
    }

    ComponentLibrary *pTempLibrary = 0; //Used in case we are loading components without dll files

    // Load Component XML (CAF Files)
    for (const QString &cafFile : cafFiles)
    {
        //Open caf XML file and load it to an XML document
        QFile file(cafFile);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            gpMessageHandler->addErrorMessage("Failed to open file or not a text file: " + cafFile);
            continue;
        }
        QDomDocument domDocument;
        QDomElement cafRoot = loadXMLDomDocument(file, domDocument, CAF_ROOT);
        file.close();
        if(cafRoot.isNull())
        {
            continue;
        }

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
                ts << file.fileName() << "\n"
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
                file.copy(newBakFile.absoluteFilePath());

                // Save (overwrite original file)
                pAppearanceData->saveToXMLFile(file.fileName());
            }
        }

        //Verify appearance data loaded from caf file
        bool success = true;
        if(!((pAppearanceData->getTypeName()==HOPSANGUISYSTEMTYPENAME) || (pAppearanceData->getTypeName()==HOPSANGUICONDITIONALSYSTEMTYPENAME) || (pAppearanceData->getTypeName()==HOPSANGUICONTAINERPORTTYPENAME)) ) //Do not check if it is Subsystem or SystemPort
        {
            //! @todo maybe they should be reserved in hopsan core instead
            success = coreAccess.hasComponent(pAppearanceData->getTypeName()) || !pAppearanceData->getHmfFile().isEmpty(); //Check so that there is such a component available in the Core
            if(!success)
            {
                gpMessageHandler->addWarningMessage("Failed to load component: "+pAppearanceData->getTypeName()+", (library is not recompilable)", "failedtoloadcomp");
                continue;
            }
        }

        //Success, add component to library
        if (success)
        {
            LibraryEntry entry;

            //Store appearance data
            entry.pAppearance = pAppearanceData;

            //Find and store library from where component belongs
            QString libPath;
            coreAccess.getLibPathForComponent(pAppearanceData->getTypeName(), libPath);
            entry.pLibrary = 0;
            for(int l=0; l<mLoadedLibraries.size(); ++l)
            {
                if(mLoadedLibraries[l].libFilePath == libPath && !libPath.isEmpty())
                {
                    entry.pLibrary = &mLoadedLibraries[l];
                    break;
                }
                else if(libPath.isEmpty())
                {
                    if(!pTempLibrary)
                    {
                        mLoadedLibraries.append(ComponentLibrary());
                        pTempLibrary = &mLoadedLibraries.last();
                        pTempLibrary->name = libraryRootDir.dirName();
                        pTempLibrary->type = ExternalLib;
                        pTempLibrary->xmlFilePath = libraryRootXmlFileInfo.canonicalFilePath();
                    }
                    //! @todo This is dangerous, pointing to a list element, what if the list is reallocated, normally it is not but still, also right now this list is never cleared when libraries are unloaded /Peter
                    entry.pLibrary = pTempLibrary;
                    pTempLibrary->guiOnlyComponents.append(entry.pAppearance->getTypeName());
                }
            }

            //Store caf file
            entry.pLibrary->cafFiles.append(cafFile);

            //Calculate path to show in library
            QString relDir = libraryRootDir.relativeFilePath(cafFile);
            entry.path = relDir.split("/");
            entry.path.removeLast();
            if(type == ExternalLib)
            {
                entry.path.prepend(libraryRootDir.dirName());
                entry.path.prepend(QString(EXTLIBSTR));
            }
            else if(type == FmuLib)
            {
                entry.path.prepend(QString(FMULIBSTR));
            }


            //Store visibility
            entry.visibility = visibility;

            //Store new entry, but only if it does not already exist
            QString fullTypeName = makeFullTypeString(pAppearanceData->getTypeName(), pAppearanceData->getSubTypeName());
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

    if(loadedSomething)
    {
        if(type != InternalLib)
        {
            // If one unique library then remember the xml
            if (loadedSubLibraryXmls.size() == 1)
            {
                gpConfig->addUserLib(loadedSubLibraryXmls.first(), type);
            }
            // Else remeber the root dir
            else
            {
                gpConfig->addUserLib(libraryRootDir.canonicalPath(), type);
            }
        }
        emit contentsChanged();
    }
    else
    {
        // If one unique library then forget the xml
        if (loadedSubLibraryXmls.size() == 1)
        {
            gpConfig->removeUserLib(loadedSubLibraryXmls.first());
        }
        // Else forget the root dir
        else
        {
            gpConfig->removeUserLib(libraryRootDir.canonicalPath());
        }
    }

    gpMessageHandler->collectHopsanCoreMessages();
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
    ComponentLibrary *pLib = selectedEntry.pLibrary;
    if(!pLib)
    {
        qDebug() << "Library with component: " << typeName << " not found.";
        return false; //No library found, ignore (should normally never happen)
    }
    qDebug() << "Unloading component: " << typeName << ".";
    return unloadLibrary(pLib);
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

bool LibraryHandler::unloadLibrary(ComponentLibrary *pLibrary)
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

        //Remove user library from configuration (might remove other libraries as well if they were loaded together, they will not appear next time Hopsan restarts then)
        gpConfig->removeUserLib(pLibrary->xmlFilePath);

        //Remove library from list of loaded libraries
        QString libFilePath = pLibrary->libFilePath;
        for(int l=0; l<mLoadedLibraries.size(); ++l)
        {
            if(mLoadedLibraries[l].libFilePath == libFilePath)
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
        // This indexing hack assumes that the load code prepends FMULIBSTR before the actaul fmuName and fmu component type name
        int i = le.path.size()-2;
        if ( (i>=0) && (le.path[i] == FMULIBSTR) )
        {
            if (le.path[i+1] == rFmuName)
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
void LibraryHandler::recompileLibrary(ComponentLibrary lib, bool showDialog, int solver, bool dontUnloadAndLoad)
{
    CoreLibraryAccess coreLibrary;
    CoreGeneratorAccess coreGenerator;

    if(!dontUnloadAndLoad)
    {
        //Save GUI state
        gpModelHandler->saveState();

        //Unload library from core
        coreLibrary.unLoadComponentLib(lib.libFilePath);
    }

    //Generate C++ code from Modelica if source files are Modelica code
    for(const QString &caf : lib.cafFiles)
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

    //Add extra libs for FMU libraries
    //! @todo Why is this needed ???? /Peter
    QString extraLibs = "";
    if(lib.type == FmuLib)
    {
        extraLibs = "-L./ -llibexpat";
    }

    //Call compile utility
    coreGenerator.compileComponentLibrary(QFileInfo(lib.xmlFilePath).absoluteFilePath(), extraLibs, showDialog);

    if(!dontUnloadAndLoad)
    {
        //Load library again
        coreLibrary.loadComponentLib(lib.libFilePath);

        //Restore GUI state
        gpModelHandler->restoreState();
    }
}





LibraryEntry::LibraryEntry()
{
    pAppearance = 0;
    pLibrary = 0;
    visibility = Hidden;
}

bool LibraryEntry::isNull() const
{
    return ((pLibrary==0) && (pAppearance==0) && (path.isEmpty())) ;
}
