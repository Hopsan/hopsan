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

#include "generators/HopsanSimulinkGenerator.h"
#include "GeneratorUtilities.h"
#include "ComponentSystem.h"
#include "Component.h"
#include <cassert>

#include <QDir>


HopsanSimulinkGenerator::HopsanSimulinkGenerator(const QString &hopsanInstallPath)
    : HopsanGeneratorBase(hopsanInstallPath, {}, "")
{

}


bool HopsanSimulinkGenerator::generateToSimulink(QString savePath, QString modelFile, hopsan::ComponentSystem *pSystem, const QStringList& externalLibraries,  bool disablePortLabels)
{
    printMessage("Initializing Simulink S-function export...");

    if(pSystem == nullptr)
    {
        printErrorMessage("System pointer is null. Aborting.");
        return false;
    }

    QDir saveDir;
    saveDir.setPath(savePath);

    QString name = pSystem->getName().c_str();
    if(name.isEmpty())
    {
        name = "UnsavedHopsanModel";
    }

    if(modelFile.isEmpty())
    {
        modelFile = "untitled.hmf";
    }

    printMessage("Copying necessary files...");
    if(!copyHopsanCoreSourceFilesToDir(savePath)) {
        printErrorMessage("Failed to copy Hopsan source code files.");
        return false;
    }
    if(!copyDefaultComponentCodeToDir(savePath)) {
        printErrorMessage("Failed to copy default component library files.");
        return false;
    }
    QStringList extraSourceFiles;
    if(!copyExternalComponentCodeToDir(savePath, externalLibraries, extraSourceFiles)) {
        printErrorMessage("Failed to export required external component library files.");
        return false;
    }

    const std::vector<hopsan::ParameterEvaluator*> *pParameters = pSystem->getParametersVectorPtr();
    int numParameters = pParameters->size();

    QList<InterfacePortSpec> interfacePortSpecs;
    QStringList path = QStringList();
    getInterfaces(interfacePortSpecs, pSystem, path);

    printMessage("Generating files...");

    QFile wrapperFile;
    wrapperFile.setFileName(savePath + "/"+name+".cpp");
    if(!wrapperFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open "+name+".cpp for writing.");
        return false;
    }

    QFile maskSetupFile;
    maskSetupFile.setFileName(savePath + "/"+name+"MaskSetup.m");
    if(!maskSetupFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open "+name+"MaskSetup.m for writing.");
        return false;
    }

    printMessage("Writing HopsanSimulinkCompile.m...");
    QFile compileScriptFile;
    compileScriptFile.setFileName(savePath + "/HopsanSimulinkCompile.m");
    if(!compileScriptFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open HopsanSimulinkCompile.m for writing.");
        return false;
    }
    QTextStream compileScriptStream(&compileScriptFile);
    QTextLineStream compileScriptLStream(compileScriptStream);

    compileScriptLStream << "disp('Compiling S-function from Hopsan model...');";
    compileScriptStream << "mex -DHOPSAN_INTERNALDEFAULTCOMPONENTS -DHOPSAN_INTERNAL_EXTRACOMPONENTS -D_USE_MATH_DEFINES";
#ifdef _WIN32
    compileScriptStream << " -DWIN32";      //!< @todo Not sure if this one is needed, the correct macro to check for is _WIN32
#endif
    Q_FOREACH(const QString &s, getHopsanCoreIncludePaths())
    {
        compileScriptStream << QString(" -I%1").arg(s);
    }
    QStringList coreSourcefiles = listHopsanCoreSourceFiles(savePath)+listInternalLibrarySourceFiles(savePath);
    Q_FOREACH(const QString &s, coreSourcefiles)
    {
        compileScriptStream << " " << s;
    }
    Q_FOREACH(const QString &s, extraSourceFiles)
    {
        compileScriptStream << " " << s;
    }
    compileScriptStream  << QString(" %1.cpp ").arg(name);
#ifndef _WIN32
    compileScriptStream << "-ldl ";
#endif
    compileScriptLStream << QString("-output %1").arg(name);

    compileScriptLStream << "disp('Finished.')";

    compileScriptFile.close();


    printMessage("Writing "+name+"MaskSetup.m...");

    QString portLabels, maskPrompts, maskVars, defaultVals;
    QTextStream maskStream(&portLabels);
    int i=1, o=1;
    foreach(const InterfacePortSpec &spec, interfacePortSpecs)
    {
        foreach(const InterfaceVarSpec &varSpec, spec.vars)
        {
            QString temp = "."+varSpec.dataName;
            if(varSpec.causality == InterfaceVarSpec::Input)
            {
                maskStream << "port_label(''input''," << i << ",''" << spec.component << temp+"''); ";
                ++i;
            }
            else
            {
                maskStream << "port_label(''output''," << o << ",''" << spec.component << temp+"''); ";
                ++o;
            }
        }
    }
    maskStream << "port_label(''output''," << o << ",''DEBUG'')";

    maskStream.setString(&maskPrompts);
    for(int p=0; p<numParameters; ++p)
    {
        maskStream << QString("'%1'").arg(pParameters->at(p)->getName().c_str());
        if(p<numParameters-1)
            maskStream << ",";
    }

    maskStream.setString(&maskVars);
    for(int p=0; p<numParameters; ++p)
    {
        maskStream << QString("%1=&%2;").arg(pParameters->at(p)->getName().c_str()).arg(p+1);
    }

    maskStream.setString(&defaultVals);
    for(int p=0; p<numParameters; ++p)
    {
        maskStream << QString("'%1'").arg(pParameters->at(p)->getValue().c_str());
        if(p<numParameters-1)
            maskStream << ",";
    }

    QString maskSetup;
    maskSetup.append("set_param(gcb,'Mask','on')\n");
    maskSetup.append(QString("set_param(gcb,'Name','%1')\n").arg(name));
    maskSetup.append("set_param(gcb,'BackgroundColor','[0.721569, 0.858824, 0.905882]')\n");
    maskSetup.append(QString("set_param(gcb,'MaskDisplay','%1');\n").arg(portLabels));
    maskSetup.append(QString("set_param(gcb,'MaskPrompts',{%1})\n").arg(maskPrompts));
    maskSetup.append(QString("set_param(gcb,'MaskVariables','%1')\n").arg(maskVars));
    maskSetup.append(QString("defaultVals = {%1};\n").arg(defaultVals));
    maskSetup.append(QString("vals = get_param(gcb,'MaskValues')';\n"));
    maskSetup.append(QString("for i=1:length(vals)\n"));
    maskSetup.append(QString("    if strcmp(vals{i}, '[]')\n"));
    maskSetup.append(QString("        vals{i} = '';\n"));
    maskSetup.append(QString("    end\n"));
    maskSetup.append(QString("    if isempty(vals{i})\n"));
    maskSetup.append(QString("        vals{i} = defaultVals{i};\n"));
    maskSetup.append(QString("    end\n"));
    maskSetup.append(QString("end\n"));
    maskSetup.append(QString("set_param(gcb,'MaskValues', vals)\n"));

    QTextStream maskFileStream(&maskSetupFile);
    maskFileStream << maskSetup;
    maskSetupFile.close();


    printMessage("Writing "+name+".cpp...");

    QFile wrapperTemplateFile(":templates/simulinkWrapperTemplate.cpp");
    assert(wrapperTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QString wrapperCode;
    QTextStream t2(&wrapperTemplateFile);
    wrapperCode = t2.readAll();
    wrapperTemplateFile.close();
    assert(!wrapperCode.isEmpty());

    QString wrapperReplace1;
    QString wrapperReplace3;
    i=0;
    o=0;

    foreach(const InterfacePortSpec &spec, interfacePortSpecs)
    {
        foreach(const InterfaceVarSpec &varSpec, spec.vars)
        {
            if(varSpec.causality == InterfaceVarSpec::Input)
            {
                wrapperReplace1.append("    ssSetInputPortWidth(S, " + QString::number(i) + ", DYNAMICALLY_SIZED);		//Input signal " + QString::number(i) + "\n");
                wrapperReplace1.append("    ssSetInputPortDirectFeedThrough(S, " + QString::number(i) + ", 1);\n");
                ++i;
            }
            else
            {
                wrapperReplace3.append("    ssSetOutputPortWidth(S, " + QString::number(o) + ", DYNAMICALLY_SIZED);		//Output signal " + QString::number(o) + "\n");
                ++o;
            }
        }
    }

    QString wrapperReplace5;
    if(!disablePortLabels)
    {
        wrapperReplace5 = "    mexCallMATLAB(0, 0, 0, 0, \""+name+"MaskSetup\");                              //Run the port label script\n";
    }

    QString wrapperReplace6;
    for(int p=0; p<numParameters; ++p)
    {
        QString parname = pParameters->at(p)->getName().c_str();
        //wrapperReplace6.append("in = ssGetSFcnParam(S, "+QString::number(p)+");\n");
        wrapperReplace6.append("in = mexGetVariable(\"caller\", \""+parname+"\");\n");
        wrapperReplace6.append("if(in == NULL )\n");
        wrapperReplace6.append("{\n");
        wrapperReplace6.append("    mexErrMsgTxt(\"Unable to read parameter \\\""+parname+"\\\"!\");\n");
        wrapperReplace6.append("	return;\n");
        wrapperReplace6.append("}\n");
        wrapperReplace6.append("parsize = mxGetN(in);\n");
        wrapperReplace6.append("pBuffer = realloc(pBuffer, parsize+1);\n");
        wrapperReplace6.append("mxGetString(in, (char*)pBuffer, parsize+1);\n");
        wrapperReplace6.append("valstr.setString((const char*)pBuffer);\n");
        wrapperReplace6.append("pComponentSystem->setParameterValue(\""+parname+"\", valstr);\n");
        wrapperReplace6.append("\n");
    }

    QString wrapperReplace7;
    QString wrapperReplace8;
    QString wrapperReplace9;
    QString wrapperReplace11;
    QString wrapperReplace12;
    QString wrapperReplace13;
    QString wrapperReplace15;
    QString wrapperReplace16;
    i=0;
    o=0;
    foreach(const InterfacePortSpec &portSpec, interfacePortSpecs)
    {
        QList<InterfaceVarSpec> varSpecs = portSpec.vars;

        QString path;
        foreach(const QString &subsystem, portSpec.path)
        {
            path.append("->getSubComponentSystem(\""+subsystem+"\")");
        }

        foreach(const InterfaceVarSpec &varSpec, varSpecs)
        {
            if(varSpec.causality == InterfaceVarSpec::Input)
            {
                wrapperReplace8.append("    double input" + QString::number(o) + " = (*uPtrs1[" + QString::number(o) + "]);\n");
                wrapperReplace11.append("        (*pInputNode"+QString::number(o)+") = input"+QString::number(o)+";\n");
                wrapperReplace15.append("double *pInputNode"+QString::number(o)+";\n");
                wrapperReplace16.append("    pInputNode"+QString::number(o)+" = pComponentSystem"+path+"->getSubComponent(\""+portSpec.component+"\")->getSafeNodeDataPtr(\""+portSpec.port+"\", "+QString::number(varSpec.dataId)+");\n");
                ++o;
            }
            else
            {
                wrapperReplace7.append("    real_T *y" + QString::number(i) + " = ssGetOutputPortRealSignal(S," + QString::number(i) + ");\n");
                wrapperReplace9.append("    double output" + QString::number(i) + ";\n");
                wrapperReplace12.append("        output"+QString::number(i)+" = (*pOutputNode"+QString::number(i)+");\n");
                wrapperReplace13.append("    *y" + QString::number(i) + " = output" + QString::number(i) + ";\n");
                wrapperReplace15.append("double *pOutputNode"+QString::number(i)+";\n");
                wrapperReplace16.append("    pOutputNode"+QString::number(i)+" = pComponentSystem"+path+"->getSubComponent(\""+portSpec.component+"\")->getSafeNodeDataPtr(\""+portSpec.port+"\", "+QString::number(varSpec.dataId)+");\n");
                ++i;
            }
        }
    }

    //Debug port
    wrapperReplace7.append("    real_T *y" + QString::number(i) + " = ssGetOutputPortRealSignal(S," + QString::number(i) + ");\n");
    wrapperReplace9.append("    double output" + QString::number(i) + ";\n");
    wrapperReplace13.append("    *y" + QString::number(i) + " = output" + QString::number(i) + ";\n");
    ++i;

    int nTotalInputs = o;
    int nTotalOutputs = i;
    QString nTotalInputsString = QString::number(nTotalInputs);
    QString nTotalOutputsString = QString::number(nTotalOutputs);

    savePath.replace("\\", "\\\\");

    wrapperCode.replace("<<<0>>>", nTotalInputsString);
    wrapperCode.replace("<<<1>>>", wrapperReplace1);
    wrapperCode.replace("<<<2>>>", nTotalOutputsString);
    wrapperCode.replace("<<<3>>>", wrapperReplace3);
    wrapperCode.replace("<<<14>>>", QString::number(nTotalOutputs-1));
    wrapperCode.replace("<<<4>>>", modelFile);
    wrapperCode.replace("<<<5>>>", wrapperReplace5);
    replacePattern("<<<6>>>", wrapperReplace6, wrapperCode);
    wrapperCode.replace("<<<7>>>", wrapperReplace7);
    wrapperCode.replace("<<<8>>>", wrapperReplace8);
    wrapperCode.replace("<<<9>>>", wrapperReplace9);
    wrapperCode.replace("<<<10>>>", QString::number(nTotalOutputs-1));
    wrapperCode.replace("<<<11>>>", wrapperReplace11);
    wrapperCode.replace("<<<12>>>", wrapperReplace12);
    wrapperCode.replace("<<<13>>>", wrapperReplace13);
    wrapperCode.replace("<<<15>>>", wrapperReplace15);
    wrapperCode.replace("<<<16>>>", wrapperReplace16);
    wrapperCode.replace("<<<name>>>", name);
    wrapperCode.replace("<<<timestep>>>", QString::number(pSystem->getDesiredTimeStep()));
    wrapperCode.replace("<<<numparams>>>", QString::number(numParameters));

    QTextStream wrapperStream(&wrapperFile);
    wrapperStream << wrapperCode;
    wrapperFile.close();

    if(!assertFilesExist(savePath, QStringList() << modelFile <<  name+".cpp" <<
                     "HopsanSimulinkCompile.m" << name+"MaskSetup.m"))
    {
        return false;
    }

    printMessage("Finished.");
    return true;
}



void HopsanSimulinkGenerator::generateToSimulinkCoSim(QString savePath, hopsan::ComponentSystem *pSystem, bool disablePortLabels, int compiler)
{
    Q_UNUSED(compiler);

    printMessage("Initializing Simulink Co-Simulation Export");

    QString name = pSystem->getName().c_str();
    if(name.isEmpty())
    {
        name = "UnsavedHopsanModel";
    }

    QDir saveDir;
    saveDir.setPath(savePath);


    std::vector<hopsan::HString> parameterNames;
    pSystem->getParameterNames(parameterNames);
    QStringList tunableParameters;
    for(size_t i=0; i<parameterNames.size(); ++i)
    {
        tunableParameters.append(QString(parameterNames[i].c_str()));
    }
    QStringList inputComponents;
    QStringList inputPorts;
    QStringList outputComponents;
    QStringList outputPorts;
    QStringList mechanicQComponents;
    QStringList mechanicQPorts;
    QStringList mechanicCComponents;
    QStringList mechanicCPorts;
    QStringList mechanicRotationalQComponents;
    QStringList mechanicRotationalQPorts;
    QStringList mechanicRotationalCComponents;
    QStringList mechanicRotationalCPorts;

    std::vector<hopsan::HString> names = pSystem->getSubComponentNames();
    for(size_t i=0; i<names.size(); ++i)
    {
        hopsan::Component *pComponent = pSystem->getSubComponent(names[i]);
        if(pComponent->getTypeName() == "SignalInputInterface")
        {
            inputComponents.append(names[i].c_str());
            inputPorts.append("out");
        }
        else if(pComponent->getTypeName() == "SignalOutputInterface")
        {
            outputComponents.append(names[i].c_str());
            outputPorts.append("in");
        }
        else if(pComponent->getTypeName() == "MechanicInterfaceQ")
        {
            mechanicQComponents.append(names[i].c_str());
            mechanicQPorts.append("P1");
        }
        else if(pComponent->getTypeName() == "MechanicInterfaceC")
        {
            mechanicCComponents.append(names[i].c_str());
            mechanicCPorts.append("P1");
        }
        else if(pComponent->getTypeName() == "MechanicRotationalInterfaceQ")
        {
            mechanicRotationalQComponents.append(names[i].c_str());
            mechanicRotationalQPorts.append("P1");
        }
        else if(pComponent->getTypeName() == "MechanicRotationalInterfaceC")
        {
            mechanicRotationalCComponents.append(names[i].c_str());
            mechanicRotationalCPorts.append("P1");
        }
        //! @todo what about pneumatic and electric nodes
        //! @todo this should not be hardcoded
    }

    int nInputs = inputComponents.size();
    QString nInputsString;
    nInputsString.setNum(nInputs);

    int nOutputs = outputComponents.size();
    QString nOutputsString;
    nOutputsString.setNum(nOutputs);

    int nMechanicQ = mechanicQComponents.size();
    QString nMechanicQString;
    nMechanicQString.setNum(nMechanicQ);

    int nMechanicC = mechanicCComponents.size();
    QString nMechanicCString;
    nMechanicCString.setNum(nMechanicC);

    int nMechanicRotationalQ = mechanicRotationalQComponents.size();
    QString nMechanicRotationalQString;
    nMechanicRotationalQString.setNum(nMechanicRotationalQ);

    int nMechanicRotationalC = mechanicRotationalCComponents.size();
    QString nMechanicRotationalCString;
    nMechanicRotationalCString.setNum(nMechanicRotationalC);

    int nTotalInputs = nInputs+nMechanicQ*2+nMechanicC*2+nMechanicRotationalQ*2+nMechanicRotationalC*2;
    QString nTotalInputsString;
    nTotalInputsString.setNum(nTotalInputs);

    int nTotalOutputs = nOutputs+nMechanicQ*2+nMechanicC*2+nMechanicRotationalQ*2+nMechanicRotationalC*2+1;
    QString nTotalOutputsString;
    nTotalOutputsString.setNum(nTotalOutputs);


    printMessage("Generating files");


    QFile wrapperFile;
    wrapperFile.setFileName(savePath + "/HopsanSimulink.cpp");
    if(!wrapperFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open HopsanSimulink.cpp for writing.");
        return;
    }

    QFile portLabelsFile;
    portLabelsFile.setFileName(savePath + "/"+name+"PortLabels.m");
    if(!portLabelsFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open "+name+"PortLabels.m for writing.");
        return;
    }


    QFile compileFile;
    compileFile.setFileName(savePath + "/HopsanSimulinkCompile.m");
    if(!compileFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open HopsanSimulinkCompile.m for writing.");
        return;
    }


    printMessage("Writing "+name+"PortLabels.m");


    QTextStream portLabelsStream(&portLabelsFile);
    portLabelsStream << "set_param(gcb,'Mask','on')\n";
    portLabelsStream << "set_param(gcb,'MaskDisplay','";
    int i,j;
    int tot=0;
    for(i=0; i<nMechanicQ; ++i)
    {
        j=tot+i*2;
        portLabelsStream << "port_label(''input''," << j+1 << ",''" << mechanicQComponents.at(i) << ".x''); ";
        portLabelsStream << "port_label(''input''," << j+2 << ",''" << mechanicQComponents.at(i) << ".v''); ";
    }
    tot+=nMechanicQ*2;
    for(i=0; i<nMechanicC; ++i)
    {
        j=tot+i*2;
        portLabelsStream << "port_label(''input''," << j+1 << ",''" << mechanicCComponents.at(i) << ".cx''); ";
        portLabelsStream << "port_label(''input''," << j+2 << ",''" << mechanicCComponents.at(i) << ".Zx''); ";
    }
    tot+=nMechanicC*2;
    for(i=0; i<nMechanicRotationalQ; ++i)
    {
        j=tot+i*2;
        portLabelsStream << "port_label(''input''," << j+1 << ",''" << mechanicRotationalQComponents.at(i) << ".a''); ";
        portLabelsStream << "port_label(''input''," << j+2 << ",''" << mechanicRotationalQComponents.at(i) << ".w''); ";
    }
    tot+=nMechanicRotationalQ*2;
    for(i=0; i<nMechanicRotationalC; ++i)
    {
        j=tot+i*2;
        portLabelsStream << "port_label(''input''," << j+1 << ",''" << mechanicRotationalCComponents.at(i) << ".cx''); ";
        portLabelsStream << "port_label(''input''," << j+2 << ",''" << mechanicRotationalCComponents.at(i) << ".Zx''); ";
    }
    tot+=nMechanicRotationalC*2;
    for(i=0; i<nInputs; ++i)
    {
        j=tot+i;
        portLabelsStream << "port_label(''input''," << j+1 << ",''" << inputComponents.at(i) << "''); ";
    }

    tot=0;
    for(i=0; i<nMechanicQ; ++i)
    {
        j=tot+i*2;
        portLabelsStream << "port_label(''output''," << j+1 << ",''" << mechanicQComponents.at(i) << ".cx''); ";
        portLabelsStream << "port_label(''output''," << j+2 << ",''" << mechanicQComponents.at(i) << ".Zx''); ";
    }
    tot+=nMechanicQ*2;
    for(i=0; i<nMechanicC; ++i)
    {
        j=tot+i*2;
        portLabelsStream << "port_label(''output''," << j+1 << ",''" << mechanicCComponents.at(i) << ".x''); ";
        portLabelsStream << "port_label(''output''," << j+2 << ",''" << mechanicCComponents.at(i) << ".v''); ";
    }
    tot+=nMechanicC*2;
    for(i=0; i<nMechanicRotationalQ; ++i)
    {
        j=tot+i*2;
        portLabelsStream << "port_label(''output''," << j+1 << ",''" << mechanicRotationalQComponents.at(i) << ".Zx''); ";
        portLabelsStream << "port_label(''output''," << j+2 << ",''" << mechanicRotationalQComponents.at(i) << ".cx''); ";
    }
    tot+=nMechanicRotationalQ*2;
    for(i=0; i<nMechanicRotationalC; ++i)
    {
        j=tot+i*2;
        portLabelsStream << "port_label(''output''," << j+1 << ",''" << mechanicRotationalCComponents.at(i) << ".a''); ";
        portLabelsStream << "port_label(''output''," << j+2 << ",''" << mechanicRotationalCComponents.at(i) << ".w''); ";
    }
    tot+=nMechanicRotationalC*2;
    for(i=0; i<nOutputs; ++i)
    {
        j=tot+i;
        portLabelsStream << "port_label(''output''," << j+1 << ",''" << outputComponents.at(i) << "''); ";
    }
    j=nTotalOutputs-1;
    portLabelsStream << "port_label(''output''," << j+1 << ",''DEBUG'')'); \n";
    portLabelsStream << "set_param(gcb,'BackgroundColor','[0.721569, 0.858824, 0.905882]')\n";
    portLabelsStream << "set_param(gcb,'Name','" << name << "')\n";
    portLabelsStream << "set_param(gcb,'MaskPrompts',{";
    for(int p=0; p<tunableParameters.size(); ++p)
    {
        portLabelsStream << "'"+tunableParameters[p]+"'";
        if(p<tunableParameters.size()-1)
            portLabelsStream << ",";
    }
    portLabelsStream << "})\n";
    portLabelsStream << "set_param(gcb,'MaskVariables','";
    for(int p=0; p<tunableParameters.size(); ++p)
    {
        portLabelsStream << tunableParameters[p]+"=&"+QString::number(p+1)+";";
    }
    portLabelsStream << "')\n";
    portLabelsFile.close();



    printMessage("Writing HopsanSimulink.cpp");


    //How to access dialog parameters:
    //double par1 = (*mxGetPr(ssGetSFcnParam(S, 0)));

    QFile wrapperTemplateFile(":templates/simulinkCoSimWrapperTemplate.cpp");
    assert(wrapperTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QString wrapperCode;
    QTextStream t(&wrapperTemplateFile);
    wrapperCode = t.readAll();
    wrapperTemplateFile.close();
    assert(!wrapperCode.isEmpty());

    QString wrapperReplace0;
    for(int i=0; i<nTotalInputs; ++i)
    {
        wrapperReplace0.append("double *in"+QString::number(i)+"_socket;\n");
    }
    for(int i=0; i<nTotalOutputs-1; ++i)
    {
        wrapperReplace0.append("double *out"+QString::number(i)+"_socket;\n");
    }

    QString wrapperReplace2;
    tot=0;
    for(i=0; i<nMechanicQ; ++i)
    {
        j=tot+i*2;
        wrapperReplace2.append("    ssSetInputPortWidth(S, " + QString::number(j) + ", DYNAMICALLY_SIZED);		//Input signal " + QString::number(j) + "\n");
        wrapperReplace2.append("    ssSetInputPortDirectFeedThrough(S, " + QString::number(j) + ", 1);\n");
        wrapperReplace2.append("    ssSetInputPortWidth(S, " + QString::number(j+1) + ", DYNAMICALLY_SIZED);		//Input signal " + QString::number(j+1) + "\n");
        wrapperReplace2.append("    ssSetInputPortDirectFeedThrough(S, " + QString::number(j+1) + ", 1);\n");
    }
    tot+=nMechanicQ*2;
    for(i=0; i<nMechanicC; ++i)
    {
        j=tot+i*2;
        wrapperReplace2.append("    ssSetInputPortWidth(S, " + QString::number(j) + ", DYNAMICALLY_SIZED);		//Input signal " + QString::number(j) + "\n");
        wrapperReplace2.append("    ssSetInputPortDirectFeedThrough(S, " + QString::number(j) + ", 1);\n");
        wrapperReplace2.append("    ssSetInputPortWidth(S, " + QString::number(j+1) + ", DYNAMICALLY_SIZED);		//Input signal " + QString::number(j+1) + "\n");
        wrapperReplace2.append("    ssSetInputPortDirectFeedThrough(S, " + QString::number(j+1) + ", 1);\n");
    }
    tot+=nMechanicC*2;
    for(i=0; i<nMechanicRotationalQ; ++i)
    {
        j=tot+i*2;
        wrapperReplace2.append("    ssSetInputPortWidth(S, " + QString::number(j) + ", DYNAMICALLY_SIZED);		//Input signal " + QString::number(j) + "\n");
        wrapperReplace2.append("    ssSetInputPortDirectFeedThrough(S, " + QString::number(j) + ", 1);\n");
        wrapperReplace2.append("    ssSetInputPortWidth(S, " + QString::number(j+1) + ", DYNAMICALLY_SIZED);		//Input signal " + QString::number(j+1) + "\n");
        wrapperReplace2.append("    ssSetInputPortDirectFeedThrough(S, " + QString::number(j+1) + ", 1);\n");
    }
    tot+=nMechanicRotationalQ*2;
    for(i=0; i<nMechanicRotationalC; ++i)
    {
        j=tot+i*2;
        wrapperReplace2.append("    ssSetInputPortWidth(S, " + QString::number(j) + ", DYNAMICALLY_SIZED);		//Input signal " + QString::number(j) + "\n");
        wrapperReplace2.append("    ssSetInputPortDirectFeedThrough(S, " + QString::number(j) + ", 1);\n");
        wrapperReplace2.append("    ssSetInputPortWidth(S, " + QString::number(j+1) + ", DYNAMICALLY_SIZED);		//Input signal " + QString::number(j+1) + "\n");
        wrapperReplace2.append("    ssSetInputPortDirectFeedThrough(S, " + QString::number(j+1) + ", 1);\n");
    }
    tot+=nMechanicRotationalC*2;
    for(i=0; i<nInputs; ++i)
    {
        j=tot+i;
        wrapperReplace2.append("    ssSetInputPortWidth(S, " + QString::number(j) + ", DYNAMICALLY_SIZED);		//Input signal " + QString::number(j) + "\n");
        wrapperReplace2.append("    ssSetInputPortDirectFeedThrough(S, " + QString::number(j) + ", 1);\n");
    }

    QString wrapperReplace4;
    tot=0;
    for(i=0; i<nMechanicQ; ++i)
    {
        j=tot+i*2;
        wrapperReplace4.append("    ssSetOutputPortWidth(S, " + QString::number(j) + ", DYNAMICALLY_SIZED);		//Output signal " + QString::number(j) + "\n");
        wrapperReplace4.append("    ssSetOutputPortWidth(S, " + QString::number(j+1) + ", DYNAMICALLY_SIZED);		//Output signal " + QString::number(j+1) + "\n");
    }
    tot+=nMechanicQ*2;
    for(i=0; i<nMechanicC; ++i)
    {
        j=tot+i*2;
        wrapperReplace4.append("    ssSetOutputPortWidth(S, " + QString::number(j) + ", DYNAMICALLY_SIZED);		//Output signal " + QString::number(j) + "\n");
        wrapperReplace4.append("    ssSetOutputPortWidth(S, " + QString::number(j+1) + ", DYNAMICALLY_SIZED);		//Output signal " + QString::number(j+1) + "\n");
    }
    tot+=nMechanicC*2;
    for(i=0; i<nMechanicRotationalQ; ++i)
    {
        j=tot+i*2;
        wrapperReplace4.append("    ssSetOutputPortWidth(S, " + QString::number(j) + ", DYNAMICALLY_SIZED);		//Output signal " + QString::number(j) + "\n");
        wrapperReplace4.append("    ssSetOutputPortWidth(S, " + QString::number(j+1) + ", DYNAMICALLY_SIZED);		//Output signal " + QString::number(j+1) + "\n");
    }
    tot+=nMechanicRotationalQ*2;
    for(i=0; i<nMechanicRotationalC; ++i)
    {
        j=tot+i*2;
        wrapperReplace4.append("    ssSetOutputPortWidth(S, " + QString::number(j) + ", DYNAMICALLY_SIZED);		//Output signal " + QString::number(j) + "\n");
        wrapperReplace4.append("    ssSetOutputPortWidth(S, " + QString::number(j+1) + ", DYNAMICALLY_SIZED);		//Output signal " + QString::number(j+1) + "\n");
    }
    tot+=nMechanicRotationalC*2;
    for(i=0; i<nOutputs; ++i)
    {
        j=tot+i;
        wrapperReplace4.append("    ssSetOutputPortWidth(S, " + QString::number(j) + ", DYNAMICALLY_SIZED);		//Output signal " + QString::number(j) + "\n");
    }
    j=nTotalOutputs-1;

    QString wrapperReplace6;
    if(!disablePortLabels)
    {
        wrapperReplace6 = "    mexCallMATLAB(0, 0, 0, 0, \""+name+"PortLabels\");                              //Run the port label script";
    }

    QFile socketDeclarationTemplateFile(":templates/simulinkSocketDeclarationTemplate.cpp");
    assert(socketDeclarationTemplateFile.open(QIODevice::ReadOnly | QIODevice::Text));
    QString socketDeclarationCode;
    QTextStream t2(&socketDeclarationTemplateFile);
    socketDeclarationCode = t2.readAll();
    socketDeclarationTemplateFile.close();
    assert(!socketDeclarationCode.isEmpty());

    QString wrapperReplace7;
    for(int i=0; i<nTotalInputs; ++i)
    {
        QString replacement = socketDeclarationCode;
        replacement.replace("<<<name>>>", "in"+QString::number(i));
        replacement.replace("<<<type>>>", "double");
        wrapperReplace7.append(replacement);
    }

    QString wrapperReplace8;
    for(int i=0; i<nTotalOutputs-1; ++i)
    {
        QString replacement = socketDeclarationCode;
        replacement.replace("<<<name>>>", "out"+QString::number(i));
        replacement.replace("<<<type>>>", "double");
        wrapperReplace8.append(replacement);
    }

    QString wrapperReplace9;
    QString replacement = socketDeclarationCode;
    replacement.replace("<<<name>>>", "sim");
    replacement.replace("<<<type>>>", "bool");
    wrapperReplace9.append(replacement);

    QString wrapperReplace10;
    replacement = socketDeclarationCode;
    replacement.replace("<<<name>>>", "stop");
    replacement.replace("<<<type>>>", "bool");
    wrapperReplace10.append(replacement);

    QString wrapperReplace11;
    for(int i=0; i<nTotalOutputs; ++i)
    {
        wrapperReplace11.append("    real_T *y" + QString::number(i) + " = ssGetOutputPortRealSignal(S," + QString::number(i) + ");\n");
    }

    QString wrapperReplace12;
    for(int i=0; i<nTotalInputs; ++i)
    {
        wrapperReplace11.append("    double input" + QString::number(i) + " = (*uPtrs1[" + QString::number(i) + "]);\n");
    }

    QString wrapperReplace13;
    for(int i=0; i<nTotalOutputs; ++i)
    {
        wrapperReplace13.append("    double output" + QString::number(i) + ";\n");
    }

    QString wrapperReplace15;
    for(int i=0; i<nTotalInputs; ++i)
    {
        wrapperReplace15.append("    (*in"+QString::number(i)+"_socket) = input"+QString::number(i)+";\n");
    }

    QString wrapperReplace16;
    for(int i=0; i<nTotalOutputs-1; ++i)
    {
        wrapperReplace16.append("    output"+QString::number(i)+" = (*out"+QString::number(i)+"_socket);\n");
    }

    QString wrapperReplace17;
    for(int i=0; i<nTotalOutputs; ++i)
    {
        wrapperReplace17.append("    *y" + QString::number(i) + " = output" + QString::number(i) + ";\n");
    }

    QString wrapperReplace18;
    for(int i=0; i<nTotalInputs; ++i)
    {
        wrapperReplace18.append("    boost::interprocess::shared_memory_object::remove(\"hopsan_in"+QString::number(i)+"\");\n");
    }
    for(int i=0; i<nTotalOutputs-1; ++i)
    {
        wrapperReplace18.append("    boost::interprocess::shared_memory_object::remove(\"hopsan_out"+QString::number(i)+"\");\n");
    }

    wrapperCode.replace("<<<0>>>", wrapperReplace0);
    wrapperCode.replace("<<<1>>>", nTotalInputsString);
    wrapperCode.replace("<<<2>>>", wrapperReplace2);
    wrapperCode.replace("<<<3>>>", nTotalOutputsString);
    wrapperCode.replace("<<<4>>>", wrapperReplace4);
    wrapperCode.replace("<<<5>>>", QString::number(nTotalOutputs-1));
    wrapperCode.replace("<<<6>>>", wrapperReplace6);
    wrapperCode.replace("<<<7>>>", wrapperReplace7);
    wrapperCode.replace("<<<8>>>", wrapperReplace8);
    wrapperCode.replace("<<<9>>>", wrapperReplace9);
    wrapperCode.replace("<<<10>>>", wrapperReplace10);
    wrapperCode.replace("<<<11>>>", wrapperReplace11);
    wrapperCode.replace("<<<12>>>", wrapperReplace12);
    wrapperCode.replace("<<<13>>>", wrapperReplace13);
    wrapperCode.replace("<<<14>>>", QString::number(nTotalOutputs-1));
    wrapperCode.replace("<<<15>>>", wrapperReplace15);
    wrapperCode.replace("<<<16>>>", wrapperReplace16);
    wrapperCode.replace("<<<17>>>", wrapperReplace17);
    wrapperCode.replace("<<<18>>>", wrapperReplace18);

    QTextStream wrapperStream(&wrapperFile);
    wrapperStream << wrapperCode;
    wrapperFile.close();


    printMessage("Writing HopsanSimulinkCompile.m");


    QTextStream compileStream(&compileFile);
#ifdef _WIN32
    compileStream << "mex -DWIN32 -D_USE_MATH_DEFINES -L./ -I./include -I./include/boost HopsanSimulink.cpp\n";
#else
    compileStream << "mex -D_USE_MATH_DEFINES -L./ -Iinclude -Iinclude/boost HopsanSimulink.cpp" << endl;
#endif
    compileFile.close();

    printMessage("Copying include files");

    //this->copyIncludeFilesToDir(savePath, true);
    this->copyBoostIncludeFilesToDir(savePath);

    //! @todo should not overwrite this file if it already exists
    QFile externalLibsFile;
    externalLibsFile.setFileName(savePath + "/externalLibs.txt");
    if(!externalLibsFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        printErrorMessage("Failed to open externalLibs.txt for writing.");
        return;
    }
    QTextStream externalLibsFileStream(&externalLibsFile);
    externalLibsFileStream << "#Enter the relative path to each external component lib that needs to be loaded" << endl;
    externalLibsFileStream << "#Enter one per line, the filename is enough if you put the lib file (.dll or.so) in this directory.";
    externalLibsFile.close();

    printMessage("Finished!");
}
