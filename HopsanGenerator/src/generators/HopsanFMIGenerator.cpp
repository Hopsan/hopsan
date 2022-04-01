
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

#include "generators/HopsanFMIGenerator.h"
#include "GeneratorUtilities.h"
#include "ComponentSystem.h"
#include <cassert>
#include <QUuid>
#include <QDateTime>
#include <QFileInfo>
#include <QDir>
#include <QUrl>
#include <QXmlStreamWriter>
#include <QDebug>

#include <stddef.h>

#include "FMI/fmi_import_context.h"
#include <FMI1/fmi1_import.h>
#include <FMI2/fmi2_import.h>
#include <JM/jm_portability.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace {
QString readTextFromFile(const QString& filePath)
{
    QString text;
    QFile fileToOpen(filePath);
    if(fileToOpen.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream ts(&fileToOpen);
        text = ts.readAll();
        fileToOpen.close();
    }
    return text;
}

void writeTextToFile(QFile& file, const QString& text)
{
    QTextStream ts(&file);
    ts << text;
}

bool fromFmiBoolean(fmi2_boolean_t b)
{
    if (b) {
        return true;
    }
    else {
        return false;
    }
}

void jmLogger(jm_callbacks *c, jm_string module, jm_log_level_enu_t log_level, jm_string message)
{
    (void)module;
    auto pGenerator = static_cast<HopsanFMIGenerator*>(c->context);
    if (pGenerator) {
        switch (log_level) {
        case jm_log_level_fatal:
        case jm_log_level_error:
            pGenerator->printErrorMessage(message);
            break;
        case jm_log_level_warning:
            pGenerator->printWarningMessage(message);
            break;
        // Typically the jm logger info messages are not something we want to see in Hopsan, so show them as debug type
        case jm_log_level_verbose:
        case jm_log_level_info:
            pGenerator->printMessage(message);
            break;
        case jm_log_level_debug:
        default:
            break;
        }
    }
}

}

using namespace hopsan;

typedef struct
{
    QString name;
    QString variableName;
    QString fmuVr;
    QString description;
    QString unit;
    fmi1_base_type_enu_t dataType;
    fmi1_import_variable_t *pFmiVariable;
} hopsan_fmi1_import_variable_t;

typedef struct
{
    QString name;
    QString variableName;
    QString fmuVr;
    QString description;
    QString unit;
    fmi2_base_type_enu_t dataType;
    fmi2_import_variable_t *pFmiVariable;
} hopsan_fmi_import_variable_t;

typedef struct
{
    QString type;
    QString name;
    QString description;
    QString codeVarName;
    QStringList portVariableNames;
    QStringList portVariableCodeVarNames;
    QStringList portVariableFmuVrs;
    QList<size_t> portVariableDataIds;
    int portVariableIOBreakN;
} hopsan_fmi_import_tlm_port_t;



bool replaceFMIVariablesWithTLMPort(QStringList &rPortVarNames, QStringList &rPortVarVars, QStringList &rPortVarRefs, QList<size_t> &rPortVarDataIds,
                                    QVector<hopsan_fmi_import_variable_t> &rActualVariables,
                                    const QStringList &rTags, const QList<int> &rDataIds, const QString &rPortName, const QDomElement portElement)
{
    for(int i=0; i<rTags.size(); ++i) {
        QString name = toValidHopsanVarName(portElement.firstChildElement(rTags[i]).text());
        int idx=-1, j=-1;
        for(const hopsan_fmi_import_variable_t &rVar : rActualVariables) {
            ++j;
            if(rVar.name == name) {
                idx = j;
                break;
            }
        }

        if (idx >= 0) {
            rPortVarNames.append(name);
            rPortVarVars.append(rPortName+"_"+name);
            rPortVarRefs.append(rActualVariables[idx].fmuVr);
            rPortVarDataIds.append(size_t(rDataIds[i]));

            rActualVariables.removeAt(idx);
        }
        else {
            //printErrorMessage("Incorrect variable name given: "+name+". Aborting!");
            return false;
        }
    }
    return true;
}


HopsanFMIGenerator::HopsanFMIGenerator(const QString &hopsanInstallPath, const QString &compilerPath, const QString &tempPath)
    : HopsanGeneratorBase(hopsanInstallPath, compilerPath, tempPath)
{
}


bool HopsanFMIGenerator::generateToFmu(QString savePath, ComponentSystem *pSystem, const QStringList &externalLibraries, int version, bool x64)
{
#ifdef _WIN32
    if(mCompilerSelection.path.isEmpty())
    {
        printErrorMessage("Compiler path not specified.");
        return false;
    }
#endif
    // Set build and stage directories
    const QString fmuBuildPath = QString("%1/fmu-build").arg(savePath);
    const QString fmuStagePath = QString("%1/fmu-stage").arg(savePath);
    // Clear old build artefacts
    removeDir(fmuBuildPath);
    removeDir(fmuStagePath);

    // Create required directories
    QDir saveDir(savePath);
    saveDir.mkpath(fmuBuildPath);
    saveDir.mkpath(QString("%1/resources").arg(fmuStagePath));
    saveDir.mkpath(QString("%1/binaries").arg(fmuStagePath));


    //------------------------------------------------------------------//
    //Obtain model name and version string
    //------------------------------------------------------------------//

    QString modelName = pSystem->getName().c_str();
    QString vStr = QString::number(version);

    //------------------------------------------------------------------//
    //Copy HopsanCore files to export directory
    //------------------------------------------------------------------//

    if(!copyHopsanCoreSourceFilesToDir(fmuBuildPath)) {
        printErrorMessage("Failed to copy Hopsan source code files.");
        return false;
    }
    if(!copyDefaultComponentCodeToDir(fmuBuildPath)) {
        printErrorMessage("Failed to copy default component library files.");
        return false;
    }
    if(!copyExternalComponentCodeToDir(fmuBuildPath, externalLibraries, mExtraSourceFiles, mIncludePaths, mLinkPaths, mLinkLibraries)) {
        printErrorMessage("Failed to export required external component library files.");
        return false;
    }

    //------------------------------------------------------------------//
    //Generate modelDescription.xml
    //------------------------------------------------------------------//

    size_t nReals, nInputs, nOutputs;
    QString guid = QUuid::createUuid().toString();
    bool genOK = generateModelDescriptionXmlFile(pSystem, fmuStagePath, guid, version, nReals, nInputs, nOutputs);
    if (!genOK) {
        return false;
    }

    //------------------------------------------------------------------//
    //Copy source files from templates
    //------------------------------------------------------------------//

    QString fmuHopsanSourceCode = readTextFromFile(":/templates/fmu"+vStr+"_model.c");
    if(fmuHopsanSourceCode.isEmpty()) {
        printErrorMessage("Unable to read template code for fmu"+vStr+"_model.c");
        return false;
    }
    QString dataStr;
    dataStr.append("#define TIMESTEP "+QString::number(pSystem->getDesiredTimeStep()));

    //Get list of interface variables
    QList<ModelVariableSpecification> vars;
    QStringList systemHierarchy = QStringList();
    getModelVariables(pSystem, vars, systemHierarchy);

    //Get list of system parameters
    QList<ParameterSpecification> pars;
    getParameters(pars, pSystem);

    dataStr.append("\n#define NUMDATAPTRS "+QString::number(vars.size()+pars.size()));
    dataStr.append("\n#define INITDATAPTRS ");
    int vr = 0;
    for(const auto &var : vars) {
        dataStr.append("\\\nfmu->dataPtrs["+QString::number(vr)+"] = fmu->pSystem");
        for(const auto &system : var.systemHierarchy) {
            dataStr.append("->getSubComponentSystem(\""+system+"\")");
        }
        dataStr.append("->getSubComponent(\""+var.componentName+"\")->getSafeNodeDataPtr(\""+var.portName+"\","+QString::number(var.dataId)+");");
        dataStr.append("\\\nfmu->parNames["+QString::number(vr)+"] = NULL;");
        ++vr;
    }
    for(const auto &par : pars) {
        dataStr.append("\\\nfmu->dataPtrs["+QString::number(vr)+"] = NULL;");
        dataStr.append("\\\nfmu->parNames["+QString::number(vr)+"] = \""+par.name+"\";");
        ++vr;
    }

    if(version == 1) {
        fmuHopsanSourceCode = replaceTag(fmuHopsanSourceCode, "modelname", modelName);
    }

    fmuHopsanSourceCode = replaceTag(fmuHopsanSourceCode, "data", dataStr);

    QFile fmuHopsanSourceFile(fmuBuildPath+"/fmu"+vStr+"_model.c");
    printMessage("Generating "+fmuHopsanSourceFile.fileName());

    if(fmuHopsanSourceFile.open(QFile::Text | QFile::WriteOnly)) {
        writeTextToFile(fmuHopsanSourceFile, fmuHopsanSourceCode);
        fmuHopsanSourceFile.close();
    }
    else {
        printErrorMessage(QString("Unable to open %1 for writing").arg(fmuHopsanSourceFile.fileName()));
        return false;
    }

    //------------------------------------------------------------------//
    // Generate model file and export assets, replacing asset paths in model
    //------------------------------------------------------------------//

    QMap<QString, QString> assetsMap;
    copyModelAssetsToDir(fmuStagePath+"/resources", pSystem, assetsMap);

    genOK = generateModelFile(pSystem, fmuBuildPath, assetsMap);
    if (!genOK) {
        printErrorMessage("Failed to generate model file");
        return false;
    }

    //------------------------------------------------------------------//
    // Replacing namespace
    //------------------------------------------------------------------//

    replaceNameSpace(fmuBuildPath, version);

    //------------------------------------------------------------------//
    // Compiling and linking
    //------------------------------------------------------------------//

    if(!compileAndLinkFMU(fmuBuildPath, fmuStagePath, modelName, version, x64))
    {
        return false;
    }

    //------------------------------------------------------------------//
    // Compressing files
    //------------------------------------------------------------------//

    bool compressOK = compressFiles(fmuStagePath, modelName);
    if (!compressOK) {
        return false;
    }

    //------------------------------------------------------------------//
    //printMessage("Cleaning up");
    //------------------------------------------------------------------//

    //Clean up temporary files
    cleanUp(savePath, QStringList() << "compile.bat" << modelName+".c" << modelName+".dll" << modelName+".so" << modelName+".o" << modelName+".hmf" <<
            "fmiModelFunctions.h" << "fmiModelTypes.h" << "fmuTemplate.c" << "fmuTemplate.h" << "HopsanFMU.cpp" << "HopsanFMU.h" << "model.hpp" <<
            "modelDescription.xml", QStringList() << "componentLibraries" << "fmu" << "HopsanCore");

    printMessage("Finished.");
    return true;
}



bool HopsanFMIGenerator::readTLMSpecsFromFile(const QString &fileName, QStringList &tlmPortTypes, QList<QStringList> &tlmPortVarNames,
                                              QList<QStringList> &tlmPortValueRefs, QStringList &inVarValueRefs, QStringList &inVarPortNames,
                                              QStringList &outVarValueRefs, QStringList &outVarPortNames, QString &cqsType)
{
    QFile tlmSpecFile;
    tlmSpecFile.setFileName(fileName);
    QDomDocument tlmDomDocument;
    QDomElement tlmRoot;
    if(tlmSpecFile.exists()) {
        printMessage("Reading TLM specifications from "+tlmSpecFile.fileName()+"...");
        tlmRoot = loadXMLDomDocument(tlmSpecFile, tlmDomDocument, "fmutlm");
        tlmSpecFile.close();
    }
    else {
        printMessage("No TLM specification file found.");
        return true;
    }

    if(tlmRoot.isNull()) {
        printErrorMessage("Unable to parse TLM specification file.");
        return true;        // Don't abort import, it could still work without the TLM stuff
    }
    else {
        QString type;
        QStringList qVars;
        QStringList cVars;

        QDomElement portElement = tlmRoot.firstChildElement("tlmport");
        while(!portElement.isNull()) {
            type = portElement.attribute("type");

            QStringList nodeTypes;
            GeneratorNodeInfo::getNodeTypes(nodeTypes);
            for(const QString &nodeType : nodeTypes) {
                GeneratorNodeInfo info(nodeType);
                if(type == info.niceName+"q" || type == info.niceName+"c")
                {
                    for(const QString &var : info.qVariables) {
                        QDomElement element = portElement.firstChildElement(var);
                        if(element.isNull()) {
                            printErrorMessage("Node type does not match variable names.");
                            return false;
                        }
                        qVars.append(element.text());
                    }
                    for(const QString &var : info.cVariables) {
                        QDomElement element = portElement.firstChildElement(var);
                        if(element.isNull()) {
                            printErrorMessage("Node type does not match variable names.");
                            return false;
                        }
                        cVars.append(element.text());
                    }
                }
            }

            for(const QString &nodeType : nodeTypes) {
                GeneratorNodeInfo info(nodeType);
                if(type == info.niceName+"q") {
                    QStringList varNames;
                    tlmPortValueRefs.append(QStringList());
                    for(const QString &var : qVars) {
                        if(!outVarPortNames.contains(var)) {
                            printErrorMessage("Error in TLM specifications: Specified variable does not exist.");
                            return false;
                        }
                        varNames << var;
                        tlmPortValueRefs.last().append(outVarValueRefs[outVarPortNames.indexOf(var)]);
                        outVarValueRefs.removeAt(outVarPortNames.indexOf(var));
                        outVarPortNames.removeAll(var);
                    }
                    for(const QString &var : cVars) {
                        if(!inVarPortNames.contains(var)) {
                            printErrorMessage("Error in TLM specifications: Specified variable does not exist.");
                            return false;
                        }
                        varNames << var;
                        tlmPortValueRefs.last().append(inVarValueRefs[inVarPortNames.indexOf(var)]);
                        inVarValueRefs.removeAt(inVarPortNames.indexOf(var));
                        inVarPortNames.removeAll(var);
                    }

                    printMessage("Adding "+info.niceName+" port of Q-type");

                    if(cqsType == "Signal" || cqsType == "Q") {
                        cqsType = "Q";
                    }
                    else {
                        cqsType = "";
                    }

                    tlmPortTypes.append(type);
                    tlmPortVarNames.append(varNames);
                }
                else if(type == info.niceName+"c") {
                    QStringList varNames;
                    tlmPortValueRefs.append(QStringList());
                    for(const QString &var : qVars) {
                        if(!inVarPortNames.contains(var)) {
                            printErrorMessage("Error in TLM specifications: Specified variable does not exist.");
                            return false;
                        }
                        varNames << var;
                        tlmPortValueRefs.last().append(inVarValueRefs[inVarPortNames.indexOf(var)]);
                        inVarValueRefs.removeAt(inVarPortNames.indexOf(var));
                        inVarPortNames.removeAll(var);
                    }
                    for(const QString &var : cVars) {
                        if(!outVarPortNames.contains(var)) {
                            printErrorMessage("Error in TLM specifications: Specified variable does not exist.");
                            return false;
                        }
                        varNames << var;
                        tlmPortValueRefs.last().append(outVarValueRefs[outVarPortNames.indexOf(var)]);
                        outVarValueRefs.removeAt(outVarPortNames.indexOf(var));
                        outVarPortNames.removeAll(var);
                    }

                    printMessage("Adding "+info.niceName+" port of C-type");

                    if(cqsType == "Signal" || cqsType == "C")
                        cqsType = "C";
                    else
                        cqsType = "";

                    tlmPortTypes.append(type);
                    tlmPortVarNames.append(varNames);
                }
            }
            cVars.clear();
            qVars.clear();
            portElement = portElement.nextSiblingElement("tlmport");
        }
    }

    return true;
}

bool HopsanFMIGenerator::generateModelDescriptionXmlFile(ComponentSystem *pSystem, QString savePath, QString guid, int version, size_t &nReals, size_t &nInputs, size_t &nOutputs)
{
    if(version == 3) {
        QString versionStr = "3.0-beta.3";
        QString dateAndTime = QDateTime::currentDateTime().toUTC().toString(Qt::ISODate);
        QString modelName = pSystem->getName().c_str();

        QFile mdFile(savePath + "/modelDescription.xml");
        if (!mdFile.open(QIODevice::WriteOnly)) {
            printErrorMessage("Unable to write to: "+savePath+"/modelDescription.xml");
            return false;
        }
        QXmlStreamWriter mdWriter(&mdFile);
        mdWriter.setAutoFormatting(true);
        mdWriter.writeStartDocument();
        mdWriter.writeStartElement("fmiModelDescription");
        mdWriter.writeAttribute("modelName", modelName);
        mdWriter.writeAttribute("fmiVersion", versionStr);
        mdWriter.writeAttribute("generationTool", "Hopsan");    //!< We should inclure version number, but not sure which one (gui/cli/core)?
        mdWriter.writeAttribute("generationDateAndTime", dateAndTime);
        mdWriter.writeAttribute("variableNamingConvention", "structured");
        mdWriter.writeAttribute("instantiationToken", guid);

        mdWriter.writeStartElement("CoSimulation");
        mdWriter.writeAttribute("modelIdentifier", modelName);
        mdWriter.writeAttribute("providesIntermediateUpdate", "false");
        mdWriter.writeAttribute("canHandleVariableCommunicationStepSize", "true");
        mdWriter.writeAttribute("hasEventMode", "false");
        mdWriter.writeEndElement(); //CoSimulation

        mdWriter.writeStartElement("DefaultExperiment");
        mdWriter.writeAttribute("startTime", QString::number(pSystem->getTime()));
        mdWriter.writeAttribute("stepSize", QString::number(pSystem->getDesiredTimeStep()));
        mdWriter.writeEndElement(); //DefaultExperiment

        mdWriter.writeStartElement("ModelVariables");

        QList<ModelVariableSpecification> vars;
        QStringList systemHierarchy = QStringList();
        getModelVariables(pSystem, vars, systemHierarchy);

        nReals=0;
        nInputs=0;
        nOutputs=0;
        long vr = 0;
        for(const auto &var : vars) {
            mdWriter.writeStartElement("Float64");
            if(var.systemHierarchy.isEmpty()) {
                mdWriter.writeAttribute("name", var.componentName+"."+var.portName+"."+var.dataName);
            }
            else {
                mdWriter.writeAttribute("name", var.systemHierarchy.join(".")+"."+var.componentName+"."+var.portName+"."+var.dataName);
            }
            mdWriter.writeAttribute("valueReference", QString::number(vr));
            mdWriter.writeAttribute("variability", "continuous");
            if(var.causality == ModelVariableCausality::Input) {
                mdWriter.writeAttribute("causality", "input");
            }
            else {
                mdWriter.writeAttribute("causality", "output");
            }
            mdWriter.writeAttribute("start", QString::number(var.startValue));  //! @todo Support start values
            ++vr;
            mdWriter.writeEndElement(); //Float64
        }

        QList<ParameterSpecification> parameterSpecs;
        getParameters(parameterSpecs, pSystem);

        for(auto &parSpec : parameterSpecs) {
            if(parSpec.type == "Real") {
                mdWriter.writeStartElement("Float64");
            }
            else if(parSpec.type == "Integer") {
                mdWriter.writeStartElement("Int32");
            }
            else if(parSpec.type == "Boolean") {
                mdWriter.writeStartElement("Boolean");
            }
            else if(parSpec.type == "String") {
                mdWriter.writeStartElement("String");
            }
            else {
                printErrorMessage("Unknown data type: "+parSpec.type);
                return false;   // Should never happen
            }
            mdWriter.writeAttribute("name", parSpec.name);
            mdWriter.writeAttribute("valueReference", QString::number(vr));
            mdWriter.writeAttribute("causality", "parameter");
            mdWriter.writeAttribute("variability", "fixed");
            mdWriter.writeAttribute("start", parSpec.init);
            mdWriter.writeAttribute("description", parSpec.description);
            mdWriter.writeAttribute("unit", parSpec.unit);
            mdWriter.writeEndElement(); //Float64/Int32/Boolean/String
            ++vr;
        }
        mdWriter.writeEndElement(); //ModelVariables
        mdWriter.writeEndElement(); //fmiModelDescription
        mdWriter.writeEndDocument();
        mdFile.close();
    }
    else {

        QString versionStr = QString::number(version, 'f', 1);
        QString dateAndTime = QDateTime::currentDateTime().toUTC().toString(Qt::ISODate);
        QString modelName = pSystem->getName().c_str();

        printMessage("Generating modelDescription.xml for FMI "+versionStr);

        //Write modelDescription.xml
        QDomDocument domDocument;
        QDomElement rootElement = domDocument.createElement("fmiModelDescription");
        rootElement.setAttribute("fmiVersion", versionStr);
        rootElement.setAttribute("modelName", modelName);
        if(version==1) {
            rootElement.setAttribute("modelIdentifier", modelName);
            rootElement.setAttribute("numberOfContinuousStates", "0");
        }
        else {
            rootElement.setAttribute("variableNamingConvention", "structured");
        }
        rootElement.setAttribute("guid", guid);
        rootElement.setAttribute("description", "");
        rootElement.setAttribute("generationTool", "HopsanGenerator");
        rootElement.setAttribute("generationDateAndTime", dateAndTime);
        rootElement.setAttribute("numberOfEventIndicators", "0");
        domDocument.appendChild(rootElement);

        if(version==2) {
            QDomElement coSimElement = domDocument.createElement("CoSimulation");
            coSimElement.setAttribute("modelIdentifier",modelName);
            coSimElement.setAttribute("canHandleVariableCommunicationStepSize", "true");
            rootElement.appendChild(coSimElement);
        }

        QDomElement varsElement = domDocument.createElement("ModelVariables");
        rootElement.appendChild(varsElement);

        QList<InterfacePortSpec> interfacePortSpecs;
        QStringList path = QStringList();
        getInterfaces(interfacePortSpecs, pSystem, path);

        size_t vr=0;
        nReals=0;
        nInputs=0;
        nOutputs=0;
        for(const auto &port : interfacePortSpecs) {
            for(const auto &var : port.vars) {
                QDomElement varElement = domDocument.createElement("ScalarVariable");
                varElement.setAttribute("name", port.component+"_"+port.port+"_"+var.dataName);
                varElement.setAttribute("valueReference", (unsigned int)vr);
                if(var.causality == InterfaceVarSpec::Input) {
                    varElement.setAttribute("causality", "input");
                    ++nInputs;
                }
                else {
                    if(version == 2) {
                        varElement.setAttribute("initial","exact");
                    }
                    varElement.setAttribute("causality", "output");
                    ++nOutputs;
                }
                ++nReals;
                varElement.setAttribute("description", "");

                QDomElement dataElement = domDocument.createElement("Real");    //We only support real data type for now
                dataElement.setAttribute("start", 0);   //! @todo Support start values
                if(version == 1) {
                    dataElement.setAttribute("fixed", "false");
                }
                varElement.appendChild(dataElement);

                varsElement.appendChild(varElement);
                ++vr;
            }
        }

        QList<ParameterSpecification> parameterSpecs;
        getParameters(parameterSpecs, pSystem);

        for(auto &parSpec : parameterSpecs) {
            QDomElement varElement = domDocument.createElement("ScalarVariable");
            varElement.setAttribute("name", parSpec.name);
            varElement.setAttribute("valueReference", (unsigned int)vr);
            if(version == 2) {
                varElement.setAttribute("causality", "parameter");
                varElement.setAttribute("initial","exact");
                varElement.setAttribute("variability", "fixed");
            }
            else if(version == 1) {
                varElement.setAttribute("causality", "input");
                varElement.setAttribute("variability", "parameter");
            }
            varElement.setAttribute("description", parSpec.description);
            QDomElement dataElement = domDocument.createElement(parSpec.type);
            dataElement.setAttribute("start", parSpec.init);   //! @todo Support start values
            if(!parSpec.unit.isEmpty()) {
                dataElement.setAttribute("unit", parSpec.unit);
            }
            if(version == 1) {
                dataElement.setAttribute("fixed", "false");
            }
            varElement.appendChild(dataElement);
            varsElement.appendChild(varElement);
            ++vr;
        }

        if(version == 1) {
            QDomElement implElement = domDocument.createElement("Implementation");
            QDomElement coSimElement = domDocument.createElement("CoSimulation_StandAlone");
            QDomElement capabilitiesElement = domDocument.createElement("Capabilities");
            capabilitiesElement.setAttribute("canHandleVariableCommunicationStepSize", "true");
            coSimElement.appendChild(capabilitiesElement);
            implElement.appendChild(coSimElement);
            rootElement.appendChild(implElement);
        }
        else {
            QDomElement structureElement = domDocument.createElement("ModelStructure");
            rootElement.appendChild(structureElement);
        }

        QDomNode xmlProcessingInstruction = domDocument.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
        domDocument.insertBefore(xmlProcessingInstruction, domDocument.firstChild());

        QFile modelDescriptionFile(savePath + "/modelDescription.xml");
        if(!modelDescriptionFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            printErrorMessage(QString("Failed to open %1 for writing.").arg(modelDescriptionFile.fileName()));
            return false;
        }
        QTextStream out(&modelDescriptionFile);
        domDocument.save(out, 4);
        modelDescriptionFile.close();
        return true;
    }
}


void HopsanFMIGenerator::replaceNameSpace(const QString &savePath, int version) const
{
    printMessage("Replacing namespace");

    int random = rand() % 1000000000;
    QString randomString = QString::number(random);
    QString nameSpace = "HopsanFMU"+randomString;
    QStringList before = QStringList() << "using namespace hopsan;" << "namespace hopsan " << "\nhopsan::" << "::hopsan::" << " hopsan::" << "*hopsan::" << "namespace hopsan{" << "(hopsan::" << "<hopsan::" << ",hopsan::";
    QStringList after = QStringList() << "using namespace "+nameSpace+";" << "namespace "+nameSpace+" " << "\n"+nameSpace+"::" << "::"+nameSpace+"::" << " "+nameSpace+"::" << "*"+nameSpace+"::" << "namespace "+nameSpace+"{" << "("+nameSpace+"::" << "<"+nameSpace+"::" << ","+nameSpace+"::";

    QStringList srcFiles = listHopsanCoreSourceFiles(savePath)+listInternalLibrarySourceFiles(savePath);
    for(const QString &file : srcFiles) {
        if(!replaceInFile(savePath+"/"+file, before, after))
            return;
    }
    QStringList includeFiles = listHopsanCoreIncludeFiles(savePath);
    for(const QString &file : includeFiles) {
        if(!replaceInFile(savePath+"/"+file, before, after))
            return;
    }
    QStringList componentFiles;
    findAllFilesInFolderAndSubFolders(savePath+"/componentLibraries/", "hpp", componentFiles);
    findAllFilesInFolderAndSubFolders(savePath+"/componentLibraries/", "h", componentFiles);
    for(const QString &file : componentFiles) {
        //qDebug() << "Replacing component file: " << file;
        if(!replaceInFile(file, before, after))
            return;
    }
    if(!replaceInFile(savePath+"/fmu"+QString::number(version)+"_model.c", before, after)) {
        return;
    }
}

bool HopsanFMIGenerator::compileAndLinkFMU(const QString &fmuBuildPath, const QString &fmuStagePath, const QString &modelName, const int version, bool x64) const
{
    const QString vStr = QString::number(version);
    const QString fmiLibDir=mHopsanRootPath+"/dependencies/fmilibrary";
    const QString fmi4cDir=mHopsanRootPath+"/dependencies/fmi4c/3rdparty/fmi";

    printMessage("------------------------------------------------------------------------");
    printMessage("Compiling FMU source code");
    printMessage("------------------------------------------------------------------------");

    bool compilationSuccessful=false;

    QString outputLibraryFile;
#ifdef _WIN32
    if(x64) {
        if(version == 3) {
            outputLibraryFile = QString("%1/binaries/x86_64-windows/%2.dll").arg(fmuStagePath).arg(modelName);
        }
        else {
            outputLibraryFile = QString("%1/binaries/win64/%2.dll").arg(fmuStagePath).arg(modelName);
        }
    } else {
        outputLibraryFile = QString("%1/binaries/win32/%2.dll").arg(fmuStagePath).arg(modelName);
    }
#elif __linux__
#ifdef __i386__
    outputLibraryFile = QString("%1/binaries/linux32/%2.so").arg(fmuStagePath).arg(modelName);
#elif __x86_64__
    outputLibraryFile = QString("%1/binaries/linux64/%2.so").arg(fmuStagePath).arg(modelName);
#endif
#endif
    //! @todo Add OSX support
    // Create output directory before compiling
    QDir systemRootDir("");
    systemRootDir.mkpath(QFileInfo(outputLibraryFile).absolutePath());

    printMessage("Generating Makefile");

    QFile makefile;
    makefile.setFileName(fmuBuildPath + "/Makefile");
    if(!makefile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open Makefile for writing.");
        return false;
    }

    //Write the compilation script file
    QTextStream makefileStream(&makefile);
    makefileStream << "CXX = g++\n";
    QString fpicFlag;
#ifndef _WIN32
    fpicFlag= "-fPIC";
#endif
    makefileStream << "CXXFLAGS = "+fpicFlag+" -c -std=c++14 -DHOPSAN_INTERNALDEFAULTCOMPONENTS -DHOPSAN_INTERNAL_EXTRACOMPONENTS -DHOPSANCORE_NOMULTITHREADING -DNOFMI4C\n";
#ifdef _WIN32
    makefileStream << "LFLAGS = "+fpicFlag+" -w -shared -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic -Wl,--rpath,'$$ORIGIN/.' -Wl,--rpath,'$$ORIGIN/../../resources'\n";
#else
    makefileStream << "LFLAGS = "+fpicFlag+" -w -shared -static-libgcc -Wl,--rpath,'$$ORIGIN/.' -Wl,--rpath,'$$ORIGIN/../../resources'\n";
#endif
    makefileStream << "INCLUDES = ";
    // Add HopsanCore (and necessary dependency) include paths
    for(const QString &includePath : getHopsanCoreIncludePaths()) {
        makefileStream << QString(" -I\"%1\"").arg(includePath);
    }
    for(const QString &includePath : mIncludePaths) {
        makefileStream << QString(" -I\"%1\"").arg(includePath);
    }
    makefileStream << " -I"+fmi4cDir+"\n";
    makefileStream << "OUTPUT = \""+outputLibraryFile+"\"\n\n";
    makefileStream << "SRC = fmu"+QString::number(version)+"_model.cpp";
    QStringList srcFiles = listHopsanCoreSourceFiles(fmuBuildPath) + listInternalLibrarySourceFiles(fmuBuildPath);
    for(const QString& srcFile : srcFiles) {
        makefileStream << " " << srcFile;
    }
    makefileStream << "\n\n";
    makefileStream << "VPATH := $(sort  $(dir $(SRC)))\n\n";
    makefileStream << "OBJ := $(patsubst %.cpp, %.o, $(notdir $(SRC)))\n";
    makefileStream << "OBJ := $(patsubst %.c, %.o, $(notdir $(OBJ)))\n\n";
    makefileStream << "all: 	$(OBJ)\n";
    makefileStream << "\t$(CXX) $(OBJ) $(LFLAGS) -o $(OUTPUT)\n\n";
    makefileStream << "%.o : %.cpp Makefile\n";
    makefileStream << "\t$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@\n\n";
    makefileStream << "%.o : %.c Makefile\n";
    makefileStream << "\t$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@\n\n";


    makefile.close();

    printMessage("Generating compilation script");

    QFile compileScriptFile;
    QString scriptExt;
#ifdef _WIN32
    scriptExt = "ps1";
#else
    scriptExt = "sh";
#endif
    compileScriptFile.setFileName(fmuBuildPath + "/compile."+scriptExt);
    if(!compileScriptFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        printErrorMessage("Failed to open compile."+scriptExt+" for writing.");
        return false;
    }

    //Write the compilation script file
    QTextStream compileScriptStream(&compileScriptFile);
#ifdef _WIN32
    compileScriptStream << "$env:Path = \""+mCompilerSelection.path+";$env:Path\"\n";
    compileScriptStream << "mingw32-make -j16 all\n";
#else
    compileScriptStream << "make -j16 all\n";
#endif
    compileScriptFile.close();

    printMessage("Calling compilation script");

#ifdef _WIN32
    compilationSuccessful = callProcess("cmd.exe", QStringList() << "/c" << "cd /d " + fmuBuildPath + " & Powershell.exe -executionpolicy remotesigned -File compile."+scriptExt);
#else
    compilationSuccessful = callProcess("/bin/sh", QStringList() << "compile.sh", fmuBuildPath);
#endif

    if (!compilationSuccessful) {
        printErrorMessage("Failed to compile exported FMU C++ Hopsan code.");
        return false;
    }

    return assertFilesExist("", QStringList() << outputLibraryFile);
}

bool HopsanFMIGenerator::compressFiles(const QString &fmuStagePath, const QString &modelName) const
{
    printMessage("Compressing files");

    QString fmuDestination = QString("../%1.fmu").arg(modelName);
    bool compressedOK = false;
#ifdef _WIN32
    QString program = mHopsanRootPath + "/dependencies/tools/7z/7za";
    QStringList arguments = QStringList() << "a" << "-tzip" << fmuDestination << "*";
    compressedOK = callProcess(program, arguments, fmuStagePath);
#elif __linux__
    QStringList arguments = QStringList() << "-r" << fmuDestination << ".";
    compressedOK = callProcess("zip", arguments, fmuStagePath);
#endif
    //! @todo Add OSX support

    if (!compressedOK) {
        printErrorMessage("Failed to compress FMU");
        return false;
    }

    return assertFilesExist(fmuStagePath, fmuDestination);
}


void HopsanFMIGenerator::getInterfaceInfo(QString typeName, QString compName,
                                          QStringList &inVars, QStringList &inComps, QStringList &inPorts, QList<int> &inDatatypes,
                                          QStringList &outVars, QStringList &outComps, QStringList &outPorts, QList<int> &outDatatypes,
                                          QList<QStringList> &tlmPorts)
{
    if(typeName == "SignalInputInterface") {
        inVars.append(compName.remove(' ').remove("-"));
        inComps.append(compName);
        inPorts.append("out");
        inDatatypes.append(0);
    }
    else if(typeName == "SignalOutputInterface") {
        outVars.append(compName.remove(' ').remove("-"));
        outComps.append(compName);
        outPorts.append("in");
        outDatatypes.append(0);
    }

    QString nodeType, cqType;
    QString portName = "P1";
    getNodeAndCqTypeFromInterfaceComponent(typeName, nodeType, cqType);

    if(cqType == "c") {
        QString name=compName;
        name.remove(' ').remove("-");

        GeneratorNodeInfo info(nodeType);

        tlmPorts.append(QStringList() << info.niceName+"q");
        for(const QString &var : info.qVariables) {
            outVars.append(name+"_"+var+"__");
            outComps.append(compName);
            outPorts.append(portName);
            outDatatypes.append(info.qVariableIds[info.qVariables.indexOf(var)]);
            tlmPorts.last() << var << name+"_"+var+"__";
        }
        for(const QString &var : info.cVariables) {
            inVars.append(name+"_"+var+"__");
            inComps.append(compName);
            inPorts.append("P1");
            inDatatypes.append(info.cVariableIds[info.cVariables.indexOf(var)]);
            tlmPorts.last() << var << name+"_"+var+"__";
        }
    }
    else if(cqType == "q")
    {
        QString name=compName.remove(' ').remove("-");

        GeneratorNodeInfo info(nodeType);

        tlmPorts.append(QStringList() << info.niceName+"c");
        for(const QString &var : info.qVariables) {
            inVars.append(name+"_"+var+"__");
            inComps.append(compName);
            inPorts.append("P1");
            inDatatypes.append(info.varIdx[info.qVariables.indexOf(var)]);
            tlmPorts.last() << var << name+"_"+var+"__";
        }
        for(const QString &var : info.cVariables) {
            outVars.append(name+"_"+var+"__");
            outComps.append(compName);
            outPorts.append("P1");
            outDatatypes.append(info.varIdx[info.qVariables.size()+info.cVariables.indexOf(var)]);
            tlmPorts.last() << var << name+"_"+var+"__";
        }
    }
}

