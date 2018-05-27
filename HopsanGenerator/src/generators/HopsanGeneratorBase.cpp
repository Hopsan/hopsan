/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   ComponentGeneratorUtilities.cpp
//! @author Robert Braun <robert.braun@liu.se
//! @date   2012-01-08
//!
//! @brief Contains component generation utilities
//!
//$Id$

#include <QStringList>
#include <QDir>
#include <QProcess>
#include <QDomElement>

#include <cassert>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "generators/HopsanGeneratorBase.h"
#include "GeneratorUtilities.h"
#include "SymHop.h"

#include "ComponentSystem.h"
#include "Port.h"
#include "HopsanCoreVersion.h"


using namespace SymHop;
using namespace hopsan;


HopsanGeneratorBase::HopsanGeneratorBase(const QString &hopsanInstallPath, const QString &compilerPath, const QString &tempPath)
{
    mHopsanRootPath = hopsanInstallPath;
    mHopsanCoreIncludePath = mHopsanRootPath+"/HopsanCore/include";
    mHopsanBinPath = mHopsanRootPath+"/bin";

    if(!compilerPath.isEmpty())
    {
        mCompilerPath = QFileInfo(compilerPath).absoluteFilePath();
#ifndef _WIN32
        // Add the last / here so that the compiler name can be directly appended to mCompilerPath.
        // If compilerPath is empty (compiler in PATH) then we do not want to check if a / should be added or not
        // when using this variable
        mCompilerPath.append("/");
#endif
    }

    if(!tempPath.isEmpty())
    {
        mTempPath = QFileInfo(tempPath).absoluteFilePath();
    }
    else
    {
        mTempPath = QDir::currentPath()+"/temp";
    }

    mOutputPath = QDir::currentPath()+"/output";
}

HopsanGeneratorBase::~HopsanGeneratorBase()
{
    // Do nothing right now
}

void HopsanGeneratorBase::setMessageHandler(std::function<void (const char*, const char, void*)> messageHandler, void* pMessageObject)
{
    mMessageHandler = messageHandler;
    mpMessageHandlerObject = pMessageObject;
}

void HopsanGeneratorBase::printMessage(const QString &msg, const QChar &type) const
{
    if(mShowMessages && mMessageHandler)
    {
        mMessageHandler(msg.toStdString().c_str(), type.toLatin1(), mpMessageHandlerObject);
    }
}


void HopsanGeneratorBase::printWarningMessage(const QString &msg) const
{
    printMessage(msg, 'W');
}


void HopsanGeneratorBase::printErrorMessage(const QString &msg) const
{
    printMessage(msg, 'E');
}


QString HopsanGeneratorBase::generateSourceCodefromComponentSpec(ComponentSpecification comp, bool overwriteStartValues) const
{
    if(comp.cqsType == "S") { comp.cqsType = "Signal"; }


    //Declare variables
    QString varDeclarations;
    for(int i=0; i<comp.parNames.size(); ++i)
    {
        varDeclarations.append("        double "+comp.parNames[i]+";\n");
    }
    for(int i=0; i<comp.varNames.size(); ++i)
    {
        varDeclarations.append("        "+comp.varTypes[i]+" "+comp.varNames[i]+";\n");
        if(comp.varTypes[i] == "double" && !comp.varNames[i].contains("]"))
            varDeclarations.append("        "+comp.varTypes[i]+" *mpOUTPUT_"+comp.varNames[i]+";\n");
    }
    for(int i=0; i<comp.utilities.size(); ++i)
    {
        varDeclarations.append("        "+comp.utilities[i]+" "+comp.utilityNames[i]+";\n");
    }
    int portId=1;
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        QStringList varNames;
        if(comp.portNodeTypes[i] == "NodeSignal")
        {
            varNames << comp.portNames[i]+"__y";
        }
        else
        {
            varNames << GeneratorNodeInfo(comp.portNodeTypes[i]).qVariables << GeneratorNodeInfo(comp.portNodeTypes[i]).cVariables;
        }
        for(int v=0; v<varNames.size(); ++v)
        {
            QString varName;
            if(comp.portNodeTypes[i] == "NodeSignal")
                varName = varNames[v];
            else
                varName = varNames[v] + QString::number(portId);
            varDeclarations.append("        double "+varName+";\n");
        }
        ++portId;
    }


    //Declare node data pointers
    QString dataPtrDeclarations = "        double ";
    portId=1;
    QStringList allVarNames;
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        QString id = QString::number(portId);
        if(comp.portNodeTypes[i] == "NodeSignal")
        {
            allVarNames << comp.portNames[i]+"__y";
        }
        else
        {
            QStringList vars;
            vars << GeneratorNodeInfo(comp.portNodeTypes[i]).qVariables << GeneratorNodeInfo(comp.portNodeTypes[i]).cVariables;

            for(int v=0; v<vars.size(); ++v)
            {
                allVarNames << vars[v]+id;
            }
        }
        ++portId;
    }
    if(!allVarNames.isEmpty())
    {
        dataPtrDeclarations.append("*mpND_"+allVarNames[0]);
        for(int i=1; i<allVarNames.size(); ++i)
        {
            dataPtrDeclarations.append(", *mpND_"+allVarNames[i]);
        }
    }
    dataPtrDeclarations.append(";\n");


    //Declare ports
    QString portDeclarations = "        Port ";
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        portDeclarations.append("*mp"+comp.portNames[i]);
        if(i<comp.portNames.size()-1)
        {
            portDeclarations.append(", ");
        }
    }
    portDeclarations.append(";\n");


//    //Initialize parameters
//    QString parameterInit;
//    for(int i=0; i<comp.parNames.size(); ++i)
//    {
//        parameterInit.append("            "+comp.parNames[i]+" = "+comp.parInits[i]+";\n");
//    }


    //Register parameters
    QString registerParameters;
    for(int i=0; i<comp.parNames.size(); ++i)
    {
        registerParameters.append("            addConstant(\""+comp.parDisplayNames[i]+"\", \""
                  +comp.parDescriptions[i]+"\", \""+comp.parUnits[i]+"\", "+comp.parInits[i]+", "+comp.parNames[i]+");\n");
    }


    //Add ports
    QString addPorts;
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        if(comp.portNodeTypes[i] == "NodeSignal")
        {
            QString init = comp.portDefaults[i];
            if(init.isEmpty())
            {
                init = "0";
            }
            if(comp.portTypes[i] == "ReadPort")
            {
                addPorts.append("            addInputVariable(\""+comp.portNames[i]+"\", \"\", \"\", "+init+", &mpND_"+comp.portNames[i]+"__y);\n");
            }
            else if(comp.portTypes[i] == "WritePort")
            {
                addPorts.append("            addOutputVariable(\""+comp.portNames[i]+"\", \"\", \"\", "+init+", &mpND_"+comp.portNames[i]+"__y);\n");
            }
        }
        else
        {
            addPorts.append("            mp"+comp.portNames[i]+" = add"+comp.portTypes[i]
                      +"(\""+comp.portNames[i]+"\", \""+comp.portNodeTypes[i]+"\"");
            if(comp.portNotReq[i])
            {
                addPorts.append(", Port::NotRequired);\n");
            }
            else
            {
                addPorts.append(");\n");
            }
        }
    }
    for(int i=0; i<comp.varNames.size(); ++i)
    {
        if(comp.varTypes[i] == "double" && !comp.varNames[i].contains("]"))
            addPorts.append("            addOutputVariable(\""+comp.varNames[i]+"\", \"\", \"\", 0, &mpOUTPUT_"+comp.varNames[i]+");\n");
    }


    //Initialize variables
    QString initializeVariables;
    for(int i=0; i<comp.varInits.size(); ++i)
    {
        if(!comp.varInits[i].isEmpty())
        {
            initializeVariables.append("            "+comp.varNames[i]+" = "+comp.varInits[i]+";\n");
        }
    }

    //Get data pointers
    QString getDataPtrs;
    portId=1;
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        QStringList varNames;
        QStringList varLabels;
        if(comp.portNodeTypes[i] != "NodeSignal")
        {
            varNames << GeneratorNodeInfo(comp.portNodeTypes[i]).qVariables << GeneratorNodeInfo(comp.portNodeTypes[i]).cVariables;
            varLabels << GeneratorNodeInfo(comp.portNodeTypes[i]).variableLabels;
        }

        for(int v=0; v<varNames.size(); ++v)
        {
            QString varName;
            if(comp.portNodeTypes[i] == "NodeSignal")
                varName = varNames[v];
            else
                varName = varNames[v]+QString::number(portId);
            getDataPtrs.append("            mpND_"+varName+" = getSafeNodeDataPtr(mp"+comp.portNames[i]+", "+comp.portNodeTypes[i]+"::"+varLabels[v]);
            if(comp.portNotReq[i])
            {
                getDataPtrs.append(", "+comp.portDefaults[i]);
            }
            getDataPtrs.append(");\n");
        }
        ++portId;
    }


    //Read input variables
    QString readInputs;
    portId=1;
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        QStringList varNames;
        if(comp.portNodeTypes[i] == "NodeSignal")
        {
            varNames << comp.portNames[i]+"__y";
        }
        else
        {
            varNames << GeneratorNodeInfo(comp.portNodeTypes[i]).qVariables << GeneratorNodeInfo(comp.portNodeTypes[i]).cVariables;
        }

        for(int v=0; v<varNames.size(); ++v)
        {
            QString varName;
            if(comp.portNodeTypes[i] == "NodeSignal")
                varName = varNames[v];
            else
                varName = varNames[v] + QString::number(portId);
            readInputs.append("            "+varName+" = (*mpND_"+varName+");\n");
        }
        ++portId;
    }


    //Configure code
    QString confCode;
    for(int i=0; i<comp.confEquations.size(); ++i)
    {
        confCode.append("            "+comp.confEquations[i]+"\n");
    }

    //Initialize code
    QString initCode;
    for(int i=0; i<comp.initEquations.size(); ++i)
    {
        initCode.append("            "+comp.initEquations[i]+"\n");
    }


    //Write back values
    QString writeOutputs;
    portId=1;
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        QStringList varNames;
        if(comp.portNodeTypes[i] == "NodeSignal" && comp.portTypes[i] == "WritePort")
        {
            varNames << comp.portNames[i];
        }
        if(comp.portNodeTypes[i] != "NodeSignal" && (comp.cqsType == "Q" || comp.cqsType == "S"))
        {
            varNames << GeneratorNodeInfo(comp.portNodeTypes[i]).qVariables;
        }
        if(comp.portNodeTypes[i] != "NodeSignal" && (comp.cqsType == "C" || comp.cqsType == "S"))
        {
            varNames << GeneratorNodeInfo(comp.portNodeTypes[i]).cVariables;
        }
        for(int v=0; v<varNames.size(); ++v)
        {
            QString varName;
            if(comp.portNodeTypes[i] == "NodeSignal")
                varName = varNames[v];
            else
                varName = varNames[v] + QString::number(portId);
            writeOutputs.append("            (*mpND_"+varName+") = "+varName+";\n");
        }
        ++portId;
    }
    for(int i=0; i<comp.varNames.size(); ++i)
    {
        if(comp.varTypes[i] == "double" && !comp.varNames[i].contains("]"))
            writeOutputs.append("            (*mpOUTPUT_"+comp.varNames[i]+") = "+comp.varNames[i]+";\n");
    }

    QString writeStartValues = "";
    if(overwriteStartValues)
    {
        writeStartValues = writeOutputs;
    }


    //Simulate code
    QString simCode;
    for(int i=0; i<comp.simEquations.size(); ++i)
    {
        simCode.append("            "+comp.simEquations[i]+"\n");
    }


    //Finalize code
    QString finalCode;
    for(int i=0; i<comp.finalEquations.size(); ++i)
    {
        finalCode.append("            "+comp.finalEquations[i]+"\n");
    }


    //Deconfiguration code
    QString deconfCode;
    for(int i=0; i<comp.deconfEquations.size(); ++i)
    {
        deconfCode.append("            "+comp.deconfEquations[i]+"\n");
    }


    //Auxiliary functions
    QString auxiliaryFunctions;
    for(int i=0; i<comp.auxiliaryFunctions.size(); ++i)
    {
        auxiliaryFunctions.append("        "+comp.auxiliaryFunctions[i]+"\n");
    }


    QFile compTemplateFile(":templates/generalComponentTemplate.hpp");
    if(!compTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open generalComponentTemplate.hpp for writing.");
        return "";
    }

    QString code;
    QTextStream t(&compTemplateFile);
    code = t.readAll();
    compTemplateFile.close();
    if(code.isEmpty())
    {
        printErrorMessage("Failed to generate code for "+comp.typeName+".hpp.");
        return "";
    }

    code.replace("<<<uppertypename>>>", comp.typeName.toUpper());
    code.replace("<<<typename>>>", comp.typeName);
    code.replace("<<<cqstype>>>", comp.cqsType);
    code.replace("<<<vardecl>>>", varDeclarations);
    code.replace("<<<dataptrdecl>>>", dataPtrDeclarations);
    code.replace("<<<portdecl>>>", portDeclarations);
    //code.replace("<<<parinit>>>", parameterInit);
    code.replace("<<<regpar>>>", registerParameters);
    code.replace("<<<addports>>>", addPorts);
    code.replace("<<<initvars>>>", initializeVariables);
    code.replace("<<<getdataptrs>>>", getDataPtrs);
    code.replace("<<<readinputs>>>", readInputs);
    code.replace("<<<confcode>>>", confCode);
    code.replace("<<<initcode>>>", initCode);
    code.replace("<<<writestartvalues>>>", writeStartValues);
    code.replace("<<<simulatecode>>>", simCode);
    code.replace("<<<writeoutputs>>>", writeOutputs);
    code.replace("<<<finalcode>>>", finalCode);
    code.replace("<<<deconfcode>>>", deconfCode);
    code.replace("<<<auxiliaryfunctions>>>", auxiliaryFunctions);

    return code;
}


//! @brief Generates appearance file from a component specification object
//! @param path Path to generated (or existing) .xml file
//! @param comp Component specification object
bool HopsanGeneratorBase::generateOrUpdateComponentAppearanceFile(QString path, ComponentSpecification comp, QString sourceFile)
{
    QFile file(path);
    QString code;
    if(file.exists())   //An xml file already exists
    {
        if(!file.open(QFile::ReadOnly)) {
            printErrorMessage(QString("Failed to open '%1' for reading.").arg(path));
            return false;
        }
        code = file.readAll();
        file.close();

        QStringList lines = code.split("\n");

        //! @todo Update typename and displayname if they have changed

        //Remove non-existing ports
        for(int l=0; l<lines.size(); ++l)
        {
            if(lines[l].trimmed().startsWith("<port "))
            {
                QString portName = lines[l].section("name=\"",1,1).section("\"",0,0);
                if(!comp.portNames.contains(portName))
                {
                    lines.removeAt(l);
                    --l;
                }
            }
        }

        //Add existing ports missing in XML
        for(int p=0; p<comp.portNames.size(); ++p)
        {
            bool found=false;
            for(int l=0; l<lines.size(); ++l)
            {
                if(lines[l].trimmed().startsWith("<port "))
                {
                    QString portName = lines[l].section("name=\"",1,1).section("\"",0,0);
                    if(portName == comp.portNames[p])
                    {
                        found=true;
                    }
                }
            }

            if(!found)
            {
                for(int l=0; l<lines.size(); ++l)
                {
                    if(lines[l].trimmed().startsWith("</ports>"))
                    {
                        lines.insert(l, "      <port name=\""+comp.portNames[p]+"\" x=\"0.0\" y=\"0.0\" a=\"0.0\"/>");
                        break;
                    }
                }
            }
        }

        code = lines.join("\n");

        if(!file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate)){
            printErrorMessage(QString("Failed to open '%1' for writing.").arg(path));
            return false;
        }
        file.write(code.toUtf8());
        file.close();
    }
    else    //No existing XML file
    {
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            printErrorMessage(QString("Failed to open %1 for writing.").arg(path));
            return false;
        }
        QTextStream xmlStream(&file);
        xmlStream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        xmlStream << "<hopsanobjectappearance version=\"0.3\">\n";
        QString sourceString;
        if(!sourceFile.isEmpty())
        {
            sourceString = " sourcecode=\""+sourceFile+"\"";
        }
        xmlStream << "  <modelobject typename=\"" << comp.typeName << "\" displayname=\"" << comp.displayName << "\" libpath=\"\""+sourceString+">\n";
        xmlStream << "    <icons/>\n";
        xmlStream << "    <ports>\n";
        double xDelay = 1.0/(comp.portNames.size()+1.0);
        double xPos = xDelay;
        double yPos = 0;
        for(int i=0; i<comp.portNames.size(); ++i)
        {
            xmlStream << "      <port name=\"" << comp.portNames[i] << "\" x=\"" << xPos << "\" y=\"" << yPos << "\" a=\"" << 270 << "\"/>\n";
            xPos += xDelay;
        }
        xmlStream << "    </ports>\n";
        xmlStream << "  </modelobject>\n";
        xmlStream << "</hopsanobjectappearance>\n";
        file.close();
    }
    return true;
}







//! @brief Generates and compiles component source code from a ComponentSpecification object
//! @param outputFile Name of output file
//! @param comp Component specification object
//! @param overwriteStartValues Tells whether or not this components overrides the built-in start values or not
void HopsanGeneratorBase::compileFromComponentSpecification(const QString &outputFile, const ComponentSpecification &comp, const bool overwriteStartValues, const QString customSourceFile)
{
    QString code;

    if(comp.plainCode.isEmpty())
    {
        code = generateSourceCodefromComponentSpec(comp, overwriteStartValues);
    }
    else
    {
        code = comp.plainCode;
    }

    //qDebug() << "Code: " << code;

    if(!QDir(mTempPath).exists())
    {
        QDir().mkpath(mTempPath);
    }

    if(!QDir(mOutputPath).exists())
    {
        QDir().mkpath(mOutputPath);
    }

    printMessage("Writing "+outputFile+".hpp...");

    //Initialize the file stream
    QFileInfo fileInfo;
    QFile file;
    fileInfo.setFile(QString(mTempPath)+outputFile+".hpp");
    file.setFileName(fileInfo.filePath());   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open file for writing: " + outputFile+".hpp");
        return;
    }
    QTextStream fileStream(&file);  //Create a QTextStream object to stream the content of file

    fileStream << code;

    file.close();

    printMessage("Writing tempLib.cpp...");

    QFile ccLibFile;
    ccLibFile.setFileName(QString(mTempPath)+"tempLib.cpp");
    if(!ccLibFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open tempLib.cpp for writing.");
        return;
    }
    QTextStream ccLibStream(&ccLibFile);
    ccLibStream << "#include \"" << outputFile << ".hpp\"\n";
    ccLibStream << "#include \"ComponentEssentials.h\"\n\n";
    ccLibStream << "using namespace hopsan;\n\n";
    ccLibStream << "extern \"C\" DLLEXPORT void register_contents(ComponentFactory* cfact_ptr, NodeFactory* /*nfact_ptr*/)\n";
    ccLibStream << "{\n";
    ccLibStream << "    cfact_ptr->registerCreatorFunction(\"" << comp.typeName<< "\", " << comp.typeName << "::Creator);\n";
    ccLibStream << "}\n\n";
    ccLibStream << "extern \"C\" DLLEXPORT void get_hopsan_info(HopsanExternalLibInfoT *pHopsanExternalLibInfo)\n";
    ccLibStream << "{\n";
    ccLibStream << "    pHopsanExternalLibInfo->libName = (char*)\"HopsanGeneratedComponent_"+comp.typeName+"\";\n";
    ccLibStream << "    pHopsanExternalLibInfo->hopsanCoreVersion = (char*)HOPSANCOREVERSION;\n";
    ccLibStream << "    pHopsanExternalLibInfo->libCompiledDebugRelease = (char*)DEBUGRELEASECOMPILED;\n";
    ccLibStream << "}\n";
    ccLibFile.close();

    printMessage("Writing " + comp.typeName + ".xml...");

    QFile xmlFile;
    xmlFile.setFileName(QString(mOutputPath)+comp.typeName+".xml");
    if(!xmlFile.exists())
    {
        if(!xmlFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            printErrorMessage("Failed to open " + comp.typeName + ".xml  for writing.");
            return;
        }
        QTextStream xmlStream(&xmlFile);
        xmlStream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        xmlStream << "<hopsanobjectappearance version=\"0.3\">\n";
        QString sourceFile = customSourceFile;
        if(sourceFile.isEmpty()) sourceFile = outputFile+".hpp";
        xmlStream << "  <modelobject typename=\"" << comp.typeName << "\" displayname=\"" << comp.displayName << "\" sourcecode=\""+sourceFile+"\" libpath=\"\" recompilable=\"true\">\n";
        xmlStream << "    <icons/>\n";
        xmlStream << "    <ports>\n";
        double xDelay = 1.0/(comp.portNames.size()+1.0);
        double xPos = xDelay;
        double yPos = 0;
        for(int i=0; i<comp.portNames.size(); ++i)
        {
            xmlStream << "      <port name=\"" << comp.portNames[i] << "\" x=\"" << xPos << "\" y=\"" << yPos << "\" a=\"" << 270 << "\"/>\n";
            xPos += xDelay;
        }
        xmlStream << "    </ports>\n";
        xmlStream << "  </modelobject>\n";
        xmlStream << "</hopsanobjectappearance>\n";
        xmlFile.close();
    }

    QString libFileName = outputFile;
#ifdef _WIN32
    libFileName.append(".dll");
#else
    libFileName.append(".so");
#endif

    //! @todo Make this work again!
    //compileComponentLibrary(mTempPath, outputFile, this);

    printMessage("Moving files to output directory...");

    QFile soFile(mTempPath+libFileName);
    QFile targetSoFile(mOutputPath+libFileName);
    if(targetSoFile.exists() && !targetSoFile.remove())
    {
        printErrorMessage("Failed to remove existing binary file:"+targetSoFile.errorString());
        return;
    }
    targetSoFile.close();

    //qDebug() << "Copying "+soFile.fileName()+" to "+mOutputPath+libFileName;
    if(!soFile.copy(mOutputPath + libFileName))
    {
        printErrorMessage("Failed to copy new binary to output directory: "+soFile.errorString());
        return;
    }
    if(!file.copy(mOutputPath+fileInfo.fileName()))
    {
        printErrorMessage("Failed to copy source code file to output directory: "+file.errorString());
    }
    QFileInfo ccLibFileInfo(ccLibFile);
    if(!ccLibFile.copy(mOutputPath+ccLibFileInfo.fileName()))
    {
        printMessage("Unable to copy library source file to output directory: "+ccLibFile.errorString() + " (not required)");
    }
}


//! @brief Generates library .cpp and .xml files for a folder with .hpp files
//! @param[in] dstPath Destination path for library
//! @param[in] hppFiles Relative path to hpp files
//! @param[in] cflags Compiler flags required for building the library
//! @param[in] lflags Linker flags required for building the library
bool HopsanGeneratorBase::generateNewLibrary(QString dstPath, QStringList hppFiles, QStringList cflags, QStringList lflags)
{
    printMessage("Creating new component library...");

    // Make sure / at end
    if (!dstPath.endsWith("/"))
    {
        dstPath.append("/");
    }

    QString libName = QDir(dstPath).dirName();

    QStringList typeNames;
    for(const QString &file : hppFiles)
    {
        QFile hppFile(dstPath+file);
        hppFile.open(QFile::ReadOnly);
        QString code = hppFile.readAll();
        hppFile.close();

        QString temp = code.section("class ",1,1);
        typeNames.append(temp.section(" : public",0,0).trimmed());
    }

    printMessage("Writing "+libName+".cpp...");

    QFile ccLibFile;
    ccLibFile.setFileName(dstPath+libName+".cpp");
    if(!ccLibFile.open(QFile::WriteOnly | QFile::Text))
    {
        printErrorMessage("Failed to open "+libName+".cpp for writing.");
        return false;
    }
    QTextStream ccLibStream(&ccLibFile);
    for(const QString &file : hppFiles)
    {
        ccLibStream << "#include \"" << file << "\"\n";
    }
    ccLibStream << "#include \"ComponentEssentials.h\"\n\n";
    ccLibStream << "using namespace hopsan;\n\n";
    ccLibStream << "extern \"C\" DLLEXPORT void register_contents(ComponentFactory* cfact_ptr, NodeFactory* /*nfact_ptr*/)\n";
    ccLibStream << "{\n";
    for(const QString &type : typeNames)
    {
        ccLibStream << "    cfact_ptr->registerCreatorFunction(\"" << type << "\", " << type << "::Creator);\n";
    }
    ccLibStream << "}\n\n";
    ccLibStream << "extern \"C\" DLLEXPORT void get_hopsan_info(HopsanExternalLibInfoT *pHopsanExternalLibInfo)\n";
    ccLibStream << "{\n";
    ccLibStream << "    pHopsanExternalLibInfo->libName = (char*)\"" << libName << "\";\n";
    ccLibStream << "    pHopsanExternalLibInfo->hopsanCoreVersion = (char*)HOPSANCOREVERSION;\n";
    ccLibStream << "    pHopsanExternalLibInfo->libCompiledDebugRelease = (char*)DEBUGRELEASECOMPILED;\n";
    ccLibStream << "}\n";
    ccLibFile.close();

    printMessage("Writing " + libName + "_lib.xml...");

    QFile xmlFile;
    xmlFile.setFileName(dstPath+libName+"_lib.xml");
    if(!xmlFile.exists())
    {
        if(!xmlFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            printErrorMessage("Failed to open " + libName + "_lib.xml  for writing.");
            return false;
        }
        QTextStream xmlStream(&xmlFile);
        xmlStream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";

        xmlStream << "<hopsancomponentlibrary xmlversion=\"0.1\" libversion=\"1\" name=\""+libName+"\">\n";
        xmlStream << "  <lib>" << libName  << "</lib>\n";
        xmlStream << "  <source>" << libName << ".cpp</source>\n";
        xmlStream << "  <buildflags>\n";
        xmlStream << "    <cflags>" << cflags.join(" ") << "</cflags>\n";
        xmlStream << "    <lflags>" << lflags.join(" ") << "</lflags>\n";
        xmlStream << "  </buildflags>\n";
        xmlStream << "</hopsancomponentlibrary>\n";
        xmlFile.close();
    }

    printMessage("Finished.");
    return true;
}



bool HopsanGeneratorBase::generateCafFile(QString &rPath, ComponentAppearanceSpecification &rCafSpec)
{
    //Create <fmuname>.xml
    QFile fmuCafFile;
    fmuCafFile.setFileName(rPath);
    if(!fmuCafFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Import of FMU failed: Could not open "+QFileInfo(rPath).absoluteFilePath()+" for writing.");
        return false;
    }

    //Create DOM tree
    QDomDocument domDocument;

    QDomNode xmlProcessingInstruction = domDocument.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
    domDocument.appendChild(xmlProcessingInstruction);

    QDomElement cafRoot = domDocument.createElement("hopsanobjectappearance");
    cafRoot.setAttribute("version", "0.3");
    domDocument.appendChild(cafRoot);

    QDomElement modelElement = domDocument.createElement("modelobject");
    modelElement.setAttribute("typename", rCafSpec.mTypeName);
    modelElement.setAttribute("displayname", rCafSpec.mDisplayName);
    modelElement.setAttribute("sourcecode", rCafSpec.mSourceCode);
    modelElement.setAttribute("libpath", rCafSpec.mLibPath);
    if(rCafSpec.mRecompilable)
    {
        modelElement.setAttribute("recompilable", "true");
    }
    else
    {
        modelElement.setAttribute("recompilable", "false");
    }
    cafRoot.appendChild(modelElement);

    QDomElement iconsElement = domDocument.createElement("icons");
    modelElement.appendChild(iconsElement);

    if(!rCafSpec.mUserIconPath.isEmpty())
    {
        QDomElement iconElement = domDocument.createElement("icon");
        iconElement.setAttribute("type", "user");
        iconElement.setAttribute("path", rCafSpec.mUserIconPath);
        if(rCafSpec.mUserIconRotation)
        {
            iconElement.setAttribute("iconrotation", "ON");
        }
        else
        {
            iconElement.setAttribute("iconrotation", "OFF");
        }
        iconElement.setAttribute("scale", rCafSpec.mUserIconScale);
        iconsElement.appendChild(iconElement);
    }

    if(!rCafSpec.mIsoIconPath.isEmpty())
    {
        QDomElement iconElement = domDocument.createElement("icon");
        iconElement.setAttribute("type", "iso");
        iconElement.setAttribute("path", rCafSpec.mIsoIconPath);
        if(rCafSpec.mIsoIconRotation)
        {
            iconElement.setAttribute("iconrotation", "ON");
        }
        else
        {
            iconElement.setAttribute("iconrotation", "OFF");
        }
        iconElement.setAttribute("scale", rCafSpec.mIsoIconScale);
        iconsElement.appendChild(iconElement);
    }


    QDomElement portsElement = domDocument.createElement("ports");
    modelElement.appendChild(portsElement);

    for(int i=0; i<rCafSpec.mPortNames.size(); ++i)
    {
        QDomElement portElement = domDocument.createElement("port");
        portElement.setAttribute("name", rCafSpec.mPortNames.at(i));
        portElement.setAttribute("x", QString::number(rCafSpec.mPortX.at(i)));
        portElement.setAttribute("y", QString::number(rCafSpec.mPortY.at(i)));
        portElement.setAttribute("a", QString::number(rCafSpec.mPortA.at(i)));
        portsElement.appendChild(portElement);
    }

    //Save to file
    QTextStream out(&fmuCafFile);
    domDocument.save(out, 2);
    fmuCafFile.close();

    return true;
}


void HopsanGeneratorBase::setOutputPath(const QString &path)
{
    mOutputPath = path;
    printMessage("Setting output path: " + path);
}


QString HopsanGeneratorBase::getHopsanCoreIncludePath() const
{
    return mHopsanCoreIncludePath;
}


QString HopsanGeneratorBase::getHopsanBinPath() const
{
    return mHopsanBinPath;
}

QString HopsanGeneratorBase::getHopsanRootPath() const
{
    return mHopsanRootPath;
}

QString HopsanGeneratorBase::getCompilerPath() const
{
    return mCompilerPath;
}

void HopsanGeneratorBase::setQuiet(bool quiet)
{
    mShowMessages = !quiet;
}


bool HopsanGeneratorBase::assertFilesExist(const QString &path, const QStringList &files) const
{
    for(const QString& file : files)
    {
        if(!QFile::exists(path+"/"+file))
        {
            printErrorMessage("File not found: "+file);
            return false;
        }
    }
    printMessage("All required files found.");
    return true;
}

bool HopsanGeneratorBase::assertFilesExist(const QString &path, const QString &file) const
{
    QStringList files;
    files << file;
    return assertFilesExist(path, files);
}


bool HopsanGeneratorBase::callProcess(const QString &name, const QStringList &args, const QString workingDirectory) const
{
    QString stdOut, stdErr;
    int i = ::callProcess(name, args, workingDirectory, 600, stdOut, stdErr);
    printMessage(stdOut);
    printMessage(stdErr);
    return (i==0);
}


//! @warning May delete contents in file if it fails to open in write mode
bool HopsanGeneratorBase::replaceInFile(const QString &filePath, const QStringList &before, const QStringList &after) const
{
    Q_ASSERT(before.size() == after.size());

    QFile file(filePath);
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        printErrorMessage("Unable to open file: "+filePath+" for reading.");
        return false;
    }
    QString contents = file.readAll();
    file.close();

    bool foundTextToReplace = false;
    for(int i=0; i<before.size(); ++i)
    {
        if(contents.contains(before[i]))
        {
            foundTextToReplace = true;
            contents.replace(before[i], after[i]);
        }
    }

    if(foundTextToReplace)
    {
        if(!file.open(QFile::ReadWrite | QFile::Truncate | QFile::Text))
        {
            printErrorMessage("Unable to open file: "+filePath+" for writing.");
            return false;
        }
        file.write(contents.toLatin1());
        file.close();
    }

    return true;
}



//! @todo should not copy .git folders
bool HopsanGeneratorBase::copyHopsanCoreSourceFilesToDir(const QString &tgtPath) const
{
    printMessage("Copying HopsanCore source, header and dependencies...");

    QDir saveDir(tgtPath);
    if (saveDir.mkpath(".") && copyDir(mHopsanRootPath+"/HopsanCore", tgtPath+"/HopsanCore"))
    {
        return true;
    }
    return false;
}



//! @todo maybe this function should not be among general utils
//! @todo should not copy .svn folders
//! @todo Error checking
bool HopsanGeneratorBase::copyDefaultComponentCodeToDir(const QString &path) const
{
    printMessage("Copying default component library...");

    QDir saveDir;
    saveDir.setPath(path);
    saveDir.mkpath("componentLibraries/defaultLibrary");
    saveDir.cd("componentLibraries");
    saveDir.cd("defaultLibrary");

    copyDir( QString(mHopsanRootPath+"/componentLibraries/defaultLibrary"), saveDir.path() );

    QStringList allFiles;
    findAllFilesInFolderAndSubFolders(saveDir.path(),"hpp",allFiles);
    findAllFilesInFolderAndSubFolders(saveDir.path(),"h",allFiles);
    findAllFilesInFolderAndSubFolders(saveDir.path(),"cc",allFiles);
    findAllFilesInFolderAndSubFolders(saveDir.path(),"cpp",allFiles);

    Q_FOREACH(const QString file, allFiles)
    {
        QFile::setPermissions(file, QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::WriteUser | QFile::ReadOther | QFile::WriteOther);
    }

    return true;
}


//! @todo maybe this function should not be among general utils
//! @todo should not copy .svn folders
//! @todo Error checking
bool HopsanGeneratorBase::copyBoostIncludeFilesToDir(const QString &path) const
{
    printMessage("Copying Boost include files...");

    QDir saveDir;
    saveDir.setPath(path);
    saveDir.mkpath("include/boost");
    saveDir.cd("include");
    saveDir.cd("boost");

    copyDir( QString("../Dependencies/boost"), saveDir.path() );

    saveDir.cd("bin");
    QStringList binFiles = saveDir.entryList(QDir::Files | QDir::NoDotAndDotDot);

    Q_FOREACH(const QString &fileName, binFiles)
    {
        QFile file(path+"/include/boost/bin/"+fileName);
        //qDebug() << "File: " << path+"/include/boost/bin/"+fileName;
        file.copy(path+"/"+fileName);
        //qDebug() << "Target: " << path << "/" << fileName;
    }

    return true;
}

//! @brief Copies a file to a target and informs user of the outcome
//! @param[in] source Source file
//! @param[in] target Target where to copy file
//! @returns True if copy successful, otherwise false
bool HopsanGeneratorBase::copyFile(const QString &source, const QString &target) const
{
    QString error;
    if (::copyFile(source, target, error))
    {
        QFileInfo sourceFileInfo(source);
        printMessage("Copying " + sourceFileInfo.fileName());
        return true;
    }
    else
    {
        printErrorMessage(error);
        return false;
    }
}

//! @brief Copy a directory with contents
//! @param[in] fromPath The absolute path to the directory to copy
//! @param[in] toPath The absolute path to the destination (including destination dir name)
//! @returns True if success else False
//! @details Copy example:  copyDir(.../files/inlude, .../files2/include)
bool HopsanGeneratorBase::copyDir(const QString &fromPath, const QString &toPath) const
{
    QString error;
    if(::copyDir(fromPath, toPath, error))
    {
        printMessage("Copying " + fromPath);
        return true;
    }
    else
    {
        printErrorMessage(error);
        return false;
    }
}

//! @brief Cleans up after import/export operation by removing all specified files and sub directories
//! @param path Path to directory to clean up
//! @param files List of files to remove
//! @param subDirs List of sub directories to remove
void HopsanGeneratorBase::cleanUp(const QString &path, const QStringList &files, const QStringList &subDirs) const
{
    printMessage("Cleaning up directory: " + path);

    QDir cleanDir(path);
    Q_FOREACH(const QString &file, files)
    {
        cleanDir.remove(file);
    }
    Q_FOREACH(const QString &subDir, subDirs)
    {
        removeDir(path+"/"+subDir);
    }
}



void HopsanGeneratorBase::getNodeAndCqTypeFromInterfaceComponent(const QString &compType, QString &nodeType, QString &cqType)
{
    if(compType == "HydraulicInterfaceC")
    {
        nodeType = "NodeHydraulic";
        cqType = "c";
    }
    else if(compType == "HydraulicInterfaceQ")
    {
        nodeType = "NodeHydraulic";
        cqType = "q";
    }
    if(compType == "MechanicInterfaceC")
    {
        nodeType = "NodeMechanic";
        cqType = "c";
    }
    else if(compType == "MechanicInterfaceQ")
    {
        nodeType = "NodeMechanic";
        cqType = "q";
    }
    if(compType == "MechanicRotationalInterfaceC")
    {
        nodeType = "NodeMechanicRotational";
        cqType = "c";
    }
    else if(compType == "MechanicRotationalInterfaceQ")
    {
        nodeType = "NodeMechanicRotational";
        cqType = "q";
    }
    if(compType == "PneumaticInterfaceC")
    {
        nodeType = "NodePneumatic";
        cqType = "c";
    }
    else if(compType == "PneumaticInterfaceQ")
    {
        nodeType = "NodePneumatic";
        cqType = "q";
    }
    if(compType == "ElectricInterfaceC")
    {
        nodeType = "NodeElectric";
        cqType = "c";
    }
    else if(compType == "ElectricInterfaceQ")
    {
        nodeType = "NodeElectric";
        cqType = "q";
    }
}
