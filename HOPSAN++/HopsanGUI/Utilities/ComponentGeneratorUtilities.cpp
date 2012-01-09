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

#include "ComponentGeneratorUtilities.h"
#include "MainWindow.h"
#include "Widgets/MessageWidget.h"
#include "common.h"
//#include "Configuration.h"
#include "Widgets/LibraryWidget.h"

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


void generateComponentSourceCode(QString outputFile, QDomElement &rDomElement)
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

    generateComponentSourceCode(outputFile, comp, true);
}


void generateComponentSourceCode(QString typeName, QString displayName, QString cqsType,
                                 QList<PortSpecification> ports, QList<ParameterSpecification> parameters,
                                 QStringList sysEquations, QStringList stateVars, QStringList jacobian,
                                 QStringList delayTerms, QStringList delaySteps)
{
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

    //DEBUG

//    comp.simEquations << "    if(mTime < 0.01)";
//    comp.simEquations << "    {";
//    comp.simEquations << "        std::stringstream ss;";
//    comp.simEquations << "        ss << \"mDelay1.getIdx(0) = \" << mDelay1.getIdx(0) << \", 2*m*v = \" << 2*m*v << \", systemEquations[0] = \" << systemEquations[0];";
//    comp.simEquations << "        addDebugMessage(ss.str());";
//    comp.simEquations << "    }";

    //END DEBUG

    comp.simEquations << "";
    comp.simEquations << "    //Jacobian Matrix";
    for(int i=0; i<sysEquations.size(); ++i)
    {
        for(int j=0; j<stateVars.size(); ++j)
        {
            comp.simEquations << "    jacobianMatrix["+QString().setNum(i)+"]["+QString().setNum(j)+"] = "+jacobian[4*i+j]+";";
        }
    }
    comp.simEquations << "";
    comp.simEquations << "    //Solving equation using LU-faktorisation";
    comp.simEquations << "    ludcmp(jacobianMatrix, order);";
    comp.simEquations << "    solvlu(jacobianMatrix,systemEquations,deltaStateVar,order);";
    comp.simEquations << "";


    //DEBUG

//    comp.simEquations << "    if(mTime < 0.01)";
//    comp.simEquations << "    {";
//    comp.simEquations << "        std::stringstream ss;";
//    comp.simEquations << "        ss << \"deltaStateVar[0] = \" << deltaStateVar[0];";
//    comp.simEquations << "        addDebugMessage(ss.str());";
//    comp.simEquations << "    }";

    //END DEBUG


    comp.simEquations << "    for(int i=0; i<"+QString().setNum(stateVars.size())+"; i++)";
    comp.simEquations << "    {";
    comp.simEquations << "        stateVar[i] = stateVark[i] - jsyseqnweight[iter - 1] * deltaStateVar[i];";
    comp.simEquations << "    }";
    comp.simEquations << "    for(int i=0; i<"+QString().setNum(stateVars.size())+"; i++)";
    comp.simEquations << "    {";
    comp.simEquations << "        stateVark[i] = stateVar[i];";
    comp.simEquations << "    }";
    comp.simEquations << "}";
    comp.simEquations << "";
    for(int i=0; i<stateVars.size(); ++i)
    {
        comp.simEquations << stateVars[i]+"=stateVark["+QString().setNum(i)+"];";
    }
    comp.simEquations << "";
    for(int i=0; i<delayTerms.size(); ++i)
    {
        comp.simEquations << "mDelay"+QString().setNum(i)+".update("+delayTerms[i]+");";
    }


    //DEBUG

//    comp.simEquations << "    if(mTime < 0.01)";
//    comp.simEquations << "    {";
//    comp.simEquations << "        std::stringstream ss;";
//    comp.simEquations << "        ss << \"v = \" << v;";
//    comp.simEquations << "        addDebugMessage(ss.str());";
//    comp.simEquations << "    }";

    //END DEBUG

    generateComponentSourceCode("equation.hpp", comp);
}



void generateComponentSourceCode(QString outputFile, ComponentSpecification comp, bool overwriteStartValues)
{
    //Initialize the file stream
    QFileInfo fileInfo;
    QFile file;
    fileInfo.setFile(outputFile);
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
    fileStream << "    private:\n";
    fileStream << "        double ";
    int portId=1;
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        QStringList varNames;
        if(comp.portNodeTypes[i] == "NodeSignal")
        {
            varNames << comp.portNames[i];
        }
        else if(comp.portNodeTypes[i] == "NodeHydraulic")
        {
            varNames << "p" << "q" << "c" << "Zc";
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
    for(int i=0; i<comp.parNames.size(); ++i)
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
    QStringList allVarNames;
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        QString id = QString().setNum(portId);
        if(comp.portNodeTypes[i] == "NodeSignal")
        {
            allVarNames << comp.portNames[i];
        }
        else if(comp.portNodeTypes[i] == "NodeHydraulic")
        {
            allVarNames << "p"+id << "q"+id << "c"+id << "Zc"+id;
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
    fileStream << ";\n";
    fileStream << "        Port ";
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        fileStream << "*mp" << comp.portNames[i];
        if(i<comp.portNames.size()-1)
        {
            fileStream << ", ";
        }
    }
    fileStream << ";\n\n";
    fileStream << "    public:\n";
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
        else if(comp.portNodeTypes[i] == "NodeHydraulic")
        {
            varNames << "p" << "q" << "c" << "Zc";
            varLabels << "PRESSURE" << "FLOW" << "WAVEVARIABLE" << "CHARIMP";
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
            else if(comp.portNodeTypes[i] == "NodeHydraulic")
            {
               varNames << "p" << "q" << "c" << "Zc";
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
                else if(comp.portNodeTypes[i] == "NodeHydraulic" && comp.cqsType == "C")
                {
                    varNames << "c" << "Zc";
                }
                else if(comp.portNodeTypes[i] == "NodeHydraulic" && comp.cqsType == "Q")
                {
                    varNames << "p" << "q";
                }
                else if(comp.portNodeTypes[i] == "NodeHydraulic" && comp.cqsType == "S")
                {
                    varNames << "p" << "q" << "c" << "Zc";
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
        else if(comp.portNodeTypes[i] == "NodeHydraulic" && comp.cqsType == "Q")
        {
            varNames << "c" << "Zc";
        }
        else if(comp.portNodeTypes[i] == "NodeHydraulic" && comp.cqsType == "C")
        {
            varNames << "p" << "q";
        }
        else if(comp.portNodeTypes[i] == "NodeHydraulic" && comp.cqsType == "S")
        {
            varNames << "p" << "q" << "c" << "Zc";
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
        else if(comp.portNodeTypes[i] == "NodeHydraulic" && comp.cqsType == "C")
        {
            varNames << "c" << "Zc";
        }
        else if(comp.portNodeTypes[i] == "NodeHydraulic" && comp.cqsType == "Q")
        {
            varNames << "p" << "q";
        }
        else if(comp.portNodeTypes[i] == "NodeHydraulic" && comp.cqsType == "S")
        {
            varNames << "p" << "q" << "c" << "Zc";
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

    QFile ccLibFile;
    ccLibFile.setFileName("tempLib.cc");
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
    ccLibStream << "    pHopsanExternalLibInfo->hopsanCoreVersion = (char*)HOPSANCOREVERSION;\n";
    ccLibStream << "    pHopsanExternalLibInfo->libCompiledDebugRelease = (char*)DEBUGRELEASECOMPILED;\n";
    ccLibStream << "}\n";
    ccLibFile.close();

    QFile clBatchFile;
    clBatchFile.setFileName("compile.bat");
    if(!clBatchFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to open compile.bat for writing.");
        return;
    }
    QTextStream clBatchStream(&clBatchFile);
    clBatchStream << "g++.exe -shared tempLib.cc -o " << comp.typeName << ".dll -I" << INCLUDEPATH << " -L./ -lHopsanCore\n";
    clBatchFile.close();

    QFile xmlFile;
    xmlFile.setFileName(comp.typeName+".xml");
    if(!xmlFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to open " + comp.typeName + ".xml  for writing.");
        return;
    }
    QTextStream xmlStream(&xmlFile);
    xmlStream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    xmlStream << "<hopsanobjectappearance version=\"0.2\">\n";
    xmlStream << "  <modelobject typename=\"" << comp.typeName << "\" displayname=\"" << comp.displayName << "\">\n";
    xmlStream << "    <icons>\n";
    //! @todo Make it possible to choose icon files
    //! @todo In the meantime, use a default "generated component" icon
    xmlStream << "      <icon type=\"user\" path=\"generatedcomponenticon.svg\" iconrotation=\"ON\"/>\n";
    xmlStream << "    </icons>\n";
    xmlStream << "    <portpositions>\n";
    QStringList leftPorts, rightPorts, topPorts;
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        if(comp.portNodeTypes[i] == "NodeSignal" && comp.portTypes[i] == "ReadPort")
            leftPorts << comp.portNames[i];
        else if(comp.portNodeTypes[i] == "NodeSignal" && comp.portTypes[i] == "WritePort")
            rightPorts << comp.portNames[i];
        else
            topPorts << comp.portNames[i];
    }
    for(int i=0; i<leftPorts.size(); ++i)
    {
        xmlStream << "      <portpose name=\"" << leftPorts[i] << "\" x=\"0.0\" y=\"" << QString().setNum((double(i)+1)/(double(leftPorts.size())+1.0)) << "\" a=\"180\"/>\n";
    }
    for(int i=0; i<rightPorts.size(); ++i)
    {
        xmlStream << "      <portpose name=\"" << rightPorts[i] << "\" x=\"1.0\" y=\"" << QString().setNum((double(i)+1)/(double(rightPorts.size())+1.0)) << "\" a=\"0\"/>\n";
    }
    for(int i=0; i<topPorts.size(); ++i)
    {
        xmlStream << "      <portpose name=\"" << topPorts[i] << "\" x=\"" << QString().setNum((double(i)+1)/(double(topPorts.size())+1.0)) << "\" y=\"0.0\" a=\"270\"/>\n";
    }
    xmlStream << "    </portpositions>\n";
    xmlStream << "  </modelobject>\n";
    xmlStream << "</hopsanobjectappearance>\n";
    xmlFile.close();

    //Execute HopsanFMU compile script
#ifdef win32
    QProcess p;
    p.start("cmd.exe", QStringList() << "/c" << "cd " + gExecPath + " & compile.bat");
    p.waitForFinished();
#else
    QString command = "g++ -shared -fPIC tempLib.cc -o " + comp.typeName + ".so -I" + INCLUDEPATH + " -L./ -lHopsanCore\n";
    qDebug() << "Command = " << command;
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

    QDir componentsDir(QString(DOCSPATH));
    QDir generatedDir(QString(DOCSPATH) + "Generated Componentes/");
    if(!generatedDir.exists())
    {
        componentsDir.mkdir("Generated Componentes");
    }

    xmlFile.copy(generatedDir.path() + "/" + xmlFile.fileName());

    QFile dllFile(gExecPath+comp.typeName+".dll");
    dllFile.copy(generatedDir.path() + "/" + comp.typeName + ".dll");

    QFile soFile(gExecPath+comp.typeName+".so");
    soFile.copy(generatedDir.path() + "/" + comp.typeName + ".so");

    QFile svgFile(QString(OBJECTICONPATH)+"generatedcomponenticon.svg");
    svgFile.copy(generatedDir.path() + "/generatedcomponenticon.svg");

/*    file.remove();
    clBatchFile.remove();
    ccLibFile.remove();
    dllFile.remove();
    soFile.remove();
    xmlFile.remove();*/

    //Load the library
    gpMainWindow->mpLibrary->unloadExternalLibrary(generatedDir.path());
    gpMainWindow->mpLibrary->loadExternalLibrary(generatedDir.path());    //Load the library
}






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
    qDebug() << "Equation: " << equation;
    qDebug() << "Identified: " << leftSideVariables << rightSideVariables;
}


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


bool verifyEquations(QStringList equations)
{
    for(int i=0; i<equations.size(); ++i)
    {
        if(!verifyEquation(equations[i]))
            return false;
    }

    //! @todo Verify equation system (number of unknowns etc)
    return true;
}


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


bool verifyEquation(QString equation)
{
    //Verify that no unsupported functions are used
    QStringList legalFunctions;
    legalFunctions << "tan" << "cos" << "sin" << "atan" << "acos" << "asin" << "exp" << "sqrt" << "der";        //"der" is not Python!
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


void replaceReservedWords(QStringList &equations)
{
    for(int i=0; i<equations.size(); ++i)
    {
        replaceReservedWords(equations[i]);
    }
}


void replaceReservedWords(QString &equation)
{
    QStringList reservedWords = QStringList() << "in";
    QStringList replacementWords = QStringList() << "RESERVEDBYPYTHON1";

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
            qDebug() << "Word = " << word;
            if(reservedWords.contains(word))
            {
                qDebug() << "Replacing: " << equation;
                equation.remove(i-word.size(), word.size());
                equation.insert(i-word.size(), replacementWords[reservedWords.indexOf(word)]);
                qDebug() << "Replaced: " << equation;
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
        qDebug() << "Word = " << word;
        if(reservedWords.contains(word))
        {
            qDebug() << "Replacing: " << equation;
            equation.remove(equation.size()-word.size(), word.size());
            equation.insert(equation.size(), replacementWords[reservedWords.indexOf(word)]);
            qDebug() << "Replaced: " << equation;
        }
    }
}



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


void identifyDerivatives(QStringList &equations)
{
    qDebug() << "Before derivative check: " << equations;
    for(int i=0; i<equations.size(); ++i)
    {
        while(equations[i].contains("der("))
        {
            int j = equations[i].indexOf("der(");
            int k = equations[i].indexOf(")",j);
            qDebug() << "j = " << j << ", k = " << k;
            equations[i].remove(k, 1);
            equations[i].remove(j, 4);
            equations[i].insert(j, "s*");
        }
    }
    qDebug() << "After derivative check: " << equations;
}


void translateDelaysFromPython(QStringList &equations, QStringList &delayTerms, QStringList &delaySteps)
{
    qDebug() << "Before delay translation: " << equations;

    int delayNum = 0;
    for(int i=0; i<equations.size(); ++i)
    {
        QStringList plusTerms = equations.at(i).split("+");
        QStringList terms;
        for(int j=0; j<plusTerms.size(); ++j)
        {
            QStringList minusTerms = plusTerms.at(j).split("-");
            terms.append(minusTerms);
        }
        //QStringList delayTerms;
        for(int j=0; j<terms.size(); ++j)
        {
            if(terms.at(j).contains("qi00"))
            {
                delayTerms.append(terms.at(j));
            }
        }
        qDebug() << "Terms with delay: " << delayTerms;
        for(int j=0; j<delayTerms.size(); ++j)
        {
            QString before = delayTerms.at(j);
            int steps = 0;
            if(before.at(before.indexOf("ui00")+4) == '*' && before.at(before.indexOf("ui00")+5) == '*')
            {
                steps = QString(before.at(before.indexOf("ui00")+6)).toInt();
            }

            QString after = "mDelay"+QString().setNum(delayNum)+".getIdx("+QString().setNum(steps+1)+")";
            equations[i].replace(delayTerms.at(j), after);
            ++delayNum;

            delayTerms[j] = before.replace("qi00", "1");        //Replace delay operators with ones, to "remove" them
            while(delayTerms[j].endsWith(" "))
                delayTerms[j].chop(1);
            delayTerms[j] = delayTerms[j].replace("*1*", "*");  //Attempt to be "smart" and remove unnecessary ones
            delayTerms[j] = delayTerms[j].replace("*1/", "/");
            delayTerms[j] = delayTerms[j].replace("/1*", "*");
            if(delayTerms[j].startsWith("1*"))
                delayTerms[j].remove(0, 2);
            if(delayTerms[j].endsWith("*1"))
                delayTerms[j].chop(2);

            delaySteps << QString().setNum(steps);
        }
    }
    qDebug() << "After delay translation: " << equations;
    qDebug() << "Delay terms: " << delayTerms;
}




void parseModelicaModel(QString code, QString &typeName, QString &displayName, QStringList &equations, QList<PortSpecification> &portList, QList<ParameterSpecification> &parametersList)
{
    qDebug() << "Hej1";
    QStringList lines = code.split("\n");
    qDebug() << "Hej2, lines = " << lines;
    QStringList portNames;
    bool equationPart = false;
    for(int l=0; l<lines.size(); ++l)
    {
        if(!equationPart)
        {
            QStringList words = lines.at(l).trimmed().split(" ");
            qDebug() << "Hej3, words = " << words;
            if(words.at(0) == "model")
            {
                typeName = words.at(1);
                if(words.size() > 2)
                {
                    displayName = words.at(2);
                    displayName.remove(0, 1);       //Remove first "
                    int j=2;
                    while(!words.at(j).endsWith("\""))
                    {
                        ++j;
                        displayName.append(" " + words.at(j));
                    }
                    displayName.chop(1);            //Remove last "
                }
            }
            else if(words.at(0) == "parameter")
            {
                QString name = words.at(2).section("(",0,0);
                QString unit = lines.at(l).section("unit=",1,1).section("\"",1,1);
                QString init;
                if(!words.at(2).section(")", 1).isEmpty())
                    init = words.at(2).section(")", 1).section("=", 1);                   //..blabla)=x
                else if(words.at(2).endsWith("="))
                    init = words.at(3);                                             //..blabla)= x
                else if(words.at(3).startsWith("=") && words.at(3).size() > 1)
                    init = words.at(3).section("=", 1);                              //..blabla) =x
                else if(words.at(3) == "=")
                    init = words.at(4);                                             // ...blabla) = x

                QString parDisplayName = lines.at(l).section("\"", -2, -2);

                ParameterSpecification par(name, parDisplayName, parDisplayName, unit, init);
                parametersList.append(par);
            }
            else if(words.at(0) == "NodeMechanic")
            {
                PortSpecification port("PowerPort", "NodeMechanic", words.at(1));
                portList.append(port);
                portNames << words.at(1);
            }
            else if(words.at(0) == "NodeMechanicRotational")
            {
                PortSpecification port("PowerPort", "NodeMechanicRotational", words.at(1));
                portList.append(port);
                portNames << words.at(1);
            }
            else if(words.at(0) == "NodeHydraulic")
            {
                for(int i=0; i<lines.at(l).count(",")+1; ++i)
                {
                    QString name = lines.at(l).trimmed().section(" ", 1).section(",",i,i).section(";",0,0).trimmed();
                    qDebug() << "lines.at(l).trimmed() = " << lines.at(l).trimmed();
                    qDebug() << "lines.at(l).trimmed().section(" ", 1) = " << lines.at(l).trimmed().section(" ", 1);
                    qDebug() << "lines.at(l).trimmed().section(" ", 1).section(\",\", i)) = " << lines.at(l).trimmed().section(" ", 1).section(",",i,i);
                    PortSpecification port("PowerPort", "NodeHydraulic", name);
                    portList.append(port);
                    portNames << name;
                }
            }
            else if(words.at(0) == "NodePneumatic")
            {
                PortSpecification port("PowerPort", "NodePneumatic", words.at(1));
                portList.append(port);
                portNames << words.at(1);
            }
            else if(words.at(0) == "NodeElectric")
            {
                PortSpecification port("PowerPort", "NodeElectric", words.at(1));
                portList.append(port);
                portNames << words.at(1);
            }
            else if(words.at(0) == "equation")
            {
                equationPart = true;
            }
        }
        else
        {
            if(lines.at(l).trimmed().startsWith("end "))            //Finished if this is the "end" line
                return;
            qDebug() << "Equation: " << lines.at(l).trimmed();
            equations << lines.at(l).trimmed();
            for(int i=0; i<portNames.size(); ++i)
            {
                QString temp = portNames.at(i)+".";
                while(equations.last().contains(temp))
                {
                    qDebug() << "BEFORE: " << equations.last();
                    int idx = equations.last().indexOf(temp);
                    int idx2=idx+temp.size()+1;
                    while(idx2 < equations.last().size()+1 && equations.last().at(idx2).isLetterOrNumber())
                        ++idx2;
                    equations.last().insert(idx2, QString().setNum(i+1));
                    equations.last().remove(idx, temp.size());
                    qDebug() << "AFTER: " << equations.last();
                }
            }
            equations.last().chop(1);
        }
    }
}
