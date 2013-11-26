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
#ifdef DEVELOPMENT
        QString libDir = QFileDialog::getOpenFileName(gpMainWindowWidget, tr("Choose file"), gpConfig->getExternalLibDir());
        libDir.replace("\\","/");   //Enforce unix-style on path
#else
        QString libDir = QFileDialog::getExistingDirectory(gpMainWindowWidget, tr("Choose Library Directory"),
                                                       gpConfig->getExternalLibDir(),
                                                       QFileDialog::ShowDirsOnly
                                                       | QFileDialog::DontResolveSymlinks);
#endif
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
    ComponentLibrary newLib;
    newLib.type = type;

    QFileInfo info(xmlPath);
    QString libPath = info.path()+"/";
    QDir dir;

    if(info.isDir())        //Fallback function for backwards compatibility, allows user to load a directory instead of an xml file
    {
        newLib.xmlFilePath = xmlPath;
        dir.setPath(xmlPath);
//        newLib.name = dir.dirName();

//        dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDot | QDir::NoDotDot);
//        dir.setNameFilters(QStringList() << "*"+QString(LIBEXT));
//        if(!dir.entryList().isEmpty())
//        {
//            newLib.libFilePath = dir.absoluteFilePath(dir.entryList().first());
//        }
    }
    else
    {
        dir.setPath(libPath);

        //Read from library description xml file
        QFile file(xmlPath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            gpTerminalWidget->mpConsole->printErrorMessage("Unable to open file: "+QFile(xmlPath).fileName());
            if(type != Internal)
            {
                gpConfig->removeUserLib(xmlPath);
            }
            return;
        }

        QDomDocument domDocument;
        QString errorStr;
        int errorLine, errorColumn;
        if (!domDocument.setContent(&file, false, &errorStr, &errorLine, &errorColumn))
        {
            gpTerminalWidget->mpConsole->printErrorMessage(QFile(xmlPath).fileName()+tr(": Parse error at line %1, column %2:\n%3")
                                     .arg(errorLine)
                                     .arg(errorColumn)
                                     .arg(errorStr));
            return;
        }
        QDomElement libRoot = domDocument.documentElement();
        if (libRoot.tagName() != QString(XML_LIBRARY))
        {
           gpTerminalWidget->mpConsole->printErrorMessage(tr("The file is not a Hopsan component library file. Incorrect hmf root tag name: ")
                                     + libRoot.tagName() + " != "+QString(XML_LIBRARY));
            return;
        }

        //Store path to own xml file
        newLib.xmlFilePath = xmlPath;

        //Store name of library
        if(libRoot.hasAttribute(QString(XML_LIBRARY_NAME)))
        {
            newLib.name = libRoot.attribute(QString(XML_LIBRARY_NAME));
        }
        else
        {
            newLib.name = QFileInfo(xmlPath).fileName().section(".", 0,0);  //Use filename in case no lib name is provided
        }

        //Store library file
        if(libRoot.firstChildElement(QString(XML_LIBRARY_LIB)).isNull())
        {
            //gpTerminalWidget->mpConsole->printErrorMessage("No binary file specified in library description file.");
            //return;
        }
        else
        {
            newLib.libFilePath = libPath+QString(LIBPREFIX)+libRoot.firstChildElement(QString(XML_LIBRARY_LIB)).text()+QString(LIBEXT);
        }

        //Store source files
        QDomElement sourceElement = libRoot.firstChildElement(QString(XML_LIBRARY_SOURCE));
        while(!sourceElement.isNull())
        {
            newLib.sourceFiles.append(libPath+sourceElement.text());
            sourceElement = sourceElement.nextSiblingElement(QString(XML_LIBRARY_SOURCE));
        }
    }

    //Recurse sub directories and find all xml files
    dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDot | QDir::NoDotDot);
    dir.setNameFilters(QStringList() << "*.xml");
    QDirIterator it(dir, QDirIterator::Subdirectories);
    while(it.hasNext())
    {
        newLib.cafFiles.append(it.next());
        if(QDir::cleanPath(newLib.cafFiles.last()) == QDir::cleanPath(xmlPath))
        {
            newLib.cafFiles.removeLast();   //Don't store own xml file as a caf file
        }
    }

    // Determine where to store any backups of updated appearance xml files
    mUpdateXmlBackupDir.setPath(gpDesktopHandler->getBackupPath() + "/updateXML_" + QDate::currentDate().toString("yyMMdd")  + "_" + QTime::currentTime().toString("HHmm"));

    bool loadedSomething=false;

    //Load the library file
    CoreLibraryAccess coreAccess;
    if(info.isDir())
    {
        //Recurse sub directories and find all dll files
        dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDot | QDir::NoDotDot);
        dir.setNameFilters(QStringList() << "*"+QString(LIBEXT));
        QDirIterator it(dir, QDirIterator::Subdirectories);
        while(it.hasNext())
        {
            it.next();
            if(!coreAccess.loadComponentLib(it.filePath()))
            {
                gpTerminalWidget->checkMessages();
                gpTerminalWidget->mpConsole->printErrorMessage("Failed to load library: "+it.filePath());
                gpTerminalWidget->checkMessages();
            }
            else
            {
                loadedSomething = true;
                ComponentLibrary tempLib;
                tempLib.name = it.filePath().section("/",-1,-1);
                tempLib.libFilePath = it.filePath();
                tempLib.type = type;
                tempLib.xmlFilePath.append(xmlPath);
                mLoadedLibraries.append(tempLib);
            }
        }
    }
    else
    {
        if(!newLib.libFilePath.isEmpty())
        {
            if(!coreAccess.loadComponentLib(newLib.libFilePath))
            {
                gpTerminalWidget->checkMessages();
                gpTerminalWidget->mpConsole->printErrorMessage("Failed to load library: "+QFile(newLib.libFilePath).fileName());
                gpTerminalWidget->checkMessages();
                if(!newLib.sourceFiles.isEmpty())
                {
                    gpTerminalWidget->mpConsole->printInfoMessage("Attempting to recompile...");
                    recompileLibrary(newLib,false);
                    gpTerminalWidget->checkMessages();
                    if(!coreAccess.loadComponentLib(newLib.libFilePath))
                    {
                        gpTerminalWidget->mpConsole->printErrorMessage("Recompilation failed.");
                        gpConfig->removeUserLib(newLib.xmlFilePath);
                        gpTerminalWidget->checkMessages();
                        return;
                    }
                    else
                    {
                        gpTerminalWidget->mpConsole->printInfoMessage("Recompilation successful!");
                    }
                }
            }
        }
    }

    if(!info.isDir())
    {
        mLoadedLibraries.append(newLib);
    }

    // Load Component XML (CAF Files)
    bool alreadyFailed=false;
    for (int i = 0; i<newLib.cafFiles.size(); ++i)        //Iterate over the file names
    {
        QFile file(newLib.cafFiles[i]);   //Create a QFile object
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))  //open each file
        {
            gpTerminalWidget->mpConsole->printErrorMessage("Failed to open file or not a text file: " + newLib.cafFiles[i]);
            continue;
        }

        ModelObjectAppearance *pAppearanceData = new ModelObjectAppearance;

        QDomDocument domDocument;        //Read appearance from file, First check if xml
        QString errorStr;
        int errorLine, errorColumn;
        if (domDocument.setContent(&file, false, &errorStr, &errorLine, &errorColumn))
        {
            //! @todo check caf version
            QDomElement cafRoot = domDocument.documentElement();
            if (cafRoot.tagName() != CAF_ROOT)
            {
                //gpTerminalWidget->mpConsole->printDebugMessage(file.fileName() + ": The file is not an Hopsan Component Appearance Data file. Incorrect caf root tag name: " + cafRoot.tagName() + "!=" + CAF_ROOT);
                continue;
            }
            else
            {
                //Read appearance data from the caf xml file, begin with the first
                QDomElement xmlModelObjectAppearance = cafRoot.firstChildElement(CAF_MODELOBJECT); //! @todo extend this code to be able to read many appearace objects from same file, aslo not hardcode tagnames
                pAppearanceData->setBasePath(QFileInfo(newLib.cafFiles[i]).absolutePath()+"/");
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
                        questionBox.setWindowTitle("A NEW appearance data format is available!");
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
                        //Close file
                        file.close();

                        // Make backup of original file
                        QFileInfo newBakFile(mUpdateXmlBackupDir.absolutePath());
                        QDir dir;
                        dir.mkpath(newBakFile.absoluteDir().absolutePath());
                        file.copy(newBakFile.absoluteFilePath());

                        // Save (overwrite original file)
                        pAppearanceData->saveToXMLFile(file.fileName());

                    }
                }
            }
        }
        else
        {
            gpTerminalWidget->mpConsole->printWarningMessage(tr("Failed to read appearance data: from %4, Parse error at line %1, column %2: %3")
                                                             .arg(errorLine)
                                                             .arg(errorColumn)
                                                             .arg(errorStr)
                                                             .arg(file.fileName()));
            continue;
        }

        //Close file
        file.close();

        //! @todo maybe use the convenient helpfunction for the stuff above (open file and check xml and root tagname) now that we have one

        bool success = true;

        //! @todo maybe we need to check appearance data for a minimuma amount of necessary data
        if(!((pAppearanceData->getTypeName()==HOPSANGUISYSTEMTYPENAME) || (pAppearanceData->getTypeName()==HOPSANGUIGROUPTYPENAME) || (pAppearanceData->getTypeName()==HOPSANGUICONTAINERPORTTYPENAME)) ) //Do not check if it is Subsystem or SystemPort
        {
            //! @todo maybe systemport should be in the core component factory (HopsanCore related), not like that right now
                //Check that component is registered in core
            success = coreAccess.hasComponent(pAppearanceData->getTypeName()); //Check so that there is such component availible in the Core
            if (!success && !alreadyFailed && !newLib.sourceFiles.isEmpty())
            {
                alreadyFailed=true;
                gpTerminalWidget->mpConsole->printWarningMessage("Failed to load component: "+pAppearanceData->getTypeName());
                gpTerminalWidget->mpConsole->printInfoMessage("Attempting to recompile library: "+newLib.name+"...");

                recompileLibrary(newLib,false);
                --i;
                continue;
            }
            else if(!success && alreadyFailed)
            {
                gpTerminalWidget->mpConsole->printErrorMessage("Recompilation failed.");
                alreadyFailed=false;
                continue;
            }
            else if(!success)
            {
                gpTerminalWidget->mpConsole->printWarningMessage("Failed to load component: "+pAppearanceData->getTypeName());
                gpTerminalWidget->mpConsole->printInfoMessage("(library is not recompilable)");
                continue;
            }
        }

        if (success)
        {
            loadedSomething = true;

            LibraryEntry entry;
            entry.pAppearance = pAppearanceData;
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

            if(entry.pLibrary)
            {
                qDebug() << "Component: " << pAppearanceData->getTypeName() << " is loaded from: " << entry.pLibrary->libFilePath;
            }

            entry.pLibrary->cafFiles.append(newLib.cafFiles[i]);

            QString relDir = dir.relativeFilePath(newLib.cafFiles[i]);
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

            //entry.pLibrary = &mLoadedLibraries.last();
            entry.visibility = visibility;
            QString fullTypeName = makeFullTypeString(pAppearanceData->getTypeName(), pAppearanceData->getSubTypeName());
            if(!mLibraryEntries.contains(fullTypeName))
            {
                mLibraryEntries.insert(fullTypeName, entry);
                qDebug() << "Adding: " << pAppearanceData->getTypeName();
            }
            else
            {
                gpTerminalWidget->mpConsole->printWarningMessage("Component with full type name \""+fullTypeName+"\" is already registered in library handler. Ignored.");
            }
            if(gpSplash)
            {
                gpSplash->showMessage("Loaded component: " + pAppearanceData->getTypeName());
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
        if(!info.isDir())
        {
            mLoadedLibraries.removeLast();
        }
        gpConfig->removeUserLib(xmlPath);
    }

    gpTerminalWidget->checkMessages();
}


//! @brief Unloads library by component type name
//! @param typeName Type name of any component in the library
void LibraryHandler::unloadLibrary(QString typeName)
{
    qDebug() << "Unloading from component: " << typeName;
    ComponentLibrary *pLib = gpLibraryHandler->getEntry(typeName).pLibrary;
    if(!pLib)
    {
        return;
    }
    qDebug() << "Unloading: " << pLib->name;
    CoreLibraryAccess core;
    QStringList components, nodes;

    core.getLibraryContents(pLib->libFilePath, components, nodes);
    core.unLoadComponentLib(pLib->libFilePath);

    qDebug() << "Unloading components: " << components;
    for(int c=0; c<components.size(); ++c)
    {
        mLibraryEntries.remove(components[c]);
    }

    gpConfig->removeUserLib(pLib->xmlFilePath);

    gpTerminalWidget->checkMessages();
    emit contentsChanged();
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
void LibraryHandler::recompileLibrary(ComponentLibrary lib, bool showDialog, int solver)
{
    CoreLibraryAccess coreLibrary;
    CoreGeneratorAccess coreGenerator;

    //Save GUI state
    gpModelHandler->saveState();

    //Unload library from core
    coreLibrary.unLoadComponentLib(lib.libFilePath);

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

    //Load library again
    coreLibrary.loadComponentLib(lib.libFilePath);

    //Restore GUI state
    gpModelHandler->restoreState();
}



