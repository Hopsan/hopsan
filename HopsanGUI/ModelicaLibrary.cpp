/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

//!
//! @file   ModelicaHandler.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-05-14
//!
//! @brief Contains a handler for modelica models
//!
//$Id$

#include <QString>
#include <QMap>
#include <QStringList>
#include <QPair>
#include <QFile>
#include <QFileDialog>

#include "global.h"
#include "ModelicaLibrary.h"
#include "SymHop.h"
#include "DesktopHandler.h"
#include "ModelHandler.h"
#include "Widgets/ModelWidget.h"
#include "GUIObjects/GUISystem.h"
#include "GUIObjects/GUIModelObject.h"
#include "Widgets/LibraryWidget.h"
#include "LibraryHandler.h"
#include "MessageHandler.h"
#include "Configuration.h"

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
//! @param rParameters Reference map between parameter name and parameter type and default value
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
//! @param rVariables Reference map between parameter names and parameter types
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


//! @brief Returns all initial algorithms in the model
//! @param rAlgorithms Reference to list of all initial algorithm lines
void ModelicaModel::getInitAlgorithms(QStringList &rAlgorithms) const
{
    bool inInitAlgs=false;
    foreach(const QString &line, mCodeLines)
    {
        if(!inInitAlgs)
        {
            if(line.simplified() == "initial algorithm")
            {
                inInitAlgs = true;
            }
        }
        else
        {
            if(line.simplified().startsWith("end ") || line.simplified().startsWith("equation") || line.simplified().startsWith("algorithm"))
            {
                break;
            }
            else
            {
                rAlgorithms.append(line.trimmed());
            }
        }
    }
}

//! @brief Returns all pre-simulation algorithms in the model
//! @param rAlgorithms Reference to list of all pre-simulation algorithm lines
void ModelicaModel::getPreAlgorithms(QStringList &rAlgorithms) const
{
    bool inPreAlgs=false;
    foreach(const QString &line, mCodeLines)
    {
        if(!inPreAlgs)
        {
            if(line.simplified() == "algorithm")
            {
                inPreAlgs = true;
            }
        }
        else
        {
            if(line.simplified().startsWith("end ") || line.simplified().startsWith("equation"))
            {
                break;
            }
            else
            {
                rAlgorithms.append(line.trimmed());
            }
        }
    }
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
    int parbal=0;
    foreach(const QString &line, mCodeLines)
    {
        if(line.simplified().startsWith("annotation("))
        {
            inAnnotations=true;
        }
        if(inAnnotations)
        {
            parbal += line.count("(")-line.count(")");
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
//! @param rEquations Reference to list of equations
//! @param rLocalVars Reference to a map between name and type of local variables
//! @param rPrefix Used to prepend a prefix to the equations (to be used for recursive calls to this function)
//! @param rSubModels Used to generate equations for only a subset of submodels. Empty means use all submodels.
void ModelicaModel::toFlatEquations(QStringList &rInitAlgorithms, QStringList &rPreAlgorithms, QStringList &rEquations, QMap<QString,QString> &rLocalVars, const QString &rPrefix, const QStringList &rSubModels)
{
    //! @todo Should be possible to get only equations from subset of submodels

    QString prefix = rPrefix;
    if(!prefix.isEmpty())
    {
        prefix = prefix+"__";
    }


    int orgSize = rInitAlgorithms.size();
    getInitAlgorithms(rInitAlgorithms);
    for(int i=orgSize; i<rInitAlgorithms.size(); ++i)
    {
        SymHop::Expression algorithm(rInitAlgorithms[i].remove(";"));
        QList<SymHop::Expression> vars = algorithm.getVariables();
        for(int v=0; v<vars.size(); ++v)
        {
            QString var = vars[v].toString();
            var.replace(".","__");
            if(!var.startsWith(prefix))
            {
                var = prefix+var;
            }
            algorithm.replace(vars[v], SymHop::Expression(var));
        }
        rInitAlgorithms[i] = algorithm.toString();
    }

    orgSize = rPreAlgorithms.size();
    getPreAlgorithms(rPreAlgorithms);
    for(int i=orgSize; i<rPreAlgorithms.size(); ++i)
    {
        SymHop::Expression algorithm(rPreAlgorithms[i].remove(";"));
        QList<SymHop::Expression> vars = algorithm.getVariables();
        for(int v=0; v<vars.size(); ++v)
        {
            QString var = vars[v].toString();
            var.replace(".","__");
            if(!var.startsWith(prefix))
            {
                var = prefix+var;
            }
            algorithm.replace(vars[v], SymHop::Expression(var));
        }
        rPreAlgorithms[i] = algorithm.toString();
    }

    orgSize = rEquations.size();
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

            foreach(const QString &var, flowVariables.keys())
            {
                rEquations.append(prefix+compName1+"__"+portName1+"__"+var+" = "+
                                 prefix+compName2+"__"+portName2+"__"+var);

                rLocalVars.insert(prefix+compName1+"__"+portName1+"__"+var, "Real");
                rLocalVars.insert(prefix+compName2+"__"+portName2+"__"+var, "Real");
            }
            foreach(const QString &var, intensityVariables.keys())
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
                if(portType1 == "NodeSignalIn")
                {
                    rLocalVars.insert(prefix+compName1+"__"+portName1, "NodeSignalIn");
                    rLocalVars.remove(prefix+compName1+"__"+portName1+"__y");
                }
                else if(portType1 == "NodeHydraulic")
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
                    rLocalVars.insert(prefix+compName1+"__"+portName1,"NodeMechanic");
                    rLocalVars.remove(prefix+compName1+"__"+portName1+"__f");
                    rLocalVars.remove(prefix+compName1+"__"+portName1+"__x");
                    rLocalVars.remove(prefix+compName1+"__"+portName1+"__v");
                }
            }
            if(compType2.startsWith("TLM_"))
            {
                if(portType2 == "NodeSignalIn")
                {
                    rLocalVars.insert(prefix+compName2+"__"+portName2, "NodeSignalIn");
                    rLocalVars.remove(prefix+compName2+"__"+portName2+"__y");
                }
                else if(portType2 == "NodeHydraulic")
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
                    rLocalVars.insert(prefix+compName2+"__"+portName2,"NodeMechanic");
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
            model.toFlatEquations(rInitAlgorithms, rPreAlgorithms, rEquations, rLocalVars, prefix+var.getName());
        }
        else
        {
            QString varName = prefix+var.getName();
            varName.replace(".","__");
            QString varType = var.getType();
            if(!rLocalVars.contains(varName) && varType != "NodeMechanic" && varType != "NodeHydraulic" && varType != "NodeMechanicRotational" && varType != "NodeSignalIn")
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
    mModelicaFiles = gpConfig->getModelicaFiles();
    reload();
}


void ModelicaLibrary::reload()
{
    //! @todo We must check that the code is ok!

    //Clear all contents first
    mModelsMap.clear();
    mConnectorsMap.clear();

    foreach(const QString &file, mModelicaFiles)
    {
        QFile moFile(file);
        if(!moFile.open(QFile::Text | QFile::ReadOnly))
        {
            gpMessageHandler->addErrorMessage("Unable to read Modelica file: "+file);
            continue;
        }
        QString code = moFile.readAll();
        moFile.close();

        qDebug() << "Code: " << code;

        QStringList moLines = code.split("\n");
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

        //Update all existing modelica components
        for(int m=0; gpModelHandler && m<gpModelHandler->count(); ++m)
        {
            SystemContainer *pSystem = gpModelHandler->getModel(m)->getTopLevelSystemContainer();
            for(int c=0; c<pSystem->getModelObjects().size(); ++c)
            {
                ModelObject *pComp = pSystem->getModelObjects().at(c);
                if(pComp->getTypeName() == MODELICATYPENAME)
                {
                    pComp->setParameterValue("model", pComp->getParameterValue("model"));
                }
            }
        }
    }




    if(gpLibraryWidget)
        gpLibraryWidget->update();
}

void ModelicaLibrary::loadModelicaFile()
{
    //Load .fmu file and create paths
    QString filePath = QFileDialog::getOpenFileName(gpMainWindowWidget, gpMainWindowWidget->tr("Load Modelica File"),
                                                    gpConfig->getStringSetting(CFG_MODELICAMODELSDIR),
                                                    gpMainWindowWidget->tr("Modelica files (*.mo)"));
    if(filePath.isEmpty())      //Cancelled by user
        return;

    gpConfig->setStringSetting(CFG_MODELICAMODELSDIR, QFileInfo(filePath).absoluteFilePath());

    loadModelicaFile(filePath);
}

void ModelicaLibrary::loadModelicaFile(const QString &fileName)
{
    if(!mModelicaFiles.contains(fileName))
    {
        mModelicaFiles.append(fileName);
        gpConfig->addModelicaFile(fileName);
        reload();
    }
}

void ModelicaLibrary::unloadModelicaFile(const QString &fileName)
{
    mModelicaFiles.removeAll(fileName);
    reload();
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

void ModelicaLibrary::getModelicaFiles(QStringList &files) const
{
    files = mModelicaFiles;
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
