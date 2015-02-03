#include "generators/HopsanFMIGenerator.h"
#include "GeneratorUtilities.h"
#include "ComponentSystem.h"
#include <QApplication>
#include <cassert>
#include <QProcess>
#include <QUuid>

//#include "../ThirdParty/FMILibrary-2.0.1/Config.cmake/fmilib.h"

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

using namespace hopsan;



HopsanFMIGenerator::HopsanFMIGenerator(QString coreIncludePath, QString binPath, QString gccPath, bool showDialog)
    : HopsanGenerator(coreIncludePath, binPath, gccPath, showDialog)
{
}



//! @brief Generates a Hopsan component from a Functional Mockup Unit (.FMU) file using the FMI standard (1.0 or 2.0)
//! @param rPath Reference to string containing the path to .FMU file
//! @param rTargetPath Reference to string containing the path where to generate the component
//! @param rTypeName Reference to string used to return the type name of the generated component
//! @param rHppPath Reference to string used to return the path to the generated .HPP file
bool HopsanFMIGenerator::generateFromFmu(QString &rPath, QString &rTargetPath, QString &rTypeName, QString &rHppPath)
{
    //----------------------------------------//
    printMessage("Initializing FMU import");
    printMessage("path = "+rPath);
    printMessage("targetPath = "+rTargetPath);
    //----------------------------------------//

    QByteArray pathArray = rPath.toLocal8Bit();
    const char* FMUPath = pathArray.data();
    rTargetPath = rTargetPath+QFileInfo(rPath).baseName();
    if(!QFileInfo(rTargetPath).exists())
    {
        QDir().mkpath(rTargetPath);
    }
    QByteArray targetArray = rTargetPath.toLocal8Bit();
    const char* tmpPath = targetArray.data();
    jm_callbacks callbacks;
    fmi_import_context_t* context;
    fmi_version_enu_t version;

    callbacks.malloc = malloc;
    callbacks.calloc = calloc;
    callbacks.realloc = realloc;
    callbacks.free = free;
    callbacks.logger = hopsanLogger;
    callbacks.log_level = jm_log_level_debug;
    callbacks.context = 0;

    context = fmi_import_allocate_context(&callbacks);

    version = fmi_import_get_fmi_version(context, FMUPath, tmpPath);

    if(version == fmi_version_1_enu)
    {
        printMessage("FMU is of version 1.0");
        return generateFromFmu1(rPath, rTargetPath, rTypeName, rHppPath, callbacks, context);
    }
    else if(version == fmi_version_2_0_enu)
    {
        printMessage("FMU is of version 2.0");
        return generateFromFmu2(rPath, rTargetPath, rTypeName, rHppPath, callbacks, context);
    }
    else if(version == fmi_version_unknown_enu)
    {
        printErrorMessage("Last JM error: "+QString(jm_get_last_error(&callbacks))+"\n");
        printErrorMessage("FMU version is unknown.");
        return false;
    }
    else if(version == fmi_version_unsupported_enu)
    {
        printErrorMessage("Last JM error: "+QString(jm_get_last_error(&callbacks))+"\n");
        printErrorMessage("FMU version is unsupported.");
        return false;
    }

    return false;   //Should never reach this
}


//! @brief Generates a Hopsan component from an FMU using FMI 1.0
//! @param rPath Reference to a string with the path to the .FMU file
//! @param rTargetPath Reference to a string with the path where to export the new component
//! @param rHppPath Reference to string used to return the actual path to the generated .HPP file
//! @param callbacks Callbacks struct used by FMILibrary
//! @param context Context struct used by FMILibrary
bool HopsanFMIGenerator::generateFromFmu1(QString &rPath, QString &rTargetPath, QString &rTypeName, QString &rHppPath, jm_callbacks &callbacks, fmi_import_context_t* context)
{
    fmi1_callback_functions_t callBackFunctions;
    jm_status_enu_t status;

    //-----------------------------------------//
    printMessage("Reading modelDescription.xml");
    //-----------------------------------------//

    QByteArray targetArray = rTargetPath.toLocal8Bit();
    const char* tmpPath = targetArray.data();

    fmi1_import_t* fmu = fmi1_import_parse_xml(context, tmpPath);

    if(!fmu)
    {
        printErrorMessage("Last JM error: "+QString(jm_get_last_error(&callbacks)));
        printErrorMessage("Error parsing XML, exiting");
        return false;
    }

    if(fmi1_import_get_fmu_kind(fmu) == fmi1_fmu_kind_enu_me)
    {
        printErrorMessage("Last JM error: "+QString(jm_get_last_error(&callbacks)));
        printErrorMessage("Only FMUs for co-simulation are supported by this code.");
        return false;
    }

    callBackFunctions.logger = fmi1_log_forwarding;
    callBackFunctions.allocateMemory = calloc;
    callBackFunctions.freeMemory = free;

    status = fmi1_import_create_dllfmu(fmu, callBackFunctions, 1);
    if (status == jm_status_error)
    {
        printErrorMessage("Last JM error: "+QString(jm_get_last_error(&callbacks))+"\n");
        printErrorMessage(QString("Could not create the DLL loading mechanism (C-API) (error: %1).\n").arg(fmi1_import_get_last_error(fmu)));
        return false;
    }

    //Declare lists for parameters, input variables and output variables
    QStringList parNames, parVars, parRefs;
    QStringList inputNames, inputVars, inputRefs;
    QStringList outputNames, outputVars, outputRefs;

    //Loop through variables in FMU and generate the lists
    fmi1_import_variable_list_t *pVarList = fmi1_import_get_variable_list(fmu);
    for(size_t i=0; i<fmi1_import_get_variable_list_size(pVarList); ++i)
    {
        fmi1_import_variable_t *pVar = fmi1_import_get_variable(pVarList, i);
        QString name = fmi1_import_get_variable_name(pVar);
        fmi1_causality_enu_t causality = fmi1_import_get_causality(pVar);
        fmi1_variability_enu_t variability = fmi1_import_get_variability(pVar);
        fmi1_base_type_enu_t type = fmi1_import_get_variable_base_type(pVar);
        fmi1_value_reference_t vr = fmi1_import_get_variable_vr(pVar);
        name = toVarName(name);
        if(causality == fmi1_causality_enu_input && variability == fmi1_variability_enu_parameter)
        {
            parNames.append(name);
            parVars.append("m"+name);
            parRefs.append(QString::number(vr));
        }
        if(causality == fmi1_causality_enu_input && variability != fmi1_variability_enu_parameter)
        {
            inputNames.append(name);
            inputVars.append("mp"+name);
            inputRefs.append(QString::number(vr));
        }
        if(causality == fmi1_causality_enu_output)
        {

            outputNames.append(name);
            outputVars.append("mp"+name);
            outputRefs.append(QString::number(vr));
        }
    }

    //Get name of FMU
    QString fmuName = fmi1_import_get_model_name(fmu);
    fmuName = toVarName(fmuName);//.remove(QRegExp(QString::fromUtf8("[-`~!@#$%^&*()_—+=|:;<>«»,.?/{}\'\"\\\[\\\]\\\\]")));

    //--------------------------------------------//
    printMessage("Creating " + fmuName + ".hpp...");
    //--------------------------------------------//

    //Create <fmuname>.hpp
    QString hppPath = rTargetPath + "/" + fmuName + "/component_code";
    if(!QFileInfo(hppPath).exists())
    {
        QDir().mkpath(hppPath);
    }
    QFile fmuComponentHppFile;
    fmuComponentHppFile.setFileName(hppPath+"/"+fmuName+".hpp");
    if(!fmuComponentHppFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Import of FMU failed: Could not open "+fmuName+".hpp for writing.");
        removeDir(rTargetPath);
        return false;
    }

    //-------------------------------------------//
    printMessage("Writing " + fmuName + ".hpp...");
    //-------------------------------------------//

    //Generate HPP file
    QFile fmuComponentTemplateFile;
    fmuComponentTemplateFile.setFileName(":templates/fmuComponentTemplate.hpp");
    assert(fmuComponentTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QString fmuComponentCode;
    QTextStream t2(&fmuComponentTemplateFile);
    fmuComponentCode = t2.readAll();
    fmuComponentTemplateFile.close();
    if(fmuComponentCode.isEmpty())
    {
        printErrorMessage("Unable to generate code for "+fmuName+".hpp.");
        removeDir(rTargetPath);
        return false;
    }

    QString headerGuard = fmuName.toUpper()+"_HPP_INCLUDED";
    QString className = "FMU_"+fmuName;

    QString localVars;
    if(!parVars.isEmpty())
    {
        localVars.append("double ");
        foreach(const QString &varName, parVars)
        {
            localVars.append(varName+", ");
        }
        if(localVars.endsWith(", "))
        {
            localVars.chop(2);
        }
        localVars.append(";\n");
    }
    if(!inputVars.isEmpty()  || !outputVars.isEmpty())
    {
        localVars.append("double ");
        foreach(const QString &varName, inputVars)
        {
            localVars.append("*"+varName+", ");
        }
        foreach(const QString &varName, outputVars)
        {
            localVars.append("*"+varName+", ");
        }
        if(localVars.endsWith(", "))
        {
            localVars.chop(2);
        }
        localVars.append(";\n");
    }

    QString addConstants;
    for(int i=0; i<parNames.size(); ++i)
    {
        if(i!=0)
        {
            addConstants.append("        ");
        }
        addConstants.append("addConstant(\""+parNames.at(i)+"\", \"\", \"\", 0, "+parVars.at(i)+");\n");
    }

    QString addInputs;
    for(int i=0; i<inputNames.size(); ++i)
    {
        if(i!=0)
        {
            addInputs.append("        ");
        }
        addInputs.append("addInputVariable(\""+inputNames.at(i)+"\", \"\", \"\", 0, &"+inputVars.at(i)+");\n");
    }

    QString addOutputs;
    for(int i=0; i<outputNames.size(); ++i)
    {
        if(i!=0)
        {
            addOutputs.append("        ");
        }
        addOutputs.append("addOutputVariable(\""+outputNames.at(i)+"\", \"\", \"\", 0, &"+outputVars.at(i)+");\n");
    }

    QString setPars;
    QString temp = extractTaggedSection(fmuComponentCode, "setpars");
    for(int i=0; i<parNames.size(); ++i)
    {
        QString tempVar = temp;
        tempVar.replace("<<<vr>>>", parRefs.at(i));
        tempVar.replace("<<<var>>>", parVars.at(i));
        setPars.append(tempVar+"\n");
    }

    QString readVars;
    temp = extractTaggedSection(fmuComponentCode, "readvars");
    for(int i=0; i<inputNames.size(); ++i)
    {
        QString tempVar = temp;
        tempVar.replace("<<<vr>>>", inputRefs.at(i));
        tempVar.replace("<<<var>>>", inputVars.at(i));
        readVars.append(tempVar+"\n");
    }

    QString writeVars;
    temp = extractTaggedSection(fmuComponentCode, "writevars");
    for(int i=0; i<outputNames.size(); ++i)
    {
        QString tempVar = temp;
        tempVar.replace("<<<vr>>>", outputRefs.at(i));
        tempVar.replace("<<<var>>>", outputVars.at(i));
        writeVars.append(tempVar+"\n");
    }

    fmuComponentCode.replace("<<<headerguard>>>", headerGuard);
    fmuComponentCode.replace("<<<className>>>", className);
    fmuComponentCode.replace("<<<localvars>>>", localVars);
    fmuComponentCode.replace("<<<addconstants>>>", addConstants);
    fmuComponentCode.replace("<<<addinputs>>>", addInputs);
    fmuComponentCode.replace("<<<addoutputs>>>", addOutputs);
    fmuComponentCode.replace("<<<fmupath>>>", rPath);
    QDir().mkpath(rTargetPath+"/temp");
    fmuComponentCode.replace("<<<temppath>>>", rTargetPath+"/temp/");
    replaceTaggedSection(fmuComponentCode, "setpars", setPars);
    replaceTaggedSection(fmuComponentCode, "readvars", readVars);
    replaceTaggedSection(fmuComponentCode, "writevars", writeVars);

    QTextStream fmuComponentHppStream(&fmuComponentHppFile);
    fmuComponentHppStream << fmuComponentCode;
    fmuComponentHppFile.close();


    //-------------------------------------------//
    printMessage("Writing " + fmuName + ".xml...");
    //-------------------------------------------//


    ComponentAppearanceSpecification cafSpec("FMU_"+fmuName);
    cafSpec.mSourceCode = "component_code/"+fmuName+".hpp";
    cafSpec.mRecompilable = true;
    cafSpec.mUserIconPath = "fmucomponent.svg";


    //These 4 variables are used for input/output port positioning
    double inputPosStep=1.0/(inputNames.size()+1.0);
    double outputPosStep=1.0/(outputNames.size()+1.0);
    double inputPos=0;
    double outputPos=0;

    for(int i=0; i<inputNames.size(); ++i)
    {
        inputPos += inputPosStep;
        cafSpec.addPort(inputNames.at(i), 0.0, inputPos, 180);
    }
    for(int i=0; i<outputNames.size(); ++i)
    {
        outputPos += outputPosStep;
        cafSpec.addPort(outputNames.at(i), 1.0, outputPos, 0.0);
    }

    QString cafPath = rTargetPath + "/" + fmuName + "/" + fmuName + ".xml";
    if(!generateCafFile(cafPath, cafSpec))
    {
        printErrorMessage("Generation of component appearance file (XML) failed.");
        removeDir(rTargetPath);
        return false;
    }

    rTypeName = fmuName;
    rHppPath = QFileInfo(fmuComponentHppFile).absoluteFilePath();

    return true;

}


//! @brief Generates a Hopsan component from an FMU using FMI 2.0
//! @param rPath Reference to a string with the path to the .FMU file
//! @param rTargetPath Reference to a string with the path where to export the new component
//! @param rHppPath Reference to string used to return the actual path to the generated .HPP file
//! @param callbacks Callbacks struct used by FMILibrary
//! @param context Context struct used by FMILibrary
bool HopsanFMIGenerator::generateFromFmu2(QString &rPath, QString &rTargetPath, QString &rTypeName, QString &rHppPath, jm_callbacks &callbacks, fmi_import_context_t* context)
{
    fmi2_callback_functions_t callBackFunctions;
    jm_status_enu_t status;

    //-----------------------------------------//
    printMessage("Reading modelDescription.xml");
    //-----------------------------------------//

    QByteArray targetArray = rTargetPath.toLocal8Bit();
    const char* tmpPath = targetArray.data();

    fmi2_import_t* fmu = fmi2_import_parse_xml(context, tmpPath, 0);

    if(!fmu)
    {
        printErrorMessage("Last JM error: "+QString(jm_get_last_error(&callbacks))+"\n");
        printErrorMessage("Error parsing XML, exiting\n");
        return false;
    }

//    if(fmi2_import_get_fmu_kind(fmu) == fmi2_fmu_kind_me)
//    {
//        printErrorMessage("Last JM error: "+QString(jm_get_last_error(&callbacks)));
//        printErrorMessage("Only FMUs for co-simulation are supported by this code");
//        return false;
//    }

    fmi2_fmu_kind_enu_t fmuKind = fmi2_import_get_fmu_kind(fmu);

    callBackFunctions.logger = fmi2_log_forwarding;
    callBackFunctions.allocateMemory = calloc;
    callBackFunctions.freeMemory = free;
    callBackFunctions.componentEnvironment = fmu;

    status = fmi2_import_create_dllfmu(fmu, fmuKind, &callBackFunctions);
    if (status == jm_status_error)
    {
        printErrorMessage("Last JM error: "+QString(jm_get_last_error(&callbacks))+"\n");
        printErrorMessage(QString("Could not create the DLL loading mechanism(C-API) (error: %1).\n").arg(fmi2_import_get_last_error(fmu)));
        return false;
    }

    //Declare lists for parameters, input variables and output variables
    QStringList parNames, parVars, parRefs;
    QStringList inputNames, inputVars, inputRefs;
    QStringList outputNames, outputVars, outputRefs;

    //Loop through variables in FMU and generate the lists
    fmi2_import_variable_list_t *pVarList = fmi2_import_get_variable_list(fmu,0);
    for(size_t i=0; i<fmi2_import_get_variable_list_size(pVarList); ++i)
    {
        fmi2_import_variable_t *pVar = fmi2_import_get_variable(pVarList, i);
        QString name = fmi2_import_get_variable_name(pVar);
        fmi2_causality_enu_t causality = fmi2_import_get_causality(pVar);
        fmi2_value_reference_t vr = fmi2_import_get_variable_vr(pVar);
        name = toVarName(name);
        if(causality == fmi2_causality_enu_parameter)
        {
            parNames.append(name);
            parVars.append("m"+name);
            parRefs.append(QString::number(vr));
        }
        if(causality == fmi2_causality_enu_input)
        {
            inputNames.append(name);
            inputVars.append("mp"+name);
            inputRefs.append(QString::number(vr));
        }
        if(causality == fmi2_causality_enu_output)
        {

            outputNames.append(name);
            outputVars.append("mp"+name);
            outputRefs.append(QString::number(vr));
        }
    }

    //Get name of FMU
    QString fmuName = fmi2_import_get_model_name(fmu);
    fmuName = toVarName(fmuName);//.remove(QRegExp(QString::fromUtf8("[-`~!@#$%^&*()_—+=|:;<>«»,.?/{}\'\"\\\[\\\]\\\\]")));



    QStringList portTypes, portNames, portVars;
    QList<QStringList> portVarNames, portVarVars, portVarRefs;

    QString tlmFileName = fmuName+"_TLM.xml";
    QFile file(rTargetPath+"/"+tlmFileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QDomDocument domDocument;
        QString errorStr;
        int errorLine, errorColumn;
        if (!domDocument.setContent(&file, false, &errorStr, &errorLine, &errorColumn))
        {
            QString lineStr = QString::number(errorLine);
            QString colStr = QString::number(errorColumn);
            printErrorMessage(tlmFileName+": Parse error at line "+lineStr+", column "+colStr+": "+errorStr);
            printMessage(tlmFileName+" ignored.");
        }
        else
        {
            QDomElement rootElement = domDocument.documentElement();
            if (rootElement.tagName() != "fmutlm")
            {
                printErrorMessage("Wrong root tag in "+tlmFileName+": "+rootElement.tagName());
                printMessage(tlmFileName+" ignored");
            }
            else
            {
                QDomElement portElement = rootElement.firstChildElement("tlmport");
                int idx = 1;
                while(!portElement.isNull())
                {
                    QString portType = portElement.attribute("type");
                    if(portType == "mechanicq")
                    {
                        portTypes.append(portType);
                        portNames.append("P"+QString::number(idx));
                        portVars.append("mpP"+QString::number(idx));

                        portVarNames.append(QStringList());
                        portVarVars.append(QStringList());
                        portVarRefs.append(QStringList());

                        QStringList outputs = QStringList() << "f" << "x" << "v" << "me";
                        QStringList inputs = QStringList() << "c" << "Zc";

                        QString name;
                        foreach(const QString &output, outputs)
                        {
                            name = portElement.firstChildElement(output).text();
                            portVarNames.last().append(name);
                            portVarVars.last().append(portVars.last()+"_"+name);
                            portVarRefs.last().append(outputRefs.at(outputNames.indexOf(name)));

                            outputVars.removeAt(outputNames.indexOf(name));
                            outputRefs.removeAt(outputNames.indexOf(name));
                            outputNames.removeAll(name);

                        }
                        foreach(const QString &input, inputs)
                        {
                            name = portElement.firstChildElement(input).text();
                            portVarNames.last().append(name);
                            portVarVars.last().append(portVars.last()+"_"+name);
                            portVarRefs.last().append(inputRefs.at(inputNames.indexOf(name)));

                            inputVars.removeAt(inputNames.indexOf(name));
                            inputRefs.removeAt(inputNames.indexOf(name));
                            inputNames.removeAll(name);
                        }

                        ++idx;
                    }
                    else if(portType == "hydraulicq")
                    {
                        portTypes.append(portType);
                        portNames.append("P"+QString::number(idx));
                        portVars.append("mpP"+QString::number(idx));

                        portVarNames.append(QStringList());
                        portVarVars.append(QStringList());
                        portVarRefs.append(QStringList());

                        QStringList outputs = QStringList() << "p" << "q";
                        QStringList inputs = QStringList() << "c" << "Zc";

                        QString name;
                        foreach(const QString &output, outputs)
                        {
                            name = portElement.firstChildElement(output).text();
                            portVarNames.last().append(name);
                            portVarVars.last().append(portVars.last()+"_"+name);
                            portVarRefs.last().append(outputRefs.at(outputNames.indexOf(name)));

                            outputVars.removeAt(outputNames.indexOf(name));
                            outputRefs.removeAt(outputNames.indexOf(name));
                            outputNames.removeAll(name);

                        }
                        foreach(const QString &input, inputs)
                        {
                            name = portElement.firstChildElement(input).text();
                            portVarNames.last().append(name);
                            portVarVars.last().append(portVars.last()+"_"+name);
                            portVarRefs.last().append(inputRefs.at(inputNames.indexOf(name)));

                            inputVars.removeAt(inputNames.indexOf(name));
                            inputRefs.removeAt(inputNames.indexOf(name));
                            inputNames.removeAll(name);
                        }

                        ++idx;
                    }
                    else
                    {
                        printErrorMessage("Unknown port type: "+portType+", ignored.");
                    }
                    portElement = portElement.nextSiblingElement("tlmport");
                }
            }
        }
        file.close();
    }

    //--------------------------------------------//
    printMessage("Creating " + fmuName + ".hpp...");
    //--------------------------------------------//

    //Create <fmuname>.hpp
    QString hppPath = rTargetPath + "/" + fmuName + "/component_code";
    if(!QFileInfo(hppPath).exists())
    {
        QDir().mkpath(hppPath);
    }
    QFile fmuComponentHppFile;
    fmuComponentHppFile.setFileName(hppPath+"/"+fmuName+".hpp");
    if(!fmuComponentHppFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Import of FMU failed: Could not open "+fmuName+".hpp for writing.");
        removeDir(rTargetPath);
        return false;
    }

    //-------------------------------------------//
    printMessage("Writing " + fmuName + ".hpp...");
    //-------------------------------------------//

    //Generate HPP file
    QFile fmuComponentTemplateFile;
    if(fmuKind == fmi2_fmu_kind_me)
    {
        fmuComponentTemplateFile.setFileName(":templates/fmi2MeComponentTemplate.hpp");
    }
    else
    {
        fmuComponentTemplateFile.setFileName(":templates/fmi2ComponentTemplate.hpp");
    }
    assert(fmuComponentTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QString fmuComponentCode;
    QTextStream t2(&fmuComponentTemplateFile);
    fmuComponentCode = t2.readAll();
    fmuComponentTemplateFile.close();
    if(fmuComponentCode.isEmpty())
    {
        printErrorMessage("Unable to generate code for "+fmuName+".hpp.");
        removeDir(rTargetPath);
        return false;
    }

    QString headerGuard = fmuName.toUpper()+"_HPP_INCLUDED";
    QString className = "FMU_"+fmuName;

    QString localVars;
    if(!parVars.isEmpty())
    {
        localVars.append("double ");
        foreach(const QString &varName, parVars)
        {
            localVars.append(varName+", ");             //Parameters
        }
        if(localVars.endsWith(", "))
        {
            localVars.chop(2);
        }
        localVars.append(";\n");
    }
    if(!inputVars.isEmpty()  || !outputVars.isEmpty() || !portVars.isEmpty())
    {
        localVars.append("    double ");
        foreach(const QString &varName, inputVars)
        {
            localVars.append("*"+varName+", ");         //Input variables
        }
        foreach(const QString &varName, outputVars)
        {
            localVars.append("*"+varName+", ");         //Output variables
        }
        for(int i=0; i<portVarVars.size(); ++i)
        {
            foreach(const QString &varName, portVarVars.at(i))
            {
                localVars.append("*"+varName+", ");     //Power port variables
            }
        }
        if(localVars.endsWith(", "))
        {
            localVars.chop(2);
        }
        localVars.append(";\n");
        if(!portVars.isEmpty())
        {
            localVars.append("    Port ");
            foreach(const QString &varName, portVars)
            {
                localVars.append("*"+varName+", ");         //Ports
            }
            if(localVars.endsWith(", "))
            {
                localVars.chop(2);
            }
            localVars.append(";\n");
        }
    }

    QString addConstants;
    for(int i=0; i<parNames.size(); ++i)
    {
        if(i!=0)
        {
            addConstants.append("        ");
        }
        addConstants.append("addConstant(\""+parNames.at(i)+"\", \"\", \"\", 0, "+parVars.at(i)+");\n");
    }

    QString addInputs;
    for(int i=0; i<inputNames.size(); ++i)
    {
        if(i!=0)
        {
            addInputs.append("        ");
        }
        addInputs.append("addInputVariable(\""+inputNames.at(i)+"\", \"\", \"\", 0, &"+inputVars.at(i)+");\n");
    }

    QString addOutputs;
    for(int i=0; i<outputNames.size(); ++i)
    {
        if(i!=0)
        {
            addOutputs.append("        ");
        }
        addOutputs.append("addOutputVariable(\""+outputNames.at(i)+"\", \"\", \"\", 0, &"+outputVars.at(i)+");\n");
    }

    QString addPorts;
    for(int i=0; i<portNames.size(); ++i)
    {
        if(i!=0)
        {
            addPorts.append("        ");
        }
        QString nodeType;
        if(portTypes.at(i) == "mechanicq")
        {
            nodeType = "NodeMechanic";
        }
        else if(portTypes.at(i) == "hydraulicq")
        {
            nodeType = "NodeHydraulic";
        }
        addPorts.append(portVars.at(i)+"= addPowerPort(\""+portNames.at(i)+"\",\""+nodeType+"\");\n");
    }

    QString setNodeDataPointers;
    for(int i=0; i<portNames.size(); ++i)
    {
        if(i!=0)
        {
            setNodeDataPointers.append("        ");
        }
        if(portTypes.at(i) == "mechanicq")
        {
            QString port = portVars.at(i);
            setNodeDataPointers.append(portVarVars.at(i).at(0)+" = getSafeNodeDataPtr("+port+", NodeMechanic::Force);\n");
            setNodeDataPointers.append(portVarVars.at(i).at(1)+" = getSafeNodeDataPtr("+port+", NodeMechanic::Position);\n");
            setNodeDataPointers.append(portVarVars.at(i).at(2)+" = getSafeNodeDataPtr("+port+", NodeMechanic::Velocity);\n");
            setNodeDataPointers.append(portVarVars.at(i).at(3)+" = getSafeNodeDataPtr("+port+", NodeMechanic::EquivalentMass);\n");
            setNodeDataPointers.append(portVarVars.at(i).at(4)+" = getSafeNodeDataPtr("+port+", NodeMechanic::WaveVariable);\n");
            setNodeDataPointers.append(portVarVars.at(i).at(5)+" = getSafeNodeDataPtr("+port+", NodeMechanic::CharImpedance);\n");
        }
        else if(portTypes.at(i) == "hydraulicq")
        {
            QString port = portVars.at(i);
            setNodeDataPointers.append(portVarVars.at(i).at(0)+" = getSafeNodeDataPtr("+port+", NodeHydraulic::Pressure);\n");
            setNodeDataPointers.append(portVarVars.at(i).at(1)+" = getSafeNodeDataPtr("+port+", NodeHydraulic::Flow);\n");
            setNodeDataPointers.append(portVarVars.at(i).at(2)+" = getSafeNodeDataPtr("+port+", NodeHydraulic::WaveVariable);\n");
            setNodeDataPointers.append(portVarVars.at(i).at(3)+" = getSafeNodeDataPtr("+port+", NodeHydraulic::CharImpedance);\n");
        }
    }

    QString setPars;
    QString temp = extractTaggedSection(fmuComponentCode, "setpars");
    for(int i=0; i<parNames.size(); ++i)
    {
        QString tempVar = temp;
        tempVar.replace("<<<vr>>>", parRefs.at(i));
        tempVar.replace("<<<var>>>", parVars.at(i));
        setPars.append(tempVar+"\n");
    }

    QString readVars;
    temp = extractTaggedSection(fmuComponentCode, "readvars");
    for(int i=0; i<inputNames.size(); ++i)
    {
        QString tempVar = temp;
        tempVar.replace("<<<vr>>>", inputRefs.at(i));
        tempVar.replace("<<<var>>>", inputVars.at(i));
        readVars.append(tempVar+"\n");
    }
    for(int i=0; i<portNames.size(); ++i)
    {
        QString tempVar;
        int nOutputs;
        if(portTypes.at(i) == "mechanicq")
        {
            nOutputs = 4;
        }
        else if(portTypes.at(i) == "hydraulicq")
        {
            nOutputs = 2;
        }
        for(int j=nOutputs; j<portVarRefs.at(i).size(); ++j)
        {
            tempVar = temp;
            tempVar.replace("<<<vr>>>", portVarRefs.at(i).at(j));
            tempVar.replace("<<<var>>>", portVarVars.at(i).at(j));
            readVars.append(tempVar+"\n");
        }
    }

    QString writeVars;
    temp = extractTaggedSection(fmuComponentCode, "writevars");
    for(int i=0; i<outputNames.size(); ++i)
    {
        QString tempVar = temp;
        tempVar.replace("<<<vr>>>", outputRefs.at(i));
        tempVar.replace("<<<var>>>", outputVars.at(i));
        writeVars.append(tempVar+"\n");
    }
    for(int i=0; i<portNames.size(); ++i)
    {
        QString tempVar;
        int nOutputs;
        if(portTypes.at(i) == "mechanicq")
        {
            nOutputs = 4;
        }
        else if(portTypes.at(i) == "hydraulicq")
        {
            nOutputs = 2;
        }
        for(int j=0; j<nOutputs; ++j)
        {
            tempVar = temp;
            tempVar.replace("<<<vr>>>", portVarRefs.at(i).at(j));
            tempVar.replace("<<<var>>>", portVarVars.at(i).at(j));
            writeVars.append(tempVar+"\n");
        }
    }

    fmuComponentCode.replace("<<<headerguard>>>", headerGuard);
    fmuComponentCode.replace("<<<className>>>", className);
    fmuComponentCode.replace("<<<localvars>>>", localVars);
    fmuComponentCode.replace("<<<addconstants>>>", addConstants);
    fmuComponentCode.replace("<<<addinputs>>>", addInputs);
    fmuComponentCode.replace("<<<addoutputs>>>", addOutputs);
    fmuComponentCode.replace("<<<addports>>>", addPorts);
    fmuComponentCode.replace("<<<setnodedatapointers>>>", setNodeDataPointers);
    fmuComponentCode.replace("<<<fmupath>>>", rPath);
    QDir().mkpath(rTargetPath+"/temp");
    fmuComponentCode.replace("<<<temppath>>>", rTargetPath+"/temp/");
    replaceTaggedSection(fmuComponentCode, "setpars", setPars);
    replaceTaggedSection(fmuComponentCode, "readvars", readVars);
    replaceTaggedSection(fmuComponentCode, "writevars", writeVars);

    QTextStream fmuComponentHppStream(&fmuComponentHppFile);
    fmuComponentHppStream << fmuComponentCode;
    fmuComponentHppFile.close();


    //-------------------------------------------//
    printMessage("Writing " + fmuName + ".xml...");
    //-------------------------------------------//


    ComponentAppearanceSpecification cafSpec("FMU_"+fmuName);
    cafSpec.mSourceCode = "component_code/"+fmuName+".hpp";
    cafSpec.mRecompilable = true;
    cafSpec.mUserIconPath = "fmucomponent.svg";


    //These 4 variables are used for input/output port positioning
    double inputPosStep=1.0/(inputNames.size()+1.0);
    double outputPosStep=1.0/(outputNames.size()+1.0);
    double portPosStep=1.0/(portNames.size()+1.0);
    double inputPos=0;
    double outputPos=0;
    double portPos=0;

    for(int i=0; i<inputNames.size(); ++i)
    {
        inputPos += inputPosStep;
        cafSpec.addPort(inputNames.at(i), 0.0, inputPos, 180.0);
    }
    for(int i=0; i<outputNames.size(); ++i)
    {
        outputPos += outputPosStep;
        cafSpec.addPort(outputNames.at(i), 1.0, outputPos, 0.0);
    }
    for(int i=0; i<portNames.size(); ++i)
    {
        portPos += portPosStep;
        cafSpec.addPort(portNames.at(i), portPos, 0.0, 270.0);
    }

    QString cafPath = rTargetPath + "/" + fmuName + "/" + fmuName + ".xml";
    if(!generateCafFile(cafPath, cafSpec))
    {
        printErrorMessage("Generation of component appearance file (XML) failed.");
        removeDir(rTargetPath);
        return false;
    }

    rTypeName = fmuName;
    rHppPath = QFileInfo(fmuComponentHppFile).absoluteFilePath();

    return true;
}


//! @brief Generates an FMU for specified component system
//! @param savePath Path where to export FMU
//! @param me Boolean for using model exchange
//! @param pSystme Pointer to system to export
void HopsanFMIGenerator::generateToFmu(QString savePath, hopsan::ComponentSystem *pSystem, bool me, bool x64)
{
    printMessage("Initializing FMU export...");

    if(pSystem == 0)
    {
        printErrorMessage("System pointer is null. Aborting.");
        return;
    }

    QDir saveDir;
    saveDir.setPath(savePath);

    //Copy HopsanCore files to export directory
    if(!this->copyIncludeFilesToDir(savePath))
        return;
    if(!this->copySourceFilesToDir(savePath))
        return;
    if(!this->copyDefaultComponentCodeToDir(savePath))
        return;

    //Write the FMU ID
    int random = rand() % 1000000000;
    QString randomString = QString::number(random);
    QString ID = QUuid::createUuid().toString();

    //Collect information about ports
    QStringList inputVariables;
    QStringList inputComponents;
    QStringList inputPorts;
    QList<int> inputDatatypes;
    QStringList outputVariables;
    QStringList outputComponents;
    QStringList outputPorts;
    QList<int> outputDatatypes;
    QList<QStringList> tlmPorts;

    std::vector<HString> names = pSystem->getSubComponentNames();
    for(size_t i=0; i<names.size(); ++i)
    {
        getInterfaceInfo(pSystem->getSubComponent(names[i])->getTypeName().c_str(), names[i].c_str(),
                         inputVariables, inputComponents, inputPorts, inputDatatypes,
                         outputVariables, outputComponents, outputPorts, outputDatatypes, tlmPorts);
    }


    //Collect information about system parameters
    QStringList parameterNames;
    QStringList parameterValues;
    std::vector<HString> parameterNamesStd;
    pSystem->getParameterNames(parameterNamesStd);
    for(size_t p=0; p<parameterNamesStd.size(); ++p)
    {
        parameterNames.append(QString(parameterNamesStd[p].c_str()));
    }
    for(int p=0; p<parameterNames.size(); ++p)
    {
        HString value;
        pSystem->getParameterValue(parameterNamesStd[p], value);
        parameterValues.append(QString(value.c_str()));
    }


    //Create file objects for all files that shall be created
    QFile modelSourceFile;
    QString modelName = pSystem->getName().c_str();
    // modelName.chop(4);
    QString realModelName = modelName;          //Actual model name (used for hmf file)
    modelName.replace(" ", "_");        //Replace white spaces with underscore, to avoid problems
    modelSourceFile.setFileName(savePath + "/" + modelName + ".c");
    if(!modelSourceFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open " + modelName + ".c for writing.");
        return;
    }

    QFile modelDescriptionFile;
    modelDescriptionFile.setFileName(savePath + "/modelDescription.xml");
    if(!modelDescriptionFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open modelDescription.xml for writing.");
        return;
    }

    QFile tlmDescriptionFile;
    tlmDescriptionFile.setFileName(savePath + "/"+modelName+"_TLM.xml");
    if(!tlmDescriptionFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open "+modelName+"_TLM.xml for writing.");
        return;
    }

    QFile fmuHeaderFile;
    fmuHeaderFile.setFileName(savePath + "/HopsanFMU.h");
    if(!fmuHeaderFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open HopsanFMU.h for writing.");
        return;
    }

    QFile fmuSourceFile;
    fmuSourceFile.setFileName(savePath + "/HopsanFMU.cpp");
    if(!fmuSourceFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open HopsanFMU.cpp for writing.");
        return;
    }

#ifdef _WIN32
    QFile clBatchFile;
    clBatchFile.setFileName(savePath + "/compile.bat");
    if(!clBatchFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open compile.bat for writing.");
        return;
    }
#endif

    QFile modelHppFile;
    modelHppFile.setFileName(savePath + "/model.hpp");
    if(!modelHppFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open model.hpp for writing.");
        return;
    }

    printMessage("Writing modelDescription.xml...");

    QFile xmlTemplatefile(":templates/fmuModelDescriptionTemplate.xml");
    if(!xmlTemplatefile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open modelDescription.xml for writing.");
        return;
    }

    QString xmlCode;
    QTextStream t(&xmlTemplatefile);
    xmlCode = t.readAll();
    xmlTemplatefile.close();
    if(xmlCode.isEmpty())
    {
        printErrorMessage("Failed to generate XML code for modelDescription.xml.");
        return;
    }

    QString xmlReplace3;
    QString scalarVarLines = extractTaggedSection(xmlCode, "variables");
    int i, j;
    for(i=0; i<inputVariables.size(); ++i)
    {
        QString refString = QString::number(i);
        xmlReplace3.append(replaceTags(scalarVarLines, QStringList() << "varname" << "varref" << "causality", QStringList() << inputVariables.at(i) << refString << "input"));
    }
    for(j=0; j<outputVariables.size(); ++j)
    {
        QString refString = QString::number(i+j);
        xmlReplace3.append(replaceTags(scalarVarLines, QStringList() << "varname" << "varref" << "causality", QStringList() << outputVariables.at(j) << refString << "output"));
    }

    QString xmlReplace4;
    QString paramLines = extractTaggedSection(xmlCode, "parameters");
    for(int k=0; k<parameterNames.size(); ++k)
    {
        QString refString = QString::number(i+j+k);
        xmlReplace4.append(replaceTags(paramLines, QStringList() << "varname" << "varref" << "variability" << "start", QStringList() << parameterNames[k] << refString << "parameter" << parameterValues[k]));
    }

    xmlCode.replace("<<<modelname>>>", modelName);
    xmlCode.replace("<<<guid>>>", ID);
    replaceTaggedSection(xmlCode, "variables", xmlReplace3);
    replaceTaggedSection(xmlCode, "parameters", xmlReplace4);

    if(me)
    {
        xmlCode.replace("<<<implementation>>>","");
    }
    else
    {
        xmlCode.replace("<<<implementation>>>","<Implementation>\n  <CoSimulation_StandAlone>\n    <Capabilities\n      canHandleVariableCommunicationStepSize=\"false\"\n      canHandleEvents=\"false\"/>\n  </CoSimulation_StandAlone>\n</Implementation>");
    }

    if(me)
    {
        xmlCode.replace("<<<nstates>>>",QString::number(inputVariables.size()+outputVariables.size()));
    }
    else
    {
        xmlCode.replace("<<<nstates>>>","0");
    }


    QTextStream modelDescriptionStream(&modelDescriptionFile);
    modelDescriptionStream << xmlCode;
    modelDescriptionFile.close();


    if(!tlmPorts.isEmpty())
    {
        printMessage("Writing "+modelName+"_TLM.xml...");

        QString tlmCode;
        tlmCode.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        tlmCode.append("<fmutlm>\n");
        for(int i=0; i<tlmPorts.size(); ++i)
        {
            tlmCode.append("  <tlmport type=\""+tlmPorts[i].first()+"\">\n");
            for(int j=1; j<tlmPorts[i].size(); j+=2)
            {
                tlmCode.append("    <"+tlmPorts[i][j]+">"+tlmPorts[i][j+1]+"</"+tlmPorts[i][j]+">\n");
            }
            tlmCode.append("  </tlmport>\n");
        }
        tlmCode.append("</fmutlm>\n");

        QTextStream tlmDescriptionStream(&tlmDescriptionFile);
        tlmDescriptionStream << tlmCode;
        tlmDescriptionFile.close();
    }


    printMessage("Writing " + modelName + ".c...");

    QString sourceTemplateFileName;
    if(me)
    {
        sourceTemplateFileName = ":templates/fmuModelSourceTemplate.c";
    }
    else
    {
        sourceTemplateFileName = ":templates/fmuCoSimSourceTemplate.c";
    }

    QFile sourceTemplateFile(sourceTemplateFileName);
    if(!sourceTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open "+modelName+".c for writing.");
        return;
    }
    QString modelSourceCode;
    QTextStream t2(&sourceTemplateFile);
    modelSourceCode = t2.readAll();
    sourceTemplateFile.close();
    if(modelSourceCode.isEmpty())
    {
        printErrorMessage("Failed to generate code for "+modelName+".c.");
        return;
    }

    QString sourceReplace4;
    QString varDefLine = extractTaggedSection(modelSourceCode, "4");
    for(i=0; i<inputVariables.size(); ++i)
        sourceReplace4.append(replaceTags(varDefLine, QStringList() << "varname" << "varref", QStringList() << inputVariables.at(i) << QString::number(i)));
    for(j=0; j<outputVariables.size(); ++j)
        sourceReplace4.append(replaceTags(varDefLine, QStringList() << "varname" << "varref", QStringList() << outputVariables.at(j) << QString::number(i+j)));
    for(int k=0; k<parameterNames.size(); ++k)
        sourceReplace4.append(replaceTags(varDefLine, QStringList() << "varname" << "varref", QStringList() << parameterNames.at(k) << QString::number(i+j+k)));

    QString sourceReplace5;
    i=0;
    j=0;
    if(!inputVariables.isEmpty())
    {
        sourceReplace5.append(inputVariables.at(0)+"_");
        ++i;
    }
    else if(!outputVariables.isEmpty())
    {
        sourceReplace5.append(outputVariables.at(0)+"_");
        ++j;
    }
    for(; i<inputVariables.size(); ++i)
        sourceReplace5.append(", "+inputVariables.at(i)+"_");
    for(; j<outputVariables.size(); ++j)
        sourceReplace5.append(", "+outputVariables.at(j)+"_");

    QString sourceReplace6;
    QString startValueLine = extractTaggedSection(modelSourceCode, "6");
    for(i=0; i<inputVariables.size(); ++i)
        sourceReplace6.append(replaceTag(startValueLine, "varname", inputVariables.at(i)));         //!< Fix start value handling
    for(j=0; j<outputVariables.size(); ++j)
        sourceReplace6.append(replaceTag(startValueLine, "varname", outputVariables.at(j)));        //!< Fix start value handling

    QString sourceReplace8;
    for(i=0; i<inputVariables.size(); ++i)
        sourceReplace8.append("           case "+inputVariables.at(i)+"_: return getVariable(vr, "+QString::number(inputDatatypes.at(i))+");\n");
    for(j=0; j<outputVariables.size(); ++j)
        sourceReplace8.append("           case "+outputVariables.at(j)+"_: return getVariable(vr, "+QString::number(outputDatatypes.at(j))+");\n");

    QString sourceReplace9;
    for(i=0; i<inputVariables.size(); ++i)
        sourceReplace9.append("           case "+inputVariables.at(i)+"_: setVariable(vr, "+QString::number(inputDatatypes.at(i))+", value); break;\n");
    for(j=0; j<outputVariables.size(); ++j)
        sourceReplace9.append("           case "+outputVariables.at(j)+"_: setVariable(vr, "+QString::number(outputDatatypes.at(j))+", value); break;\n");
    for(int k=0; k<parameterNames.size(); ++k)
        sourceReplace9.append("           case "+parameterNames.at(k)+"_: setParameter(\""+parameterNames.at(k)+"\", value); break;\n");

    modelSourceCode.replace("<<<0>>>", modelName);
    modelSourceCode.replace("<<<1>>>", ID);
    modelSourceCode.replace("<<<2>>>", QString::number(inputVariables.size() + outputVariables.size() + parameterNames.size()));
    modelSourceCode.replace("<<<3>>>", QString::number(inputVariables.size() + outputVariables.size() + parameterNames.size()));  //!< @todo Does number of variables equal number of states?
    replaceTaggedSection(modelSourceCode, "4", sourceReplace4);
    modelSourceCode.replace("<<<5>>>", sourceReplace5);
    replaceTaggedSection(modelSourceCode, "6", sourceReplace6);
    modelSourceCode.replace("<<<7>>>", modelName);
    modelSourceCode.replace("<<<8>>>", sourceReplace8);
    modelSourceCode.replace("<<<9>>>", sourceReplace9);

    QTextStream modelSourceStream(&modelSourceFile);
    modelSourceStream << modelSourceCode;
    modelSourceFile.close();


    printMessage("Writing HopsanFMU.h...");

    QFile fmuHeaderTemplateFile(":templates/fmuHeaderTemplate.h");
    if(!fmuHeaderTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open HopsanFMU.h for writing.");
        return;
    }

    QString fmuHeaderCode;
    QTextStream t3(&fmuHeaderTemplateFile);
    fmuHeaderCode = t3.readAll();
    fmuHeaderTemplateFile.close();
    if(fmuHeaderCode.isEmpty())
    {
        printErrorMessage("Failed to generate code for HopsanFMU.h.");
        return;
    }

    QTextStream fmuHeaderStream(&fmuHeaderFile);
    fmuHeaderStream << fmuHeaderCode;
    fmuHeaderFile.close();


    printMessage("Writing HopsanFMU.cpp...");

    QString fmuSourceTemplateFileName(":templates/fmuSourceTemplate.c");
    QFile fmuSourceTemplateFile(fmuSourceTemplateFileName);
    if(!fmuSourceTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open HopsanFMU.cpp for writing.");
        return;
    }

    QString fmuSourceCode;
    QTextStream t4(&fmuSourceTemplateFile);
    fmuSourceCode = t4.readAll();
    fmuSourceTemplateFile.close();
    if(fmuSourceCode.isEmpty())
    {
        printErrorMessage("Failed to generate code for HopsanFMU.cpp.");
        return;
    }

    fmuSourceCode = replaceTag(fmuSourceCode, "nports", QString::number(inputVariables.size() + outputVariables.size()));

    QString fmuSourceReplace1;
    QString portPointerSection = extractTaggedSection(fmuSourceCode, "assignportpointers");
    for(i=0; i<inputVariables.size(); ++i)
    {
        QStringList tags = QStringList() << "idx" << "comp" << "port";
        QStringList replacements = QStringList() << QString::number(i) << inputComponents.at(i) << inputPorts.at(i);
        fmuSourceReplace1.append(replaceTags(portPointerSection, tags, replacements));
    }
    for(j=0; j<outputVariables.size(); ++j)
    {
        QStringList tags = QStringList() << "idx" << "comp" << "port";
        QStringList replacements = QStringList() << QString::number(i+j) << outputComponents.at(j) << outputPorts.at(j);
        fmuSourceReplace1.append(replaceTags(portPointerSection, tags, replacements));
    }

    replaceTaggedSection(fmuSourceCode, "assignportpointers", fmuSourceReplace1);
    QTextStream fmuSourceStream(&fmuSourceFile);
    fmuSourceStream << fmuSourceCode;
    fmuSourceFile.close();

    printMessage("Copying HopsanCore files...");

    printMessage("Copying FMI files...");

    QFile fmuModelFunctionsHFile(mBinPath + "/../ThirdParty/fmi/fmiModelFunctions.h");
    fmuModelFunctionsHFile.copy(savePath + "/fmiModelFunctions.h");
    QFile fmuFunctionsHFile(mBinPath + "/../ThirdParty/fmi/fmiFunctions.h");
    fmuFunctionsHFile.copy(savePath + "/fmiFunctions.h");
    QFile fmuPlatformTypesHFile(mBinPath + "/../ThirdParty/fmi/fmiPlatformTypes.h");
    fmuPlatformTypesHFile.copy(savePath + "/fmiPlatformTypes.h");
    QFile fmiModelTypesHFile(mBinPath + "/../ThirdParty/fmi/fmiModelTypes.h");
    fmiModelTypesHFile.copy(savePath + "/fmiModelTypes.h");
    QFile fmiTemplateCFile(mBinPath + "/../ThirdParty/fmi/fmuTemplate.c");
    fmiTemplateCFile.copy(savePath + "/fmuTemplate.c");
    QFile fmiTemplateHFile(mBinPath + "/../ThirdParty/fmi/fmuTemplate.h");
    fmiTemplateHFile.copy(savePath + "/fmuTemplate.h");

    if(!assertFilesExist(savePath, QStringList() << "fmiModelFunctions.h" << "fmiFunctions.h" << "fmiModelTypes.h" << "fmuTemplate.c" << "fmuTemplate.h"))
        return;

    printMessage("Generating model file...");

    QStringList modelLines;
    QFile modelFile(savePath + "/" + realModelName + ".hmf");
    if (!modelFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        printErrorMessage("Could not open "+realModelName+".hmf for reading.");
        return;
    }
    while (!modelFile.atEnd())
    {
        QString line = modelFile.readLine();
        line.chop(1);
        line.replace("\"", "\\\"");
        modelLines.append(line);
    }
    modelLines.last().append("\\n");
    modelFile.close();

    QTextStream modelHppStream(&modelHppFile);
    modelHppStream << "#include <vector>\n\n";
    modelHppStream << "std::string getModelString()\n{\n";
    modelHppStream << "    std::string model = ";
    Q_FOREACH(const QString line, modelLines)
    {
        modelHppStream << "\""+line+"\"\n";
    }
    modelHppStream << "    ;\n\n";
    modelHppStream << "    return model;\n}\n";
    modelHppFile.close();

    printMessage("Replacing namespace...");

    QString nameSpace = "HopsanFMU"+randomString;
    QStringList before = QStringList() << "using namespace hopsan;" << "namespace hopsan " << "\nhopsan::" << "::hopsan::" << " hopsan::" << "*hopsan::" << "namespace hopsan{";
    QStringList after = QStringList() << "using namespace "+nameSpace+";" << "namespace "+nameSpace+" " << "\n"+nameSpace+"::" << "::"+nameSpace+"::" << " "+nameSpace+"::" << "*"+nameSpace+"::" << "namespace "+nameSpace+"{";
    Q_FOREACH(const QString &file, getHopsanCoreSourceFiles())
    {
        if(!replaceInFile(savePath+"/HopsanCore/"+file, before, after))
            return;
    }
    Q_FOREACH(const QString &file, getHopsanCoreIncludeFiles())
    {
        if(!replaceInFile(savePath+"/HopsanCore/"+file, before, after))
            return;
    }
    QStringList componentFiles;
    findAllFilesInFolderAndSubFolders(savePath+"/componentLibraries/defaultLibrary", "hpp", componentFiles);
    findAllFilesInFolderAndSubFolders(savePath+"/componentLibraries/defaultLibrary", "h", componentFiles);
    Q_FOREACH(const QString &file, componentFiles)
    {
        //qDebug() << "Replacing component file: " << file;
        if(!replaceInFile(file, before, after))
            return;
    }
    if(!replaceInFile(savePath+"/HopsanFMU.cpp", before, after))
        return;
    if(!replaceInFile(savePath+"/HopsanFMU.h", before, after))
        return;

#ifdef _WIN32
    printMessage("Compiling "+modelName+".dll...");
    //Write the compilation script file
    QTextStream clBatchStream(&clBatchFile);
    clBatchStream << mGccPath+"gcc.exe -c -w -shared -static -static-libgcc -fPIC -Wl,--rpath,'$ORIGIN/.' "+modelName+".c\n";
    clBatchStream << mGccPath+"g++ -w -shared -static -static-libgcc -DDOCOREDLLEXPORT -DBUILTINDEFAULTCOMPONENTLIB -o "+modelName+".dll "+modelName+".o HopsanFMU.cpp HopsanCore/src/Component.cc HopsanCore/src/ComponentSystem.cc HopsanCore/src/HopsanEssentials.cc HopsanCore/src/HopsanTypes.cc HopsanCore/src/Node.cc HopsanCore/src/Nodes.cc HopsanCore/src/Parameters.cc HopsanCore/src/Port.cc HopsanCore/src/ComponentUtilities/AuxiliarySimulationFunctions.cc HopsanCore/src/ComponentUtilities/CSVParser.cc HopsanCore/src/ComponentUtilities/DoubleIntegratorWithDamping.cc HopsanCore/src/ComponentUtilities/PLOParser.cc HopsanCore/src/ComponentUtilities/DoubleIntegratorWithDampingAndCoulumbFriction.cc HopsanCore/src/ComponentUtilities/EquationSystemSolver.cpp HopsanCore/src/ComponentUtilities/FirstOrderTransferFunction.cc HopsanCore/src/ComponentUtilities/HopsanPowerUser.cc HopsanCore/src/ComponentUtilities/Integrator.cc HopsanCore/src/ComponentUtilities/IntegratorLimited.cc HopsanCore/src/ComponentUtilities/ludcmp.cc HopsanCore/src/ComponentUtilities/matrix.cc HopsanCore/src/ComponentUtilities/SecondOrderTransferFunction.cc HopsanCore/src/ComponentUtilities/WhiteGaussianNoise.cc HopsanCore/src/CoreUtilities/CoSimulationUtilities.cpp HopsanCore/src/CoreUtilities/GeneratorHandler.cpp HopsanCore/src/CoreUtilities/HmfLoader.cc HopsanCore/src/CoreUtilities/HopsanCoreMessageHandler.cc HopsanCore/src/CoreUtilities/LoadExternal.cc HopsanCore/src/CoreUtilities/MultiThreadingUtilities.cpp HopsanCore/src/CoreUtilities/StringUtilities.cpp componentLibraries/defaultLibrary/defaultComponentLibraryInternal.cc HopsanCore/Dependencies/libcsv_parser++-1.0.0/csv_parser.cpp -IHopsanCore/include -IcomponentLibraries/defaultLibrary -IHopsanCore/Dependencies/rapidxml-1.13 -IHopsanCore/Dependencies/libcsv_parser++-1.0.0/include/csv_parser\n";
    clBatchFile.close();

    callProcess("cmd.exe", QStringList() << "/c" << "cd " + savePath + " & compile.bat");

    if(!assertFilesExist(savePath, QStringList() << modelName+".dll"))
        return;

#elif linux
    printMessage("Compiling "+modelName+".so...");

    QString gccCommand1 = "cd "+savePath+" && gcc -c -w -shared -static -static-libgcc -fPIC -Wl,--rpath,'$ORIGIN/.' "+modelName+".c\n";
    QString gccCommand2 = "cd "+savePath+" && g++ -w -shared -static -static-libgcc -fPIC -DDOCOREDLLEXPORT -DBUILTINDEFAULTCOMPONENTLIB -o "+modelName+".so "+modelName+".o HopsanFMU.cpp HopsanCore/src/Component.cc HopsanCore/src/ComponentSystem.cc HopsanCore/src/HopsanEssentials.cc HopsanCore/src/Node.cc HopsanCore/src/Nodes.cc HopsanCore/src/Parameters.cc HopsanCore/src/Port.cc HopsanCore/src/ComponentUtilities/AuxiliarySimulationFunctions.cc HopsanCore/src/ComponentUtilities/CSVParser.cc HopsanCore/src/ComponentUtilities/DoubleIntegratorWithDamping.cc HopsanCore/src/ComponentUtilities/PLOParser.cc HopsanCore/src/ComponentUtilities/DoubleIntegratorWithDampingAndCoulumbFriction.cc HopsanCore/src/ComponentUtilities/EquationSystemSolver.cpp HopsanCore/src/ComponentUtilities/FirstOrderTransferFunction.cc HopsanCore/src/ComponentUtilities/Integrator.cc HopsanCore/src/ComponentUtilities/IntegratorLimited.cc HopsanCore/src/ComponentUtilities/ludcmp.cc HopsanCore/src/ComponentUtilities/matrix.cc HopsanCore/src/ComponentUtilities/SecondOrderTransferFunction.cc HopsanCore/src/ComponentUtilities/WhiteGaussianNoise.cc HopsanCore/src/CoreUtilities/CoSimulationUtilities.cpp HopsanCore/src/CoreUtilities/GeneratorHandler.cpp HopsanCore/src/CoreUtilities/HmfLoader.cc HopsanCore/src/CoreUtilities/HopsanCoreMessageHandler.cc HopsanCore/src/CoreUtilities/LoadExternal.cc HopsanCore/src/CoreUtilities/MultiThreadingUtilities.cpp componentLibraries/defaultLibrary/defaultComponentLibraryInternal.cc HopsanCore/Dependencies/libcsv_parser++-1.0.0/csv_parser.cpp -IHopsanCore/include -IcomponentLibraries/defaultLibrary -IHopsanCore/Dependencies/rapidxml-1.13 -IHopsanCore/Dependencies/libcsv_parser++-1.0.0/include/csv_parser\n";

    //qDebug() << "Command 1: " << gccCommand1;
    //qDebug() << "Command 2: " << gccCommand2;

    callProcess("gcc", QStringList() << "-c" << "-w" << "-shared" << "-fPIC" << "-Wl,--rpath,'$ORIGIN/.'" << modelName+".c", savePath);
    callProcess("g++", QStringList() << "-w" << "-shared" << "-fPIC" << "-DDOCOREDLLEXPORT" << "-DBUILTINDEFAULTCOMPONENTLIB" << "-o"+modelName+".so" << modelName+".o" << "HopsanFMU.cpp" << "HopsanCore/src/Component.cc" << "HopsanCore/src/ComponentSystem.cc" << "HopsanCore/src/HopsanEssentials.cc" << "HopsanCore/src/Node.cc" << "HopsanCore/src/Nodes.cc" << "HopsanCore/src/Parameters.cc" << "HopsanCore/src/Port.cc" << "HopsanCore/src/ComponentUtilities/AuxiliarySimulationFunctions.cc" << "HopsanCore/src/ComponentUtilities/CSVParser.cc" << "HopsanCore/src/ComponentUtilities/PLOParser.cc"<< "HopsanCore/src/ComponentUtilities/DoubleIntegratorWithDamping.cc" << "HopsanCore/src/ComponentUtilities/DoubleIntegratorWithDampingAndCoulumbFriction.cc" << "HopsanCore/src/ComponentUtilities/EquationSystemSolver.cpp" << "HopsanCore/src/ComponentUtilities/FirstOrderTransferFunction.cc" << "HopsanCore/src/ComponentUtilities/Integrator.cc" << "HopsanCore/src/ComponentUtilities/IntegratorLimited.cc" << "HopsanCore/src/ComponentUtilities/ludcmp.cc" << "HopsanCore/src/ComponentUtilities/matrix.cc" << "HopsanCore/src/ComponentUtilities/SecondOrderTransferFunction.cc" << "HopsanCore/src/ComponentUtilities/TurbulentFlowFunction.cc" << "HopsanCore/src/ComponentUtilities/ValveHysteresis.cc" << "HopsanCore/src/ComponentUtilities/WhiteGaussianNoise.cc" << "HopsanCore/src/CoreUtilities/CoSimulationUtilities.cpp" << "HopsanCore/src/CoreUtilities/GeneratorHandler.cpp" << "HopsanCore/src/CoreUtilities/HmfLoader.cc" << "HopsanCore/src/CoreUtilities/HopsanCoreMessageHandler.cc" << "HopsanCore/src/CoreUtilities/LoadExternal.cc" << "HopsanCore/src/CoreUtilities/MultiThreadingUtilities.cpp" << "componentLibraries/defaultLibrary/defaultComponentLibraryInternal.cc" << "HopsanCore/Dependencies/libcsv_parser++-1.0.0/csv_parser.cpp" << "-IHopsanCore/include" << "-IcomponentLibraries/defaultLibrary" << "-IHopsanCore/Dependencies/rapidxml-1.13" << "-IHopsanCore/Dependencies/libcsv_parser++-1.0.0/include/csv_parser", savePath);

    if(!assertFilesExist(savePath, QStringList() << modelName+".so"))
        return;
#endif

    printMessage("Sorting files...");

#ifdef _WIN32
    if(x64)
    {
        saveDir.mkpath("fmu/binaries/win64");
        saveDir.mkpath("fmu/resources");
        QFile modelDllFile(savePath + "/" + modelName + ".dll");
        modelDllFile.copy(savePath + "/fmu/binaries/win64/" + modelName + ".dll");
        QFile modelLibFile(savePath + "/" + modelName + ".lib");
        modelLibFile.copy(savePath + "/fmu/binaries/win64/" + modelName + ".lib");
    }
    else
    {
        saveDir.mkpath("fmu/binaries/win32");
        saveDir.mkpath("fmu/resources");
        QFile modelDllFile(savePath + "/" + modelName + ".dll");
        modelDllFile.copy(savePath + "/fmu/binaries/win32/" + modelName + ".dll");
        QFile modelLibFile(savePath + "/" + modelName + ".lib");
        modelLibFile.copy(savePath + "/fmu/binaries/win32/" + modelName + ".lib");
    }
#elif linux && __i386__
    saveDir.mkpath("fmu/binaries/linux32");
    saveDir.mkpath("fmu/resources");
    QFile modelSoFile(savePath + "/" + modelName + ".so");
    modelSoFile.copy(savePath + "/fmu/binaries/linux32/" + modelName + ".so");
#elif linux && __x86_64__
    saveDir.mkpath("fmu/binaries/linux64");
    saveDir.mkpath("fmu/resources");
    QFile modelSoFile(savePath + "/" + modelName + ".so");
    modelSoFile.copy(savePath + "/fmu/binaries/linux64/" + modelName + ".so");
#endif
    // QFile modelFile(savePath + "/" + realModelName + ".hmf");
    modelFile.copy(savePath + "/fmu/resources/" + realModelName + ".hmf");
    modelDescriptionFile.copy(savePath + "/fmu/modelDescription.xml");
    tlmDescriptionFile.copy(savePath+"/fmu/"+modelName+"_TLM.xml");

    printMessage("Compressing files...");

#ifdef _WIN32
    QString program = mBinPath + "../ThirdParty/7z/7z";
    QStringList arguments = QStringList() << "a" << "-tzip" << "../"+modelName+".fmu" << savePath+"/fmu/modelDescription.xml" << savePath+"/fmu/"+modelName+"_TLM.xml" << "-r" << savePath + "/fmu/binaries";
    callProcess(program, arguments, savePath+"/fmu");
#elif linux && __i386__
    QStringList arguments = QStringList() << "-r" << "../"+modelName+".fmu" << "modelDescription.xml" << modelName+"_TLM.xml" << "binaries/linux32/"+modelName+".so";
    callProcess("zip", arguments, savePath+"/fmu");
#elif linux && __x86_64__
    QStringList arguments = QStringList() << "-r" << "../"+modelName+".fmu" << "modelDescription.xml" << modelName+"_TLM.xml" << "binaries/linux64/"+modelName+".so";
    callProcess("zip", arguments, savePath+"/fmu");
#endif

    if(!assertFilesExist(savePath, QStringList() << modelName+".fmu"))
        return;

    //Clean up temporary files
   // cleanUp(savePath, QStringList() << "compile.bat" << modelName+".c" << modelName+".dll" << modelName+".so" << modelName+".o" << modelName+".hmf" <<
   //         "fmiModelFunctions.h" << "fmiModelTypes.h" << "fmuTemplate.c" << "fmuTemplate.h" << "HopsanFMU.cpp" << "HopsanFMU.h" << "model.hpp" <<
   //         "modelDescription.xml", QStringList() << "componentLibraries" << "fmu" << "HopsanCore");

    printMessage("Finished.");
}


bool HopsanFMIGenerator::readTLMSpecsFromFile(const QString &fileName, QStringList &tlmPortTypes, QList<QStringList> &tlmPortVarNames,
                                              QList<QStringList> &tlmPortValueRefs, QStringList &inVarValueRefs, QStringList &inVarPortNames,
                                              QStringList &outVarValueRefs, QStringList &outVarPortNames, QString &cqsType)
{
    QFile tlmSpecFile;
    tlmSpecFile.setFileName(fileName);
    QDomDocument tlmDomDocument;
    QDomElement tlmRoot;
    if(tlmSpecFile.exists())
    {
        printMessage("Reading TLM specifications from "+tlmSpecFile.fileName()+"...");
        tlmRoot = loadXMLDomDocument(tlmSpecFile, tlmDomDocument, "fmutlm");
        tlmSpecFile.close();
    }
    else
    {
        printMessage("No TLM specification file found.");
        return true;
    }

    if(tlmRoot == QDomElement())
    {
        printErrorMessage("Unable to parse TLM specification file.");
        return true;        // Don't abort import, it could still work without the TLM stuff
    }
    else
    {
        QString type;
        QStringList qVars;
        QStringList cVars;

        QDomElement portElement = tlmRoot.firstChildElement("tlmport");
        while(!portElement.isNull())
        {
            type = portElement.attribute("type");

            QStringList nodeTypes;
            GeneratorNodeInfo::getNodeTypes(nodeTypes);
            Q_FOREACH(const QString &nodeType, nodeTypes)
            {
                GeneratorNodeInfo info = GeneratorNodeInfo(nodeType);
                if(type == info.niceName+"q" || type == info.niceName+"c")
                {
                    Q_FOREACH(const QString &var, info.qVariables)
                    {
                        QDomElement element = portElement.firstChildElement(var);
                        if(element.isNull())
                        {
                            printErrorMessage("Node type does not match variable names.");
                            return false;
                        }
                        qVars.append(element.text());
                    }
                    Q_FOREACH(const QString &var, info.cVariables)
                    {
                        QDomElement element = portElement.firstChildElement(var);
                        if(element.isNull())
                        {
                            printErrorMessage("Node type does not match variable names.");
                            return false;
                        }
                        cVars.append(element.text());
                    }
                }
            }

            Q_FOREACH(const QString &nodeType, nodeTypes)
            {
                GeneratorNodeInfo info = GeneratorNodeInfo(nodeType);
                if(type == info.niceName+"q")
                {
                    QStringList varNames;
                    tlmPortValueRefs.append(QStringList());
                    Q_FOREACH(const QString &var, qVars)
                    {
                        if(!outVarPortNames.contains(var))
                        {
                            printErrorMessage("Error in TLM specifications: Specified variable does not exist.");
                            return false;
                        }
                        varNames << var;
                        tlmPortValueRefs.last().append(outVarValueRefs[outVarPortNames.indexOf(var)]);
                        outVarValueRefs.removeAt(outVarPortNames.indexOf(var));
                        outVarPortNames.removeAll(var);
                    }
                    Q_FOREACH(const QString &var, cVars)
                    {
                        if(!inVarPortNames.contains(var))
                        {
                            printErrorMessage("Error in TLM specifications: Specified variable does not exist.");
                            return false;
                        }
                        varNames << var;
                        tlmPortValueRefs.last().append(inVarValueRefs[inVarPortNames.indexOf(var)]);
                        inVarValueRefs.removeAt(inVarPortNames.indexOf(var));
                        inVarPortNames.removeAll(var);
                    }

                    printMessage("Adding "+info.niceName+" port of Q-type");

                    if(cqsType == "Signal" || cqsType == "Q")
                        cqsType = "Q";
                    else
                        cqsType = "";

                    tlmPortTypes.append(type);
                    tlmPortVarNames.append(varNames);
                }
                else if(type == info.niceName+"c")
                {
                    QStringList varNames;
                    tlmPortValueRefs.append(QStringList());
                    Q_FOREACH(const QString &var, qVars)
                    {
                        if(!inVarPortNames.contains(var))
                        {
                            printErrorMessage("Error in TLM specifications: Specified variable does not exist.");
                            return false;
                        }
                        varNames << var;
                        tlmPortValueRefs.last().append(inVarValueRefs[inVarPortNames.indexOf(var)]);
                        inVarValueRefs.removeAt(inVarPortNames.indexOf(var));
                        inVarPortNames.removeAll(var);
                    }
                    Q_FOREACH(const QString &var, cVars)
                    {
                        if(!outVarPortNames.contains(var))
                        {
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


void HopsanFMIGenerator::getInterfaceInfo(QString typeName, QString compName,
                                          QStringList &inVars, QStringList &inComps, QStringList &inPorts, QList<int> &inDatatypes,
                                          QStringList &outVars, QStringList &outComps, QStringList &outPorts, QList<int> &outDatatypes,
                                          QList<QStringList> &tlmPorts)
{
    if(typeName == "SignalInputInterface")
    {
        inVars.append(compName.remove(' ').remove("-"));
        inComps.append(compName);
        inPorts.append("out");
        inDatatypes.append(0);
    }
    else if(typeName == "SignalOutputInterface")
    {
        outVars.append(compName.remove(' ').remove("-"));
        outComps.append(compName);
        outPorts.append("in");
        outDatatypes.append(0);
    }

    QString nodeType, cqType;
    QString portName = "P1";
    getNodeAndCqTypeFromInterfaceComponent(typeName, nodeType, cqType);

    if(cqType == "c")
    {
        QString name=compName;
        name.remove(' ').remove("-");

        GeneratorNodeInfo info = GeneratorNodeInfo(nodeType);

        tlmPorts.append(QStringList() << info.niceName+"q");
        Q_FOREACH(const QString &var, info.qVariables)
        {
            outVars.append(name+"_"+var+"__");
            outComps.append(compName);
            outPorts.append(portName);
            outDatatypes.append(info.varIdx[info.qVariables.indexOf(var)]);
            tlmPorts.last() << var << name+"_"+var+"__";
        }
        Q_FOREACH(const QString &var, info.cVariables)
        {
            inVars.append(name+"_"+var+"__");
            inComps.append(compName);
            inPorts.append("P1");
            inDatatypes.append(info.varIdx[info.qVariables.size()+info.cVariables.indexOf(var)]);
            tlmPorts.last() << var << name+"_"+var+"__";
        }
    }
    else if(cqType == "q")
    {
        QString name=compName.remove(' ').remove("-");

        GeneratorNodeInfo info = GeneratorNodeInfo(nodeType);

        tlmPorts.append(QStringList() << info.niceName+"c");
        Q_FOREACH(const QString &var, info.qVariables)
        {
            inVars.append(name+"_"+var+"__");
            inComps.append(compName);
            inPorts.append("P1");
            inDatatypes.append(info.varIdx[info.qVariables.indexOf(var)]);
            tlmPorts.last() << var << name+"_"+var+"__";
        }
        Q_FOREACH(const QString &var, info.cVariables)
        {
            outVars.append(name+"_"+var+"__");
            outComps.append(compName);
            outPorts.append("P1");
            outDatatypes.append(info.varIdx[info.qVariables.size()+info.cVariables.indexOf(var)]);
            tlmPorts.last() << var << name+"_"+var+"__";
        }
    }
}

