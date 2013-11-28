/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   LibraryHandler.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2013-10-23
//!
//! @brief Contains a class for handling component libraries
//!
//$Id: InitializationThread.cpp 5930 2013-10-03 08:10:27Z robbr48 $

//Defines
#define XML_LIBRARY "hopsancomponentlibrary"
#define XML_LIBRARY_NAME "name"
#define XML_LIBRARY_LIB "lib"
#define XML_LIBRARY_CAF "caf"
#define XML_LIBRARY_SOURCE "source"

//Hopsan includes
#include "global.h"
#include "Configuration.h"
#include "CoreAccess.h"
#include "DesktopHandler.h"
#include "GUIObjects/GUIModelObjectAppearance.h"
#include "LibraryHandler.h"
#include "ModelHandler.h"
#include "version_gui.h"
#include "Widgets/HcomWidget.h"
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
                                                   gpConfig->getExternalLibDir(),
                                                   QFileDialog::ShowDirsOnly
                                                   | QFileDialog::DontResolveSymlinks);
    if(libDir.isEmpty())
    {
        return;
    }
    else
    {
        gpConfig->setExternalLibDir(libDir);

        if(!gpConfig->hasUserLib(libDir))     //Check so that path does not already exist
        {
            loadLibrary(libDir/*, QStringList() << EXTLIBSTR << libDir.section("/",-1,-1)*/);    //Load and register the library in configuration
        }
        else
        {
            gpTerminalWidget->mpConsole->printErrorMessage("Error: Library " + libDir + " is already loaded!");
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
        QString typeName = pEditDialog->getCode().section("model ", 1, 1).section(" ",0,0);
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
    QFileInfo info(xmlPath);
    QStringList cafFiles;
    QStringList loadedLibs;     //! @todo Used for fallback function, remove before 0.7
    CoreLibraryAccess coreAccess;
    bool loadedSomething=false;
    QDir dir;
    if(info.isDir())
        dir.setPath(info.absoluteFilePath());
    else
        dir.setPath(info.absolutePath());


    //Recurse sub directories and find all xml files
    dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDot | QDir::NoDotDot);
    dir.setNameFilters(QStringList() << "*.xml");
    QDirIterator it(dir, QDirIterator::Subdirectories);
    while(it.hasNext())
    {
        //Read from the xml file
        QFile file(it.next());
        QFileInfo fileInfo(file);

        //Iterate over all xml files in folder and subfolders
        qDebug() << "Reading xml from " << fileInfo.filePath();
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
                    tempLib.xmlFilePath = info.filePath();//QFileInfo(file).filePath();

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
                    if(!xmlRoot.firstChildElement(QString(XML_LIBRARY_LIB)).isNull())
                    {
                        tempLib.libFilePath = QFileInfo(file).absolutePath()+"/"+QString(LIBPREFIX)+xmlRoot.firstChildElement(QString(XML_LIBRARY_LIB)).text()+QString(LIBEXT);
                    }

                    //Store source files
                    QDomElement sourceElement = xmlRoot.firstChildElement(QString(XML_LIBRARY_SOURCE));
                    while(!sourceElement.isNull())
                    {
                        tempLib.sourceFiles.append(QFileInfo(file).absolutePath()+"/"+sourceElement.text());
                        sourceElement = sourceElement.nextSiblingElement(QString(XML_LIBRARY_SOURCE));
                    }

                    //Store library entry
                    mLoadedLibraries.append(tempLib);

                    //Try to load specified library file
                    if(!tempLib.libFilePath.isEmpty())
                    {
                        if(!coreAccess.loadComponentLib(tempLib.libFilePath))
                        {
                            //Failed to load, attempt recompilation
                            gpTerminalWidget->checkMessages();
                            gpTerminalWidget->mpConsole->printErrorMessage("Failed to load library: "+it.filePath());
                            gpTerminalWidget->mpConsole->printInfoMessage("Attempting to recompile library: "+tempLib.name+"...");
                            recompileLibrary(tempLib,false,0,true);
                            gpTerminalWidget->checkMessages();

                            //Try to load again
                            if(!coreAccess.loadComponentLib(tempLib.libFilePath))
                            {
                                //Still no success, recompilation failed. Ignore and go on.
                                gpTerminalWidget->mpConsole->printErrorMessage("Recompilation failed.");
                            }
                            else
                            {
                                //Successful loading after recompilation
                                gpTerminalWidget->mpConsole->printInfoMessage("Recompilation successful!");
                                loadedLibs.append(tempLib.libFilePath);
                                loadedSomething = true;
                            }
                        }
                        else
                        {
                            //Successful loading
                            loadedLibs.append(tempLib.libFilePath);
                            loadedSomething = true;
                        }
                    }
                }
            }
        }
    }

    // Determine where to store any backups of updated appearance xml files
    mUpdateXmlBackupDir.setPath(gpDesktopHandler->getBackupPath() + "/updateXML_" + QDate::currentDate().toString("yyMMdd")  + "_" + QTime::currentTime().toString("HHmm"));

    //Recurse sub directories, find all dll files and load them
    //! @todo Fallback, remove this before 0.7
    dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDot | QDir::NoDotDot);
    dir.setNameFilters(QStringList() << "*"+QString(LIBEXT));
    QDirIterator itd(dir, QDirIterator::Subdirectories);
    while(itd.hasNext())
    {
        itd.next();
        if(loadedLibs.contains(itd.filePath()))      //Ignore libraries already loaded above
        {
            continue;
        }
        if(!coreAccess.loadComponentLib(itd.filePath()))
        {

            gpTerminalWidget->checkMessages();
            gpTerminalWidget->mpConsole->printErrorMessage("Failed to load library: "+it.filePath());
            gpTerminalWidget->checkMessages();
        }
        else
        {
            loadedSomething = true;
            ComponentLibrary tempLib;
            tempLib.name = itd.filePath().section("/",-1,-1);
            tempLib.libFilePath = itd.filePath();
            tempLib.type = type;
            tempLib.xmlFilePath.append(info.filePath());
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

    // Load Component XML (CAF Files)
    for (int i = 0; i<cafFiles.size(); ++i)        //Iterate over the file names
    {
        //Open caf XML file and load it to an XML document
        QFile file(cafFiles[i]);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            gpTerminalWidget->mpConsole->printErrorMessage("Failed to open file or not a text file: " + cafFiles[i]);
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
        QDomElement xmlModelObjectAppearance = cafRoot.firstChildElement(CAF_MODELOBJECT); //! @todo extend this code to be able to read many appearace objects from same file, aslo not hardcode tagnames
        ModelObjectAppearance *pAppearanceData = new ModelObjectAppearance;
        pAppearanceData->setBasePath(QFileInfo(cafFiles[i]).absolutePath()+"/");
        pAppearanceData->readFromDomElement(xmlModelObjectAppearance);

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
                   << "If you want to update manually, see the documantation about the latest format version.";
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
        if(!((pAppearanceData->getTypeName()==HOPSANGUISYSTEMTYPENAME) || (pAppearanceData->getTypeName()==HOPSANGUIGROUPTYPENAME) || (pAppearanceData->getTypeName()==HOPSANGUICONTAINERPORTTYPENAME)) ) //Do not check if it is Subsystem or SystemPort
        {
            //! @todo maybe systemport should be in the core component factory (HopsanCore related), not like that right now
            success = coreAccess.hasComponent(pAppearanceData->getTypeName()); //Check so that there is such component availible in the Core
            if(!success)
            {
                gpTerminalWidget->mpConsole->printWarningMessage("Failed to load component: "+pAppearanceData->getTypeName()+", (library is not recompilable)", "failedtoloadcomp");
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
                if(mLoadedLibraries[l].libFilePath == libPath)
                {
                    entry.pLibrary = &mLoadedLibraries[l];
                    break;
                }
            }

            //Store caf file
            entry.pLibrary->cafFiles.append(cafFiles[i]);

            //Calculate path to show in library
            QString relDir = dir.relativeFilePath(cafFiles[i]);
            entry.path = relDir.split("/");
            if(type == External)
            {
                entry.path.prepend(dir.dirName());
                entry.path.prepend(QString(EXTLIBSTR));
            }
            else if(type == FMU)
            {
                entry.path.prepend(QString(FMULIBSTR));
            }
            entry.path.removeLast();

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
                gpTerminalWidget->mpConsole->printWarningMessage("Component with full type name \""+fullTypeName+"\" is already registered in library handler. Ignored.");
            }
        }
    }

    if(loadedSomething)
    {
        if(type != Internal)
        {
            gpConfig->addUserLib(xmlPath);
        }
        emit contentsChanged();
    }
    else
    {
        gpConfig->removeUserLib(xmlPath);
    }

    gpTerminalWidget->checkMessages();
}


//! @brief Unloads library by component type name
//! @param typeName Type name of any component in the library
void LibraryHandler::unloadLibrary(QString typeName)
{
    //Find the library that the component belongs to
    LibraryEntry entry = getEntry(typeName);
    if(!getLoadedTypeNames().contains(typeName))
    {
        qDebug() << "Component: " << typeName << " not found.";
        return; //No component found, probably already unloaded
    }
    ComponentLibrary *pLib = entry.pLibrary;
    if(!pLib)
    {
        qDebug() << "Library with component: " << typeName << " not found.";
        return; //No library found, ignore (should normally never happen)
    }
    qDebug() << "Unloading component: " << typeName << ".";

    CoreLibraryAccess core;

    //Generate list of all components and nodes in library
    QStringList components, nodes;
    core.getLibraryContents(pLib->libFilePath, components, nodes);

    //Unload the library from HopsanCore
    core.unLoadComponentLib(pLib->libFilePath);

    //Remove all unloaded components from library
    for(int c=0; c<components.size(); ++c)
    {
        mLibraryEntries.remove(components[c]);
    }

    //Remove user library from configuration (might remove other libraries as well if they were loaded together, they will not appear next time Hopsan restarts then)
    gpConfig->removeUserLib(pLib->xmlFilePath);

    //Remove library from list of loaded libraries
    QString libFilePath = pLib->libFilePath;
    for(int l=0; l<mLoadedLibraries.size(); ++l)
    {
        if(mLoadedLibraries[l].libFilePath == libFilePath)
        {
            mLoadedLibraries.removeAt(l);
            --l;
        }
    }

    gpTerminalWidget->checkMessages();
    emit contentsChanged();
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
    if(mLibraryEntries.contains(fullTypeString))
    {
        return mLibraryEntries.find(fullTypeString).value();
    }
    else
    {
        return LibraryEntry();
    }
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
    qDebug() << "Adding replacement: " << type1 << ", " << type2;

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
                                                    gpConfig->getFmuImportDir(),
                                                    tr("Functional Mockup Unit (*.fmu)"));
    if(filePath.isEmpty())      //Cancelled by user
        return;

    QFileInfo fmuFileInfo = QFileInfo(filePath);
    if(!fmuFileInfo.exists())
    {
        gpTerminalWidget->mpConsole->printErrorMessage("File not found: "+filePath);
        return;
    }
    gpConfig->setFmuImportDir(fmuFileInfo.absolutePath());

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
    Q_FOREACH(const QString &caf, lib.cafFiles)
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
    QString extraLibs = "";
    if(lib.type == FMU)
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



