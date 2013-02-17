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
#include "generators/HopsanGenerator.h"
#include "symhop/SymHop.h"

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
void copyIncludeFilesToDir2(QString path)
{
    QDir saveDir;
    saveDir.setPath(path);
    saveDir.mkpath("include");
    saveDir.cd("include");

    copyDir( QString("../HopsanCore/include"), saveDir.path() );

    saveDir.cdUp();
    bool success = saveDir.mkpath("Dependencies/rapidxml-1.13");
    qDebug() << "The success is: " << success;

    saveDir.cd("Dependencies");
    saveDir.cd("rapidxml-1.13");

    copyDir(QString("../HopsanCore/Dependencies/rapidxml-1.13"), saveDir.path());

    saveDir.cdUp();
    saveDir.mkpath("libcsv_parser++-1.0.0");
    saveDir.cd("libcsv_parser++-1.0.0");

    copyDir(QString("../HopsanCore/Dependencies/libcsv_parser++-1.0.0"), saveDir.path());
}


//! @todo maybe this function should not be among general utils
//! @todo should not copy .svn folders
void copySourceFilesToDir(QString path)
{
    QDir saveDir;
    saveDir.setPath(path);
    saveDir.mkpath("src");
    saveDir.cd("src");

    copyDir( QString("../HopsanCore/src"), saveDir.path() );
}


//! @todo maybe this function should not be among general utils
//! @todo should not copy .svn folders
void copyDefaultComponentCodeToDir(QString path)
{
    QDir saveDir;
    saveDir.setPath(path);
    saveDir.mkpath("componentLibraries/defaultLibrary/code");
    saveDir.cd("componentLibraries");
    saveDir.cd("defaultLibrary");
    saveDir.cd("code");

    copyDir( QString("../componentLibraries/defaultLibrary/code"), saveDir.path() );
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

    saveDir.cd("bin");
    QStringList binFiles = saveDir.entryList(QDir::Files | QDir::NoDotAndDotDot);

    Q_FOREACH(const QString &fileName, binFiles)
    {
        QFile file(path+"/include/boost/bin/"+fileName);
        qDebug() << "File: " << path+"/include/boost/bin/"+fileName;
        file.copy(path+"/"+fileName);
        qDebug() << "Target: " << path << "/" << fileName;
    }
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
    QString c;
    Q_FOREACH(const QString &file, ccFiles)
        c.append(file+" ");
    c.chop(1);

    QString i = pGenerator->getCoreIncludePath();
    i.prepend("-I");

    QString binDir = pGenerator->getBinPath();
    QString l = "-L"+binDir+" -lHopsanCore "+extraLinks;

    pGenerator->printMessage("\nCalling compiler utility:");
    pGenerator->printMessage("Path: "+path);
    pGenerator->printMessage("Objective: "+name);
    pGenerator->printMessage("Source files: "+c);
    pGenerator->printMessage("Includes: "+i);
    pGenerator->printMessage("Links: "+l+"\n");

    QString output;
    bool success = compile(path, name, c, i, l, "-fPIC -w -Wl,--rpath -Wl,"+path+" -shared", output);
    pGenerator->printMessage(output);
    return success;
}


//! @brief Calls GCC or MinGW compiler with specified parameters
//! @param path Absolute path where compiler shall be run
//! @param o Objective file name (without file extension)
//! @param c List with source files, example: "file1.cpp file2.cc"
//! @param i Include command, example: "-Ipath1 -Ipath2"
//! @param l Link command, example: "-Lpath1 -lfile1 -lfile2"
//! @param flags Compiler flags
//! @param output Reference to string where output messages are stored
bool compile(QString path, QString o, QString c, QString i, QString l, QString flags, QString &output)
{
    //Create compilation script file
#ifdef WIN32
    QFile clBatchFile;
    clBatchFile.setFileName(path + "/compile.bat");
    if(!clBatchFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        output = "Could not open compile.bat for writing.";
        return false;
    }
    QTextStream clBatchStream(&clBatchFile);
    clBatchStream << "gcc.exe "+flags;
    clBatchStream << c+" -o "+o+".dll "+i+" "+l+"\n";
    clBatchFile.close();
#endif

    //Call compilation script file
#ifdef WIN32
    QProcess gccProcess;
    gccProcess.setWorkingDirectory(path);
    gccProcess.start("cmd.exe", QStringList() << "/c" << "compile.bat");
    gccProcess.waitForFinished();
    QByteArray gccResult = gccProcess.readAll();
    QList<QByteArray> gccResultList = gccResult.split('\n');
    for(int i=0; i<gccResultList.size(); ++i)
    {
        output = gccResultList.at(i);
        output = output.remove(output.size()-1, 1);
    }
#elif linux
    QString gccCommand = "cd "+path+" && gcc "+flags+" ";
    gccCommand.append(c+" -fpermissive -o "+o+".so "+i+" "+l);
    qDebug() << "Command = " << gccCommand;
    gccCommand +=" 2>&1";
    FILE *fp;
    char line[130];
    fp = popen(  (const char *) gccCommand.toStdString().c_str(), "r");
    if ( !fp )
    {
        output="Could not execute '" + gccCommand + "'! err=%d";
        return false;
    }
    else
    {
        while ( fgets( line, sizeof line, fp))
        {
            output = line;
        }
    }
#endif

    QDir targetDir(path);
#ifdef WIN32
    if(!targetDir.exists(o + ".dll"))
    {
        output.append("Compilation failed.");
        return false;
    }
#elif linux
    if(!targetDir.exists(o + ".so"))
    {
        qDebug() << targetDir.absolutePath();
        qDebug() << o + ".so";
        output.append("Compilation failed.");
        return false;
    }
#endif

    output.append("Compilation successful.");
    return true;
}



//! @brief Removes all illegal characters from the string, so that it can be used as a variable name.
//! @param org Original string
//! @returns String without illegal characters
QString toVarName(const QString org)
{
    QString ret = org;
    while(!ret.isEmpty() && !ret[0].isLetter())
    {
        ret = ret.right(ret.size()-1);
    }
    for(int i=1; i<ret.size(); ++i)
    {
        if(!ret[i].isLetterOrNumber())
        {
            ret.remove(i,1);
            i--;
        }
    }
    return ret;
}


QString extractTaggedSection(QString str, QString tag)
{
    QString startStr = ">>>"+tag+">>>";
    QString endStr = "<<<"+tag+"<<<";
    if(!str.contains(startStr) || !str.contains(endStr))
    {
        return QString();
    }
    else
    {
        int i = str.indexOf(startStr)+startStr.size();
        int n = str.indexOf(endStr)-i;
        return str.mid(i, n);
    }
}


void replaceTaggedSection(QString &str, QString tag, QString replacement)
{
    QString taggedSection = ">>>"+tag+">>>"+extractTaggedSection(str, tag)+"<<<"+tag+"<<<";
    str.replace(taggedSection, replacement);
}


QString replaceTag(QString str, QString tag, QString replacement)
{
    QString retval = str;
    retval.replace("<<<"+tag+">>>", replacement);
    return retval;
}


QString replaceTags(QString str, QStringList tags, QStringList replacements)
{
    QString retval = str;
    for(int i=0; i<tags.size(); ++i)
    {
        retval.replace("<<<"+tags[i]+">>>", replacements[i]);
    }
    return retval;
}


//! @note First and last q-type variable must represent intensity and flow
QStringList getQVariables(QString nodeType)
{
    QStringList retval;
    if(nodeType == "NodeMechanic")
    {
        retval << "F" << "x" <<  "me" << "v";
    }
    if(nodeType == "NodeMechanicRotational")
    {
        retval << "T" << "th" << "w";
    }
    if(nodeType == "NodeHydraulic")
    {
        retval << "p" << "q";
    }
    if(nodeType == "NodePneumatic")
    {
        retval << "p" << "qm" << "qe";
    }
    if(nodeType == "NodeElectric")
    {
        retval << "U" << "i";
    }
    return retval;
}


//! @note c must come first and Zc last
QStringList getCVariables(QString nodeType)
{
    QStringList retval;
    if(nodeType == "NodeMechanic")
    {
        retval << "c" << "Zc";
    }
    if(nodeType == "NodeMechanicRotational")
    {
        retval << "c" << "Zc";
    }
    if(nodeType == "NodeHydraulic")
    {
        retval << "c" << "Zc";
    }
    if(nodeType == "NodePneumatic")
    {
        retval << "c" << "Zc";
    }
    if(nodeType == "NodeElectric")
    {
        retval << "c" << "Zc";
    }
    return retval;
}


//! @brief Returns list of variable enum names for specified node type
//! @param nodeType Node type to use
//! @note c must come first and Zc last
QStringList getVariableLabels(QString nodeType)
{
    QStringList retval;
    if(nodeType == "NodeMechanic")
    {
        retval << "FORCE" << "POSITION" << "EQMASS"  << "VELOCITY"<< "WAVEVARIABLE" << "CHARIMP";
    }
    if(nodeType == "NodeMechanicRotational")
    {
        retval << "TORQUE" << "ANGLE" << "ANGULARVELOCITY" << "WAVEVARIABLE" << "CHARIMP";
    }
    if(nodeType == "NodeHydraulic")
    {
        retval << "PRESSURE" << "FLOW" << "WAVEVARIABLE" << "CHARIMP";
    }
    if(nodeType == "NodePneumatic")
    {
        retval << "PRESSURE" << "MASSFLOW" << "ENERGYFLOW" << "WAVEVARIABLE" << "CHARIMP";
    }
    if(nodeType == "NodeElectric")
    {
        retval << "VOLTAGE" << "CURRENT" << "WAVEVARIABLE" << "CHARIMP";
    }
    if(nodeType == "NodeSignal")
    {
        retval << "VALUE";
    }
    return retval;
}





//! @brief Verifies that a system of equations is solveable (number of equations = number of unknowns etc)
bool verifyEquationSystem(QList<Expression> equations, QList<Expression> stateVars, HopsanGenerator *pGenerator)
{
    bool retval = true;

    if(equations.size() != stateVars.size())
    {
        QStringList equationList;
        for(int s=0; s<equations.size(); ++s)
        {
            equationList.append(equations[s].toString());
        }
        qDebug() << "Equations: " << equationList;

        QStringList stateVarList;
        for(int s=0; s<stateVars.size(); ++s)
        {
            stateVarList.append(stateVars[s].toString());
        }
        qDebug() << "State vars: " << stateVarList;

        pGenerator->printErrorMessage("Number of equations = " + QString::number(equations.size()) + ", number of state variables = " + QString::number(stateVars.size()));
        retval = false;
    }

    return retval;
}



void copyHopsanCoreSourceFiles(QString targetPath)
{
    QStringList coreFiles;
    coreFiles << ":hopsancore/include/Component.h";
    coreFiles << ":hopsancore/include/ComponentEssentials.h";
    coreFiles << ":hopsancore/include/ComponentSystem.h";
    coreFiles << ":hopsancore/include/ComponentUtilities.h";
    coreFiles << ":hopsancore/include/HopsanCore.h";
    coreFiles << ":hopsancore/include/HopsanEssentials.h";
    coreFiles << ":hopsancore/include/Node.h";
    coreFiles << ":hopsancore/include/Nodes.h";
    coreFiles << ":hopsancore/include/Parameters.h";
    coreFiles << ":hopsancore/include/Port.h";
    coreFiles << ":hopsancore/include/svnrevnum.h";
    coreFiles << ":hopsancore/include/version.h";
    coreFiles << ":hopsancore/include/win32dll.h";
    coreFiles << ":hopsancore/include/Components/DummyComponent.hpp";
    coreFiles << ":hopsancore/include/ComponentUtilities/AuxiliaryMathematicaWrapperFunctions.h";
    coreFiles << ":hopsancore/include/ComponentUtilities/AuxiliarySimulationFunctions.h";
    coreFiles << ":hopsancore/include/ComponentUtilities/CSVParser.h";
    coreFiles << ":hopsancore/include/ComponentUtilities/Delay.hpp";
    coreFiles << ":hopsancore/include/ComponentUtilities/DoubleIntegratorWithDamping.h";
    coreFiles << ":hopsancore/include/ComponentUtilities/DoubleIntegratorWithDampingAndCoulumbFriction.h";
    coreFiles << ":hopsancore/include/ComponentUtilities/EquationSystemSolver.h";
    coreFiles << ":hopsancore/include/ComponentUtilities/FirstOrderTransferFunction.h";
    coreFiles << ":hopsancore/include/ComponentUtilities/Integrator.h";
    coreFiles << ":hopsancore/include/ComponentUtilities/IntegratorLimited.h";
    coreFiles << ":hopsancore/include/ComponentUtilities/ludcmp.h";
    coreFiles << ":hopsancore/include/ComponentUtilities/matrix.h";
    coreFiles << ":hopsancore/include/ComponentUtilities/num2string.hpp";
    coreFiles << ":hopsancore/include/ComponentUtilities/SecondOrderTransferFunction.h";
    coreFiles << ":hopsancore/include/ComponentUtilities/TurbulentFlowFunction.h";
    coreFiles << ":hopsancore/include/ComponentUtilities/ValveHysteresis.h";
    coreFiles << ":hopsancore/include/ComponentUtilities/WhiteGaussianNoise.h";
    coreFiles << ":hopsancore/include/CoreUtilities/ClassFactory.hpp";
    coreFiles << ":hopsancore/include/CoreUtilities/ClassFactoryStatusCheck.hpp";
    coreFiles << ":hopsancore/include/CoreUtilities/CoSimulationUtilities.h";
    coreFiles << ":hopsancore/include/CoreUtilities/FindUniqueName.h";
    coreFiles << ":hopsancore/include/CoreUtilities/GeneratorHandler.h";
    coreFiles << ":hopsancore/include/CoreUtilities/HmfLoader.h";
    coreFiles << ":hopsancore/include/CoreUtilities/HopsanCoreMessageHandler.h";
    coreFiles << ":hopsancore/include/CoreUtilities/LoadExternal.h";
    coreFiles << ":hopsancore/include/CoreUtilities/MultiThreadingUtilities.h";
    coreFiles << ":hopsancore/src/Component.cc";
    coreFiles << ":hopsancore/src/ComponentSystem.cc";
    coreFiles << ":hopsancore/src/HopsanEssentials.cc";
    coreFiles << ":hopsancore/src/Node.cc";
    coreFiles << ":hopsancore/src/Nodes.cc";
    coreFiles << ":hopsancore/src/Parameters.cc";
    coreFiles << ":hopsancore/src/Port.cc";
    coreFiles << ":hopsancore/src/ComponentUtilities/AuxiliarySimulationFunctions.cc";
    coreFiles << ":hopsancore/src/ComponentUtilities/CSVParser.cc";
    coreFiles << ":hopsancore/src/ComponentUtilities/DoubleIntegratorWithDamping.cc";
    coreFiles << ":hopsancore/src/ComponentUtilities/DoubleIntegratorWithDampingAndCoulumbFriction.cc";
    coreFiles << ":hopsancore/src/ComponentUtilities/EquationSystemSolver.cpp";
    coreFiles << ":hopsancore/src/ComponentUtilities/FirstOrderTransferFunction.cc";
    coreFiles << ":hopsancore/src/ComponentUtilities/Integrator.cc";
    coreFiles << ":hopsancore/src/ComponentUtilities/IntegratorLimited.cc";
    coreFiles << ":hopsancore/src/ComponentUtilities/ludcmp.cc";
    coreFiles << ":hopsancore/src/ComponentUtilities/matrix.cc";
    coreFiles << ":hopsancore/src/ComponentUtilities/SecondOrderTransferFunction.cc";
    coreFiles << ":hopsancore/src/ComponentUtilities/TurbulentFlowFunction.cc";
    coreFiles << ":hopsancore/src/ComponentUtilities/ValveHysteresis.cc";
    coreFiles << ":hopsancore/src/ComponentUtilities/WhiteGaussianNoise.cc";
    coreFiles << ":hopsancore/src/CoreUtilities/CoSimulationUtilities.cpp";
    coreFiles << ":hopsancore/src/CoreUtilities/GeneratorHandler.cpp";
    coreFiles << ":hopsancore/src/CoreUtilities/HmfLoader.cc";
    coreFiles << ":hopsancore/src/CoreUtilities/HopsanCoreMessageHandler.cc";
    coreFiles << ":hopsancore/src/CoreUtilities/LoadExternal.cc";
    coreFiles << ":hopsancore/src/CoreUtilities/MultiThreadingUtilities.cpp";

    Q_FOREACH(const QString file, coreFiles)
    {
        QFile::copy(file, targetPath+file.right(file.size()-file.lastIndexOf("/")));
    }
}




