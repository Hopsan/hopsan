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
//! @brief Contains component generation utilities
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
#include <QDirIterator>

#include <cassert>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "GeneratorUtilities.h"
#include "generators/HopsanGenerator.h"
#include "SymHop.h"

#include "HopsanEssentials.h"
#include "ComponentSystem.h"
#include "Port.h"
#include "version.h"


using namespace std;
using namespace SymHop;
using namespace hopsan;




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


VariableSpecification::VariableSpecification(QString name, QString init)
{
    this->name = name;
    this->init = init;
}




ComponentSpecification::ComponentSpecification(QString typeName, QString displayName, QString cqsType)
{
    this->typeName = typeName;
    this->displayName = displayName;
    if(cqsType == "S")
        cqsType = "Signal";
    this->cqsType = cqsType;

    this->auxiliaryFunctions = QStringList();
}






//! @brief Function for loading an XML DOM Document from file
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
            if(QFile::exists(toPath+"/"+info.fileName()))
            {
                QFile::remove(toPath+"/"+info.fileName());
            }
            QFile::copy(info.absoluteFilePath(), toPath+"/"+info.fileName());
        }
    }
}



// Operators
QTextLineStream& operator <<(QTextLineStream &rLineStream, const char* input)
{
    (*rLineStream.mpQTextSream) << input << endl;
    return rLineStream;
}

QTextLineStream& operator <<(QTextLineStream &rLineStream, const QString &input)
{
    (*rLineStream.mpQTextSream) << input << endl;
    return rLineStream;
}



bool compileComponentLibrary(QString path, HopsanGenerator *pGenerator, QString extraLFlags, QString extraIncludes)
{
    pGenerator->printMessage("Writing compilation script...");

    QStringList ccFiles;
    QString name;
    if(QFileInfo(path).isFile())
    {
        QFile xmlFile(path);
        xmlFile.open(QFile::ReadOnly);
        QString xmlCode = xmlFile.readAll();
        xmlFile.close();
        QStringList xmlLines = xmlCode.split("\n");

        QFileInfo xmlFileInfo(xmlFile);
        path = xmlFileInfo.path();

        for(int l=0; l<xmlLines.size(); ++l)
        {
            if(xmlLines[l].trimmed().startsWith("<source>"))
            {
                ccFiles.append(xmlLines[l].section("<source>",1,1).section("</source>",0,0));
            }
            else if(xmlLines[l].trimmed().startsWith("<lib>"))
            {
                name = QString(LIBPREFIX)+xmlLines[l].section("<lib>",1,1).section("</lib>",0,0).section(".",0,0);
            }
        }
    }
    else
    {
        name = QString(LIBPREFIX)+QDir(path).dirName();
        ccFiles = QDir(path).entryList(QStringList() << "*.cc");
    }

    QString c;
    Q_FOREACH(const QString &file, ccFiles)
        c.append(file+" ");
    c.chop(1);

    QString hopsanBinDir = pGenerator->getBinPath();
    QString iflags = QString("-I\"%1\"").arg(pGenerator->getCoreIncludePath())+" "+extraIncludes;
    QString lflags = QString("-L%1 -l%2").arg(hopsanBinDir).arg("HopsanCore"TO_STR(DEBUG_EXT))+" "+extraLFlags;

    pGenerator->printMessage("\nCalling compiler utility:");
    pGenerator->printMessage("Path: "+path);
    pGenerator->printMessage("Objective: "+name);
    qDebug() << "Objective: " << name;
    pGenerator->printMessage("Source files: "+c);
    pGenerator->printMessage("Includes: "+iflags);
    pGenerator->printMessage("Links: "+lflags+"\n");

    //! @todo setting rpath here is strange, as it will hardcode given path inte dll (so if you move it it wont work)
    QString cflags = QString(" -Dhopsan=hopsan -fPIC -w -Wl,--rpath,\"%1\" -shared ").arg(path);

    QString output;
    QString gccPath = pGenerator->getGccPath();
    bool success = compile(path, gccPath, name, c, iflags, cflags, lflags, output);
    pGenerator->printMessage(output);
    return success;
}


//! @brief Calls GCC or MinGW compiler with specified parameters
//! @param wdPath Absolute path where compiler shall be run
//! @param o Objective file name (without file extension)
//! @param srcFiles List with source files, example: "file1.cpp file2.cc"
//! @param inclPaths Include paths, example: "-Ipath1 -Ipath2"
//! @param cflags Compiler flags
//! @param lflags Link paths and libs, example: "-Lpath1 -lfile1 -lfile2"
//! @param output Reference to string where output messages are stored
bool compile(QString wdPath, QString gccPath, QString o, QString srcFiles, QString inclPaths, QString cflags, QString lflags, QString &output)
{
    // Create compilation script file
    QFile compileScript;
#ifdef _WIN32
    compileScript.setFileName(wdPath + "/compile.bat");
    if(!compileScript.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        output = "Could not open compile.bat for writing.";
        return false;
    }
    QTextStream clBatchStream(&compileScript);
    clBatchStream << "@echo off\n";
    clBatchStream << "set PATH=" << gccPath << ";%PATH%\n";
    clBatchStream << "@echo on\n";
    clBatchStream << "g++.exe " << cflags << " " << srcFiles << " " << inclPaths;
    clBatchStream << " -o " << o+".dll" << " " << lflags <<"\n";
    compileScript.close();
#elif __linux__
    compileScript.setFileName(wdPath + "/compile.sh");
    if(!compileScript.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        output = "Could not open compile.sh for writing.";
        return false;
    }
    QTextStream compileStream(&compileScript);
    compileStream << "#!/bin/sh\n";
    compileStream << "gcc " << cflags << " " << srcFiles;
    compileStream << " -fpermissive -o " << o << ".so ";
    compileStream << inclPaths << " " << lflags;
    compileScript.close();
#endif

    //Call compilation script file
#ifdef _WIN32
    QProcess gccProcess;
    gccProcess.setWorkingDirectory(wdPath);
    gccProcess.start("cmd.exe", QStringList() << "/c" << "compile.bat");
    gccProcess.waitForFinished();
    QByteArray gccResult = gccProcess.readAllStandardOutput();
    QByteArray gccError = gccProcess.readAllStandardError();
    QList<QByteArray> gccResultList = gccResult.split('\n');
    for(int i=0; i<gccResultList.size(); ++i)
    {
        output = gccResultList.at(i);
        //output = output.remove(output.size()-1, 1);
    }
    QList<QByteArray> gccErrorList = gccError.split('\n');
    for(int i=0; i<gccErrorList.size(); ++i)
    {
        if(gccErrorList.at(i).trimmed() == "")
        {
            gccErrorList.removeAt(i);
            --i;
            continue;
        }
        output = output+ gccErrorList.at(i);
        //output = output.remove(output.size()-1, 1);
    }
#elif __linux__
    QString stdOut,stdErr;
    callProcess("/bin/sh", QStringList() << "compile.sh", wdPath, stdOut, stdErr);
    output.append(stdOut);
    output.append("\n");
    output.append(stdErr);
    output.append("\n");

//    QString gccCommand = "cd \""+wdPath+"\" && gcc "+cflags+" ";
//    gccCommand.append(srcFiles+" -fpermissive -o "+o+".so "+inclPaths+" "+lflags);
//    //qDebug() << "Command = " << gccCommand;
//    gccCommand +=" 2>&1";
//    FILE *fp;
//    char line[130];
//    output.append("Compiler command: \""+gccCommand+"\"\n");
//    qDebug() << "Compiler command: \"" << gccCommand << "\"";
//    fp = popen(  (const char *) gccCommand.toStdString().c_str(), "r");
//    if ( !fp )
//    {
//        output.append("Could not execute '" + gccCommand + "'! err=%d\n");
//        return false;
//    }
//    else
//    {
//        while ( fgets( line, sizeof line, fp))
//        {
//            output.append(QString::fromUtf8(line));
//        }
//    }
#endif

    QDir targetDir(wdPath);
#ifdef _WIN32

    if(!targetDir.exists(o + ".dll") || !gccErrorList.isEmpty())
    {
        output.append("Compilation failed.");
        return false;
    }
#elif __linux__
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
QString toValidVarName(const QString &rOrg)
{
    QString ret;
    if (!rOrg.isEmpty())
    {
        // Reserve memory for entire string (we will only append as many chars as we decide to keep)
        ret.reserve(rOrg.size());
        // First discard any non letter char in the begining
        int c=0;
        while ( (c<rOrg.size()) && !rOrg[c].isLetter() )
        {
            ++c;
        }
        // Now remove any non letter or nummber
        while ( c < rOrg.size() )
        {
            if ( rOrg[c].isLetterOrNumber() /*|| rOrg[c] == '_' */)
            {
                ret.append(rOrg[c]);
            }
            ++c;
        }
    }
    return ret;
//    QString ret = org;
//    while(!ret.isEmpty() && !ret[0].isLetter())
//    {
//        ret = ret.right(ret.size()-1);
//    }
//    for(int i=1; i<ret.size(); ++i)
//    {
//        if(!ret[i].isLetterOrNumber())
//        {
//            ret.remove(i,1);
//            i--;
//        }
//    }
//    return ret;
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


//! @brief Verifies that a system of equations is solvable (number of equations = number of unknowns etc)
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




void findAllFilesInFolderAndSubFolders(QString path, QString ext, QStringList &files)
{
    QDir dir(path);
    QDirIterator iterator(dir.absolutePath(), QDirIterator::Subdirectories);
    while (iterator.hasNext())
    {
        iterator.next();
        if (!iterator.fileInfo().isDir())
        {
            QString fileName = iterator.filePath();
            if (fileName.endsWith("."+ext))
                files.append(fileName);
        }
    }
}



QStringList getHopsanCoreSourceFiles()
{
    QStringList srcFiles;
    srcFiles << "HopsanCore/src/Component.cc" <<
                "HopsanCore/src/ComponentSystem.cc" <<
                "HopsanCore/src/HopsanEssentials.cc" <<
                "HopsanCore/src/HopsanTypes.cc" <<
                "HopsanCore/src/Node.cc" <<
                "HopsanCore/src/Nodes.cc" <<
                "HopsanCore/src/Parameters.cc" <<
                "HopsanCore/src/Port.cc" <<
                "HopsanCore/src/ComponentUtilities/AuxiliarySimulationFunctions.cc" <<
                "HopsanCore/src/ComponentUtilities/CSVParser.cc" <<
                "HopsanCore/src/ComponentUtilities/DoubleIntegratorWithDamping.cc" <<
                "HopsanCore/src/ComponentUtilities/DoubleIntegratorWithDampingAndCoulumbFriction.cc" <<
                "HopsanCore/src/ComponentUtilities/EquationSystemSolver.cpp" <<
                "HopsanCore/src/ComponentUtilities/FirstOrderTransferFunction.cc" <<
                "HopsanCore/src/ComponentUtilities/HopsanPowerUser.cc" <<
                "HopsanCore/src/ComponentUtilities/Integrator.cc" <<
                "HopsanCore/src/ComponentUtilities/IntegratorLimited.cc" <<
                "HopsanCore/src/ComponentUtilities/ludcmp.cc" <<
                "HopsanCore/src/ComponentUtilities/matrix.cc" <<
                "HopsanCore/src/ComponentUtilities/SecondOrderTransferFunction.cc" <<
                "HopsanCore/src/ComponentUtilities/WhiteGaussianNoise.cc" <<
                "HopsanCore/src/ComponentUtilities/PLOParser.cc" <<
                "HopsanCore/src/CoreUtilities/CoSimulationUtilities.cpp" <<
                "HopsanCore/src/CoreUtilities/GeneratorHandler.cpp" <<
                "HopsanCore/src/CoreUtilities/HmfLoader.cc" <<
                "HopsanCore/src/CoreUtilities/HopsanCoreMessageHandler.cc" <<
                "HopsanCore/src/CoreUtilities/LoadExternal.cc" <<
                "HopsanCore/src/CoreUtilities/MultiThreadingUtilities.cpp" <<
                "HopsanCore/src/CoreUtilities/StringUtilities.cpp" <<
                "componentLibraries/defaultLibrary/defaultComponentLibraryInternal.cc" <<
                "Dependencies/libcsv_parser++-1.0.0/csv_parser.cpp" <<
                "Dependencies/IndexingCSVParser/IndexingCSVParser.cpp";
    return srcFiles;
}


QStringList getHopsanCoreIncludeFiles(bool skipDependencies)
{
    QStringList includeFiles;
    includeFiles << "HopsanCore/include/Component.h" <<
                    "HopsanCore/include/ComponentEssentials.h" <<
                    "HopsanCore/include/ComponentSystem.h" <<
                    "HopsanCore/include/ComponentUtilities.h" <<
                    "HopsanCore/include/HopsanCore.h" <<
                    "HopsanCore/include/HopsanCoreMacros.h" <<
                    "HopsanCore/include/HopsanEssentials.h" <<
                    "HopsanCore/include/HopsanTypes.h" <<
                    "HopsanCore/include/Node.h" <<
                    "HopsanCore/include/Nodes.h" <<
                    "HopsanCore/include/Parameters.h" <<
                    "HopsanCore/include/Port.h" <<
                    "HopsanCore/include/svnrevnum.h" <<
                    "HopsanCore/include/version.h" <<
                    "HopsanCore/include/win32dll.h" <<
                    "HopsanCore/include/Components/DummyComponent.hpp" <<
                    "HopsanCore/include/Components/ModelicaComponent.hpp" <<
                    "HopsanCore/include/ComponentUtilities/AuxiliaryMathematicaWrapperFunctions.h" <<
                    "HopsanCore/include/ComponentUtilities/AuxiliarySimulationFunctions.h" <<
                    "HopsanCore/include/ComponentUtilities/CSVParser.h" <<
                    "HopsanCore/include/ComponentUtilities/Delay.hpp" <<
                    "HopsanCore/include/ComponentUtilities/DoubleIntegratorWithDamping.h" <<
                    "HopsanCore/include/ComponentUtilities/DoubleIntegratorWithDampingAndCoulumbFriction.h" <<
                    "HopsanCore/include/ComponentUtilities/EquationSystemSolver.h" <<
                    "HopsanCore/include/ComponentUtilities/FirstOrderTransferFunction.h" <<
                    "HopsanCore/include/ComponentUtilities/HopsanPowerUser.h" <<
                    "HopsanCore/include/ComponentUtilities/Integrator.h" <<
                    "HopsanCore/include/ComponentUtilities/IntegratorLimited.h" <<
                    "HopsanCore/include/ComponentUtilities/ludcmp.h" <<
                    "HopsanCore/include/ComponentUtilities/LookupTable.h" <<
                    "HopsanCore/include/ComponentUtilities/matrix.h" <<
                    "HopsanCore/include/ComponentUtilities/num2string.hpp" <<
                    "HopsanCore/include/ComponentUtilities/SecondOrderTransferFunction.h" <<
                    "HopsanCore/include/ComponentUtilities/TurbulentFlowFunction.h" <<
                    "HopsanCore/include/ComponentUtilities/ValveHysteresis.h" <<
                    "HopsanCore/include/ComponentUtilities/WhiteGaussianNoise.h" <<
                    "HopsanCore/include/ComponentUtilities/PLOParser.h" <<
                    "HopsanCore/include/CoreUtilities/ClassFactory.hpp" <<
                    "HopsanCore/include/CoreUtilities/ClassFactoryStatusCheck.hpp" <<
                    "HopsanCore/include/CoreUtilities/CoSimulationUtilities.h" <<
                    "HopsanCore/include/CoreUtilities/GeneratorHandler.h" <<
                    "HopsanCore/include/CoreUtilities/HmfLoader.h" <<
                    "HopsanCore/include/CoreUtilities/HopsanCoreMessageHandler.h" <<
                    "HopsanCore/include/CoreUtilities/LoadExternal.h" <<
                    "HopsanCore/include/CoreUtilities/MultiThreadingUtilities.h" <<
                    "HopsanCore/include/CoreUtilities/StringUtilities.h" <<
                    "HopsanCore/include/HopsanTypes.h" <<
                    "HopsanCore/include/ComponentUtilities/HopsanPowerUser.h" <<
                    "HopsanCore/include/HopsanCoreMacros.h" <<
                    //"componentLibraries/defaultLibrary/code/defaultComponents.h" <<
                    //"componentLibraries/defaultLibrary/code/defaultComponentLibraryInternal.h" <<
                    "HopsanCore/include/compiler_info.h";

    if (!skipDependencies)
    {
        includeFiles << "Dependencies/IndexingCSVParser/IndexingCSVParser.h" <<
                        "Dependencies/IndexingCSVParser/IndexingCSVParserImpl.hpp" <<
                        "Dependencies/libcsv_parser++-1.0.0/include/csv_parser/csv_parser.hpp" <<
                        "Dependencies/rapidxml-1.13/hopsan_rapidxml.hpp" <<
                        "Dependencies/rapidxml-1.13/rapidxml.hpp" <<
                        "Dependencies/rapidxml-1.13/rapidxml_iterators.hpp" <<
                        "Dependencies/rapidxml-1.13/rapidxml_print.hpp" <<
                        "Dependencies/rapidxml-1.13/rapidxml_utils.hpp";
    }

    return includeFiles;
}

QStringList getHopsanCoreIncludePaths(bool skipDependencies)
{
    QStringList includePaths;
    includePaths << "HopsanCore/include" <<
                    "componentLibraries/defaultLibrary";
    if (!skipDependencies)
    {
        includePaths << "Dependencies/libcsv_parser++-1.0.0/include/csv_parser/" <<
                        "Dependencies/rapidxml-1.13/" <<
                        "Dependencies/IndexingCSVParser/";
    }

    return includePaths;
}



GeneratorNodeInfo::GeneratorNodeInfo(QString nodeType)
{
    hopsan::HopsanEssentials hopsanCore;
    Node *pNode = hopsanCore.createNode(nodeType.toStdString().c_str());
    if (pNode)
    {
        niceName = pNode->getNiceName().c_str();
        for(size_t i=0; i<pNode->getDataDescriptions()->size(); ++i)
        {
            const hopsan::NodeDataDescription *pVarDesc = pNode->getDataDescription(i);
            NodeDataVariableTypeEnumT varType = pVarDesc->varType;
            // Ceck if  "Q-type variable"
            if(varType == DefaultType || varType == FlowType || varType == IntensityType)
            {
                qVariables << pVarDesc->shortname.c_str();
                qVariableIds << pVarDesc->id;
                variableLabels << QString(pVarDesc->name.c_str());
                varIdx << pVarDesc->id;
            }
            // Check if "C-type variable"
            else if(varType == TLMType)
            {
                cVariables << pVarDesc->shortname.c_str();
                cVariableIds << pVarDesc->id;
                variableLabels << QString(pVarDesc->name.c_str());
                varIdx << pVarDesc->id;
            }

        }
        hopsanCore.removeNode(pNode);
    }
}

void GeneratorNodeInfo::getNodeTypes(QStringList &nodeTypes)
{
    nodeTypes << "NodeMechanic" << "NodeMechanicRotational" << "NodeHydraulic" << "NodePneumatic" << "NodeElectric";
}


void hopsanLogger(jm_callbacks *c, jm_string module, jm_log_level_enu_t log_level, jm_string message)
{
    if(log_level == jm_log_level_error || log_level == jm_log_level_fatal)
    {
        //printErrorMessage(QString("Module = %s, log level = %d: %s\n").arg(module).arg(log_level).arg(message));
    }
    else if(log_level == jm_log_level_error || log_level == jm_log_level_fatal)
    {
        //printWarningMessage(QString("Module = %s, log level = %d: %s\n").arg(module).arg(log_level).arg(message));
    }
    else
    {
        //printMessage(QString("Module = %s, log level = %d: %s\n").arg(module).arg(log_level).arg(message));
    }
}




InterfacePortSpec::InterfacePortSpec(InterfaceTypesEnumT type, QString component, QString port, QStringList path)
{
    this->type = type;
    this->path = path;
    this->component = component;
    this->port = port;

    QStringList inputDataNames;
    QStringList outputDataNames;
    QList<size_t> inputDataIds, outputDataIds;

    switch(type)
    {
    case InterfacePortSpec::Input:
        inputDataNames << "";
        inputDataIds << 0;
        break;
    case InterfacePortSpec::Output:
        outputDataNames << "";
        outputDataIds << 0;
        break;
    case InterfacePortSpec::MechanicQ:
    {
        GeneratorNodeInfo gni("NodeMechanic");
        inputDataNames  << gni.qVariables;
        inputDataIds    << gni.qVariableIds;
        outputDataNames << gni.cVariables;
        outputDataIds   << gni.cVariableIds;
        break;
    }
    case InterfacePortSpec::MechanicC:
    {
        GeneratorNodeInfo gni("NodeMechanic");
        inputDataNames  << gni.cVariables;
        inputDataIds    << gni.cVariableIds;
        outputDataNames << gni.qVariables;
        outputDataIds   << gni.qVariableIds;
        break;
    }
    case InterfacePortSpec::MechanicRotationalQ:
    {
        GeneratorNodeInfo gni("NodeMechanicRotational");
        inputDataNames  << gni.qVariables;
        inputDataIds    << gni.qVariableIds;
        outputDataNames << gni.cVariables;
        outputDataIds   << gni.cVariableIds;
        break;
    }
    case InterfacePortSpec::MechanicRotationalC:
    {
        GeneratorNodeInfo gni("NodeMechanicRotational");
        inputDataNames  << gni.cVariables;
        inputDataIds    << gni.cVariableIds;
        outputDataNames << gni.qVariables;
        outputDataIds   << gni.qVariableIds;
        break;
    }
    case InterfacePortSpec::HydraulicQ:
    {
        GeneratorNodeInfo gni("NodeHydraulic");
        inputDataNames  << gni.qVariables;
        inputDataIds    << gni.qVariableIds;
        outputDataNames << gni.cVariables;
        outputDataIds   << gni.cVariableIds;
        break;
    }
    case InterfacePortSpec::HydraulicC:
    {
        GeneratorNodeInfo gni("NodeHydraulic");
        inputDataNames  << gni.cVariables;
        inputDataIds    << gni.cVariableIds;
        outputDataNames << gni.qVariables;
        outputDataIds   << gni.qVariableIds;
        break;
    }
    case InterfacePortSpec::PneumaticQ:
    {
        GeneratorNodeInfo gni("NodePneumatic");
        inputDataNames  << gni.qVariables;
        inputDataIds    << gni.qVariableIds;
        outputDataNames << gni.cVariables;
        outputDataIds   << gni.cVariableIds;
        break;
    }
    case InterfacePortSpec::PneumaticC:
    {
        GeneratorNodeInfo gni("NodePneumatic");
        inputDataNames  << gni.cVariables;
        inputDataIds    << gni.cVariableIds;
        outputDataNames << gni.qVariables;
        outputDataIds   << gni.qVariableIds;
        break;
    }
    case InterfacePortSpec::ElectricQ:
    {
        GeneratorNodeInfo gni("NodeElectric");
        inputDataNames  << gni.qVariables;
        inputDataIds    << gni.qVariableIds;
        outputDataNames << gni.cVariables;
        outputDataIds   << gni.cVariableIds;
        break;
    }
    case InterfacePortSpec::ElectricC:
    {
        GeneratorNodeInfo gni("NodeElectric");
        inputDataNames  << gni.cVariables;
        inputDataIds    << gni.cVariableIds;
        outputDataNames << gni.qVariables;
        outputDataIds   << gni.qVariableIds;
        break;
    }
    default:
        break;
    }

    foreach(const QString &dataName, inputDataNames)
    {
        vars.append(InterfaceVarSpec(dataName, inputDataIds.takeFirst(), InterfaceVarSpec::Input));
    }
    foreach(const QString &dataName, outputDataNames)
    {
        vars.append(InterfaceVarSpec(dataName, outputDataIds.takeFirst(), InterfaceVarSpec::Output));
    }
}



void getInterfaces(QList<InterfacePortSpec> &interfaces, ComponentSystem *pSystem, QStringList &path)
{
    std::vector<HString> names = pSystem->getSubComponentNames();
    for(size_t i=0; i<names.size(); ++i)
    {
        Component *pComponent = pSystem->getSubComponent(names[i]);
        HString typeName = pComponent->getTypeName();

        if(typeName == "SignalInputInterface")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::Input, names[i].c_str(), "out", path));
        }
        else if(typeName == "SignalOutputInterface")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::Output, names[i].c_str(), "in", path));
        }
        else if(typeName == "MechanicInterfaceQ")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::MechanicQ, names[i].c_str(), "P1", path));
        }
        else if(typeName == "MechanicInterfaceC")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::MechanicC, names[i].c_str(), "P1", path));
        }
        else if(typeName == "MechanicRotationalInterfaceQ")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::MechanicRotationalQ, names[i].c_str(), "P1", path));
        }
        else if(typeName == "MechanicRotationalInterfaceC")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::MechanicRotationalC, names[i].c_str(), "P1", path));
        }
        else if(typeName == "HydraulicInterfaceQ")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::HydraulicQ, names[i].c_str(), "P1", path));
        }
        else if(typeName == "HydraulicInterfaceC")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::HydraulicC, names[i].c_str(), "P1", path));
        }
        else if(typeName == "PneumaticInterfaceQ")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::PneumaticQ, names[i].c_str(), "P1", path));
        }
        else if(typeName == "PneumaticInterfaceC")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::PneumaticC, names[i].c_str(), "P1", path));
        }
        else if(typeName == "ElectricInterfaceQ")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::ElectricQ, names[i].c_str(), "P1", path));
        }
        else if(typeName == "ElectricInterfaceC")
        {
            interfaces.append(InterfacePortSpec(InterfacePortSpec::ElectricC, names[i].c_str(), "P1", path));
        }
        else if(typeName == "Subsystem")
        {
            QStringList path2 = path;
            getInterfaces(interfaces, dynamic_cast<hopsan::ComponentSystem *>(pComponent), path2 << pComponent->getName().c_str());
        }
    }
}



InterfaceVarSpec::InterfaceVarSpec(QString dataName, int dataId, InterfaceVarSpec::CausalityEnumT causality)
{
    this->dataName = dataName;
    this->dataId = dataId;
    this->causality = causality;
}

//! @brief Replaces a pattern preserving pattern indentation, adding indentation to each line of text
//! @param[in] rPattern The pattern to replace
//! @param[in] rReplacement The replacement text
//! @param[in] rText The text to replace in
//! @returns True if pattern was found (and replaced), else False
bool replacePattern(const QString &rPattern, const QString &rReplacement, QString &rText)
{
    bool didReplace=false;
    while (true)
    {
        // First find pattern start in text
        int b = rText.indexOf(rPattern);
        if (b > -1)
        {
            // From beginning search backwards to count number of white spaces
            int nIndent = 0;
            QString indentString;
            --b;
            while (rText[b].isSpace() && (rText[b] != '\n'))
            {
                ++nIndent;
                indentString.append(" ");
                --b;
            }

            //Add indentation to each line in replacement text
            QString newrepl, repl=rReplacement;
            QTextStream ts(&repl);
            while (!ts.atEnd())
            {
                newrepl += indentString+ts.readLine()+"\n";
            }
            // If original replacement string lacks newline at end, the last newline should be removed
            if (!rReplacement.endsWith("\n"))
            {
                //! @todo will this work with crlf
                newrepl.chop(1);
            }

            // Now replace pattern
            rText.replace(indentString+rPattern, newrepl);

            didReplace =  true;
        }
        else
        {
            break;
        }
    }
    return didReplace;
}


void callProcess(const QString &name, const QStringList &args, const QString workingDirectory, QString &rStdOut, QString &rStdErr)
{
    QProcess p;
    if(!workingDirectory.isEmpty())
    {
        p.setWorkingDirectory(workingDirectory);
    }
    p.start(name, args);
    p.waitForFinished(60000);

    rStdOut = p.readAllStandardOutput();
    rStdErr = p.readAllStandardError();
    //p.readAll();
}
