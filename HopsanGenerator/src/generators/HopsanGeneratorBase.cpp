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
#include <QDomElement>
#include <QUuid>
#include <QProcess>

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


HopsanGeneratorBase::HopsanGeneratorBase(const QString &hopsanInstallPath, const CompilerSelection &compilerSelection, const QString &tempPath)
{
    mHopsanRootPath = hopsanInstallPath;
    mCompilerSelection = compilerSelection;

    if(!mCompilerSelection.path.isEmpty())
    {
        mCompilerSelection.path = QFileInfo(mCompilerSelection.path).absoluteFilePath();
#if !defined(_WIN32)
        // Add the last / here so that the compiler name can be directly appended to mCompilerPath.
        // If compilerPath is empty (compiler in PATH) then we do not want to check if a / should be added or not
        // when using this variable
        mCompilerSelection.path.append("/");
#endif
    }

#if !defined(_WIN32)
    // Special hack for SNAP package version,
    // if we are inside a SNAP then overwrite compiler selection with the bundled compiler
    QString snapGCCPath="INVALID_PATH";
#if QT_VERSION < 0x051000
    QByteArray snapEnv = qgetenv("SNAP");
    if (!snapEnv.isNull()) {
        snapGCCPath = QString(snapEnv)+"/usr/bin/";
    }
#else
    if (qEnvironmentVariableIsSet("SNAP")) {
        snapGCCPath = qEnvironmentVariable("SNAP")+"/usr/bin/";
    }
#endif
    if (QFile::exists(snapGCCPath+"g++")) {
        mCompilerSelection.path = snapGCCPath;
    }
#endif


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
        if(!varDeclarations.isEmpty()) {
            varDeclarations.append("        ");
        }
        varDeclarations.append("double "+comp.parNames[i]+";\n");
    }
    for(int i=0; i<comp.varNames.size(); ++i)
    {
        if(!varDeclarations.isEmpty()) {
            varDeclarations.append("        ");
        }
        varDeclarations.append(comp.varTypes[i]+" "+comp.varNames[i]+";\n");
        if(comp.varTypes[i] == "double" && !comp.varNames[i].contains("]"))
            varDeclarations.append("        "+comp.varTypes[i]+" *mpOUTPUT_"+comp.varNames[i]+";\n");
    }
    for(int i=0; i<comp.utilities.size(); ++i)
    {
        if(!varDeclarations.isEmpty()) {
            varDeclarations.append("        ");
        }
        varDeclarations.append(comp.utilities[i]+" "+comp.utilityNames[i]+";\n");
    }
    int portId=1;
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        QStringList varNames;
        if(comp.portNodeTypes[i] == "NodeSignal")
        {
            varNames << comp.portNames[i];
        }
        else
        {
            varNames << GeneratorNodeInfo(comp.portNodeTypes[i]).qVariables << GeneratorNodeInfo(comp.portNodeTypes[i]).cVariables;
        }
        for(int v=0; v<varNames.size(); ++v)
        {
            QString varName;
            if(comp.portNodeTypes[i] == "NodeSignal") {
                varName = varNames[v];
            }
            else {
                varName = varNames[v] + QString::number(portId);
            }
            if(!varDeclarations.isEmpty()) {
                varDeclarations.append("        ");
            }
            varDeclarations.append("double "+varName+";\n");
        }
        ++portId;
    }
    if(varDeclarations.endsWith("\n")) {
        varDeclarations.chop(1);
    }


    //Declare node data pointers
    portId=1;
    QStringList allVarNames;
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        QString id = QString::number(portId);
        if(comp.portNodeTypes[i] == "NodeSignal")
        {
            allVarNames << "*mp"+comp.portNames[i]+"";
        }
        else
        {
            QStringList vars;
            vars << GeneratorNodeInfo(comp.portNodeTypes[i]).qVariables << GeneratorNodeInfo(comp.portNodeTypes[i]).cVariables;

            for(int v=0; v<vars.size(); ++v)
            {
                allVarNames << "*mp"+comp.portNames[i]+"_"+vars[v];
            }
        }
        ++portId;
    }
    QString dataPtrDeclarations;
    if(!allVarNames.isEmpty())
    {
        if(!dataPtrDeclarations.isEmpty()) {
            dataPtrDeclarations.append("        ");
        }
        dataPtrDeclarations = "double ";
        dataPtrDeclarations.append(allVarNames[0]);
        for(int i=1; i<allVarNames.size(); ++i)
        {
            dataPtrDeclarations.append(", "+allVarNames[i]);
        }
        dataPtrDeclarations.append(";\n");
    }
    if(dataPtrDeclarations.endsWith("\n")) {
        dataPtrDeclarations.chop(1);
    }

    //Declare ports
    QString portDeclarations;
    if(!comp.portNames.isEmpty()) {
        portDeclarations.append("Port ");
        bool portFound=false;
        for(int i=0; i<comp.portNames.size(); ++i)
        {
            if(comp.portNodeTypes[i] != "NodeSignal") {
                portDeclarations.append("*mp"+comp.portNames[i]+", ");
                portFound = true;
            }
        }
        if(portFound) { //At least one port to declare
            portDeclarations.chop(2);   //Remove trailing comma
            portDeclarations.append(";");
        }
        else {  //No ports to declare
            portDeclarations.clear();
        }
    }


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
        if(!registerParameters.isEmpty()) {
            registerParameters.append("            ");
        }
        registerParameters.append("addConstant(\""+comp.parDisplayNames[i]+"\", \""
                  +comp.parDescriptions[i]+"\", \""+comp.parUnits[i]+"\", "+comp.parInits[i]+", "+comp.parNames[i]+");\n");
    }
    if(registerParameters.endsWith("\n")) {
        registerParameters.chop(1);
    }


    //Add ports
    QString addPorts;
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        if(!addPorts.isEmpty()) {
            addPorts.append("            ");
        }
        if(comp.portNodeTypes[i] == "NodeSignal")
        {
            QString init = comp.portDefaults[i];
            if(init.isEmpty())
            {
                init = "0";
            }
            if(comp.portTypes[i] == "ReadPort")
            {
                addPorts.append("addInputVariable(\""+comp.portNames[i]+"\", \""+comp.portDescriptions[i]+"\", \""+comp.portUnits[i]+"\", "+init+", &mp"+comp.portNames[i]+");\n");
            }
            else if(comp.portTypes[i] == "WritePort")
            {
                addPorts.append("addOutputVariable(\""+comp.portNames[i]+"\", \""+comp.portDescriptions[i]+"\", \""+comp.portUnits[i]+"\", "+init+", &mp"+comp.portNames[i]+");\n");
            }
        }
        else
        {
            addPorts.append("mp"+comp.portNames[i]+" = add"+comp.portTypes[i]
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
        if(!addPorts.isEmpty()) {
            addPorts.append("            ");
        }
        if(comp.varTypes[i] == "double" && !comp.varNames[i].contains("]"))
            addPorts.append("addOutputVariable(\""+comp.varNames[i]+"\", \"\", \"\", 0, &mpOUTPUT_"+comp.varNames[i]+");\n");
    }
    if(addPorts.endsWith("\n")) {
        addPorts.chop(1);
    }


    //Initialize variables
    QString initializeVariables;
    for(int i=0; i<comp.varInits.size(); ++i)
    {
        if(!comp.varInits[i].isEmpty())
        {
            if(!initializeVariables.isEmpty()) {
                initializeVariables.append("            ");
            }
            initializeVariables.append(comp.varNames[i]+" = "+comp.varInits[i]+";\n");
        }
    }
    if(initializeVariables.endsWith("\n")) {
        initializeVariables.chop(1);
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
            QString varName = varNames[v];
            if(!getDataPtrs.isEmpty()) {
                getDataPtrs.append("            ");
            }
            getDataPtrs.append("mp"+comp.portNames[i]+"_"+varName+" = getSafeNodeDataPtr(mp"+comp.portNames[i]+", "+comp.portNodeTypes[i]+"::"+varLabels[v]);
            if(comp.portNotReq[i])
            {
                getDataPtrs.append(", "+comp.portDefaults[i]);
            }
            getDataPtrs.append(");\n");
        }
        ++portId;
    }
    if(getDataPtrs.endsWith("\n")) {
        getDataPtrs.chop(1);
    }


    //Read input variables
    QString readInputs;
    portId=1;
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        QStringList varNames;
        if(comp.portNodeTypes[i] == "NodeSignal")
        {
            varNames << comp.portNames[i];
        }
        else
        {
            varNames << GeneratorNodeInfo(comp.portNodeTypes[i]).qVariables << GeneratorNodeInfo(comp.portNodeTypes[i]).cVariables;
        }

        for(int v=0; v<varNames.size(); ++v)
        {
            QString varName;
            if(comp.portNodeTypes[i] == "NodeSignal") {
                varName = varNames[v];
            }
            else {
                varName = varNames[v] + QString::number(portId);
            }
            if(!readInputs.isEmpty()) {
                readInputs.append("            ");
            }
            if(comp.portNodeTypes[i] == "NodeSignal") {
                readInputs.append(varName+" = (*mp"+comp.portNames[i]+");\n");
            }
            else {
                readInputs.append(varName+" = (*mp"+comp.portNames[i]+"_"+varNames[v]+");\n");
            }
        }
        ++portId;
    }
    if(readInputs.endsWith("\n")) {
        readInputs.chop(1);
    }


    //Configure code
    QString confCode;
    for(int i=0; i<comp.confEquations.size(); ++i)
    {
        if(!confCode.isEmpty()) {
            confCode.append("            ");
        }
        confCode.append(comp.confEquations[i]+"\n");
    }
    if(confCode.endsWith("\n")) {
        confCode.chop(1);
    }

    //Initialize code
    QString initCode;
    for(int i=0; i<comp.initEquations.size(); ++i)
    {
        if(!initCode.isEmpty()) {
            initCode.append("            ");
        }
        initCode.append(comp.initEquations[i]+"\n");
    }
    if(initCode.endsWith("\n")) {
        initCode.chop(1);
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
            if(comp.portNodeTypes[i] == "NodeSignal") {
                varName = varNames[v];
            }
            else {
                varName = varNames[v] + QString::number(portId);
            }
            if(!writeOutputs.isEmpty()) {
                writeOutputs.append("            ");
            }
            if(comp.portNodeTypes[i] == "NodeSignal") {
                writeOutputs.append("(*mp"+comp.portNames[i]+") = "+varName+";\n");
            }
            else {
                writeOutputs.append("(*mp"+comp.portNames[i]+"_"+varNames[v]+") = "+varName+";\n");
            }
        }
        ++portId;
    }
    for(int i=0; i<comp.varNames.size(); ++i)
    {
        if(comp.varTypes[i] == "double" && !comp.varNames[i].contains("]")) {
            if(!writeOutputs.isEmpty()) {
                writeOutputs.append("            ");
            }
            writeOutputs.append("(*mpOUTPUT_"+comp.varNames[i]+") = "+comp.varNames[i]+";\n");
        }
    }
    if(writeOutputs.endsWith("\n")) {
        writeOutputs.chop(1);
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
        if(!simCode.isEmpty()) {
            simCode.append("            ");
        }
        simCode.append(comp.simEquations[i]+"\n");
    }
    if(simCode.endsWith("\n")) {
        simCode.chop(1);
    }


    //Finalize code
    QString finalCode;
    for(int i=0; i<comp.finalEquations.size(); ++i)
    {
        if(!finalCode.isEmpty()) {
            finalCode.append("            ");
        }
        finalCode.append(comp.finalEquations[i]+"\n");
    }
    if(finalCode.endsWith("\n")) {
        finalCode.chop(1);
    }


    //Deconfiguration code
    QString deconfCode;
    for(int i=0; i<comp.deconfEquations.size(); ++i)
    {
        if(!deconfCode.isEmpty()) {
            deconfCode.append("            ");
        }
        deconfCode.append(comp.deconfEquations[i]+"\n");
    }
    if(deconfCode.endsWith("\n")) {
        deconfCode.chop(1);
    }


    //Auxiliary functions
    QString auxiliaryFunctions;
    for(int i=0; i<comp.auxiliaryFunctions.size(); ++i)
    {
        if(!auxiliaryFunctions.isEmpty()) {
            auxiliaryFunctions.append("        ");
        }
        auxiliaryFunctions.append(comp.auxiliaryFunctions[i]+"\n");
    }
    if(auxiliaryFunctions.endsWith("\n")) {
        auxiliaryFunctions.chop(1);
    }


    QFile compTemplateFile(":/templates/generalComponentTemplate.hpp");
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
    ccLibStream << "    pHopsanExternalLibInfo->libCompiledDebugRelease = (char*)HOPSAN_BUILD_TYPE_STR;\n";
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
bool HopsanGeneratorBase::generateNewLibrary(QString dstPath, QString libName, QStringList hppFiles, QStringList cafFiles, QStringList cflags, QStringList lflags, QStringList includePaths, QStringList linkPaths, QStringList linkLibraries)
{
    printMessage("Creating new component library...");

    // Make sure / at end
    if (!dstPath.endsWith("/"))
    {
        dstPath.append("/");
    }

    const QString libID = QUuid::createUuid().toString().remove('{').remove('}');

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
    ccLibStream << "    pHopsanExternalLibInfo->libCompiledDebugRelease = (char*)HOPSAN_BUILD_TYPE_STR;\n";
    ccLibStream << "}\n";
    ccLibFile.close();

    printMessage("Writing " + libName + "_lib.xml...");

    ComponentLibrary lib;
    lib.mId = libID;
    lib.mName = libName;
    lib.mSharedLibraryName = libName;
    lib.mSharedLibraryDebugExtension = "_d";
    lib.mSourceFiles.append(libName+".cpp");
    lib.mBuildFlags.append(BuildFlags(cflags, lflags));
    lib.mIncludePaths = includePaths;
    lib.mLinkPaths = linkPaths;
    lib.mLinkLibraries = linkLibraries;
    lib.mComponentXMLFiles = cafFiles;

    const QString libFilePath = dstPath+libName+"_lib.xml";
    bool saveOK = lib.saveToXML(libFilePath);
    if(!saveOK)
    {
        printErrorMessage("Failed to open "+libFilePath+" for writing.");
        return false;
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
    QByteArray temp_data;
    QTextStream temp_data_stream(&temp_data);
    domDocument.save(temp_data_stream, 2);
    fmuCafFile.write(temp_data);
    fmuCafFile.close();

    return true;
}

bool HopsanGeneratorBase::generateComponentSourceFile(QString &path, ComponentSpecification &comp, TargetLanguageT target)
{
    QFile sourceFile;
    sourceFile.setFileName(path);
    if(!sourceFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Could not open "+QFileInfo(path).absoluteFilePath()+" for writing.");
        return false;
    }

    QTextStream out(&sourceFile);
    if(target == Cpp) {
        out << generateSourceCodefromComponentSpec(comp);
    }
    else if(target == Modelica) {
        out << generateModelicaCodeFromComponentSpec(comp);
    }

    sourceFile.close();
    return true;
}

bool HopsanGeneratorBase::generateLibrarySourceFile(const ComponentLibrary &lib)
{
    if(lib.mSourceFiles.isEmpty()) {
        printErrorMessage("No library source file specified in "+lib.mLoadFilePath);
        return false;
    }
    QString srcPath = QFileInfo(lib.mLoadFilePath).absoluteDir().absoluteFilePath(lib.mSourceFiles.first());
    QFile srcFile(srcPath);
    if(!srcFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        printErrorMessage("Cannot open "+srcPath+" for writing.");
        return false;
    }

    QString output;
    output.append(QString("#include <iostream>\n"));
    output.append(QString("#include \"ComponentEssentials.h\"\n"));
    for(const QString& srcFile : lib.mComponentCodeFiles) {
        output.append("#include \""+srcFile+"\"\n");
    }
    output.append("\n");
    output.append("using namespace hopsan;\n");
    output.append("\n");
    output.append("extern \"C\" DLLEXPORT void register_contents(ComponentFactory* pComponentFactory, NodeFactory* pNodeFactory)\n");
    output.append("{\n");
    output.append("    //Register Components\n");
    for(const QString& cafPath : lib.mComponentXMLFiles) {
        QFile cafFile(QFileInfo(lib.mLoadFilePath).path()+"/"+cafPath);
        QDomDocument cafDomDocument;
        QDomElement cafRootElement;
        cafRootElement = loadXMLDomDocument(cafFile,cafDomDocument,"hopsanobjectappearance");
        if(QDomElement() == cafRootElement) {
            printErrorMessage("Unable to parse XML file: "+QString(lib.mLoadFilePath+"/"+cafPath));
            return false;
        }
        QDomElement modelObjectElement = cafRootElement.firstChildElement("modelobject");
        if(modelObjectElement.isNull()) {
            printErrorMessage("Unable to parse XML file: "+QString(lib.mLoadFilePath+"/"+cafPath)+" (cannot find \"modelobject\" element)");
            return false;
        }
        QString typeName = modelObjectElement.attribute("typename");
        if(typeName.isEmpty()) {
            printErrorMessage("Type name not specified in component XML file.");
            return false;
        }
        cafFile.close();
        if(modelObjectElement.hasAttribute("subtypename") || modelObjectElement.hasAttribute("hmffile")) {
            continue;
        }
        output.append("    pComponentFactory->registerCreatorFunction(\""+typeName+"\", "+typeName+"::Creator);\n");
    }
    output.append("\n");
    output.append("    //Register custom nodes (if any)\n");
    output.append("    HOPSAN_UNUSED(pNodeFactory);\n");
    output.append("}\n");
    output.append("\n");
    output.append("extern \"C\" DLLEXPORT void get_hopsan_info(HopsanExternalLibInfoT *pHopsanExternalLibInfo)\n");
    output.append("{\n");
    output.append("    pHopsanExternalLibInfo->hopsanCoreVersion = (char*)HOPSANCOREVERSION;\n");
    output.append("    pHopsanExternalLibInfo->libCompiledDebugRelease = (char*)HOPSAN_BUILD_TYPE_STR;\n");
    output.append("    pHopsanExternalLibInfo->libName = (char*)\""+lib.mName+"\";\n");
    output.append("}\n");

    QTextStream out(&srcFile);
    out << output;
    srcFile.close();
    return true;
}

QString HopsanGeneratorBase::generateModelicaCodeFromComponentSpec(ComponentSpecification comp) const
{
    QString output;
    QTextStream outStream(&output);
    QString indent = "    "; //Hard-coded for now

    outStream << "model " << comp.typeName << " \"" << comp.displayName << "\"\n";
    outStream << indent << "annotation(hopsanCqsType = \"" << comp.cqsType << ", linearTransform = \"" << comp.transform << "\")\n";
    outStream << "\n";
    for(int p=0; p<comp.portNodeTypes.size(); ++p) {
        QString nodeType = comp.portNodeTypes[p];
        if(nodeType == "NodeSignal" && comp.portTypes[p] == "ReadPort") {
            nodeType = "input Real";
        }
        else if(nodeType == "NodeSignal" && comp.portTypes[p] == "WritePort") {
            nodeType = "output Real";
        }
        outStream << indent << nodeType << " " << comp.portNames[p] << ";\n";
    }
    outStream << "\n";
    for(int p=0; p<comp.parNames.size(); ++p) {
        outStream << indent << "parameter Real " << comp.parNames[p] << "(unit=\"" << comp.parUnits[p] << "\")=" << comp.parInits[p] << "\n";
    }
    outStream << "\n";
    for(int v=0; v<comp.varNames.size(); ++v) {
        QString varType = comp.varTypes[v];
        if("double" == varType) {
            varType = "Real";   //Might need more transformations here
        }
        outStream << indent << varType << " " << comp.varNames[v] << "(start=" << comp.varInits[v] << ");\n";
    }
    outStream << "\n";
    outStream << "equation\n";
    outStream << indent << "// TLM equations\n";
    for(int p=0; p<comp.portNodeTypes.size(); ++p) {
        QString portName = comp.portNames[p];
        QString nodeType = comp.portNodeTypes[p];
        if(nodeType == "NodeHydraulic") {
            outStream << indent << portName+".p = "+portName+".c + "+portName+".Zc*"+portName+".q;\n";
        }
        else if(nodeType == "NodeMechanic") {
            outStream << indent << portName+".f = "+portName+".c + "+portName+".Zc*"+portName+".v;\n";
        }
        else if(nodeType == "NodeMechanicRotational") {
            outStream << indent << portName+".T = "+portName+".c + "+portName+".Zc*"+portName+".w;\n";
        }
        else if(nodeType == "NodeElectric") {
            outStream << indent << portName+".U = "+portName+".c + "+portName+".Zc*"+portName+".I;\n";
        }
        else if(nodeType == "NodePneumatic") {
            outStream << indent << portName+".p = "+portName+".c + "+portName+".Zc*"+portName+".Qdot;\n";
        }
    }

    outStream << indent << "\n";
    outStream << "algorithm\n";
    outStream << indent << "\n";
    outStream << "end " << comp.typeName << ";\n";

    return output;
}


void HopsanGeneratorBase::setOutputPath(const QString &path)
{
    mOutputPath = path;
    printMessage("Setting output path: " + path);
}


QString HopsanGeneratorBase::getHopsanCoreIncludePath() const
{
    return mHopsanRootPath+"/HopsanCore/include";
}


QString HopsanGeneratorBase::getHopsanBinPath() const
{
    return mHopsanRootPath+"/bin";
}

QString HopsanGeneratorBase::getHopsanLibPath() const
{
    return mHopsanRootPath+"/lib";
}

QString HopsanGeneratorBase::getHopsanRootPath() const
{
    return mHopsanRootPath;
}

const CompilerSelection& HopsanGeneratorBase::getCompilerSelection() const
{
    return mCompilerSelection;
}

void HopsanGeneratorBase::setQuiet(bool quiet)
{
    mShowMessages = !quiet;
}


bool HopsanGeneratorBase::assertFilesExist(const QString &path, const QStringList &files) const
{
    for(const QString& file : files)
    {
        QString absFilePath = (path.isEmpty() ? file : path+"/"+file);
        if(!QFile::exists(absFilePath))
        {
            printErrorMessage(QString("File '%1' not found in directory '%2'").arg(file).arg(path));
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

    QProcess p;
    if(!workingDirectory.isEmpty())
    {
        p.setWorkingDirectory(workingDirectory);
    }
    p.start(name, args);
    double time = 0;
    double maxTime = 600000;    //Ten minutes in milliseconds
    while(!p.waitForFinished(100) && time < maxTime) {
        stdOut = p.readAllStandardOutput();
        if(!stdOut.isEmpty()) {
            printMessage(stdOut);
        }
        stdErr = p.readAllStandardError();
        if(!stdErr.isEmpty()) {
            printMessage(stdErr);
        }
        time += 100;    //Print output every .1 seconds
    };

    return (p.exitStatus() == QProcess::ExitStatus::NormalExit);
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
    QRegExp excludedFiles("*.o|*.cmake|*.txt|*.html|*.pdf|*.md|*.json|*.py");
    excludedFiles.setPatternSyntax(QRegExp::Wildcard);

    if (saveDir.mkpath(".") && copyDir(mHopsanRootPath+"/HopsanCore", tgtPath+"/HopsanCore", {excludedFiles}))
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

    copyDir( QString(mHopsanRootPath+"/componentLibraries/defaultLibrary"), saveDir.path(), {} );

    QStringList allFiles;
    for (const auto& suffix : {"hpp", "h", "cc", "cpp"}) {
        findAllFilesInFolderAndSubFolders(saveDir.path(), suffix, allFiles);
    }
    setRW_RW_RW_FilePermissions(allFiles);

    return true;
}


//! @brief Copies external component code to specified directory
//! @param[in] destinationPath Path to copy to
//! @param[in] externalLibraries List with paths to external component libraries
//! @param[in,out] extraSourceFiles List with additional source files, which will not be copied (absolute paths are returned)
bool HopsanGeneratorBase::copyExternalComponentCodeToDir(const QString &destinationPath, const QStringList &externalLibraries, QStringList &rExtraSourceFiles, QStringList &rIncludePaths, QStringList &rLinkPaths, QStringList &rLinkLibraries) const
{
    QDir componentLibrariesDestinationPath;
    componentLibrariesDestinationPath.setPath(destinationPath);
    componentLibrariesDestinationPath.mkpath("componentLibraries");

    QString externalLibraryIncludeCode =
R"(// This file was automatically generated

// Basic includes
#include "ComponentEssentials.h"

// Declare the registration function
namespace hopsan {
  void register_extra_components(hopsan::ComponentFactory* pComponentFactory);
}

//  Use hopsan namespace (if components do not specify it explicitly)
using namespace hopsan;

// Includes of external components from each library

)";

    QString externalLibraryRegistrationCode =
R"(
// Registration of components from each library
void hopsan::register_extra_components(hopsan::ComponentFactory* pComponentFactory)
{
)";

    QStringList takenNames;
    for (const auto& libpath : externalLibraries) {
        ComponentLibrary lib;
        bool loadOK = lib.loadFromXML(libpath);
        if (takenNames.contains(lib.mName)) {
            printErrorMessage(QString("A library with name: %1 has already been exported. Unique names are required").arg(lib.mName));
            return false;
        }
        if (loadOK)
        {
            printMessage(QString("Copying external component library: %1 ...").arg(lib.mName));

            QDir libDestinationPath;
            libDestinationPath.setPath(destinationPath+"/componentLibraries");
            libDestinationPath.mkpath(lib.mName);
            libDestinationPath.cd(lib.mName);

            const QFileInfo libInfo(libpath);
            const QString libraryRootPath = libInfo.absolutePath();
            // Exclude pre-compiled dynamic library assuming typical suffixes
            // Note! other dynamic libraries are included, in case they are needed by the code (unlikely)
            QList<QRegExp> excludeFileRegexps;
            QString regexp = "(lib)?"+lib.mSharedLibraryName;
            if (!lib.mSharedLibraryDebugExtension.isEmpty()) {
                regexp.append(QString(R"((%1)?)").arg(lib.mSharedLibraryDebugExtension));
            }
            regexp.append(R"(\.(dll|so|dylib|a).*)");
            excludeFileRegexps.append(QRegExp(regexp));

            copyDir( libraryRootPath, libDestinationPath.path(), excludeFileRegexps);

            QStringList allFiles;
            for (const auto& suffix : {"hpp", "h", "cc", "cpp"}) {
                findAllFilesInFolderAndSubFolders(libDestinationPath.path(), suffix, allFiles);
            }
            setRW_RW_RW_FilePermissions(allFiles);

            QString generatorErrorMessage;
            for(const QString& extraSourceFile : lib.mExtraSourceFiles) {
                rExtraSourceFiles.append(QDir(libraryRootPath).filePath(extraSourceFile));
            }
            for(const QString& includePath : lib.mIncludePaths) {
                rIncludePaths.append(QDir(libraryRootPath).filePath(includePath));
            }
            for(const QString& linkPath : lib.mLinkPaths) {
                rLinkPaths.append(QDir(libraryRootPath).filePath(linkPath));
            }
            for(const QString& linkLibrary : lib.mLinkLibraries) {
                rLinkLibraries.append(QDir(libraryRootPath).filePath(linkLibrary));
            }
            bool genOK = lib.generateRegistrationCode(libraryRootPath, externalLibraryIncludeCode, externalLibraryRegistrationCode, generatorErrorMessage);
            if (!genOK) {
                printErrorMessage(QString("Failed to generate code for library %1, Error: %2").arg(libpath).arg(generatorErrorMessage));
                return false;
            }
        }
        else
        {
            printErrorMessage(QString("Could not load %1 for some reason").arg(libpath));
            return false;
        }
    }
    externalLibraryRegistrationCode.append("}\n");

    QFile commonLibraryCpp(destinationPath+"/componentLibraries/extra-components.cpp");
    if (commonLibraryCpp.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream ts(&commonLibraryCpp);
        ts << externalLibraryIncludeCode;
        ts << externalLibraryRegistrationCode;
    } else {
        //! @todo some error
        return false;
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

    copyDir( QString("../dependencies/boost"), saveDir.path(), {} );

    saveDir.cd("bin");
    QStringList binFiles = saveDir.entryList(QDir::Files | QDir::NoDotAndDotDot);

    for(const QString &fileName : binFiles) {
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
//! @param[in] excludeRegExps List of regexps for files to exclude
//! @returns True if success else False
//! @details Copy example:  copyDir(.../files/inlude, .../files2/include)
bool HopsanGeneratorBase::copyDir(const QString &fromPath, const QString &toPath, const QList<QRegExp>& excludeRegExps) const
{
    QString error;
    if(::copyDir(fromPath, toPath, excludeRegExps, error))
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
    for(const QString &file : files) {
        cleanDir.remove(file);
    }
    for(const QString &subDir : subDirs) {
        removeDir(path+"/"+subDir);
    }
}

bool HopsanGeneratorBase::generateModelFile(const ComponentSystem *pSystem, const QString &buildPath, const QMap<QString, QString> &replaceMap) const
{
    QString modelName = pSystem->getName().c_str();
    QFile modelHppFile(buildPath + "/model.hpp");
    QFile modelFile(buildPath + "/../" + modelName + ".hmf");

    printMessage("Generating "+modelHppFile.fileName());

    if (!modelFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        printErrorMessage(QString("Could not open %1 for reading.").arg(modelFile.fileName()));
        return false;
    }

    QStringList modelLines;
    while (!modelFile.atEnd())
    {
        QString line = modelFile.readLine();
        line.chop(1);
        for (auto it = replaceMap.begin(); it != replaceMap.end(); ++it) {
            line.replace(it.key(), it.value());
        }
        line.replace(R"(")", R"(\")");
        modelLines.append(line);
    }
    modelLines.last().append("\\n");
    modelFile.close();


    if(!modelHppFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage(QString("Failed to open %1 for writing.").arg(modelHppFile.fileName()));
        return false;
    }

    QTextStream modelHppStream(&modelHppFile);
    modelHppStream << "#include <string>\n\n";
    modelHppStream << "std::string getModelString()\n{\n";
    modelHppStream << "    std::string model = ";
    for(const QString& line : modelLines)
    {
        modelHppStream << "\""+line+"\"\n";
    }
    modelHppStream << "    ;\n\n";
    modelHppStream << "    return model;\n}\n";
    modelHppFile.close();
    return true;
}


void HopsanGeneratorBase::copyModelAssetsToDir(const QString &tgtDirPath, hopsan::ComponentSystem *pSystem, QMap<QString, QString> &assetsMap) const
{
    QDir targetDir(tgtDirPath);
    std::list<hopsan::HString> assets = pSystem->getModelAssets();
    if (!assets.empty()) {
        printMessage("Exporting model assets");
    }
    for (const auto& asset : assets) {
        QFileInfo assetInfo(asset.c_str());
        QString absSourceAssetPath = pSystem->findFilePath(asset).c_str();
        QString targetAssetPath;
        if (assetInfo.isAbsolute()) {
            // For absolute windows paths, replace \ with /
            targetAssetPath = absSourceAssetPath.replace(R"(\)", "/");
            // For absolute windows paths, replace :/ with /
            targetAssetPath = absSourceAssetPath.replace(":/", "/");
            // For Unix paths remove leading /
            if (targetAssetPath.startsWith("/")) {
                targetAssetPath.remove(0,1);
            }
        } else {
            // For relative windows paths, replace \ with /
            targetAssetPath = assetInfo.filePath().replace(R"(\)", "/");
            // For relative paths, replace leading ../ with a number, to ensure uniqueness
            int ctr=0;
            while(targetAssetPath.startsWith("../")) {
                ++ctr;
                targetAssetPath.remove(0,3);
            }
            if(ctr>0) {
                targetAssetPath.prepend(QString("%1_").arg(ctr));
            }
        }
        targetAssetPath.prepend(tgtDirPath+"/");

        copyFile(absSourceAssetPath, targetAssetPath);
        setRW_RW_RW_FilePermissions(targetAssetPath);

        QString newRelativeAssetPath = targetDir.relativeFilePath(targetAssetPath);
        assetsMap.insert(asset.c_str(), newRelativeAssetPath);
    }
}
