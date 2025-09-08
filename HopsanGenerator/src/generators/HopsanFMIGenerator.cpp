
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
#include "HopsanEssentials.h"
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

    if(version == 3) {
        dataStr.append("\n#define NUMDATAPTRS "+QString::number(vars.size()+pars.size()+1));
    }
    else {
        dataStr.append("\n#define NUMDATAPTRS "+QString::number(vars.size()+pars.size()));
    }
    dataStr.append("\n#define INITDATAPTRS ");
    int vr = 1; //Value reference 0 reserved for timestep
    if(version == 3) {
        vr = 2; //In FMI 3, 0 = time and 1 = timestep
    }
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
    if(version < 1 || version > 3) {
        printErrorMessage("Illegal FMI version: "+QString::number(version));
        return false;
    }

    QString hopsanStr = "Hopsan"+QString(hopsan::HopsanEssentials().getCoreVersion());
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

    if(version == 1) {
        mdWriter.writeStartElement("fmiModelDescription");
        mdWriter.writeAttribute("modelName", modelName);
        mdWriter.writeAttribute("modelIdentifier", modelName);
        mdWriter.writeAttribute("fmiVersion", "1.0");
        mdWriter.writeAttribute("generationTool", hopsanStr);
        mdWriter.writeAttribute("generationDateAndTime", dateAndTime);
        mdWriter.writeAttribute("variableNamingConvention", "flat");
        mdWriter.writeAttribute("guid", guid);
        mdWriter.writeAttribute("numberOfEventIndicators", "0");
        mdWriter.writeAttribute("numberOfContinuousStates", "0");

        mdWriter.writeStartElement("DefaultExperiment");
        mdWriter.writeAttribute("startTime", QString::number(pSystem->getTime()));
        mdWriter.writeEndElement(); //DefaultExperiment

        mdWriter.writeStartElement("ModelVariables");

        QList<ModelVariableSpecification> vars;
        QStringList systemHierarchy = QStringList();
        getModelVariables(pSystem, vars, systemHierarchy);

        mdWriter.writeStartElement("ScalarVariable");
        mdWriter.writeAttribute("name", "timestep");
        mdWriter.writeAttribute("valueReference", "0");
        mdWriter.writeAttribute("causality", "input");
        mdWriter.writeAttribute("variability", "parameter");
        mdWriter.writeAttribute("description", "Hopsan time step");
        mdWriter.writeStartElement("Real");
        mdWriter.writeAttribute("start", QString::number(pSystem->getDesiredTimeStep()));
        mdWriter.writeAttribute("unit", "s");
        mdWriter.writeEndElement(); //Real
        mdWriter.writeEndElement(); //ScalarVariable

        long vr = 1;    //vr=0 reserved for timestep
        for(const auto &var : vars) {
            mdWriter.writeStartElement("ScalarVariable");
            mdWriter.writeAttribute("name", var.getName());
            mdWriter.writeAttribute("valueReference", QString::number(vr));
            mdWriter.writeAttribute("causality", var.getCausalityStr());
            mdWriter.writeAttribute("variability", "continuous");
            mdWriter.writeStartElement("Real");
            mdWriter.writeAttribute("start", QString::number(var.startValue));
            mdWriter.writeEndElement(); //Real
            mdWriter.writeEndElement(); //ScalarVariable
            ++vr;
        }

        QList<ParameterSpecification> parameterSpecs;
        getParameters(parameterSpecs, pSystem);

        for(auto &parSpec : parameterSpecs) {
            mdWriter.writeStartElement("ScalarVariable");
            mdWriter.writeAttribute("name", parSpec.name);
            mdWriter.writeAttribute("valueReference", QString::number(vr));
            mdWriter.writeAttribute("causality", "input");
            mdWriter.writeAttribute("variability", "parameter");
            mdWriter.writeAttribute("description", parSpec.description);

            if(parSpec.type == "Real") {
                mdWriter.writeStartElement("Real");
            }
            else if(parSpec.type == "Integer") {
                mdWriter.writeStartElement("Integer");
            }
            else if(parSpec.type == "Boolean") {
                mdWriter.writeStartElement("Boolean");
            }
            else if(parSpec.type == "String") {
                mdWriter.writeStartElement("String");
            }

            mdWriter.writeAttribute("start", parSpec.init);

            mdWriter.writeAttribute("unit", parSpec.unit);
            mdWriter.writeEndElement(); //Float64/Int32/Boolean/String
            mdWriter.writeEndElement(); //ScalarVariable
            ++vr;
        }
        mdWriter.writeEndElement(); //ModelVariables

        mdWriter.writeStartElement("Implementation");
        mdWriter.writeStartElement("CoSimulation_StandAlone");
        mdWriter.writeStartElement("Capabilities");
        mdWriter.writeAttribute("canHandleVariableCommunicationStepSize", "true");
        mdWriter.writeEndElement(); //Capabilities
        mdWriter.writeEndElement(); //CoSimulation_StandAlone
        mdWriter.writeEndElement(); //Implementation

        mdWriter.writeEndElement(); //fmiModelDescription
    }
    else if(version == 2) {
        mdWriter.writeStartElement("fmiModelDescription");
        mdWriter.writeAttribute("modelName", modelName);
        mdWriter.writeAttribute("fmiVersion", "2.0");
        mdWriter.writeAttribute("generationTool", hopsanStr);
        mdWriter.writeAttribute("generationDateAndTime", dateAndTime);
        mdWriter.writeAttribute("variableNamingConvention", "flat");
        mdWriter.writeAttribute("guid", guid);
        mdWriter.writeAttribute("numberOfEventIndicators", "0");

        mdWriter.writeStartElement("CoSimulation");
        mdWriter.writeAttribute("modelIdentifier", modelName);
        mdWriter.writeAttribute("canHandleVariableCommunicationStepSize", "true");
        mdWriter.writeEndElement(); //CoSimulation


        QList<ModelVariableSpecification> vars;
        QStringList systemHierarchy = QStringList();
        getModelVariables(pSystem, vars, systemHierarchy);

        QList<ParameterSpecification> parameterSpecs;
        getParameters(parameterSpecs, pSystem);

        mdWriter.writeStartElement("UnitDefinitions");
        mdWriter.writeStartElement("Unit");
        mdWriter.writeAttribute("name", "s");
        QStringList usedUnits = QStringList() << "s";
        mdWriter.writeEndElement(); //Unit
        for(const auto &var : qAsConst(vars)) {
            if(var.unit != "" && !usedUnits.contains(var.unit)) {
                usedUnits << var.unit;
                mdWriter.writeStartElement("Unit");
                mdWriter.writeAttribute("name", var.unit);
                mdWriter.writeEndElement(); //Unit
            }
        }
        for(const auto &par : qAsConst(parameterSpecs)) {
            if(par.unit != "" && !usedUnits.contains(par.unit)) {
                usedUnits << par.unit;
                mdWriter.writeStartElement("Unit");
                mdWriter.writeAttribute("name", par.unit);
                mdWriter.writeEndElement(); //Unit
            }
        }
        mdWriter.writeEndElement(); //UnitDefinitions

        mdWriter.writeStartElement("DefaultExperiment");
        mdWriter.writeAttribute("startTime", QString::number(pSystem->getTime()));
        mdWriter.writeEndElement(); //DefaultExperiment

        mdWriter.writeStartElement("ModelVariables");

        mdWriter.writeStartElement("ScalarVariable");
        mdWriter.writeAttribute("name", "timestep");
        mdWriter.writeAttribute("valueReference", "0");
        mdWriter.writeAttribute("causality", "parameter");
        mdWriter.writeAttribute("variability", "fixed");
        mdWriter.writeAttribute("description", "Hopsan time step");
        mdWriter.writeStartElement("Real");
        mdWriter.writeAttribute("start", QString::number(pSystem->getDesiredTimeStep()));
        mdWriter.writeAttribute("unit", "s");
        mdWriter.writeEndElement(); //Real
        mdWriter.writeEndElement(); //ScalarVariable

        long vr = 1;    //vr = 0 reserved for timestep
        for(const auto &var : vars) {
            mdWriter.writeStartElement("ScalarVariable");
            mdWriter.writeAttribute("name", var.getName());
            mdWriter.writeAttribute("valueReference", QString::number(vr));
            mdWriter.writeAttribute("causality", var.getCausalityStr());
            mdWriter.writeAttribute("variability", "continuous");
            mdWriter.writeStartElement("Real");
            if(!var.unit.isEmpty()) {
                mdWriter.writeAttribute("unit", var.unit);
            }
            if(var.causality != Output) {
                mdWriter.writeAttribute("start", QString::number(var.startValue));
            }
            mdWriter.writeEndElement(); //Real
            mdWriter.writeEndElement(); //ScalarVariable
            ++vr;
        }

        for(auto &parSpec : parameterSpecs) {
            mdWriter.writeStartElement("ScalarVariable");
            mdWriter.writeAttribute("name", parSpec.name);
            mdWriter.writeAttribute("valueReference", QString::number(vr));
            mdWriter.writeAttribute("causality", "parameter");
            mdWriter.writeAttribute("variability", "fixed");
            mdWriter.writeAttribute("description", parSpec.description);

            if(parSpec.type == "Real") {
                mdWriter.writeStartElement("Real");
            }
            else if(parSpec.type == "Integer") {
                mdWriter.writeStartElement("Integer");
            }
            else if(parSpec.type == "Boolean") {
                mdWriter.writeStartElement("Boolean");
            }
            else if(parSpec.type == "String") {
                mdWriter.writeStartElement("String");
            }

            mdWriter.writeAttribute("start", parSpec.init);
            if(!parSpec.unit.isEmpty()) {
                mdWriter.writeAttribute("unit", parSpec.unit);
            }
            mdWriter.writeEndElement(); //Float64/Int32/Boolean/String
            mdWriter.writeEndElement(); //ScalarVariable
            ++vr;
        }
        mdWriter.writeEndElement(); //ModelVariables

        mdWriter.writeStartElement("ModelStructure");
        mdWriter.writeStartElement("Outputs");
        vr = 1;    //vr = 0 reserved for timestep
        for(const auto &var : qAsConst(vars)) {
            if(var.causality == Output) {
                mdWriter.writeStartElement("Unknown");
                mdWriter.writeAttribute("index", QString::number(vr+1));    //Index counting starts at 1, not 0
                mdWriter.writeAttribute("dependencies", "");
                mdWriter.writeEndElement();
            }
            ++vr;
        }
        mdWriter.writeEndElement(); //Outputs
        mdWriter.writeStartElement("InitialUnknowns");
        vr = 1;    //vr = 0 reserved for timestep
        for(const auto &var : qAsConst(vars)) {
            if(var.causality == Output) {
                mdWriter.writeStartElement("Unknown");
                mdWriter.writeAttribute("index", QString::number(vr+1));    //Index counting starts at 1, not 0
                mdWriter.writeAttribute("dependencies", "");
                mdWriter.writeEndElement();
            }
            ++vr;
        }
        mdWriter.writeEndElement(); //InitialUnknowns
        mdWriter.writeEndElement(); //ModelStructure

        mdWriter.writeEndElement(); //fmiModelDescription
    }
    else { //version == 3
        mdWriter.writeStartElement("fmiModelDescription");
        mdWriter.writeAttribute("modelName", modelName);
        mdWriter.writeAttribute("fmiVersion", "3.0-rc.1");
        mdWriter.writeAttribute("generationTool", hopsanStr);
        mdWriter.writeAttribute("generationDateAndTime", dateAndTime);
        mdWriter.writeAttribute("variableNamingConvention", "flat");
        mdWriter.writeAttribute("instantiationToken", guid);

        mdWriter.writeStartElement("CoSimulation");
        mdWriter.writeAttribute("modelIdentifier", modelName);
        mdWriter.writeAttribute("providesIntermediateUpdate", "false");
        mdWriter.writeAttribute("canHandleVariableCommunicationStepSize", "true");
        mdWriter.writeAttribute("hasEventMode", "false");
        mdWriter.writeEndElement(); //CoSimulation

        QList<ModelVariableSpecification> vars;
        QStringList systemHierarchy = QStringList();
        getModelVariables(pSystem, vars, systemHierarchy);

        QList<ParameterSpecification> parameterSpecs;
        getParameters(parameterSpecs, pSystem);

        mdWriter.writeStartElement("UnitDefinitions");
        mdWriter.writeStartElement("Unit");
        mdWriter.writeAttribute("name", "s");
        QStringList usedUnits = QStringList() << "s";
        mdWriter.writeEndElement(); //Unit
        for(const auto &var : qAsConst(vars)) {
            if(var.unit != "" && !usedUnits.contains(var.unit)) {
                usedUnits << var.unit;
                mdWriter.writeStartElement("Unit");
                mdWriter.writeAttribute("name", var.unit);
                mdWriter.writeEndElement(); //Unit
            }
        }
        for(const auto &par : qAsConst(parameterSpecs)) {
            if(par.unit != "" && !usedUnits.contains(par.unit)) {
                usedUnits << par.unit;
                mdWriter.writeStartElement("Unit");
                mdWriter.writeAttribute("name", par.unit);
                mdWriter.writeEndElement(); //Unit
            }
        }
        mdWriter.writeEndElement(); //UnitDefinitions

        mdWriter.writeStartElement("DefaultExperiment");
        mdWriter.writeAttribute("startTime", QString::number(pSystem->getTime()));
        mdWriter.writeAttribute("stepSize", QString::number(pSystem->getDesiredTimeStep()));
        mdWriter.writeEndElement(); //DefaultExperiment

        mdWriter.writeStartElement("ModelVariables");

        mdWriter.writeStartElement("Float64");
        mdWriter.writeAttribute("name", "time");
        mdWriter.writeAttribute("valueReference", "0");
        mdWriter.writeAttribute("causality", "independent");
        mdWriter.writeAttribute("variability", "continuous");
        mdWriter.writeAttribute("description", "Simulation time");
        mdWriter.writeAttribute("unit", "s");
        mdWriter.writeEndElement(); //Float64

        mdWriter.writeStartElement("Float64");
        mdWriter.writeAttribute("name", "timestep");
        mdWriter.writeAttribute("valueReference", "1");
        mdWriter.writeAttribute("causality", "parameter");
        mdWriter.writeAttribute("variability", "fixed");
        mdWriter.writeAttribute("description", "Hopsan time step");
        mdWriter.writeAttribute("start", QString::number(pSystem->getDesiredTimeStep()));
        mdWriter.writeAttribute("unit", "s");
        mdWriter.writeEndElement(); //Float64

        nReals=0;
        nInputs=0;
        nOutputs=0;
        long vr = 2;    //! vr = 0 and vr = 1 are reserved for time and timestep
        for(const auto &var : qAsConst(vars)) {
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
                mdWriter.writeAttribute("start", QString::number(var.startValue));
            }
            else {
                mdWriter.writeAttribute("causality", "output");
            }
            if(!var.unit.isEmpty()) {
                mdWriter.writeAttribute("unit", var.unit);
            }
            ++vr;
            mdWriter.writeEndElement(); //Float64
        }

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
            if(!parSpec.unit.isEmpty()) {
                mdWriter.writeAttribute("unit", parSpec.unit);
            }
            mdWriter.writeEndElement(); //Float64/Int32/Boolean/String
            ++vr;
        }
        mdWriter.writeEndElement(); //ModelVariables

        mdWriter.writeStartElement("ModelStructure");
        vr = 2;    //vr = 0 and vr = 1 are reserved for time and timestep
        for(const auto &var : qAsConst(vars)) {
            if(var.causality == Output) {
                mdWriter.writeStartElement("Output");
                mdWriter.writeAttribute("valueReference", QString::number(vr));
                mdWriter.writeAttribute("dependencies", "");
                mdWriter.writeEndElement(); //Output
            }
            ++vr;
        }
        vr = 2;   //vr = 0 and vr = 1 are reserved for time and timestep
        for(const auto &var : qAsConst(vars)) {
            if(var.causality == Output) {
                mdWriter.writeStartElement("InitialUnknown");
                mdWriter.writeAttribute("valueReference", QString::number(vr));
                mdWriter.writeAttribute("dependencies", "");
                mdWriter.writeEndElement();
            }
            ++vr;
        }
        mdWriter.writeEndElement(); //ModelStructure

        mdWriter.writeEndElement(); //fmiModelDescription
    }
    mdWriter.writeEndDocument();
    mdFile.close();

    return true;
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
    const QString fmiLibDir="\""+mHopsanRootPath+"/dependencies/fmilibrary\"";
    const QString fmi4cIncludeDir="\""+mHopsanRootPath+"/dependencies/fmi4c/include\"";

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
    makefileStream << "CXXFLAGS = "+fpicFlag+" -c -std=c++14 -DHOPSAN_INTERNALDEFAULTCOMPONENTS -DHOPSAN_INTERNAL_EXTRACOMPONENTS -DHOPSANCORE_NOMULTITHREADING\n";
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
    makefileStream << " -I"+fmi4cIncludeDir+"\n";
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

