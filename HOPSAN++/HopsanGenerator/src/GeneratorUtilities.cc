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
//! @file   ComponentGeneratorUtilities.cc
//! @author Robert Braun <robert.braun@liu.se
//! @date   2012-01-08
//!
//! @brief Contains component generation utiluties
//!
//$Id$

#include <QStringList>
#include <QProcess>
#include <QDialog>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QApplication>
#include <QPushButton>
#include <QProgressDialog>
#include <QDomElement>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QLabel>

#include <cassert>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "GeneratorUtilities.h"
#include "HopsanComponentGenerator.h"
#include "SymHop.h"

#include "ComponentSystem.h"
#include "Port.h"
#include "version.h"


using namespace std;
using namespace SymHop;
using namespace hopsan;


FMIPortSpecification::FMIPortSpecification(QString varName, QString portName, QString mpndName, QString valueRef, QString portType, QString nodeType, QString dataType, QString causality)
{
    this->varName = varName;
    this->portName = portName;
    this->mpndName = mpndName;
    this->valueRef = valueRef;
    this->portType = portType;
    this->nodeType = nodeType;
    this->dataType = dataType;
    this->causality = causality;
}


FMIParameterSpecification::FMIParameterSpecification(QString varName, QString parName, QString description, QString initValue, QString valueRef)
{
    this->varName = varName;
    this->parName = parName;
    this->description = description;
    this->initValue = initValue;
    this->valueRef = valueRef;
}


PortSpecification::PortSpecification(QString porttype, QString nodetype, QString name, bool notrequired, QString defaultvalue)
{
    this->porttype = porttype;
    this->nodetype = nodetype;
    this->name = name;
    this->notrequired = notrequired;
    this->defaultvalue = defaultvalue;
}


ParameterSpecification::ParameterSpecification(QString name, QString displayName, QString description, QString unit, QString init)
{
    this->name = name;
    this->displayName = displayName;
    this->description = description;
    this->unit = unit;
    this->init = init;
}


UtilitySpecification::UtilitySpecification(QString utility, QString name)
{
    this->utility = utility;
    this->name = name;
}


StaticVariableSpecification::StaticVariableSpecification(QString datatype, QString name)
{
    this->datatype = datatype;
    this->name = name;
}



ComponentSpecification::ComponentSpecification(QString typeName, QString displayName, QString cqsType)
{
    this->typeName = typeName;
    this->displayName = displayName;
    if(cqsType == "S")
        cqsType = "Signal";
    this->cqsType = cqsType;
}






//! @brief Function for loading an XML DOM Documunt from file
//! @param[in] rFile The file to load from
//! @param[in] rDomDocument The DOM Document to load into
//! @param[in] rootTagName The expected root tag name to extract from the Dom Document
//! @returns The extracted DOM root element from the loaded DOM document
QDomElement loadXMLDomDocument(QFile &rFile, QDomDocument &rDomDocument, QString rootTagName)
{
    QString errorStr;
    int errorLine, errorColumn;
    if (!rDomDocument.setContent(&rFile, false, &errorStr, &errorLine, &errorColumn))
    {
        //! @todo Error message somehow
    }
    else
    {
        QDomElement xmlRoot = rDomDocument.documentElement();
        if (xmlRoot.tagName() != rootTagName)
        {
            //! @todo Error message somehow
        }
        else
        {
            return xmlRoot;
        }
    }
    return QDomElement(); //NULL
}



bool removeDir(QString path)
{
    QDir dir;
    dir.setPath(path);
    Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst))
    {
        if (info.isDir())
        {
            removeDir(info.absoluteFilePath());
        }
        else
        {
            if(QFile::remove(info.absoluteFilePath()))
            {
                qDebug() << "Successfully removed " << info.absoluteFilePath();
            }
            else
            {
                qDebug() << "Failed to remove " << info.absoluteFilePath();
            }
        }
    }
    return dir.rmdir(path);     //If removing files fails, this will fail, so we only need to check this
}


//! @brief Copy a directory with contents
//! @param [in] fromPath The absolute path to the directory to copy
//! @param [in] toPath The absolute path to the destination (including resulting dir name)
//! @details Copy example:  copyDir(.../files/inlude, .../files2/include)
void copyDir(const QString fromPath, QString toPath)
{
    QDir toDir(toPath);
    toDir.mkpath(toPath);
    if (toPath.endsWith('/'))
    {
        toPath.chop(1);
    }

    QDir fromDir(fromPath);
    foreach(QFileInfo info, fromDir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst))
    {
        if (info.isDir())
        {
            copyDir(info.absoluteFilePath(), toPath+"/"+info.fileName());
        }
        else
        {
            QFile::copy(info.absoluteFilePath(), toPath+"/"+info.fileName());
        }
    }
}


//! @todo maybe this function should not be among general utils
//! @todo should not copy .svn folders
void copyIncludeFilesToDir(QString path)
{
    QDir saveDir;
    saveDir.setPath(path);
    saveDir.mkpath("include");
    saveDir.cd("include");

    copyDir( QString("../HopsanCore/include"), saveDir.path() );
}


//! @todo maybe this function should not be among general utils
//! @todo should not copy .svn folders
void copyBoostIncludeFilesToDir(QString path)
{
    QDir saveDir;
    saveDir.setPath(path);
    saveDir.mkpath("include/boost");
    saveDir.cd("include");
    saveDir.cd("boost");

    copyDir( QString("../HopsanCore/Dependencies/boost"), saveDir.path() );
}




// Operators
QTextLineStream& operator <<(QTextLineStream &rLineStream, const char* input)
{
    (*rLineStream.mpQTextSream) << input << endl;
    return rLineStream;
}



bool compileComponentLibrary(QString path, QString name, HopsanGenerator *pGenerator, QString extraLinks)
{
    pGenerator->printMessage("Writing compilation script");

    QStringList ccFiles;
    QDir targetDir = QDir(path);
    ccFiles = targetDir.entryList(QStringList() << "*.cc" << "*.cpp");
    QString msg = "Found source files: ";
    Q_FOREACH(const QString &file, ccFiles)
        msg.append(file+", ");
    msg.chop(2);
    pGenerator->printMessage(msg);

    QString includeDir = pGenerator->getCoreIncludePath();
    QString binDir = pGenerator->getBinPath();

    //Create compilation script file
#ifdef WIN32
    QFile clBatchFile;
    clBatchFile.setFileName(path + "/compile.bat");
    if(!clBatchFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        pGenerator->printErrorMessage("Could not open compile.bat for writing.");
        removeDir(fmuDir.path());
        return false;
    }
    QTextStream clBatchStream(&clBatchFile);
    clBatchStream << "g++.exe -shared";
    Q_FOREACH(const QString &file, ccFiles)
        clBatchStream << file+" ";
    clBatchStream << "-o "+name+".dll -I"+includeDir+" -L"+binDir+" -lHopsanCore "+extraLinks+"\n";
    clBatchFile.close();
#endif

#ifdef WIN32
    pGenerator->printMessage("Compiling " + name + ".dll in folder " + path);
#elif linux
    pGenerator->printMessage("Compiling " + name + ".so in folder " + path);
#endif




    //Call compilation script file
#ifdef WIN32
    QProcess gccProcess;
    gccProcess.start("cmd.exe", QStringList() << "/c" << "cd " + path + " & compile.bat");
    gccProcess.waitForFinished();
    QByteArray gccResult = gccProcess.readAll();
    QList<QByteArray> gccResultList = gccResult.split('\n');
    for(int i=0; i<gccResultList.size(); ++i)
    {
        QString msg = gccResultList.at(i);
        msg = msg.remove(msg.size()-1, 1);
        if(!msg.isEmpty())
        {
            pGenerator->printMessage(msg);
        }
    }
#elif linux
    QString gccCommand = "cd "+path+" && g++ -fPIC -w -Wl,--rpath -Wl,"+path+" -shared ";
    Q_FOREACH(const QString &file, ccFiles)
        gccCommand.append(file+" ");
    gccCommand.append("-fpermissive -o "+name+".so -I./ -I"+includeDir+" -L"+binDir+" -lHopsanCore "+extraLinks);
    qDebug() << "Command = " << gccCommand;
    gccCommand +=" 2>&1";
    FILE *fp;
    char line[130];
    fp = popen(  (const char *) gccCommand.toStdString().c_str(), "r");
    if ( !fp )
    {
        pGenerator->printErrorMessage("Could not execute '" + gccCommand + "'! err=%d");
        return false;
    }
    else
    {
        while ( fgets( line, sizeof line, fp))
        {
            pGenerator->printMessage((const QString &)line);
        }
    }
#endif

#ifdef WIN32
    if(!targetDir.exists(name + ".dll"))
    {
        pGenerator->printErrorMessage("Compilation failed.");
        //removeDir(targetDir.path());
        return false;
    }
#elif linux
    if(!targetDir.exists(name + ".so"))
    {
        qDebug() << targetDir.absolutePath();
        qDebug() << name + ".so";
        pGenerator->printErrorMessage("Compilation failed.");
        //removeDir(targetDir.path());
        return false;
    }
#endif

    pGenerator->printMessage("Compilation successful.");
    return true;
}
