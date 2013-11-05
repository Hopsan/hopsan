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
{
    mUpConvertAllCAF = UndecidedToAll;
}


//! @brief Loads a component library from either XML or folder (deprecated, for backwards compatibility)
//! @param xmlPath Path to .xml file (or folder)
//! @param type Specifies whether library is internal or external
//! @param visibility Specifies whether library is visible or invisible
void LibraryHandler::loadLibrary(QString xmlPath, InternalExternalEnumT type, HiddenVisibleEnumT visibility)
{
    ComponentLibrary newLib;

    QFileInfo info(xmlPath);
    QString libPath = info.path()+"/";
    QDir dir;

    if(info.isDir())        //Fallback function for backwards compatibility, allows user to load a directory instead of an xml file
    {
        newLib.xmlFilePath = xmlPath;
        dir.setPath(xmlPath);
        newLib.name = dir.dirName();

        dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDot | QDir::NoDotDot);
        dir.setNameFilters(QStringList() << "*"+QString(LIBEXT));
        newLib.libFilePath = dir.absoluteFilePath(dir.entryList().first());
    }
    else
    {
        dir.setPath(libPath);

        //Read from library description xml file
        QFile file(xmlPath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            gpTerminalWidget->mpConsole->printErrorMessage("Unable to open file: "+QFile(xmlPath).fileName());
            if(type == External)
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
            newLib.libFilePath = libPath+libRoot.firstChildElement(QString(XML_LIBRARY_LIB)).text();
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

    //Load the library file
    CoreLibraryAccess coreAccess;
    if(!newLib.libFilePath.isEmpty())
    {
        if(!coreAccess.loadComponentLib(newLib.libFilePath))
        {
            gpTerminalWidget->mpConsole->printErrorMessage("Failed to load library: "+QFile(newLib.libFilePath).fileName());
            gpTerminalWidget->checkMessages();
            if(!newLib.sourceFiles.isEmpty())
            {
                gpTerminalWidget->mpConsole->printInfoMessage("Attempting to recompile...");
                recompileLibrary(newLib);
                if(!coreAccess.loadComponentLib(newLib.libFilePath))
                {
                    gpTerminalWidget->mpConsole->printErrorMessage("Recompilation failed.");
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

    mLoadedLibraries.append(newLib);

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
                gpTerminalWidget->mpConsole->printDebugMessage(file.fileName() + ": The file is not an Hopsan Component Appearance Data file. Incorrect caf root tag name: " + cafRoot.tagName() + "!=" + CAF_ROOT);
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
            QMessageBox::warning(0, tr("Failed to read appearance data from %4"),
                                     QString(file.fileName() + "\nParse error at line %1, column %2:\n%3")
                                     .arg(errorLine)
                                     .arg(errorColumn)
                                     .arg(errorStr)
                                     .arg(file.fileName()));

            //! @todo give smart warning message, this is not an xml file

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

                recompileLibrary(newLib);
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
            if(type==External)
            {
                gpConfig->addUserLib(xmlPath);
            }

            LibraryEntry entry;
            QString relDir = dir.relativeFilePath(newLib.cafFiles[i]);
            entry.path = relDir.split("/");
            if(type == External)
            {
                entry.path.prepend(newLib.name);
                entry.path.prepend(QString(EXTLIBSTR));
            }
            entry.path.removeLast();
            entry.pAppearance = pAppearanceData;
            entry.pLibrary = &mLoadedLibraries.last();
            entry.visibility = visibility;
            mLibraryEntries.insert(makeFullTypeString(pAppearanceData->getTypeName(),pAppearanceData->getSubTypeName()), entry);
            qDebug() << "Adding: " << pAppearanceData->getTypeName();
            if(gpSplash)
            {
                gpSplash->showMessage("Loaded component: " + pAppearanceData->getTypeName());
            }
        }
    }

    gpTerminalWidget->checkMessages();
    emit contentsChanged();
}


//! @brief Unloads library by component type name
//! @param typeName Type name of any component in the library
void LibraryHandler::unloadLibrary(QString typeName)
{
    ComponentLibrary lib = (*gpLibraryHandler->getEntry(typeName).pLibrary);
    qDebug() << "Unloading: " << lib.name;
    CoreLibraryAccess core;
    QStringList components, nodes;

    core.getLibraryContents(lib.libFilePath, components, nodes);
    core.unLoadComponentLib(lib.libFilePath);

    qDebug() << "Unloading components: " << components;
    for(int c=0; c<components.size(); ++c)
    {
        mLibraryEntries.remove(components[c]);
    }

    gpConfig->removeUserLib(lib.xmlFilePath);

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
    return mLibraryEntries.find(makeFullTypeString(typeName, subTypeName)).value();
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


//! @brief Recompiles specified component library (safe to use with opened models)
//! @param lib Component library to recompile
//! @param solver Solver to use (for Modelica code only)
void LibraryHandler::recompileLibrary(ComponentLibrary lib, int solver)
{
    CoreLibraryAccess coreLibrary;
    CoreGeneratorAccess coreGenerator(0);

    //Save GUI state
    gpModelHandler->saveState();

    //Unload library from core
    coreLibrary.unLoadComponentLib(lib.libFilePath);

    //Generate C++ code from Modelica if source files are Modelica code
    Q_FOREACH(const QString &caf, lib.cafFiles)
    {
        QString path = QFileInfo(lib.xmlFilePath).path();
        QFile cafFile(caf);
        cafFile.open(QFile::ReadOnly);
        QString code = cafFile.readAll();
        cafFile.close();
        QString sourceFile = code.section("sourcecode=\"",1,1).section("\"",0,0);
        if(sourceFile.endsWith(".mo"))
        {
            coreGenerator.generateFromModelica(path+"/"+sourceFile, solver);
        }
    }

    //Call compile utility
    coreGenerator.compileComponentLibrary(QFileInfo(lib.xmlFilePath).absolutePath());

    //Load library again
    coreLibrary.loadComponentLib(lib.libFilePath);

    //Restore GUI state
    gpModelHandler->restoreState();
}



