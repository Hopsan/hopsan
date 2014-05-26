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
//! @file   ModelicaHandler.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-05-14
//!
//! @brief Contains a handler for modelica models
//!
//$Id: ModelHandler.h 6629 2014-02-24 14:06:24Z petno25 $

#include <QString>
#include <QMap>
#include <QStringList>
#include <QPair>
#include <QFile>

#include "global.h"
#include "ModelicaLibrary.h"
#include "SymHop.h"
#include "DesktopHandler.h"


//! @class ModelicaModel
//! @brief A class representing a model in the Modelica language

//! @brief Constructor
//! @param rCode Model code
ModelicaModel::ModelicaModel(const QString &rCode)
{
    mCodeLines = rCode.split("\n");
}


//! @brief Copy constructor
ModelicaModel::ModelicaModel(const ModelicaModel &rOther)
{
    mCodeLines = rOther.mCodeLines;
}


//! @brief Returns a list of all parameters in the model
//! @param rParameters Referens map between parameter name and parameter type and defaultvalue
QList<ModelicaVariable> ModelicaModel::getParameters() const
{
    QList<ModelicaVariable> retval;
    foreach(const QString constLine, mCodeLines)
    {
        QString line = constLine;
        line = line.simplified();
        if(line.startsWith("parameter "))
        {
            QString name = line.section(" ",2,2).section(";",0,0).section("(",0,0);
            QString type = line.section(" ",1,1);
            QString defaultValue = line.section("=",-1,-1).simplified().remove(";");
            retval.append(ModelicaVariable(name, type, defaultValue.toDouble()));
        }
    }

    return retval;
}


//! @brief Returns all variables in the model
//! @param rVariables Referens map between parameter names and parameter types
QList<ModelicaVariable> ModelicaModel::getVariables() const
{
    QList<ModelicaVariable> retval;
    bool inAnnotations=false;
    int parCount=0;
    foreach(const QString &constLine, mCodeLines)
    {
        QString line = constLine.simplified();

        //Ignore model declaration line
        if(line.startsWith("model "))
            continue;

        //Ignore annotations section
        if(line.startsWith("annotation("))
        {
            inAnnotations=true;
            parCount = line.count("(")-line.count(")");
            continue;
        }
        if(parCount != 0)
        {
            parCount += line.count("(")-line.count(")");
            continue;
        }
        else if(inAnnotations)
        {
            inAnnotations = false;
        }

        //Ignore equations section and everything after
        if(line.trimmed() == "equation")
            break;

        //If equation is "Tank tank1,tank2;", then type="Tank" and names = ["tank1", "tank2"]
        line = line.remove(";").simplified();
        line.remove("parameter ");
        while(line.contains(", "))
            line.replace(", ", ",");
        while(line.contains(" ,"))
            line.replace(" ,",",");
        QString type = line.section(" ",0,0);
        QString nameStr = line.section(" ",1,1);
        QStringList names;
        while(!nameStr.isEmpty())
        {
            if(!nameStr.contains(",") && !nameStr.contains("("))
            {
                names.append(nameStr);
                nameStr.clear();
            }
            else if((nameStr.indexOf(",") > -1 && nameStr.indexOf("(") == -1) ||
                    (nameStr.indexOf(",") > -1 && nameStr.indexOf(",") < nameStr.indexOf("(")))
            {
                names.append(nameStr.section(",",0,0));
                nameStr.remove(0, names.last().size()+1);
            }
            else
            {
                names.append(nameStr.section("(",0,0));
                nameStr.remove(0,nameStr.indexOf(")")+1);
            }
        }

        foreach(const QString &name, names)
        {
            retval.append(ModelicaVariable(name, type));
        }
    }

    return retval;
}


//! @brief Returns all equations in the model
//! @param rEquations Reference to list of all equation lines
void ModelicaModel::getEquations(QStringList &rEquations) const
{
    bool inEquations=false;
    foreach(const QString &line, mCodeLines)
    {
        if(!inEquations)
        {
            if(line.simplified() == "equation")
            {
                inEquations = true;
            }
        }
        else
        {
            if(!line.trimmed().startsWith("end "))
            {
                rEquations.append(line.trimmed());
            }
        }
    }
}

QString ModelicaModel::getAnnotations() const
{
    QString retval;
    bool inAnnotations=false;
    foreach(const QString &line, mCodeLines)
    {
        if(line.simplified().startsWith("annotation("))
        {
            inAnnotations=true;
        }
        if(inAnnotations)
        {
            int parbal=line.count("(")-line.count(")");
            retval.append(line);
            if(parbal == 0)
            {
                return retval;
            }
        }
    }
    return QString();
}


//! @brief Returns a flattened equation system, which contains all model equations and equations from submodels in one equation system
//! @param rEquations Referens to list of equations
//! @param rLocalVars Reference to a map between name and type of local variables
//! @param rPrefix Used to prepend a prefix to the equations (to be used for recursive calls to this function)
//! @param rSubModels Used to generate equations for only a subset of submodels. Empty means use all submodels.
void ModelicaModel::toFlatEquations(QStringList &rEquations, QMap<QString,QString> &rLocalVars, const QString &rPrefix, const QStringList &rSubModels)
{
    //! @todo Should be possible to get only equations from subset of submodels

    QString prefix = rPrefix;
    if(!prefix.isEmpty())
    {
        prefix = prefix+"__";
    }

    int orgSize = rEquations.size();
    getEquations(rEquations);
    for(int i=orgSize; i<rEquations.size(); ++i)
    {
        if(rEquations[i].startsWith("connect("))
        {
            QString compName1,compName2,portName1,portName2,compType1,compType2,portType1,portType2;
            compName1=rEquations[i].section("(",1,1).section(".",0,0).trimmed();
            compName2=rEquations[i].section(",",1,1).section(".",0,0).trimmed();
            portName1=rEquations[i].section(".",1,1).section(",",0,0).trimmed();
            portName2=rEquations[i].section(".",2,2).section(")",0,0).trimmed();

            if(!rSubModels.isEmpty() && !rSubModels.contains(compName1) && !rSubModels.contains(compName2))
            {
                rEquations.removeAt(i);
                --i;
                continue;
            }

            //qDebug() << "Connect: " << compName1 << portName1 << compName2 << portName2;

            QList<ModelicaVariable> variables = this->getVariables();
            foreach(const ModelicaVariable &var, variables)
            {
                if(var.getName() == compName1)
                    compType1 = var.getType();
                if(var.getName() == compName2)
                    compType2 = var.getType();
            }

            //qDebug() << "Types: " << compType1 << compType2;

            QList<ModelicaVariable> variables1, variables2;
            if(!compType1.startsWith("TLM_"))
            {
                ModelicaModel model1 = gpModelicaLibrary->getModel(compType1);
                variables1 = model1.getVariables();
                foreach(const ModelicaVariable &var, variables1)
                {
                    if(var.getName() == portName1)
                        portType1 = var.getType();
                }
            }
            if(!compType2.startsWith("TLM_"))
            {
                ModelicaModel model2 = gpModelicaLibrary->getModel(compType2);
                variables2 = model2.getVariables();
                foreach(const ModelicaVariable &var, variables2)
                {
                    if(var.getName() == portName2)
                        portType2 = var.getType();
                }
            }

            if(portType1.isEmpty())
                portType1 = portType2;
            else if(portType2.isEmpty())
                portType2 = portType1;

            //qDebug() << "Port types: " << portType1 << portType2;

            //! @todo Check that both ports are of same type! (should not be allowed to connect different types though)

            ModelicaConnector connector;
            gpModelicaLibrary->getConnector(portType1, connector);

            QMap<QString,QString> intensityVariables, flowVariables;
            connector.getIntensityVariables(intensityVariables);
            connector.getFlowVariables(flowVariables);

            //qDebug() << "Intensity variables: " << intensityVariables;
            //qDebug() << "Flow variables: " << flowVariables;

            foreach(const QString &var, intensityVariables.keys())
            {
                rEquations.append(prefix+compName1+"__"+portName1+"__"+var+" = "+
                                 prefix+compName2+"__"+portName2+"__"+var);

                rLocalVars.insert(prefix+compName1+"__"+portName1+"__"+var, "Real");
                rLocalVars.insert(prefix+compName2+"__"+portName2+"__"+var, "Real");
            }
            foreach(const QString &var, flowVariables.keys())
            {
                if(compType1.startsWith("TLM_") || compType2.startsWith("TLM_"))        //No minus sign for flow in connection to TLM component (due to convention)
                {
                    rEquations.append(prefix+compName1+"__"+portName1+"__"+var+" = "+
                                     prefix+compName2+"__"+portName2+"__"+var);
                }
                else        //Modelica connection, use minus sign for flow (q1 = -q2)
                {
                    rEquations.append(prefix+compName1+"__"+portName1+"__"+var+" = -"+
                                     prefix+compName2+"__"+portName2+"__"+var);

                }
                rLocalVars.insert(prefix+compName1+"__"+portName1+"__"+var, "Real");
                rLocalVars.insert(prefix+compName2+"__"+portName2+"__"+var, "Real");
            }

            //Handle TLM connections
            // - add the TLM equation
            // - add the port as a local variable
            // - remove corresponding Q variables from local variables (added due to connection above)
            if(compType1.startsWith("TLM_"))
            {
                if(portType1 == "NodeHydraulic")
                {
                    QString portStr = prefix+compName1+"__"+portName1;
                    rEquations.append(portStr+"__p = "+portStr+"__c+"+portStr+"__q*"+portStr+"__Zc");
                    rLocalVars.insert(prefix+compName1+"__"+portName1,"NodeHydraulic");
                    rLocalVars.remove(prefix+compName1+"__"+portName1+"__p");
                    rLocalVars.remove(prefix+compName1+"__"+portName1+"__q");
                }
                else if(portType1 == "NodeMechanic")
                {
                    QString portStr = prefix+compName1+"__"+portName1;
                    rEquations.append(portStr+"__f = "+portStr+"__c+"+portStr+"__v*"+portStr+"__Zc");
                    rLocalVars.insert(prefix+compName1+"__"+portName1,"NodeHydraulic");
                    rLocalVars.remove(prefix+compName1+"__"+portName1+"__f");
                    rLocalVars.remove(prefix+compName1+"__"+portName1+"__x");
                    rLocalVars.remove(prefix+compName1+"__"+portName1+"__v");
                }
            }
            if(compType2.startsWith("TLM_"))
            {
                if(portType2 == "NodeHydraulic")
                {
                    QString portStr = prefix+compName2+"__"+portName2;
                    rEquations.append(portStr+"__p = "+portStr+"__c+"+portStr+"__q*"+portStr+"__Zc");
                    rLocalVars.insert(prefix+compName2+"__"+portName2,"NodeHydraulic");
                    rLocalVars.remove(prefix+compName2+"__"+portName2+"__p");
                    rLocalVars.remove(prefix+compName2+"__"+portName2+"__q");
                }
                else if(portType2 == "NodeMechanic")
                {
                    QString portStr = prefix+compName2+"__"+portName2;
                    rEquations.append(portStr+"__f = "+portStr+"__c+"+portStr+"__v*"+portStr+"__Zc");
                    rLocalVars.insert(prefix+compName2+"__"+portName2,"NodeHydraulic");
                    rLocalVars.remove(prefix+compName2+"__"+portName2+"__f");
                    rLocalVars.remove(prefix+compName2+"__"+portName2+"__x");
                    rLocalVars.remove(prefix+compName2+"__"+portName2+"__v");
                }
            }

            rEquations.removeAt(i);
            --i;
        }
        else
        {
            //qDebug() << "Equation: " << equations[i];
            //if(!equations[i].startsWith(prefix+"."))
            //    equations[i].prepend(prefix+".");
            SymHop::Expression equation(rEquations[i].remove(";"));
            QList<SymHop::Expression> vars = equation.getVariables();
            for(int v=0; v<vars.size(); ++v)
            {
                QString var = vars[v].toString();
                var.replace(".","__");
                if(!var.startsWith(prefix))
                {
                    var = prefix+var;
                }
                equation.replace(vars[v], SymHop::Expression(var));
            }
            rEquations[i] = equation.toString();
            //qDebug() << "New equation: " << equations[i];


            //equations[i].replace(".", "__");   //Replace dots with underscores, to make variable names C++ compatible
        }
    }

    //Add parameters to local vars
    QList<ModelicaVariable> parameters = this->getParameters();
    foreach(const ModelicaVariable &par, parameters)
    {
        QString varName = prefix+par.getName();
        varName.replace(".","__");
        QString varType = "parameter "+par.getType();
        QString defaultValue = QString::number(par.getDefaultValue());
        rLocalVars.insert(varName+"(unit=\"\") = "+defaultValue, varType);
    }

    QList<ModelicaVariable> variables = this->getVariables();
    foreach(const ModelicaVariable &par, parameters)
    {
        for(int v=0; v<variables.size(); ++v)
        {
            if(variables[v].getName() == par.getName())
            {
                variables.removeAt(v);
                --v;
            }
        }
    }

    foreach(const ModelicaVariable &var, variables)
    {
        if(rSubModels.contains(var.getName()))
        {
            ModelicaModel model = gpModelicaLibrary->getModel(var.getType());
            model.toFlatEquations(rEquations, rLocalVars, prefix+var.getName());
        }
        else
        {
            QString varName = prefix+var.getName();
            varName.replace(".","__");
            QString varType = var.getType();
            if(!rLocalVars.contains(varName))
            {
                rLocalVars.insert(varName, varType);
            }
        }
    }
}


//! @class ModelicaConnector
//! @brief A class representing connectors in the Modelica language

//! @brief Constructor
//! @param rCode  Code for the connector
ModelicaConnector::ModelicaConnector(const QString &rCode)
{
    mCodeLines = rCode.split("\n");
}


//! @brief Returns all intensity variables in the connector
//! @param rVariables Reference to map between name and type of the variables
void ModelicaConnector::getIntensityVariables(QMap<QString, QString> &rVariables)
{
    foreach(const QString &line, mCodeLines.mid(1,mCodeLines.size()-2))
    {
        if(!line.simplified().startsWith("flow "))
        {
            QString type = line.simplified().section(" ",0,0).trimmed();
            QString name = line.simplified().section(" ",1,1).remove(";").trimmed();
            rVariables.insert(name, type);
        }
    }
}


//! @brief Returns all flow variables in the connector
//! @param rVariables Reference to map between name and type of the variables
void ModelicaConnector::getFlowVariables(QMap<QString, QString> &rVariables)
{
    foreach(const QString &line, mCodeLines.mid(1,mCodeLines.size()-2))
    {
        if(line.simplified().startsWith("flow "))
        {
            QString type = line.simplified().section(" ",1,1).trimmed();
            QString name = line.simplified().section(" ",2,2).remove(";").trimmed();
            rVariables.insert(name, type);
        }
    }
}








//! @class ModelicaLibrary
//! @brief A class for a library of models in the Modelica language


//! @brief Constructor
ModelicaLibrary::ModelicaLibrary()
{
    QFile moFile(gpDesktopHandler->getDocumentsPath()+"/Models/modelica.mo");   //Hard-coded for now, should not be like this at all
    moFile.open(QFile::ReadOnly | QFile::Text);
    QString moCode = moFile.readAll();
    moFile.close();

    QStringList moLines = moCode.split("\n");
    bool inConnector = false;
    bool inModel = false;
    QString temp, name;
    foreach(const QString &line, moLines)
    {
        if(line.simplified().startsWith("connector "))
        {
            inConnector = true;
            name = line.simplified().section(" ",1,1).section(" ",0,0);
        }
        else if(line.simplified().startsWith("model "))
        {
            inModel = true;
            name = line.simplified().section(" ",1,1).section(" ",0,0);
        }
        else if(line.simplified().startsWith("end ") && inConnector)
        {
            inConnector = false;
            temp.append(line);
            mConnectorsMap.insert(name, temp);
            temp.clear();
        }
        else if(line.simplified().startsWith("end ") && inModel)
        {
            inModel = false;
            temp.append(line);
            mModelsMap.insert(name, temp);
            temp.clear();
        }

        if(inConnector || inModel)
        {
            temp.append(line+"\n");
        }
    }



//    mConnectorsMap.insert("NodeHydraulic", ModelicaConnector("connector NodeHydraulic \"Hydraulic Connector\"\n"
//                                                             "Real         p \"Pressure\";\n"
//                                                             "flow Real    q \"Volume flow\"\n"
//                                                             "end Pin;"));

//    mModelsMap.insert("FlowSource", ModelicaModel("model FlowSource\n"
//                                                  "    annotation("
//                                                  "hopsanIcon = \"/home/robbr48/Documents/Subversion/HOPSAN++/componentLibraries/defaultLibrary/Hydraulic/Sources&Sinks/flowsource_user.svg\""
//                                                  ","
//                                                  "portPos = \"P1,1,0.5,0\""
//                                                  ");\n"
//                                                  "    parameter Real q_ref(unit=\"m^3/s\") = 0.001;\n"
//                                                  "    NodeHydraulic P1;\n"
//                                                  "equation\n"
//                                                  "    P1.q = q_ref;\n"
//                                                  "end FlowSource;"));

//    mModelsMap.insert("Tank", ModelicaModel("model Tank\n"
//                                            "    NodeHydraulic P1;\n"
//                                            "equation\n"
//                                            "    P1.p = 100000;\n"
//                                            " end Tank;"));

//    mModelsMap.insert("LaminarOrifice", ModelicaModel("model LaminarOrifice\n"
//                       "    annotation("
//                       "hopsanIcon = \"/home/robbr48/Documents/Subversion/HOPSAN++/componentLibraries/defaultLibrary/Hydraulic/Restrictors/laminarorifice_user.svg\""
//                       ","
//                       "portPos = \"P1,0,0.5,0\""
//                       ","
//                       "portPos = \"P2,1,0.5,0\""
//                       ");\n"
//                       "    parameter Real Kc = 1e-11;\n"
//                       "    NodeHydraulic P1, P2;\n"
//                       "equation\n"
//                       "    P2.q = Kc*(P1.p-P2.p);\n"
//                       "    0 = P1.q + P2.q;\n"
//                       "end LaminarOrifice;"));

//    mModelsMap.insert("Volume", ModelicaModel("model Volume\n"
//                      "    parameter Real V(unit=\"m^3\") = 0.001;\n"
//                      "    parameter Real betae(unit=\"Pa\") = 1e9;\n"
//                      "    NodeHydraulic P1, P2;\n"
//                      "equation\n"
//                      "    -V/betae*der(P1.p) = P1.q+P2.q;\n"
//                      "    P1.p = P2.p;\n"
//                      "end Volume;"));

}

QStringList ModelicaLibrary::getModelNames() const
{
    return mModelsMap.keys();
}

QStringList ModelicaLibrary::getConnectorNames() const
{
    return mConnectorsMap.keys();
}





//! @brief Tells whether or not specified model exists in library
//! @param rModelName Reference string with model name
bool ModelicaLibrary::hasModel(const QString &rModelName)
{
    return mModelsMap.contains(rModelName);
}


//! @brief Returns a modelica model object for specified model name
//! @param rModelName Reference to model name string
ModelicaModel ModelicaLibrary::getModel(const QString &rModelName)
{
    if(mModelsMap.contains(rModelName))
    {
        return mModelsMap.find(rModelName).value();
    }
    else
    {
        return ModelicaModel();
    }
}


//! @brief Returns a Modelica connector for specified connector name
//! @param rConnectorName Reference to name string of connector
//! @param rConnector Reference to connector object
void ModelicaLibrary::getConnector(const QString &rConnectorName, ModelicaConnector &rConnector)
{
    rConnector = mConnectorsMap.find(rConnectorName).value();
}



ModelicaVariable::ModelicaVariable(const QString &name, const QString &type, const double defaultValue)
{
    mName = name;
    mType = type;
    mDefaultValue = defaultValue;
}


const QString &ModelicaVariable::getName() const
{
    return mName;
}

const QString &ModelicaVariable::getType() const
{
    return mType;
}

double ModelicaVariable::getDefaultValue() const
{
    return mDefaultValue;
}

bool ModelicaVariable::operator==(const ModelicaVariable &rhs)
{
    return (mDefaultValue == rhs.mDefaultValue &&
            mName == rhs.mName &&
            mType == rhs.mType);
}
