#include "generators/HopsanFMIGenerator.h"
#include "GeneratorUtilities.h"
#include "ComponentSystem.h"
#include <QApplication>
#include <cassert>
#include <QProcess>
#include <QUuid>
#include <QDateTime>
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
        int nOutputs=0;
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



void HopsanFMIGenerator::generateToFmu(QString savePath, ComponentSystem *pSystem, int version, bool x64)
{
#ifdef _WIN32
    if(mGccPath.isEmpty())
    {
        printErrorMessage("Compiler path not specified.");
        return;
    }
#endif

    //------------------------------------------------------------------//
    //Obtain model name and version string
    //------------------------------------------------------------------//

    QString modelName = pSystem->getName().c_str();
    QString vStr = QString::number(version);

    //------------------------------------------------------------------//
    //Copy HopsanCore files to export directory
    //------------------------------------------------------------------//

    if(!this->copyIncludeFilesToDir(savePath))
        return;
    if(!this->copySourceFilesToDir(savePath))
        return;
    if(!this->copyDefaultComponentCodeToDir(savePath))
        return;

    //------------------------------------------------------------------//
    //Generate identifier string
    //------------------------------------------------------------------//

    QString guid = QUuid::createUuid().toString();

    //------------------------------------------------------------------//
    //Generate modelDescription.xml
    //------------------------------------------------------------------//

    size_t nReals, nInputs, nOutputs;
    generateModelDescriptionXmlFile(pSystem, savePath, guid, version, nReals, nInputs, nOutputs);

    //------------------------------------------------------------------//
    //Copy source files from templates
    //------------------------------------------------------------------//

    copyFile(":/templates/fmu"+vStr+"_model.h", savePath+"/fmu"+vStr+"_model.h");
    copyFile(":/templates/fmu"+vStr+"_model.c", savePath+"/fmu"+vStr+"_model.c");
    copyFile(":/templates/fmu"+vStr+"_model_cs.c", savePath+"/fmu"+vStr+"_model_cs.c");
    copyFile(":/templates/fmu_hopsan.h", savePath+"/fmu_hopsan.h");

    //------------------------------------------------------------------//
    printMessage("Generating fmu1_model_defines.h");
    //------------------------------------------------------------------//

    QFile fmuModelDefinesHeaderTemplateFile;
    fmuModelDefinesHeaderTemplateFile.setFileName(":templates/fmu"+vStr+"_model_defines.h");
    if(!fmuModelDefinesHeaderTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open fmu"+vStr+"_model_defines.h for reading.");
        return;
    }
    QString fmuModelDefinesHeaderCode;
    QTextStream t3(&fmuModelDefinesHeaderTemplateFile);
    fmuModelDefinesHeaderCode = t3.readAll();
    fmuModelDefinesHeaderTemplateFile.close();
    if(fmuModelDefinesHeaderCode.isEmpty())
    {
        printErrorMessage("Unable to generate code for fmu"+vStr+"_model_defines.h");
        return;
    }
    fmuModelDefinesHeaderCode.replace("<<<n_reals>>>", QString::number(nReals));
    fmuModelDefinesHeaderCode.replace("<<<n_inputs>>>", QString::number(nInputs));
    fmuModelDefinesHeaderCode.replace("<<<n_outputs>>>", QString::number(nOutputs));
    fmuModelDefinesHeaderCode.replace("<<<guid>>>", guid);
    if(version == 1)
    {
        fmuModelDefinesHeaderCode.replace("<<<modelname>>>", modelName);
    }
    QFile fmuModelDefinesHeaderFile(savePath+"/fmu"+vStr+"_model_defines.h");
    if(!fmuModelDefinesHeaderFile.open(QFile::Text | QFile::WriteOnly))
    {
        printErrorMessage("Unable to open fmu"+vStr+"_model_defines.h for writing");
        return;
    }
    QTextStream fmuModelDefinesHeaderStream(&fmuModelDefinesHeaderFile);
    fmuModelDefinesHeaderStream << fmuModelDefinesHeaderCode;
    fmuModelDefinesHeaderFile.close();

    //------------------------------------------------------------------//
    printMessage("Generating fmu_hopsan.c");
    //------------------------------------------------------------------//

    QFile fmuHopsanSourceFileTemplate;
    fmuHopsanSourceFileTemplate.setFileName(":templates/fmu_hopsan.c");
    assert(fmuHopsanSourceFileTemplate.open(QIODevice::ReadOnly | QIODevice::Text));
    QString fmuHopsanSourceCode;
    QTextStream t6(&fmuHopsanSourceFileTemplate);
    fmuHopsanSourceCode = t6.readAll();
    fmuHopsanSourceFileTemplate.close();
    if(fmuHopsanSourceCode.isEmpty())
    {
        printErrorMessage("Unable to generate code for fmu_hopsan.c");
        return;
    }

    QString setDataPtrsString;
    size_t vr=0;
    QList<InterfacePortSpec> interfacePortSpecs;
    QStringList path = QStringList();
    getInterfaces(interfacePortSpecs, pSystem, path);
    foreach(const InterfacePortSpec &portSpec, interfacePortSpecs)
    {
        foreach(const InterfaceVarSpec &varSpec, portSpec.vars)
        {
            QString temp = "dataPtrs["+QString::number(vr)+"] = spCoreComponentSystem";
            foreach(const QString &subsystem, portSpec.path)
            {
                temp.append("->getSubComponentSystem(\""+subsystem+"\")");
            }
            temp.append("->getSubComponent(\""+portSpec.component+"\")->getSafeNodeDataPtr(\""+portSpec.port+"\", "+QString::number(varSpec.dataId)+");\n");
            setDataPtrsString.append(temp);
            ++vr;
        }
    }
    fmuHopsanSourceCode.replace("<<<nports>>>", QString::number(nReals));
    fmuHopsanSourceCode.replace("<<<setdataptrs>>>", setDataPtrsString);
    QFile fmuHopsanSourceFile(savePath+"/fmu_hopsan.c");
    if(!fmuHopsanSourceFile.open(QFile::Text | QFile::WriteOnly))
    {
        printErrorMessage("Unable to open fmu_hopsan.c for writing");
        return;
    }
    QTextStream fmuHopsanSourceStream(&fmuHopsanSourceFile);
    fmuHopsanSourceStream << fmuHopsanSourceCode;
    fmuHopsanSourceFile.close();

    //------------------------------------------------------------------//
    // Generate model file
    //------------------------------------------------------------------//

    generateModelFile(pSystem, savePath);

    //------------------------------------------------------------------//
    // Replacing namespace
    //------------------------------------------------------------------//

    replaceNameSpace(savePath);

    //------------------------------------------------------------------//
    // Compiling and linking
    //------------------------------------------------------------------//

    if(!compileAndLinkFMU(savePath, modelName, version))
    {
        return;
    }

    //------------------------------------------------------------------//
    // Sorting files
    //------------------------------------------------------------------//

    sortFiles(savePath, modelName, x64);

    //------------------------------------------------------------------//
    // Compressing files
    //------------------------------------------------------------------//

    compressFiles(savePath, modelName);

    //------------------------------------------------------------------//
    //printMessage("Cleaning up");
    //------------------------------------------------------------------//

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

void HopsanFMIGenerator::generateModelDescriptionXmlFile(ComponentSystem *pSystem, QString savePath, QString guid, int version, size_t &nReals, size_t &nInputs, size_t &nOutputs)
{
    QString versionStr = QString::number(version, 'f', 1);
    QString today = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh_mm");
    QString modelName = pSystem->getName().c_str();

    printMessage("Generating modelDescription.xml for FMI "+versionStr);

    //Write modelDescription.xml
    QDomDocument domDocument;
    QDomElement rootElement = domDocument.createElement("fmiModelDescription");
    rootElement.setAttribute("fmiVersion", versionStr);
    rootElement.setAttribute("modelName", modelName);
    if(version==1)
    {
        rootElement.setAttribute("modelIdentifier", modelName);
        rootElement.setAttribute("numberOfContinuousStates", "0");
    }
    else
    {
        rootElement.setAttribute("variableNamingConvention", "structured");
    }
    rootElement.setAttribute("guid", guid);
    rootElement.setAttribute("description", "");
    rootElement.setAttribute("generationTool", "HopsanGenerator");
    rootElement.setAttribute("generationDateAndTime", today);
    rootElement.setAttribute("numberOfEventIndicators", "0");
    domDocument.appendChild(rootElement);

    if(version==2)
    {
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
    for(int i=0; i<interfacePortSpecs.size(); ++i)
    {
        for(int j=0; j<interfacePortSpecs[i].vars.size(); ++j)
        {
            InterfacePortSpec port = interfacePortSpecs[i];
            InterfaceVarSpec var = port.vars[j];
            QDomElement varElement = domDocument.createElement("ScalarVariable");
            varElement.setAttribute("name", port.component+"_"+port.port+"_"+var.dataName);
            varElement.setAttribute("valueReference", (unsigned int)vr);
            if(var.causality == InterfaceVarSpec::Input)
            {
                varElement.setAttribute("causality", "input");
                ++nInputs;
            }
            else
            {
                if(version == 2)
                {
                    varElement.setAttribute("initial","exact");
                }
                varElement.setAttribute("causality", "output");
                ++nOutputs;
            }
            ++nReals;
            varElement.setAttribute("description", "");

            QDomElement dataElement = domDocument.createElement("Real");    //We only support real data type for now
            dataElement.setAttribute("start", 0);   //! @todo Support start values
            if(version == 1)
            {
                dataElement.setAttribute("fixed", "false");
            }
            varElement.appendChild(dataElement);

            varsElement.appendChild(varElement);
            ++vr;
        }
    }

    if(version == 1)
    {
        QDomElement implElement = domDocument.createElement("Implementation");
        QDomElement coSimElement = domDocument.createElement("CoSimulation_StandAlone");
        QDomElement capabilitiesElement = domDocument.createElement("Capabilities");
        capabilitiesElement.setAttribute("canHandleVariableCommunicationStepSize", "true");
        coSimElement.appendChild(capabilitiesElement);
        implElement.appendChild(coSimElement);
        rootElement.appendChild(implElement);
    }
    else
    {
        QDomElement structureElement = domDocument.createElement("ModelStructure");
        rootElement.appendChild(structureElement);
    }

    QDomNode xmlProcessingInstruction = domDocument.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
    domDocument.insertBefore(xmlProcessingInstruction, domDocument.firstChild());

    QFile modelDescriptionFile(savePath + "/modelDescription.xml");
    if(!modelDescriptionFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open modelDescription.xml for writing.");
        return;
    }
    QTextStream out(&modelDescriptionFile);
    domDocument.save(out, 4);
    modelDescriptionFile.close();
}

void HopsanFMIGenerator::generateModelFile(const ComponentSystem *pSystem, const QString &savePath) const
{
    printMessage("Generating model.hpp");

    QString modelName = pSystem->getName().c_str();

    QFile modelHppFile;
    modelHppFile.setFileName(savePath + "/model.hpp");
    if(!modelHppFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open model.hpp for writing.");
        return;
    }

    QStringList modelLines;
    QFile modelFile(savePath + "/" + modelName + ".hmf");
    if (!modelFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        printErrorMessage("Could not open "+modelName+".hmf for reading.");
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
}

void HopsanFMIGenerator::replaceNameSpace(const QString &savePath) const
{
    printMessage("Replacing namespace");

    int random = rand() % 1000000000;
    QString randomString = QString::number(random);
    QString nameSpace = "HopsanFMU"+randomString;
    QStringList before = QStringList() << "using namespace hopsan;" << "namespace hopsan " << "\nhopsan::" << "::hopsan::" << " hopsan::" << "*hopsan::" << "namespace hopsan{";
    QStringList after = QStringList() << "using namespace "+nameSpace+";" << "namespace "+nameSpace+" " << "\n"+nameSpace+"::" << "::"+nameSpace+"::" << " "+nameSpace+"::" << "*"+nameSpace+"::" << "namespace "+nameSpace+"{";
    Q_FOREACH(const QString &file, getHopsanCoreSourceFiles())
    {
        if(!replaceInFile(savePath+"/"+file, before, after))
            return;
    }
    Q_FOREACH(const QString &file, getHopsanCoreIncludeFiles())
    {
        if(!replaceInFile(savePath+"/"+file, before, after))
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
    if(!replaceInFile(savePath+"/fmu_hopsan.c", before, after))
        return;
    if(!replaceInFile(savePath+"/fmu_hopsan.h", before, after))
        return;
}

bool HopsanFMIGenerator::compileAndLinkFMU(const QString &savePath, const QString &modelName, int version) const
{
    QString vStr = QString::number(version);

    printMessage("------------------------------------------------------------------------");
    printMessage("Compiling C files");
    printMessage("------------------------------------------------------------------------");

#ifdef _WIN64 || __x86_64__
    QString fmiLibDir="/HopsanGenerator/Dependencies/FMILibrary-2.0.1_64/";
#else
    QString fmiLibDir="/HopsanGenerator/Dependencies/FMILibrary-2.0.1/";
#endif

#ifdef _WIN32
    QFile compileCBatchFile;
    compileCBatchFile.setFileName(savePath + "/compileC.bat");
    if(!compileCBatchFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open compile1.bat for writing.");
        return false;
    }
    printMessage("Compiling "+modelName+".dll...");
    //Write the compilation script file
    QTextStream compileBatchCStream(&compileCBatchFile);
    compileBatchCStream << mGccPath+"gcc.exe -c fmu"+vStr+"_model_cs.c " <<
                           "-I"+mHopsanRootPath+fmiLibDir+"install/include\n";
    compileCBatchFile.close();

    callProcess("cmd.exe", QStringList() << "/c" << "cd " + savePath + " & compileC.bat");
#elif linux
    QString gccCommand = "cd \""+savePath+"\" && gcc -fPIC -c fmu"+vStr+"_model_cs.c "+
                         "-I"+mHopsanRootPath+fmiLibDir+"install/include\n";
    gccCommand +=" 2>&1";
    FILE *fp;
    char line[130];
    printMessage("Compiler command: \""+gccCommand+"\"\n");

    fp = popen(  (const char *) gccCommand.toStdString().c_str(), "r");
    if ( !fp )
    {
        printErrorMessage("Could not execute '" + gccCommand + "'! err=%d\n");
        return false;
    }
    else
    {
        while ( fgets( line, sizeof line, fp))
        {
            printMessage(QString::fromUtf8(line));
        }
    }
#endif
    if(!assertFilesExist(savePath, QStringList() << "fmu"+vStr+"_model_cs.o"))
        return false;


    printMessage("------------------------------------------------------------------------");
    printMessage("Compiling C++ files");
    printMessage("------------------------------------------------------------------------");

#ifdef _WIN32
    QFile compileCppBatchFile;
    compileCppBatchFile.setFileName(savePath + "/compileCpp.bat");
    if(!compileCppBatchFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open compile1.bat for writing.");
        return false;
    }
    printMessage("Compiling "+modelName+".dll...");
    //Write the compilation script file
    QTextStream compileCppBatchStream(&compileCppBatchFile);
    compileCppBatchStream << mGccPath+"g++ -c -DDOCOREDLLEXPORT -DBUILTINDEFAULTCOMPONENTLIB " << "fmu_hopsan.c";
    Q_FOREACH(const QString &srcFile, getHopsanCoreSourceFiles())
    {
        compileCppBatchStream << " " << srcFile;
    }
    // Add HopsanCore (and necessary dependency) include paths
    Q_FOREACH(const QString &incPath, getHopsanCoreIncludePaths())
    {
       compileCppBatchStream << " -I"+incPath;
    }
    compileCppBatchFile.close();

    callProcess("cmd.exe", QStringList() << "/c" << "cd /d " + savePath + " & compileCpp.bat");
#elif linux
    QString gppCommand = "cd \""+savePath+"\" && g++ -fPIC -c -DDOCOREDLLEXPORT -DBUILTINDEFAULTCOMPONENTLIB "+"fmu_hopsan.c ";
    Q_FOREACH(const QString &srcFile, getHopsanCoreSourceFiles())
    {
        gppCommand.append(" "+srcFile);
    }
    // Add HopsanCore (and necessary dependency) include paths
    Q_FOREACH(const QString &incPath, getHopsanCoreIncludePaths())
    {
       gppCommand.append(" -I"+incPath);
    }
    gppCommand.append(" 2>&1");
    printMessage("Compiler command: \""+gppCommand+"\"\n");

    fp = popen(  (const char *) gppCommand.toStdString().c_str(), "r");
    if ( !fp )
    {
        printErrorMessage("Could not execute '" + gppCommand + "'! err=%d\n");
        return false;
    }
    else
    {
        while ( fgets( line, sizeof line, fp))
        {
            printMessage(QString::fromUtf8(line));
        }
    }
#endif
    QStringList objectFiles;
    objectFiles << "fmu_hopsan.o";
    Q_FOREACH(const QString &srcFile, getHopsanCoreSourceFiles())
    {
        QFileInfo fi(srcFile);
        objectFiles << fi.baseName()+".o";
    }

    if(!assertFilesExist(savePath, objectFiles))
        return false;

    printMessage("------------------------------------------------------------------------");
    printMessage("Linking");
    printMessage("------------------------------------------------------------------------");

#ifdef _WIN32
    QFile linkBatchFile;
    linkBatchFile.setFileName(savePath + "/link.bat");
    if(!linkBatchFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open link.bat for writing.");
        return false;
    }
    printMessage("Compiling "+modelName+".dll...");
    //Write the compilation script file
    QTextStream linkBatchStream(&linkBatchFile);
    linkBatchStream << mGccPath+"g++ -w -shared -static -static-libgcc -fPIC -Wl,--rpath,'$ORIGIN/.' " <<
                     "fmu"+vStr+"_model_cs.o";
    Q_FOREACH(const QString &objFile, objectFiles)
    {
        linkBatchStream << " " << objFile;
    }
    //! @todo should not hardcode .dll should use define from Common.prf
    linkBatchStream << "-o "+modelName+".dll\n";
    linkBatchFile.close();

    callProcess("cmd.exe", QStringList() << "/c" << "cd /d " + savePath + " & link.bat");

    if(!assertFilesExist(savePath, QStringList() << modelName+".dll"))
        return false;
#elif linux
    QString linkCommand = "cd \""+savePath+"\" && g++ -fPIC  -w -shared -static-libgcc -Wl,--rpath,'$ORIGIN/.' "
            "fmu"+vStr+"_model_cs.o";
    Q_FOREACH(const QString &objFile, objectFiles)
    {
        linkCommand.append(" "+objFile);
    }
    linkCommand.append("-o "+modelName+".so");
    linkCommand.append(" 2>&1");
    printMessage("Compiler command: \""+linkCommand+"\"\n");

    fp = popen(  (const char *) linkCommand.toStdString().c_str(), "r");
    if ( !fp )
    {
        printErrorMessage("Could not execute '" + linkCommand + "'! err=%d\n");
        return false;
    }
    else
    {
        while ( fgets( line, sizeof line, fp))
        {
            printMessage(QString::fromUtf8(line));
        }
    }

    return assertFilesExist(savePath, QStringList() << modelName+".so");

#endif
}

void HopsanFMIGenerator::sortFiles(const QString &savePath, const QString &modelName, bool x64) const
{
    printMessage("Sorting files");

    QDir saveDir(savePath);
#ifdef _WIN32
    if(x64)
    {
        saveDir.mkpath("fmu/binaries/win64");
        QFile modelDllFile(savePath + "/" + modelName + ".dll");
        modelDllFile.copy(savePath + "/fmu/binaries/win64/" + modelName + ".dll");
    }
    else
    {
        saveDir.mkpath("fmu/binaries/win32");
        QFile modelDllFile(savePath + "/" + modelName + ".dll");
        modelDllFile.copy(savePath + "/fmu/binaries/win32/" + modelName + ".dll");
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
    QFile modelDescriptionFile(savePath+"/modelDescription.xml");
    modelDescriptionFile.copy(savePath + "/fmu/modelDescription.xml");
    //tlmDescriptionFile.copy(savePath+"/fmu/"+modelName+"_TLM.xml");
}

void HopsanFMIGenerator::compressFiles(const QString &savePath, const QString &modelName) const
{
    printMessage("Compressing files");

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

