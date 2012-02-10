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
//! @file   ComponentGeneratorUtilities.h
//! @author Robert Braun <robert.braun@liu.se
//! @date   2012-01-08
//!
//! @brief Contains component generation utiluties
//!
//$Id: GUIUtilities.cpp 3813 2012-01-05 17:11:57Z robbr48 $

//#include <qmath.h>
//#include <QPoint>
//#include <QDir>
//#include <QDebug>
#include <QStringList>
//#include <limits>
//#include <math.h>
//#include <complex>
#include <QProcess>


#include "Configuration.h"
#include "MainWindow.h"
#include "GUIObjects/GUIModelObjectAppearance.h"
#include "Utilities/ComponentGeneratorUtilities.h"
#include "Widgets/LibraryWidget.h"
#include "Widgets/MessageWidget.h"
#include "common.h"

using namespace std;


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


//! @brief Transforms a DOM element component description to a ComponentSpecification object and calls actual generator utility
//! @param outputFile Filename for output
//! @param rDomElement Reference to dom element with information about component
void generateComponentSourceCode(QString outputFile, QDomElement &rDomElement, ModelObjectAppearance *pAppearance, QProgressDialog *pProgressBar)
{
    QString typeName = rDomElement.attribute("typename");
    QString displayName = rDomElement.attribute("displayname");
    QString cqsType = rDomElement.attribute("cqstype");

    ComponentSpecification comp = ComponentSpecification(typeName, displayName, cqsType);

    QDomElement utilitiesElement = rDomElement.firstChildElement("utilities");
    QDomElement utilityElement = utilitiesElement.firstChildElement("utility");
    while(!utilityElement.isNull())
    {
        comp.utilities.append(utilityElement.attribute ("utility"));
        comp.utilityNames.append(utilityElement.attribute("name"));
        utilityElement=utilityElement.nextSiblingElement("utility");
    }

    QDomElement parametersElement = rDomElement.firstChildElement("parameters");
    QDomElement parameterElement = parametersElement.firstChildElement("parameter");
    while(!parameterElement.isNull())
    {
        comp.parNames.append(parameterElement.attribute("name"));
        comp.parInits.append(parameterElement.attribute("init"));
        comp.parDisplayNames.append(parameterElement.attribute("displayname"));
        comp.parDescriptions.append(parameterElement.attribute("description"));
        comp.parUnits.append(parameterElement.attribute("unit"));
        parameterElement=parameterElement.nextSiblingElement("parameter");
    }

    QDomElement variablesElemenet = rDomElement.firstChildElement("staticvariables");
    QDomElement variableElement = variablesElemenet.firstChildElement("staticvariable");
    while(!variableElement.isNull())
    {
        comp.varNames.append(variableElement.attribute("name"));
        comp.varTypes.append(variableElement.attribute("datatype"));
        variableElement=variableElement.nextSiblingElement("staticvariable");
    }

    QDomElement portsElement = rDomElement.firstChildElement("ports");
    QDomElement portElement = portsElement.firstChildElement("port");
    while(!portElement.isNull())
    {
        comp.portNames.append(portElement.attribute("name"));
        comp.portTypes.append(portElement.attribute("type"));
        comp.portNodeTypes.append(portElement.attribute("nodetype"));
        comp.portDefaults.append(portElement.attribute("default"));
        comp.portNotReq.append(portElement.attribute("notrequired") == "True");
        portElement=portElement.nextSiblingElement("port");
    }

    QDomElement initializeElement = rDomElement.firstChildElement("initialize");
    QDomElement initEquationElement = initializeElement.firstChildElement("equation");
    while(!initEquationElement.isNull())
    {
        comp.initEquations.append(initEquationElement.text());
        initEquationElement=initEquationElement.nextSiblingElement("equation");
    }

    QDomElement simulateElement = rDomElement.firstChildElement("simulate");
    QDomElement equationElement = simulateElement.firstChildElement("equation");
    while(!equationElement.isNull())
    {
        comp.simEquations.append(equationElement.text());
        equationElement=equationElement.nextSiblingElement("equation");
    }

    generateComponentSourceCode(outputFile, comp, pAppearance, true);
}


//! @brief Generates a ComponentSpecification object from equations and a jacobian
//! @param typeName Type name of component
//! @param displayName Display name of component
//! @param cqsType CQS type of component
//! @param ports List of port specifications
//! @param parameteres List of parameter specifications
//! @param sysEquations List of system equations
//! @param stateVars List of state variables
//! @param jacobian List of Jacobian elements
//! @param delayTerms List of delay terms
//! @param delaySteps List of number of delay steps for each delay term
void generateComponentSourceCode(QString typeName, QString displayName, QString cqsType,
                                 QList<PortSpecification> ports, QList<ParameterSpecification> parameters,
                                 QStringList sysEquations, QStringList stateVars, QStringList jacobian,
                                 QStringList delayTerms, QStringList delaySteps, QStringList localVars,
                                 QStringList initAlgorithms, ModelObjectAppearance *pAppearance, QProgressDialog *pProgressBar)
{
    if(pProgressBar)
    {
        pProgressBar->setLabelText("Creating component object");
        pProgressBar->setValue(pProgressBar->value()+1);
    }

    ComponentSpecification comp(typeName, displayName, cqsType);

    for(int i=0; i<delayTerms.size(); ++i)
    {
        comp.utilities << "Delay";
        comp.utilityNames << "mDelay"+QString().setNum(i);
    }

    for(int i=0; i<ports.size(); ++i)
    {
        comp.portNames << ports[i].name;
        comp.portNodeTypes << ports[i].nodetype;
        comp.portTypes << ports[i].porttype;
        comp.portNotReq << ports[i].notrequired;
        comp.portDefaults << ports[i].defaultvalue;
    }

    for(int i=0; i<parameters.size(); ++i)
    {
        comp.parNames << parameters[i].name;
        comp.parDisplayNames << parameters[i].displayName;
        comp.parDescriptions << parameters[i].description;
        comp.parUnits << parameters[i].unit;
        comp.parInits << parameters[i].init;
    }

    comp.varNames << "order["+QString().setNum(stateVars.size())+"]" << "jsyseqnweight[4]" << "jacobianMatrix" << "systemEquations";
    comp.varTypes << "int" << "double" << "Matrix" << "Vec";

    comp.initEquations << "jacobianMatrix.create("+QString().setNum(sysEquations.size())+","+QString().setNum(stateVars.size())+");";
    comp.initEquations << "systemEquations.create("+QString().setNum(sysEquations.size())+");";
    comp.initEquations << "";
    comp.initEquations << "jsyseqnweight[0]=1.0;";
    comp.initEquations << "jsyseqnweight[1]=0.67;";
    comp.initEquations << "jsyseqnweight[2]=0.5;";
    comp.initEquations << "jsyseqnweight[3]=0.5;";
    comp.initEquations << "";
    for(int i=0; i<delayTerms.size(); ++i)
    {
        comp.initEquations << "mDelay"+QString().setNum(i)+".initialize("+QString().setNum(delaySteps.at(i).toInt()+1)+", "+delayTerms[i]+");";
    }

    comp.simEquations << "Vec stateVar("+QString().setNum(stateVars.size())+");";
    comp.simEquations << "Vec stateVark("+QString().setNum(stateVars.size())+");";
    comp.simEquations << "Vec deltaStateVar("+QString().setNum(stateVars.size())+");";
    comp.simEquations << "";

    if(pProgressBar)
    {
        pProgressBar->setValue(pProgressBar->value()+1);
    }

    for(int i=0; i<initAlgorithms.size(); ++i)
    {
        comp.simEquations << initAlgorithms[i]+";";
    }
    comp.simEquations << "";

    for(int i=0; i<stateVars.size(); ++i)
    {
        comp.simEquations << "stateVark["+QString().setNum(i)+"] = "+stateVars[i]+";";
    }

    comp.simEquations << "for(int iter=1;iter<=2;iter++)" << "{";
    comp.simEquations << "    //System Equations";
    for(int i=0; i<sysEquations.size(); ++i)
    {
        comp.simEquations << "    systemEquations["+QString().setNum(i)+"] = "+sysEquations[i]+";";
    }
    comp.simEquations << "";

    comp.simEquations << "";
    comp.simEquations << "    //Jacobian Matrix";
    for(int i=0; i<sysEquations.size(); ++i)
    {
        for(int j=0; j<stateVars.size(); ++j)
        {
            comp.simEquations << "    jacobianMatrix["+QString().setNum(i)+"]["+QString().setNum(j)+"] = "+jacobian[sysEquations.size()*i+j]+";";
        }
    }
    comp.simEquations << "";
    comp.simEquations << "    //Solving equation using LU-faktorisation";
    comp.simEquations << "    ludcmp(jacobianMatrix, order, mpSystemParent);";
    comp.simEquations << "    solvlu(jacobianMatrix,systemEquations,deltaStateVar,order);";
    comp.simEquations << "";
    comp.simEquations << "    for(int i=0; i<"+QString().setNum(stateVars.size())+"; i++)";
    comp.simEquations << "    {";
    comp.simEquations << "        stateVar[i] = stateVark[i] - jsyseqnweight[iter - 1] * deltaStateVar[i];";
    comp.simEquations << "    }";
    comp.simEquations << "    for(int i=0; i<"+QString().setNum(stateVars.size())+"; i++)";
    comp.simEquations << "    {";
    comp.simEquations << "        stateVark[i] = stateVar[i];";
    comp.simEquations << "    }";
    comp.simEquations << "";
    for(int i=0; i<stateVars.size(); ++i)
    {
        comp.simEquations << "    " << stateVars[i]+"=stateVark["+QString().setNum(i)+"];";
    }
    comp.simEquations << "}";
    comp.simEquations << "";
    for(int i=0; i<delayTerms.size(); ++i)
    {
        comp.simEquations << "mDelay"+QString().setNum(i)+".update("+delayTerms[i]+");";
    }

    for(int i=0; i<localVars.size(); ++i)
    {
        comp.varNames << localVars[i];
        comp.varTypes << "double";
    }

    generateComponentSourceCode("equation.hpp", comp, pAppearance, false, pProgressBar);
}


//! @brief Generates and compiles component source code from a ComponentSpecification object
//! @param outputFile Name of output file
//! @param comp Component specification object
//! @param overwriteStartValues Tells whether or not this components overrides the built-in start values or not
void generateComponentSourceCode(QString outputFile, ComponentSpecification comp, ModelObjectAppearance *pAppearance, bool overwriteStartValues, QProgressDialog *pProgressBar)
{
    if(pProgressBar)
    {
        pProgressBar->setLabelText("Creating .hpp file");
        pProgressBar->setValue(pProgressBar->value()+1);
    }

    //Initialize the file stream
    QFileInfo fileInfo;
    QFile file;
    fileInfo.setFile(QString(DATAPATH)+outputFile);
    file.setFileName(fileInfo.filePath());   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to open file for writing: " + outputFile);
        return;
    }
    QTextStream fileStream(&file);  //Create a QTextStream object to stream the content of file

    fileStream << "#ifndef " << comp.typeName.toUpper() << "_HPP_INCLUDED\n";
    fileStream << "#define " << comp.typeName.toUpper() << "_HPP_INCLUDED\n\n";
    fileStream << "#include <math.h>\n";
    fileStream << "#include \"ComponentEssentials.h\"\n";
    fileStream << "#include \"ComponentUtilities.h\"\n";
    fileStream << "#include <sstream>\n";
    fileStream << "\n";
    fileStream << "namespace hopsan {\n\n";
    fileStream << "    class " << comp.typeName << " : public Component" << comp.cqsType << "\n";
    fileStream << "    {\n";
    fileStream << "    private:\n";                         // Private section
    fileStream << "        double ";
    int portId=1;
    for(int i=0; i<comp.portNames.size(); ++i)              //Declare variables
    {
        QStringList varNames;
        if(comp.portNodeTypes[i] == "NodeSignal")
        {
            varNames << comp.portNames[i];
        }
        else
        {
            varNames << getQVariables(comp.portNodeTypes[i]) << getCVariables(comp.portNodeTypes[i]);
        }

        for(int v=0; v<varNames.size()-1; ++v)
        {
            QString varName;
            if(comp.portNodeTypes[i] == "NodeSignal")
                varName = varNames[v];
            else
                varName = varNames[v] + QString().setNum(portId);
            fileStream << varName << ", ";
        }
        QString varName;
        if(comp.portNodeTypes[i] == "NodeSignal")
            varName = varNames.last();
        else
            varName = varNames.last() + QString().setNum(portId);
        fileStream << varName;
        ++portId;
        if(i < comp.portNames.size()-1)
        {
            fileStream << ", ";
        }
    }
    fileStream << ";\n";
    for(int i=0; i<comp.parNames.size(); ++i)                   //Declare parameters
    {
        fileStream << "        double " << comp.parNames[i] << ";\n";
    }
    for(int i=0; i<comp.varNames.size(); ++i)
    {
        fileStream << "        " << comp.varTypes[i] << " " << comp.varNames[i] << ";\n";
    }
    for(int i=0; i<comp.utilities.size(); ++i)
    {
        fileStream << "        " << comp.utilities[i] << " " << comp.utilityNames[i] << ";\n";
    }
    fileStream << "        double ";
    portId=1;
    QStringList allVarNames;                                    //Declare node data pointers
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        QString id = QString().setNum(portId);
        if(comp.portNodeTypes[i] == "NodeSignal")
        {
            allVarNames << comp.portNames[i];
        }
        else
        {
            QStringList vars;
            vars << getQVariables(comp.portNodeTypes[i]) << getCVariables(comp.portNodeTypes[i]);

            for(int v=0; v<vars.size(); ++v)
            {
                allVarNames << vars[v]+id;
            }
        }
        ++portId;
    }
    if(!allVarNames.isEmpty())
    {
        fileStream << "*mpND_" << allVarNames[0];
        for(int i=1; i<allVarNames.size(); ++i)
        {
            fileStream << ", *mpND_" << allVarNames[i];
        }
    }

    if(pProgressBar)
    {
        pProgressBar->setValue(pProgressBar->value()+1);
    }

    fileStream << ";\n";
    fileStream << "        Port ";                              //Declare ports
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        fileStream << "*mp" << comp.portNames[i];
        if(i<comp.portNames.size()-1)
        {
            fileStream << ", ";
        }
    }
    fileStream << ";\n\n";
    fileStream << "    public:\n";                              //Public section
    fileStream << "        static Component *Creator()\n";
    fileStream << "        {\n";
    fileStream << "            return new " << comp.typeName << "();\n";
    fileStream << "        }\n\n";
    fileStream << "        " << comp.typeName << "() : Component" << comp.cqsType << "()\n";
    fileStream << "        {\n";
    for(int i=0; i<comp.parNames.size(); ++i)
    {
        fileStream << "            " << comp.parNames[i] << " = " << comp.parInits[i] << ";\n";
    }
    fileStream << "\n";
    for(int i=0; i<comp.parNames.size(); ++i)
    {
        fileStream << "            registerParameter(\"" << comp.parDisplayNames[i] << "\", \""
                   << comp.parDescriptions[i] << "\", \"" << comp.parUnits[i] << "\", " << comp.parNames[i] << ");\n";
    }
    fileStream << "\n";
    for(int i=0; i<comp.portNames.size(); ++i)
    {

        fileStream << "            mp" << comp.portNames[i] << " = add" << comp.portTypes[i]
                   << "(\"" << comp.portNames[i] << "\", \"" << comp.portNodeTypes[i] << "\"";
        if(comp.portNotReq[i])
        {
            fileStream << ", Port::NOTREQUIRED);\n";
        }
        else
        {
            fileStream << ");\n";
        }
    }
    fileStream << "        }\n\n";
    fileStream << "        void initialize()\n";
    fileStream << "        {\n";
    portId=1;
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        QStringList varNames;
        QStringList varLabels;
        if(comp.portNodeTypes[i] == "NodeSignal")
        {
            varNames << comp.portNames[i];
            varLabels << "VALUE";
        }
        else
        {
            varNames << getQVariables(comp.portNodeTypes[i]) << getCVariables(comp.portNodeTypes[i]);
            varLabels << getVariableLabels(comp.portNodeTypes[i]);
        }

        for(int v=0; v<varNames.size(); ++v)
        {
            QString varName;
            if(comp.portNodeTypes[i] == "NodeSignal")
                varName = varNames[v];
            else
                varName = varNames[v]+QString().setNum(portId);
            fileStream << "            mpND_" << varName << " = getSafeNodeDataPtr(mp" << comp.portNames[i] << ", " << comp.portNodeTypes[i] << "::" << varLabels[v];
            if(comp.portNotReq[i])
            {
                fileStream << ", " << comp.portDefaults[i];
            }
            fileStream << ");\n";
        }
        ++portId;
    }

    if(pProgressBar)
    {
        pProgressBar->setValue(pProgressBar->value()+1);
    }

    fileStream << "\n";
    if(!comp.initEquations.isEmpty())
    {
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
                varNames << getQVariables(comp.portNodeTypes[i]) << getCVariables(comp.portNodeTypes[i]);
            }

            for(int v=0; v<varNames.size(); ++v)
            {
                QString varName;
                if(comp.portNodeTypes[i] == "NodeSignal")
                    varName = varNames[v];
                else
                    varName = varNames[v] + QString().setNum(portId);
                fileStream << "            " << varName << " = (*mpND_" << varName << ");\n";
            }
            ++portId;
        }
        fileStream << "\n";
        for(int i=0; i<comp.initEquations.size(); ++i)
        {
            fileStream << "            " << comp.initEquations[i] << "\n";
        }
        if(overwriteStartValues)
        {
            fileStream << "\n";
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
                    varNames << getQVariables(comp.portNodeTypes[i]);
                }
                if(comp.portNodeTypes[i] != "NodeSignal" && (comp.cqsType == "C" || comp.cqsType == "S"))
                {
                    varNames << getCVariables(comp.portNodeTypes[i]);
                }
                for(int v=0; v<varNames.size(); ++v)
                {
                    QString varName;
                    if(comp.portNodeTypes[i] == "NodeSignal")
                        varName = varNames[v];
                    else
                        varName = varNames[v] + QString().setNum(portId);
                    fileStream << "            (*mpND_" << varName << ") = " << varName << ";\n";
                }
            }
            ++portId;
        }
    }
    fileStream << "        }\n\n";

    //Simulate one time step
    fileStream << "        void simulateOneTimestep()\n";
    fileStream << "        {\n";
    portId=1;
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        QStringList varNames;
        if(comp.portNodeTypes[i] == "NodeSignal" && comp.portTypes[i] == "ReadPort")
        {
            varNames << comp.portNames[i];
        }
        if(comp.portNodeTypes[i] != "NodeSignal" && (comp.cqsType == "C" || comp.cqsType == "S"))
        {
            varNames << getQVariables(comp.portNodeTypes[i]);
        }
        if(comp.portNodeTypes[i] != "NodeSignal" && (comp.cqsType == "Q" || comp.cqsType == "S"))
        {
            varNames << getCVariables(comp.portNodeTypes[i]);
        }

        for(int v=0; v<varNames.size(); ++v)
        {
            QString varName;
            if(comp.portNodeTypes[i] == "NodeSignal")
                varName = varNames[v];
            else
                varName = varNames[v] + QString().setNum(portId);
            fileStream << "            " << varName << " = (*mpND_" << varName << ");\n";
        }
        ++portId;
    }
    fileStream << "\n";
    for(int i=0; i<comp.simEquations.size(); ++i)
    {
        fileStream << "            " << comp.simEquations[i] << "\n";
    }
    fileStream << "\n";
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
            varNames << getQVariables(comp.portNodeTypes[i]);
        }
        if(comp.portNodeTypes[i] != "NodeSignal" && (comp.cqsType == "C" || comp.cqsType == "S"))
        {
            varNames << getCVariables(comp.portNodeTypes[i]);
        }

        for(int v=0; v<varNames.size(); ++v)
        {
            QString varName;
            if(comp.portNodeTypes[i] == "NodeSignal")
                varName = varNames[v];
            else
                varName = varNames[v] + QString().setNum(portId);
            fileStream << "            (*mpND_" << varName << ") = " << varName << ";\n";
        }
        ++portId;
    }
    fileStream << "        }\n";
    //! @todo Support finalize equations
    fileStream << "    };\n";
    fileStream << "}\n\n";
    fileStream << "#endif // " << comp.typeName.toUpper() << "_HPP_INCLUDED\n";
    file.close();


    if(pProgressBar)
    {
        pProgressBar->setLabelText("Creating tempLib.cc");
        pProgressBar->setValue(pProgressBar->value()+1);
    }


    QFile ccLibFile;
    ccLibFile.setFileName(QString(DATAPATH)+"tempLib.cc");
    if(!ccLibFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to open tempLib.cc for writing.");
        return;
    }
    QTextStream ccLibStream(&ccLibFile);
    ccLibStream << "#include \"" << outputFile << "\"\n";
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

    if(pProgressBar)
    {
        pProgressBar->setLabelText("Creating compile script");
        pProgressBar->setValue(pProgressBar->value()+1);
    }


    QFile clBatchFile;
    clBatchFile.setFileName(QString(DATAPATH)+"compile.bat");
    if(!clBatchFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to open compile.bat for writing.");
        return;
    }
    QTextStream clBatchStream(&clBatchFile);
    clBatchStream << "g++.exe -shared tempLib.cc -o " << comp.typeName << ".dll -I\"" << gExecPath+QString(INCLUDEPATH) << "\" -L\""+gExecPath+"\" -lHopsanCore\n";
    clBatchFile.close();

    if(pProgressBar)
    {
        pProgressBar->setLabelText("Creating appearance file");
        pProgressBar->setValue(pProgressBar->value()+1);
    }


    QDir componentsDir(QString(DOCSPATH));
    QDir generatedDir(QString(DOCSPATH) + "Generated Componentes/");
    if(!generatedDir.exists())
    {
        componentsDir.mkdir("Generated Componentes");
    }

    QFile xmlFile;
    xmlFile.setFileName(generatedDir.path()+"/"+comp.typeName+".xml");
    if(!xmlFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to open " + comp.typeName + ".xml  for writing.");
        return;
    }
    QTextStream xmlStream(&xmlFile);
    xmlStream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    xmlStream << "<hopsanobjectappearance version=\"0.3\">\n";
    xmlStream << "  <modelobject typename=\"" << comp.typeName << "\" displayname=\"" << comp.displayName << "\">\n";
    xmlStream << "    <icons>\n";
    //! @todo Make it possible to choose icon files
    //! @todo In the meantime, use a default "generated component" icon
    xmlStream << "      <icon type=\"user\" path=\""+pAppearance->getIconPath(USERGRAPHICS, ABSOLUTE)+"\" iconrotation=\""+pAppearance->getIconRotationBehaviour()+"\" scale=\"1\"/>\n";
    xmlStream << "    </icons>\n";
    xmlStream << "    <ports>\n";
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        PortAppearance portApp = pAppearance->getPortAppearanceMap().find(comp.portNames[i]).value();
        xmlStream << "      <port name=\"" << comp.portNames[i] << "\" x=\"" << portApp.x << "\" y=\"" << portApp.y << "\" a=\"" << portApp.rot << "\"/>\n";
    }
    xmlStream << "    </ports>\n";
    xmlStream << "  </modelobject>\n";
    xmlStream << "</hopsanobjectappearance>\n";
    xmlFile.close();

    if(pProgressBar)
    {
        pProgressBar->setLabelText("Compiling component library");
        pProgressBar->setValue(pProgressBar->value()+1);
    }


    //Execute HopsanFMU compile script
#ifdef WIN32
    QProcess p;
    p.start("cmd.exe", QStringList() << "/c" << "cd " + QString(DATAPATH) + " & compile.bat");
    p.waitForFinished();
#else
    QString command = "g++ -shared -fPIC tempLib.cc -o " + comp.typeName + ".so -I" + INCLUDEPATH + " -L./ -lHopsanCore\n";
    //qDebug() << "Command = " << command;
    FILE *fp;
    char line[130];
    command +=" 2>&1";
    fp = popen(  (const char *) command.toStdString().c_str(), "r");
    if ( !fp )
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Could not execute '" + command + "'! err=%d");
        return;
    }
    else
    {
        while ( fgets( line, sizeof line, fp))
        {
            gpMainWindow->mpMessageWidget->printGUIInfoMessage((const QString &)line);
        }
    }
#endif

    if(pProgressBar)
    {
        pProgressBar->setLabelText("Moving files");
        pProgressBar->setValue(pProgressBar->value()+1);
    }

    QFile::remove(generatedDir.path() + "/" + xmlFile.fileName());
    xmlFile.copy(generatedDir.path() + "/" + xmlFile.fileName());

    QFile dllFile(QString(DATAPATH)+comp.typeName+".dll");
    QFile::remove(generatedDir.path() + "/" + comp.typeName + ".dll");
    dllFile.copy(generatedDir.path() + "/" + comp.typeName + ".dll");

    QFile soFile(QString(DATAPATH)+comp.typeName+".so");
    QFile::remove(generatedDir.path() + "/" + comp.typeName + ".so");
    soFile.copy(generatedDir.path() + "/" + comp.typeName + ".so");

    QFile svgFile(QString(OBJECTICONPATH)+"generatedcomponenticon.svg");
    QFile::remove(generatedDir.path() + "/generatedcomponenticon.svg");
    svgFile.copy(generatedDir.path() + "/generatedcomponenticon.svg");

/*    file.remove();
    clBatchFile.remove();
    ccLibFile.remove();
    dllFile.remove();
    soFile.remove();
    xmlFile.remove();*/

    //Load the library

    if(pProgressBar)
    {
        pProgressBar->setLabelText("Loading new library");
        pProgressBar->setValue(pProgressBar->value()+1);
    }


    QString libPath = QDir().cleanPath(generatedDir.path());

    //qDebug() << "libPath = " << libPath;
    //qDebug() << "user libs: " << gConfig.getUserLibs();
    if(gConfig.hasUserLib(libPath))
    {
        //qDebug() << "Updated user libs: " << gConfig.getUserLibs();
        gpMainWindow->mpLibrary->updateExternalLibraries();
    }
    else
    {
        //qDebug() << "Loaded user libs: " << gConfig.getUserLibs();
        gpMainWindow->mpLibrary->loadAndRememberExternalLibrary(libPath);
    }
}




//! @brief Finds all variables in an equations
//! @param equations Equation
//! @param leftSideVariables List with variables left of the equality sign
//! @param rightSideVariables list with variables right of the equatity sign
void identifyVariables(QString equation, QStringList &leftSideVariables, QStringList &rightSideVariables)
{
    QString word;
    bool leftSide=true;
    for(int i=0; i<equation.size(); ++i)
    {
        QChar currentChar = equation.at(i);
        if(currentChar.isLetter() || (currentChar.isNumber() && !word.isEmpty()))
        {
            word.append(currentChar);
        }
        else if(!word.isEmpty() && currentChar != '(')
        {
            if(leftSide)
            {
                leftSideVariables.append(word);
            }
            else
            {
                rightSideVariables.append(word);
            }
            word.clear();
        }
        else
        {
            word.clear();
        }

        if(currentChar == '=')
        {
            leftSide = false;
        }
    }
    if(!word.isEmpty())
    {
        if(leftSide)
        {
            leftSideVariables.append(word);
        }
        else
        {
            rightSideVariables.append(word);
        }
    }
    ////qDebug() << "Equation: " << equation;
    ////qDebug() << "Identified: " << leftSideVariables << rightSideVariables;
}


//! @brief Identifies all function calls in an equation
//! @param equation Equation
//! @param functions List with all function names
void identifyFunctions(QString equation, QStringList &functions)
{
    QString word;
    for(int i=0; i<equation.size(); ++i)
    {
        QChar currentChar = equation.at(i);
        if(currentChar.isLetter() || (currentChar.isNumber() && !word.isEmpty()))
        {
            word.append(currentChar);
        }
        else if(!word.isEmpty() && currentChar == '(')
        {
            functions.append(word);
            word.clear();
        }
        else
        {
            word.clear();
        }
    }
}


//! @brief Verfies that a system of equations is Hopsan-solveable
bool verifyEquations(QStringList equations)
{
    //Loop through and verify each equation
    for(int i=0; i<equations.size(); ++i)
    {
        if(!verifyEquation(equations[i]))
            return false;
    }

    //! @todo Verify equation system (number of unknowns etc)
    return true;
}


//! @brief Verifies that a list of parameter specifications is correct
//! @param parameters List of parameter specifications
bool verifyParameteres(QList<ParameterSpecification> parameters)
{
    for(int i=0; i<parameters.size(); ++i)
    {
        if(parameters.at(i).name.isEmpty())
        {
            gpMainWindow->mpMessageWidget->printGUIErrorMessage("Parameter " + QString().setNum(i+1) + " has no name specified.");
            return false;
        }
        if(parameters.at(i).init.isEmpty())
        {
            gpMainWindow->mpMessageWidget->printGUIErrorMessage("Parameter " + QString().setNum(i+1) + " has no initial value specified.");
            return false;
        }
    }
    return true;
}


//! @brief Verifies that a list of ports specifications is correct
//! @param ports List of ports specifications
bool verifyPorts(QList<PortSpecification> ports)
{
    for(int i=0; i<ports.size(); ++i)
    {
        if(ports.at(i).name.isEmpty())
        {
            gpMainWindow->mpMessageWidget->printGUIErrorMessage("Port " + QString().setNum(i+1) + " has no name specified.");
            return false;
        }
        if(ports.at(i).notrequired && ports.at(i).defaultvalue.isEmpty())
        {
            gpMainWindow->mpMessageWidget->printGUIErrorMessage("Port \"" + ports.at(i).name + " is not required but has no default value.");
            return false;
        }
    }
    return true;
}


//! @brief Verifies that a list of utilities specifications is correct
//! @param utilities List of utilities specifications
bool verifyUtilities(QList<UtilitySpecification> utilities)
{
    for(int i=0; i<utilities.size(); ++i)
    {
        if(utilities.at(i).name.isEmpty())
        {
            gpMainWindow->mpMessageWidget->printGUIErrorMessage("Utility " + QString().setNum(i+1) + " has no name specified.");
            return false;
        }
    }
    return true;
}


//! @brief Verifies that a list of variables specifications is correct
//! @param variables List of variables specifications
bool verifyStaticVariables(QList<StaticVariableSpecification> variables)
{
    for(int i=0; i<variables.size(); ++i)
    {
        if(variables.at(i).name.isEmpty())
        {
            gpMainWindow->mpMessageWidget->printGUIErrorMessage("Static variable " + QString().setNum(i+1) + " has no name specified.");
            return false;
        }
    }
    return true;
}


//! @brief Verifies that an equation is correct
//! @param equation Equation
bool verifyEquation(QString equation)
{
    //Verify that no unsupported functions are used
    QStringList legalFunctions;
    //! @todo These functions are hard coded on several places, not a good solution
    legalFunctions << "tan" << "cos" << "sin" << "atan" << "acos" << "asin" << "exp" << "sqrt" << "sign" << "abs" << "der" << "onPositive" << "onNegative" << "signedSquareL" ;        //"der" is not Python, but still allowed
    QStringList usedFunctions;
    identifyFunctions(equation, usedFunctions);
    for(int i=0; i<usedFunctions.size(); ++i)
    {
        if(!legalFunctions.contains(usedFunctions[i]))
        {
            gpMainWindow->mpMessageWidget->printGUIErrorMessage("Equations contain unsupported function: "+usedFunctions[i]);
            return false;
        }
    }
    return true;
}


//! @brief Replaces reserved words in a system of equations
//! @param equations List of system equations
void replaceReservedWords(QStringList &equations)
{
    for(int i=0; i<equations.size(); ++i)
    {
        replaceReservedWords(equations[i]);
    }
}


//! @brief Replaces reserved words in an equation with dummy words
//! @param equation Equation
void replaceReservedWords(QString &equation)
{
    QStringList reservedWords = QStringList() << "in" << "qi00" << "qi00_OLD";
    QStringList replacementWords = QStringList() << "RESERVED_WORD1" << "RESERVED_WORD2" << "RESERVED_WORD3";

    QString word;
    for(int i=0; i<equation.size(); ++i)
    {
        QChar currentChar = equation.at(i);
        if(currentChar.isLetter() || (currentChar.isNumber() && !word.isEmpty()))
        {
            word.append(currentChar);
        }
        else if(!word.isEmpty() && currentChar != '(')
        {
            ////qDebug() << "Word = " << word;
            if(reservedWords.contains(word))
            {
                //qDebug() << "Replacing: " << equation;
                equation.remove(i-word.size(), word.size());
                equation.insert(i-word.size(), replacementWords[reservedWords.indexOf(word)]);
                //qDebug() << "Replaced: " << equation;
                i=0;
            }
            word.clear();
        }
        else
        {
            word.clear();
        }
    }

    if(!word.isEmpty())
    {
       // //qDebug() << "Word = " << word;
        if(reservedWords.contains(word))
        {
            ////qDebug() << "Replacing: " << equation;
            equation.remove(equation.size()-word.size(), word.size());
            equation.insert(equation.size(), replacementWords[reservedWords.indexOf(word)]);
            ////qDebug() << "Replaced: " << equation;
        }
    }
}


//! @brief Replaces reserved words in a list of port specifications
//! @param ports List if ports
void replaceReservedWords(QList<PortSpecification> &ports)
{
    QStringList reservedWords = QStringList() << "in";
    QStringList replacementWords = QStringList() << "RESERVEDBYPYTHON1";

    for(int i=0; i<ports.size(); ++i)
    {
        if(reservedWords.contains(ports[i].name))
        {
            ports[i].name = replacementWords[reservedWords.indexOf(ports[i].name)];
        }
    }
}


//! @brief Identifies all derivatives in a system of equations and replaces them with s opereator
//! Replaces der(x) with s*x
//! @param equations List of system equations
void identifyDerivatives(QStringList &equations)
{
    ////qDebug() << "Before derivative check: " << equations;
    for(int i=0; i<equations.size(); ++i)
    {
        while(equations[i].contains("der("))
        {
            int j = equations[i].indexOf("der(");
            int k = equations[i].indexOf(")",j);
            ////qDebug() << "j = " << j << ", k = " << k;
            equations[i].remove(k, 1);
            equations[i].remove(j, 4);
            equations[i].insert(j, "s*");
        }
    }
    ////qDebug() << "After derivative check: " << equations;
}


//! @brief Replaces delay operator in equations with Hopsan delay utilities
//! @param equations List of system equations
//! @param delayTerms List with delay terms, i.e. for "der(x+y)", the delay term is "x+y"
//! @param delaySteps List with strings of integers telling how many steps each term is delayed
void translateDelaysFromPython(QStringList &equations, QStringList &delayTerms, QStringList &delaySteps)
{
    qDebug() << "Before delay translation: " << equations;

    int delayNum = 0;
    for(int i=0; i<equations.size(); ++i)
    {
        QStringList localDelays;

        //Find all terms
        QStringList terms;
        getAllTerms(equations[i], terms);

        ////qDebug() << "Terms = " << terms;

        //Find all terms containing a delay operator
        for(int j=0; j<terms.size(); ++j)
        {
            if(terms.at(j).contains("qi00"))
            {
                bool ok = false;
                int idxpar1 = terms.at(j).indexOf("(");
                int idxpar2 = terms.at(j).lastIndexOf(")");
                int idx = terms.at(j).indexOf("qi00");
                while(idx >= 0)
                {
                    if(idx < idxpar1 || idx > idxpar2)
                    {
                        ok = true;      //Found a delay operator outside all parentheses, so it is ok to treat term as a delay term
                        break;
                    }
                    idx = terms.at(j).indexOf("(", idx+1);
                }

                if(ok)
                    localDelays.append(terms.at(j));
            }
        }
        qDebug() << "Terms with delay: " << localDelays;

        //Remove delay operators and make a delay of the term
        for(int j=0; j<localDelays.size(); ++j)
        {
            delayNum = j+delayTerms.size();
            QString before = localDelays.at(j);
            int steps = 0;
            //More than one delay step if the delay operator is raised to something
            qDebug() << "Now checking.";
            qDebug() << "Checking if " << before.size() << " is bigger than " << before.indexOf("qi00")+6;
            if(before.size() > before.indexOf("qi00")+6)
            {
                if(before.at(before.indexOf("qi00")+4) == '*' && before.at(before.indexOf("qi00")+5) == '*')
                {
                    steps = QString(before.at(before.indexOf("qi00")+6)).toInt()-1;   //! @todo This method will limit maximum number of steps to 9. Not a big deal really, but it is not nice.
                }
            }

            QString after = "mDelay"+QString().setNum(delayNum)+".getIdx("+QString().setNum(steps)+")";
            equations[i].replace(localDelays.at(j), after);

            localDelays[j] = before.replace("qi00", "1");        //Replace delay operators with ones, to "remove" them
            while(localDelays[j].endsWith(" "))
                localDelays[j].chop(1);
            localDelays[j] = localDelays[j].replace("**1**", "**");  //Attempt to be "smart" and remove unnecessary multiplications/divisions with one
            localDelays[j] = localDelays[j].replace("**1*", "*");
            //localDelays[j] = localDelays[j].replace("*1*", "*");      //Would not work if it actually says "*1**"
            localDelays[j] = localDelays[j].replace("**1/", "/");
            localDelays[j] = localDelays[j].replace("*1/", "/");
            //localDelays[j] = localDelays[j].replace("/1*", "*");      //Same as above
            if(localDelays[j].startsWith("1*") && !localDelays[j].startsWith("1**"))
                localDelays[j].remove(0, 2);
            if(localDelays[j].endsWith("**1"))
                localDelays[j].chop(3);
            if(localDelays[j].endsWith("*1"))
                localDelays[j].chop(2);

            delaySteps << QString().setNum(steps);
        }

        delayTerms.append(localDelays);
    }
    qDebug() << "After delay translation: " << equations;
    qDebug() << "Delay terms: " << delayTerms;
}


//! @brief Replaces all Python power operators ("**") in the equations with C++ pow() functions
//! @param equations Equations to check
void translatePowersFromPython(QStringList &equations)
{
    for(int e=0; e<equations.size(); ++e)
    {
        while(equations[e].contains("**"))
        {
            QString before, after;
            int idx = equations[e].indexOf("**");
            int idxb = max(idx-1, 0);
            int idxa = min(idx+2, equations[e].size()-1);
            if(equations[e][idx-1] != ')' && idxb != 0)      //Not a paretheses before
            {
                while(equations[e][idxb].isLetterOrNumber() || equations[e][idxb] == '.')       //The '.' is because it may be a member function (for example "mDelay1.getIdx(0)**2")
                {
                    --idxb;
                    if(idxb < 0)
                        break;
                }
                ++idxb;
            }
            else if(idxb != 0)                           //Parenthesis before
            {
                int parBalance=-1;
                --idxb;
                while(parBalance != 0)
                {
                    if(equations[e][idxb] == ')')
                        --parBalance;
                    if(equations[e][idxb] == '(')
                        ++parBalance;
                    --idxb;
                }
                while(equations[e][idxb].isLetterOrNumber() || equations[e][idxb] == '.')
                {
                    --idxb;
                }
                ++idxb;
            }
            if(equations[e][idx+2] != '(' && idxa != equations[e].size()-1)      //Not a paretheses after
            {
                while(equations[e][idxa].isLetterOrNumber())
                {
                    ++idxa;
                }
                --idxa;
            }
            else if('(' && idxa != equations[e].size()-1)            //Parenthesis after
            {
                int parBalance=1;
                ++idxa;
                while(parBalance != 0)
                {
                    if(equations[e][idxa] == ')')
                        --parBalance;
                    if(equations[e][idxa] == '(')
                        ++parBalance;
                    ++idxa;
                }
                --idxa;
            }
            before = equations[e].mid(idxb, idx-idxb);
            after = equations[e].mid(idx+2, idxa-idx-1);

            equations[e].remove(idxb, idxa-idxb+1);
            equations[e].insert(idxb, "pow("+before+","+after+")");
        }
    }
}


//! @brief Translate functions from SymPy syntax to Hopsan/C++ syntax
//! @param equations Reference to a list of equations
void translateFunctionsFromPython(QStringList &equations)
{
    for(int e=0; e<equations.size(); ++e)
    {
        //qDebug() << "Before Subs() replacement: " << equations[e];

        //Replace "Subs(f(x),x,y)" with "f(y)"
        int idx = equations[e].indexOf("Subs(", 0);
        while(idx > -1)
        {
            int parBal = 0;         //Find index of first comma
            int idx2 = idx+4;
            while(true)
            {
                ++idx2;

                if(equations[e].at(idx2) == '(')
                {
                    ++parBal;
                }
                if(equations[e].at(idx2) == ')')
                {
                    --parBal;
                }
                if(equations[e].at(idx2) == ',' && parBal == 0)
                    break;
            }

            parBal = 0;         //Find index of second comma
            int idx3 = idx2+1;
            while(true)
            {
                ++idx3;

                if(equations[e].at(idx3) == '(')
                {
                    ++parBal;
                }
                if(equations[e].at(idx3) == ')')
                {
                    --parBal;
                }
                if(equations[e].at(idx3) == ',' && parBal == 0)
                    break;
            }

            parBal = 1;         //Find index of end parenthesis
            int idx4 = idx+5;
            while(parBal > 0)
            {
                ++idx4;

                if(equations[e].at(idx4) == '(')
                {
                    ++parBal;
                }
                if(equations[e].at(idx4) == ')')
                {
                    --parBal;
                }
            }

            QString old = equations.at(e).mid(idx+5, idx2-idx-5);
            ////qDebug() << "old = " << old;
            QString replaceWord = equations.at(e).mid(idx2+3, idx3-idx2-5);
            ////qDebug() << "replaceWord = " << replaceWord;
            QString replacement = equations.at(e).mid(idx3+3, idx4-idx3-5);
            ////qDebug() << "replacement = " << replacement;

            old = old.replace(replaceWord, replacement);      //Do the replacement

            ////qDebug() << "new = " << old;

            equations[e].remove(idx, idx4-idx+1);
            //qDebug() << "After remove: " << equations[e];
            equations[e].insert(idx, old);      //Replace whole function with new string

            idx = equations[e].indexOf("Subs(", 0);
        }

        //qDebug() << "After Subs() replacement: " << equations[e];

        //Replace "abs()" with "fabs()"
        idx = equations[e].indexOf("abs", 0);
        while(idx > -1)
        {
            if((idx == 0 || !equations[e].at(idx-1).isLetterOrNumber()) && equations[e].at(idx+3) == '(')
            {
                equations[e].insert(idx, "f");
            }
            idx = equations[e].indexOf("abs", idx+3);
        }


        //Replace "abs()" with "fabs()"
        idx = equations[e].indexOf("abs", 0, Qt::CaseInsensitive);
        while(idx > -1)
        {
            if((idx == 0 || !equations[e].at(idx-1).isLetterOrNumber()) && equations[e].at(idx+3) == '(')
            {
                equations[e].replace(idx, 3, "fabs");
            }
            idx = equations[e].indexOf("abs", idx+3, Qt::CaseInsensitive);
        }


        replaceDerivative(equations[e], "hopsanLimit", "hopsanDxLimit");


        //Replace "D(hopsanDxLimit(x, min, max))" with 0
        idx = equations[e].indexOf("D(hopsanDxLimit(", 0);
        while(idx > -1)
        {
            int parBal = 2;         //Find index of end parenthesis
            int idx2 = idx+15;
            while(parBal != 0)
            {
                ++idx2;

                if(equations[e].at(idx2) == '(')
                {
                    ++parBal;
                }
                if(equations[e].at(idx2) == ')')
                {
                    --parBal;
                }
            }

            equations[e].replace(idx, idx2-idx+1, "0");     //Replace entire expression with "0"

            idx = equations[e].indexOf("D(hopsanDxLimit(", idx);
        }


        //Replace "D(hopsanDxLimit(x, min, max))" with 0
        idx = equations[e].indexOf("Derivative(hopsanDxLimit(", 0);
        while(idx > -1)
        {
            int parBal = 2;         //Find index of end parenthesis
            int idx2 = idx+24;
            while(parBal != 0)
            {
                ++idx2;

                if(equations[e].at(idx2) == '(')
                {
                    ++parBal;
                }
                if(equations[e].at(idx2) == ')')
                {
                    --parBal;
                }
            }

            equations[e].replace(idx, idx2-idx+1, "0");     //Replace entire expression with "0"

            idx = equations[e].indexOf("Derivative(hopsanDxLimit(", idx);
        }


        //Replace "hopsanLimit" with "limit"
        idx = equations[e].indexOf("hopsanLimit(", 0);
       // qDebug() << "Replacing hopsanLimit for equation: " << equations[e];
        while(idx > -1)
        {
            qDebug() << "Found hopsanLimit at " << idx;
            //qDebug() << "idx = " << idx;

            if((idx == 0 || !equations[e].at(idx-1).isLetterOrNumber()) && equations[e].at(idx+11) == '(')
            {
                equations[e].replace(idx, 11, "limit");      //Do the replacement

                idx = equations[e].indexOf("hopsanLimit(", idx+1);
            }
            else
            {
                idx = equations[e].indexOf("hopsanLimit(", idx+1);
            }

            qDebug() << "Equation after replacement: " << equations[e];
        }


        //Replace "hopsanDxLimit" with "dxLimit"
        idx = equations[e].indexOf("hopsanDxLimit(", 0);
        while(idx > -1)
        {
            if((idx == 0 || !equations[e].at(idx-1).isLetterOrNumber()) && equations[e].at(idx+13) == '(')
            {
                equations[e].replace(idx, 13, "dxLimit");      //Do the replacement

                idx = equations[e].indexOf("hopsanDxLimit(", idx);
            }
            else
            {
                idx = equations[e].indexOf("hopsanDxLimit(", idx+1);
            }
        }


        //Replace "D(sign(x), x)" with 0 (derivative of sign(x) = 0 for all x except 0, but it should be ok)
        idx = equations[e].indexOf("D(sign(", 0);
        while(idx > -1)
        {
            int parBal = 2;         //Find index of end parenthesis
            int idx2 = idx+6;
            while(parBal != 0)
            {
                ++idx2;
                if(equations[e].at(idx2) == '(')
                    ++parBal;
                if(equations[e].at(idx2) == ')')
                    --parBal;
            }

            equations[e].remove(idx, idx2-idx+1);
            equations[e].insert(idx, "0");

            idx = equations[e].indexOf("D(sign(", idx);
        }


        //Replace "re(x)" with "x" (we only work with real numbers anyway)
        idx = equations[e].indexOf("re(", 0);
        while(idx > -1)
        {
            if((idx == 0 || !equations[e].at(idx-1).isLetterOrNumber()) && equations[e].at(idx+2) == '(')
            {
                //qDebug() << "Found \"re\" at index: " << idx;
                int parBal = 1;         //Find index of end parenthesis
                int idx2 = idx+3;
                while(parBal != 0)
                {
                    ++idx2;
                    if(equations[e].at(idx2) == '(')
                        ++parBal;
                    if(equations[e].at(idx2) == ')')
                        --parBal;
                }

                equations[e].remove(idx2, 1);
                equations[e].remove(idx, 3);

                idx = equations[e].indexOf("re(", idx);
            }
            else
            {
                idx = equations[e].indexOf("re(", idx+1);
            }
        }

        replaceDerivative(equations[e], "onPositive", "dxOnPositive");
        replaceDerivative(equations[e], "onNegative", "dxOnNegative");
        replaceDerivative(equations[e], "signedSquareL", "dxSignedSquareL");
    }
}



void replaceDerivative(QString &equation, QString f, QString dxf)
{
    //Replace "Derivative(f(x),x)" with "dxf(x)"
    int idx = equation.indexOf("Derivative("+f+"(", 0);
    while(idx > -1)
    {
        int parBal = 2;         //Find index of end parenthesis
        int idx2 = 0;
        int idx3 = idx+11+f.size();
        while(parBal != 0)
        {
            ++idx3;

            if(equation.at(idx3) == '(')
            {
                ++parBal;
            }
            if(equation.at(idx3) == ')')
            {
                --parBal;
                if(parBal == 1 && idx2 == 0)        //Only do this first time (idx2 = 0 first time)
                {
                    idx2 = idx3+1;      //idx2 is index after second last end parenthesis
                }
            }
        }

        equation.remove(idx2, idx3-idx2+1);     //Remove everything between second last and last parenthesis
        equation.replace(idx, 11+f.size(), dxf);        //Rename function

        idx = equation.indexOf("Derivative("+f+"(", idx);
    }


    //Replace "D(f(x))" with "dxf(x)"
    idx = equation.indexOf("Derivative("+f+"(", 0);
    while(idx > -1)
    {
        int parBal = 2;         //Find index of end parenthesis
        int idx2 = 0;
        int idx3 = idx+2+f.size();
        while(parBal != 0)
        {
            ++idx3;

            if(equation.at(idx3) == '(')
            {
                ++parBal;
            }
            if(equation.at(idx3) == ')')
            {
                --parBal;
                if(parBal == 1 && idx2 == 0)        //Only do this first time (idx2 = 0 first time)
                {
                    idx2 = idx3+1;      //idx2 is index after second last end parenthesis
                }
            }
        }

        equation.remove(idx2, idx3-idx2+1);     //Remove everything between second last and last parenthesis
        equation.replace(idx, 2+f.size(), dxf);        //Rename function

        idx = equation.indexOf("Derivative("+f+"(", idx);
    }
}


//! @brief Shuffles the elements in a stringlist to a pseudo-random order
void shuffle(QStringList &list)
{
    for(int i=list.size()-1; i>1; --i)
    {
        int j = qrand() % (i+1);
        list.swap(i, j);
    }
}


//! @brief Makes sure all variables in equations are float (to avoid 1/2=0 phenomena)
//! @param equations Equations to check
void translateIntsToDouble(QStringList &equations)
{
    for(int e=0; e<equations.size(); ++e)
    {
        int startId = 0;
        int stopId = 0;
        bool par = false;
        bool var = false;
        for(int i=0; i<equations[e].size(); ++i)
        {
            if(equations[e][i].isLetter())
            {
                par = true;
            }

            if(equations[e][i].isNumber() && !par && !var)
            {
                startId = i;
                var = true;
            }

            if(!equations[e][i].isLetterOrNumber())
            {
                par = false;

                if(var)
                {
                    stopId = i-1;
                    if((startId>0 && equations[e][startId-1] != '.') && (stopId < equations[e].size()-1 && equations[e][stopId+1] != '.'))     //Don't modify float numbers
                    {
                        equations[e].insert(stopId+1, ".0");
                        i=i+2;
                    }
                    var = false;
                }
            }
        }
        if(var)
        {
            stopId = equations[e].size()-1;
            if(startId>0 && equations[e][startId-1] != '.')     //Don't modify float numbers
            {
                equations[e].append(".0");
            }
        }
    }

}


//! @brief Parses a modelica model code to Hopsan classes
//! @param code Input Modelica code
//! @param typeName Type name of new component
//! @param displayName Display name of new component
//! @param equations Equations for new component
//! @param portList List of port specifications for new component
//! @param parametersList List of parameter specifications for new component
void parseModelicaModel(QString code, QString &typeName, QString &displayName, QStringList &equations,
                        QList<PortSpecification> &portList, QList<ParameterSpecification> &parametersList)
{
    //qDebug() << "Hej1";
    QStringList lines = code.split("\n");
    //qDebug() << "Hej2, lines = " << lines;
    QStringList portNames;
    bool equationPart = false;          //Are we in the "equations" part or not?
    for(int l=0; l<lines.size(); ++l)
    {
        if(!equationPart)
        {
            QStringList words = lines.at(l).trimmed().split(" ");
            //qDebug() << "Hej3, words = " << words;
            if(words.at(0) == "model")              //"model" keyword
            {
                typeName = words.at(1);
                if(words.size() > 2)
                {
                    displayName = words.at(2);
                    displayName.remove(0, 1);
                    int j=2;
                    while(!words.at(j).endsWith("\""))
                    {
                        ++j;
                        displayName.append(" " + words.at(j));
                    }
                    displayName.chop(1);
                }
            }
            else if(words.at(0) == "parameter")         //"parameter" keyword
            {
                QString name = words.at(2).section("(",0,0);
                QString unit = lines.at(l).section("unit=",1,1).section("\"",1,1);
                QString init;
                //Default value can be written with white spaces in different way, test them all
                if(!words.at(2).section(")", 1).isEmpty())
                    init = words.at(2).section(")", 1).section("=", 1);             //...blabla)=x
                else if(words.at(2).endsWith("="))
                    init = words.at(3);                                             //...blabla)= x
                else if(words.at(3).startsWith("=") && words.at(3).size() > 1)
                    init = words.at(3).section("=", 1);                             //...blabla) =x
                else if(words.at(3) == "=")
                    init = words.at(4);                                             // ...blabla) = x

                QString parDisplayName = lines.at(l).section("\"", -2, -2);

                ParameterSpecification par(name, parDisplayName, parDisplayName, unit, init);
                parametersList.append(par);
            }
            else if(words.at(0) == "NodeMechanic")              //Mechanic connector
            {
                for(int i=0; i<lines.at(l).count(",")+1; ++i)
                {
                    QString name = lines.at(l).trimmed().section(" ", 1).section(",",i,i).section(";",0,0).trimmed();
                    PortSpecification port("PowerPort", "NodeMechanic", name);
                    portList.append(port);
                    portNames << name;
                }
            }
            else if(words.at(0) == "NodeMechanicRotational")    //Mechanic rotational connector
            {
                for(int i=0; i<lines.at(l).count(",")+1; ++i)
                {
                    QString name = lines.at(l).trimmed().section(" ", 1).section(",",i,i).section(";",0,0).trimmed();
                    PortSpecification port("PowerPort", "NodeMechanicRotational", name);
                    portList.append(port);
                    portNames << name;
                }
            }
            else if(words.at(0) == "NodeHydraulic")             //Hydraulic connector
            {
                for(int i=0; i<lines.at(l).count(",")+1; ++i)
                {
                    QString name = lines.at(l).trimmed().section(" ", 1).section(",",i,i).section(";",0,0).trimmed();
                    PortSpecification port("PowerPort", "NodeHydraulic", name);
                    portList.append(port);
                    portNames << name;
                }
            }
            else if(words.at(0) == "NodePneumatic")             //Pneumatic connector
            {
                for(int i=0; i<lines.at(l).count(",")+1; ++i)
                {
                    QString name = lines.at(l).trimmed().section(" ", 1).section(",",i,i).section(";",0,0).trimmed();
                    PortSpecification port("PowerPort", "NodePneumatic", name);
                    portList.append(port);
                    portNames << name;
                }
            }
            else if(words.at(0) == "NodeElectric")              //Electric connector
            {
                for(int i=0; i<lines.at(l).count(",")+1; ++i)
                {
                    QString name = lines.at(l).trimmed().section(" ", 1).section(",",i,i).section(";",0,0).trimmed();
                    PortSpecification port("PowerPort", "NodeElectric", name);
                    portList.append(port);
                    portNames << name;
                }
            }
            else if(words.at(0) == "equation")                  //Equation part begins!
            {
                equationPart = true;
            }
        }
        else
        {
            if(lines.at(l).trimmed().startsWith("end "))        //"end" line, we are finished
                return;
            ////qDebug() << "Equation: " << lines.at(l).trimmed();
            equations << lines.at(l).trimmed();
            //Replace variables with Hopsan syntax, i.e. P2.q => q2
            for(int i=0; i<portNames.size(); ++i)
            {
                QString temp = portNames.at(i)+".";
                while(equations.last().contains(temp))
                {
                    //qDebug() << "BEFORE: " << equations.last();
                    int idx = equations.last().indexOf(temp);
                    int idx2=idx+temp.size()+1;
                    while(idx2 < equations.last().size()+1 && equations.last().at(idx2).isLetterOrNumber())
                        ++idx2;
                    equations.last().insert(idx2, QString().setNum(i+1));
                    equations.last().remove(idx, temp.size());
                    //qDebug() << "AFTER: " << equations.last();
                }
            }
            equations.last().chop(1);
        }
    }
}



//! @note First and last q-type variable must represent intensity and flow
QStringList getQVariables(QString nodeType)
{
    QStringList retval;
    if(nodeType == "NodeMechanic")
    {
        retval << "F" << "x" << "v";
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



//! @note c must come first and Zc last
QStringList getVariableLabels(QString nodeType)
{
    QStringList retval;
    if(nodeType == "NodeMechanic")
    {
        retval << "FORCE" << "POSITION" << "VELOCITY" << "WAVEVARIABLE" << "CHARIMP";
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
    return retval;
}



void getAllTerms(QString equation, QStringList &terms)
{
    QString term;
    for(int i=0; i<equation.size(); ++i)
    {
        QChar c = equation[i];
        if(c == '+' || c == '-' || c == ',')                //End of term, append it to terms list
        {
            terms << term;
            term.clear();
        }
        else if(c == '(')                                   //New start parenthesis, create substring to end parenthesis and recurse substring
        {
            int parBal = 1;
            int j = i;
            while(parBal != 0)
            {
                ++j;
                c = equation[j];
                if(c == '(')
                    ++parBal;
                else if(c == ')')
                    --parBal;
            }
            getAllTerms(equation.mid(i+1, j-i-1), terms);   //Recurse
            term.append(equation.mid(i, j-i+1));            //Append parenthesis to current term
            i = j;                                          //Continue with current term
        }
        else
        {
            if(c != ' ')                                    //Ignore spaces
            {
                term.append(c);                             //Not a new term, so append character and continue
            }
        }
    }
    if(!term.isEmpty())
    {
        terms << term;
    }
}


bool isSingular(QStringList matrix)
{
    int cols = sqrt(matrix.size());
    int rows = sqrt(matrix.size());
    for(int c=0; c<cols; ++c)
    {
        bool noElementBelowOrOnDiagonal = true;
        for(int r=c; r<rows; ++r)
        {
            if(matrix[r*rows+c] != "0")
            {
                noElementBelowOrOnDiagonal = false;
            }
        }
        if(noElementBelowOrOnDiagonal)
        {
            return true;
        }
    }
    return false;
}
