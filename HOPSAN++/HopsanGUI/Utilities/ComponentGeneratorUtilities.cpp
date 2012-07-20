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

#include <QStringList>
#include <QProcess>


#include "Configuration.h"
#include "MainWindow.h"
#include "GUIObjects/GUIModelObjectAppearance.h"
#include "Utilities/ComponentGeneratorUtilities.h"
#include "Utilities/SymHop.h"
#include "Widgets/LibraryWidget.h"
#include "Widgets/MessageWidget.h"
#include "common.h"


using namespace std;
using namespace SymHop;


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
void generateComponentObject(QString outputFile, QDomElement &rDomElement, ModelObjectAppearance appearance, QProgressDialog *pProgressBar)
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

    compileComponentObject(outputFile, comp, appearance, true);
}






//! @brief Generates XML and compiles the new component
void generateComponentObject(QString &typeName, QString &displayName, QString &cqsType, QStringList &plainInitAlgorithms, QStringList &plainEquations, QStringList &plainFinalAlgorithms, QList<PortSpecification> &ports, QList<ParameterSpecification> &parameters)
{

    QProgressDialog *pProgressBar = new QProgressDialog("Verifying", QString(), 0, 0, gpMainWindow);
    pProgressBar->setWindowFlags(Qt::Window);
    pProgressBar->setWindowModality(Qt::ApplicationModal);
    pProgressBar->setWindowTitle("Generating Hopsan Component");
    pProgressBar->setMinimum(0);
    pProgressBar->setMaximum(100);
    pProgressBar->setValue(0);
    pProgressBar->setMinimumDuration(0);
    pProgressBar->show();

            pProgressBar->setLabelText("Collecting equations");
            pProgressBar->setValue(5);

    //Create list of initial algorithms
    QList<Expression> initAlgorithms;
    for(int i=0; i<plainInitAlgorithms.size(); ++i)
    {
        initAlgorithms.append(Expression(plainInitAlgorithms.at(i)));
    }

            pProgressBar->setValue(3);

    //Create list of equqtions
    QList<Expression> equations;
    for(int i=0; i<plainEquations.size(); ++i)
    {
        equations.append(Expression(plainEquations.at(i)));
    }

    //Create list of final algorithms
    QList<Expression> finalAlgorithms;
    for(int i=0; i<plainFinalAlgorithms.size(); ++i)
    {
        finalAlgorithms.append(Expression(plainFinalAlgorithms.at(i)));
    }

            pProgressBar->setValue(4);

    //Identify variable limitations, and remove them from the equations list
    QList<Expression> limitedVariables;
    QList<Expression> limitedDerivatives;
    QList<Expression> limitMinValues;
    QList<Expression> limitMaxValues;
    QList<int> limitedVariableEquations;
    QList<int> limitedDerivativeEquations;
    for(int i=0; i<equations.size(); ++i)
    {
        if(equations[i].getFunctionName() == "VariableLimits")
        {
            if(i<1)
            {
                //! @todo Use sorting instead?
                gpMainWindow->mpMessageWidget->printGUIErrorMessage("VariableLimits not preceeded by equations defining variable.");
                return;
            }

            limitedVariables << equations[i].getArgument(0);
            limitedDerivatives << Expression();
            limitMinValues << equations[i].getArgument(1);
            limitMaxValues << equations[i].getArgument(2);
            limitedVariableEquations << i-1;
            limitedDerivativeEquations << -1;

            equations.removeAt(i);
            --i;
        }
        else if(equations[i].getFunctionName()== "Variable2Limits")
        {
            if(i<2)
            {
                gpMainWindow->mpMessageWidget->printGUIErrorMessage("Variable2Limits not preeded by equations defining variable and derivative.");
                return;
            }

            limitedVariables << equations[i].getArgument(0);
            limitedDerivatives << equations[i].getArgument(1);
            limitMinValues << equations[i].getArgument(2);
            limitMaxValues << equations[i].getArgument(3);
            limitedVariableEquations << i-2;
            limitedDerivativeEquations << i-1;

            equations.removeAt(i);
            --i;
        }
        pProgressBar->setValue(4+4*double(i+1)/double(equations.size()));
    }

            pProgressBar->setValue(8);

    //Verify each equation
    for(int i=0; i<equations.size(); ++i)
    {
        if(!equations[i].verifyExpression())
        {
            gpMainWindow->mpMessageWidget->printGUIErrorMessage("Component generation failed: Verification of variables failed.");
            return;
        }
    }

    QList<QList<Expression> > leftSymbols2, rightSymbols2;
    for(int i=0; i<equations.size(); ++i)
    {
        leftSymbols2.append(equations[i].getChild(0).getSymbols());
        rightSymbols2.append(equations[i].getChild(1).getSymbols());
    }

            pProgressBar->setValue(9);

    //Sum up all used variables to a single list
    QList<Expression> allSymbols;
    for(int i=0; i<equations.size(); ++i)
    {
        allSymbols.append(leftSymbols2.at(i));
        allSymbols.append(rightSymbols2.at(i));
    }

    QList<Expression> initSymbols2;
    for(int i=0; i<initAlgorithms.size(); ++i)
    {
        if(!initAlgorithms[i].isAssignment())
        {
            gpMainWindow->mpMessageWidget->printGUIErrorMessage("Component generation failed: Initial algorithms section contains non-algorithms.");
            return;
        }
        initSymbols2.append(initAlgorithms[i].getChild(0));
    }

            pProgressBar->setValue(10);

    QList<Expression> finalSymbols2;
    for(int i=0; i<finalAlgorithms.size(); ++i)
    {
        //! @todo We must check that all algorithms are actually algorithms before doing this!
        if(!finalAlgorithms[i].isAssignment())
        {
            gpMainWindow->mpMessageWidget->printGUIErrorMessage("Component generation failed: Final algorithms section contains non-algorithms.");
            return;
        }
        finalSymbols2.append(finalAlgorithms[i].getChild(0));
    }

    for(int i=0; i<parameters.size(); ++i)
    {
        allSymbols.append(Expression(parameters[i].name));
    }
    allSymbols.append(initSymbols2);
    allSymbols.append(finalSymbols2);
    removeDuplicates(allSymbols);

            pProgressBar->setValue(11);

    //Generate a list of state variables (= "output" variables & local variables)
    QList<Expression> nonStateVars;

    for(int i=0; i<ports.size(); ++i)
    {
        QString num = QString().setNum(i+1);
        if(ports[i].porttype == "ReadPort")
        {
            nonStateVars.append(Expression(ports[i].name));
        }
        else if(ports[i].porttype == "PowerPort" && cqsType == "C")
        {
            QStringList qVars;
            qVars << getQVariables(ports[i].nodetype);
            for(int v=0; v<qVars.size(); ++v)
            {
                nonStateVars.append(Expression(qVars[v]+num));
            }
        }
        else if(ports[i].porttype == "PowerPort" && cqsType == "Q")
        {
            QStringList cVars;
            cVars << getCVariables(ports[i].nodetype);
            for(int v=0; v<cVars.size(); ++v)
            {
                nonStateVars.append(Expression(cVars[v]+num));
            }
        }
    }

    for(int i=0; i<parameters.size(); ++i)
    {
        nonStateVars.append(Expression(parameters[i].name));
    }
    for(int i=0; i<initSymbols2.size(); ++i)
    {
        nonStateVars.append(initSymbols2[i]);
    }
    for(int i=0; i<finalSymbols2.size(); ++i)
    {
        nonStateVars.append(finalSymbols2[i]);
    }

    QList<Expression> stateVars = allSymbols;
    for(int i=0; i<nonStateVars.size(); ++i)
    {
        stateVars.removeAll(nonStateVars[i]);
    }

            pProgressBar->setValue(12);

    //Verify equation system
    if(!verifyEquationSystem(equations, stateVars))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Verification of equation system failed.");
        delete(pProgressBar);
        return;
    }


    //Generate list of local variables (variables that are neither input nor output)
    QList<Expression> nonLocals;

    for(int i=0; i<ports.size(); ++i)
    {
        QString num = QString().setNum(i+1);
        if(ports[i].porttype == "ReadPort" || ports[i].porttype == "WritePort")
        {
            nonLocals.append(Expression(ports[i].name));     //Remove all readport/writeport varibles
        }
        else if(ports[i].porttype == "PowerPort")
        {
            QStringList qVars;
            QStringList cVars;
            qVars << getQVariables(ports[i].nodetype);
            cVars << getCVariables(ports[i].nodetype);
            for(int v=0; v<qVars.size(); ++v)
            {
                nonLocals.append(Expression(qVars[v]+num));      //Remove all Q-type variables
            }
            for(int v=0; v<cVars.size(); ++v)
            {
                nonLocals.append(Expression(cVars[v]+num));      //Remove all C-type variables
            }
        }
    }
    for(int i=0; i<parameters.size(); ++i)
    {
        nonLocals.append(Expression(parameters[i].name));   //Remove all parameters
    }

            pProgressBar->setValue(14);

    QList<Expression> localVars = allSymbols;
    for(int i=0; i<nonLocals.size(); ++i)
    {
        localVars.removeAll(nonLocals[i]);
    }
    for(int i=0; i<localVars.size(); ++i)
    {
        allSymbols.removeAll(localVars[i]);
    }

    for(int i=0; i<equations.size(); ++i)
    {
        equations[i].toLeftSided();
        equations[i].replaceBy(equations[i].getChild(0));
    }

            pProgressBar->setValue(15);

    for(int i=0; i<equations.size(); ++i)
    {
        equations[i] = equations[i].bilinearTransform();
    }

    for(int i=0; i<limitedVariableEquations.size(); ++i)
    {
        equations[limitedVariableEquations[i]].factor(limitedVariables[i]);

        QString num = equations[limitedVariableEquations[i]].getChild(1).toString();
        QString den = equations[limitedVariableEquations[i]].getChild(0).getChild(1).toString();
        equations[limitedVariableEquations[i]] = Expression(QStringList() << limitedVariables[i].toString() << "-" << "limit((-"+num+")/("+den+"))");

        qDebug() << "Limited: " << equations[limitedVariableEquations[i]].toString();


        if(!limitedDerivatives[i].toString().isEmpty())      //Variable2Limits (has a derivative)
        {
            equations[limitedDerivativeEquations[i]].factor(limitedDerivatives[i]);

            QString num = equations[limitedDerivativeEquations[i]].getChild(1).toString();
            QString den = equations[limitedDerivativeEquations[i]].getChild(0).getChild(1).toString();
            equations[limitedDerivativeEquations[i]] = Expression(QStringList() << limitedDerivatives[i].toString() << "-" << "dxLimit((-"+num+")/("+den+"))");

            qDebug() << "Limited: " << equations[limitedDerivativeEquations[i]].toString();
        }

        pProgressBar->setValue(16+5*double(i+1)/double(limitedVariableEquations.size()));
    }

    //Linearize equations
    for(int e=0; e<equations.size(); ++e)
    {
        equations[e].linearize();
        equations[e].expandPowers();
    }

           pProgressBar->setValue(21);


    //Differentiate each equation for each state variable to generate the Jacobian matrix
    QList<QList<Expression> > jacobian;
    for(int e=0; e<equations.size(); ++e)
    {
        //Remove all delay operators, since they shall not be in the Jacobian anyway
        Expression tempExpr = equations[e];

        tempExpr.replace(Expression("Z", Expression::NoSimplifications), Expression("0.0", Expression::NoSimplifications));
        tempExpr.replace(Expression("-Z",Expression::NoSimplifications), Expression("0.0", Expression::NoSimplifications));

        tempExpr._simplify(Expression::SimplifyWithoutMakingPowers, Expression::Recursive);

        //Now differentiate all jacobian elements
        jacobian.append(QList<Expression>());
        for(int j=0; j<stateVars.size(); ++j)
        {
            bool ok = true;
            tempExpr.replace(Expression(stateVars[j].negative()), Expression(stateVars[j], "*", Expression("-1.0", Expression::NoSimplifications), Expression::NoSimplifications));
            jacobian[e].append(tempExpr.derivative(stateVars[j], ok));
            if(!ok)
            {
                gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to differentiate expression: " + equations[e].toString() + " for variable " + stateVars[j].toString());
                return;
            }
        }
                pProgressBar->setValue(22+double(e)/double(equations.size())*34);
    }



    //Sort equation system so that each equation contains its corresponding state variable
    if(!sortEquationSystem(equations, jacobian, stateVars, limitedVariableEquations, limitedDerivativeEquations))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Could not sort equations. System is probably under-determined.");
        delete(pProgressBar);
        return;
    }



    //Differentiate the diagonal element and divide the equation with it, to get only ones on the diagonal later
    for(int i=0; i<stateVars.size(); ++i)
    {
        bool ok = true;
        Expression div = jacobian[i][i];
        equations[i].divideBy(div);

        for(int j=0; j<stateVars.size(); ++j)
        {
            if(!(jacobian[i][j] == Expression("0.0", Expression::NoSimplifications)))
            {
                jacobian[i][j].divideBy(div);
            }
        }

        jacobian[i][i] = Expression("1.0", Expression::NoSimplifications);
    }


    //Transform delay operators to delay functions and store delay terms separately
    QList<Expression> delayTerms;
    QStringList delaySteps;
    for(int e=0; e<equations.size(); ++e)
    {
        equations[e].expand();
        equations[e].toDelayForm(delayTerms, delaySteps);
    }

            pProgressBar->setValue(56);

            pProgressBar->setLabelText("Compiling component");

    //Generate appearance object
    ModelObjectAppearance appearance = generateAppearance(ports, cqsType);

            pProgressBar->setValue(57);

    //Display equation system dialog
    showOutputDialog(jacobian, equations, stateVars);

    //Call utility to generate and compile the source code
//    generateComponentObject(typeName, displayName, cqsType, ports, parameters, equations, stateVars, jacobian, delayTerms, delaySteps, localVars, algorithms, finalAlgorithms, appearance, pProgressBar);

//    //Delete the progress bar to avoid memory leaks
//    delete(pProgressBar);


//}





//////! @brief Generates a ComponentSpecification object from equations and a jacobian
//////! @param typeName Type name of component
//////! @param displayName Display name of component
//////! @param cqsType CQS type of component
//////! @param ports List of port specifications
//////! @param parameteres List of parameter specifications
//////! @param sysEquations List of system equations
//////! @param stateVars List of state variables
//////! @param jacobian List of Jacobian elements
//////! @param delayTerms List of delay terms
//////! @param delaySteps List of number of delay steps for each delay term
//void generateComponentObject(QString typeName, QString displayName, QString cqsType,
//                             QList<PortSpecification> ports, QList<ParameterSpecification> parameters,
//                             QList<Expression> equations, QList<Expression> stateVars, QList<QList<Expression> > jacobian,
//                             QList<Expression> delayTerms, QStringList delaySteps, QList<Expression> localVars,
//                             QList<Expression> initAlgorithms, QList<Expression> finalAlgorithms, ModelObjectAppearance appearance, QProgressDialog *pProgressBar)
//{
            if(pProgressBar)
            {
                pProgressBar->setLabelText("Creating component object");
                pProgressBar->setValue(58);
            }

    ComponentSpecification comp(typeName, displayName, cqsType);

    for(int i=0; i<delayTerms.size(); ++i)
    {
        comp.utilities << "Delay";
        comp.utilityNames << "mDelay"+QString().setNum(i);
    }

            if(pProgressBar)
            {
                pProgressBar->setValue(59);
            }

    for(int i=0; i<ports.size(); ++i)
    {
        comp.portNames << ports[i].name;
        comp.portNodeTypes << ports[i].nodetype;
        comp.portTypes << ports[i].porttype;
        comp.portNotReq << ports[i].notrequired;
        comp.portDefaults << ports[i].defaultvalue;
    }

            if(pProgressBar)
            {
                pProgressBar->setValue(60);
            }

    for(int i=0; i<parameters.size(); ++i)
    {
        comp.parNames << parameters[i].name;
        comp.parDisplayNames << parameters[i].displayName;
        comp.parDescriptions << parameters[i].description;
        comp.parUnits << parameters[i].unit;
        comp.parInits << parameters[i].init;
    }

            if(pProgressBar)
            {
                pProgressBar->setValue(61);
            }

    if(!jacobian.isEmpty())
    {
        comp.varNames << "order["+QString().setNum(stateVars.size())+"]" << "jacobianMatrix" << "systemEquations" << "stateVariables" << "mpSolver";
        comp.varTypes << "double" << "Matrix" << "Vec" << "Vec" << "EquationSystemSolver*";

        comp.initEquations << "jacobianMatrix.create("+QString().setNum(equations.size())+","+QString().setNum(stateVars.size())+");";
        comp.initEquations << "systemEquations.create("+QString().setNum(equations.size())+");";
        comp.initEquations << "stateVariables.create("+QString().setNum(equations.size())+");";
        comp.initEquations << "";
    }

            if(pProgressBar)
            {
                pProgressBar->setValue(62);
            }

    for(int i=0; i<delayTerms.size(); ++i)
    {
        comp.initEquations << "mDelay"+QString().setNum(i)+".initialize("+QString().setNum(delaySteps.at(i).toInt())+", "+delayTerms[i].toString()+");";
    }

            if(pProgressBar)
            {
                pProgressBar->setValue(63);
            }

    if(!jacobian.isEmpty())
    {
        comp.initEquations << "";
        //comp.initEquations << "mpSolver = new EquationSystemSolver(this, "+QString().setNum(sysEquations.size())+");";
        comp.initEquations << "mpSolver = new EquationSystemSolver(this, "+QString().setNum(equations.size())+", &jacobianMatrix, &systemEquations, &stateVariables);";
    }

        if(pProgressBar)
        {
            pProgressBar->setValue(64);
        }

    comp.simEquations << "//Initial algorithm section";
    for(int i=0; i<initAlgorithms.size(); ++i)
    {
        comp.simEquations << initAlgorithms[i].toString()+";";
    }
    comp.simEquations << "";

        if(pProgressBar)
        {
            pProgressBar->setValue(65);
        }


    //! @todo Add support for using more than one iteration

    if(!jacobian.isEmpty())
    {
        for(int i=0; i<stateVars.size(); ++i)
        {
            comp.simEquations << "stateVariables["+QString().setNum(i)+"] = "+stateVars[i].toString()+";";
        }

        comp.simEquations << "";
        comp.simEquations << "    //System Equations";
        for(int i=0; i<equations.size(); ++i)
        {
            comp.simEquations << "    systemEquations["+QString().setNum(i)+"] = "+equations[i].toString()+";";
   //         comp.simEquations << "    "+stateVars[i]+" = " + resEquations[i]+";";
        }
        comp.simEquations << "";
        comp.simEquations << "    //Jacobian Matrix";
        for(int i=0; i<equations.size(); ++i)
        {
            for(int j=0; j<stateVars.size(); ++j)
            {
//                bool constant = true;
//                for(int k=0; k<jacobian[sysEquations.size()*i+j].size(); ++k)
//                {
//                    if(!jacobian[sysEquations.size()*i+j].at(k).isNumber())
//                        constant = false;
//                }

//                if(!constant)
                    comp.simEquations << "    jacobianMatrix["+QString().setNum(i)+"]["+QString().setNum(j)+"] = "+jacobian[i][j].toString()+";";
            }
        }

                if(pProgressBar)
                {
                    pProgressBar->setValue(66);
                }

        comp.simEquations << "";
        comp.simEquations << "    //Solving equation using LU-faktorisation";
        comp.simEquations << "    mpSolver->solve();";
        comp.simEquations << "";
        for(int i=0; i<stateVars.size(); ++i)
        {
            comp.simEquations << "    "+stateVars[i].toString()+"=stateVariables["+QString().setNum(i)+"];";
        }
        comp.simEquations << "";
        for(int i=0; i<delayTerms.size(); ++i)
        {
            comp.simEquations << "mDelay"+QString().setNum(i)+".update("+delayTerms[i].toString()+");";
        }
    }

            if(pProgressBar)
            {
                pProgressBar->setValue(67);
            }

    comp.simEquations << "";
    comp.simEquations << "//Final algorithm section";
    for(int i=0; i<finalAlgorithms.size(); ++i)
    {
        comp.simEquations << finalAlgorithms[i].toString()+";";
    }

            if(pProgressBar)
            {
                pProgressBar->setValue(68);
            }

    if(!jacobian.isEmpty())
    {
        comp.finalEquations << "delete(mpSolver);";
    }

            if(pProgressBar)
            {
                pProgressBar->setValue(69);
            }

    for(int i=0; i<localVars.size(); ++i)
    {
        comp.varNames << localVars[i].toString();
        comp.varTypes << "double";
    }

            if(pProgressBar)
            {
                pProgressBar->setValue(70);
            }

    compileComponentObject("equation.hpp", comp, appearance, false, pProgressBar);

    delete(pProgressBar);
}


//! @brief Generates and compiles component source code from a ComponentSpecification object
//! @param outputFile Name of output file
//! @param comp Component specification object
//! @param overwriteStartValues Tells whether or not this components overrides the built-in start values or not
void compileComponentObject(QString outputFile, ComponentSpecification comp, ModelObjectAppearance appearance, bool overwriteStartValues, QProgressDialog *pProgressBar)
{
    if(pProgressBar)
    {
        pProgressBar->setLabelText("Creating .hpp file");
        pProgressBar->setValue(71);
    }

    if(!QDir(DATAPATH).exists())
    {
        QDir().mkpath(DATAPATH);
    }

        if(pProgressBar)
        {
            pProgressBar->setValue(72);
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

        if(pProgressBar)
        {
            pProgressBar->setValue(73);
        }

    fileStream << "#ifndef " << comp.typeName.toUpper() << "_HPP_INCLUDED\n";
    fileStream << "#define " << comp.typeName.toUpper() << "_HPP_INCLUDED\n\n";
    fileStream << "#include <math.h>\n";
    fileStream << "#include \"ComponentEssentials.h\"\n";
    fileStream << "#include \"ComponentUtilities.h\"\n";
    fileStream << "#include <sstream>\n\n";
    fileStream << "using namespace std;\n\n";
    fileStream << "namespace hopsan {\n\n";
    fileStream << "    class " << comp.typeName << " : public Component" << comp.cqsType << "\n";
    fileStream << "    {\n";
    fileStream << "    private:\n";                         // Private section
    fileStream << "        double ";
    int portId=1;

        if(pProgressBar)
        {
            pProgressBar->setValue(74);
        }

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

        if(pProgressBar)
        {
            pProgressBar->setValue(75);
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

        if(pProgressBar)
        {
            pProgressBar->setValue(76);
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
    fileStream << "        Port ";                              //Declare ports
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        fileStream << "*mp" << comp.portNames[i];
        if(i<comp.portNames.size()-1)
        {
            fileStream << ", ";
        }
    }

        if(pProgressBar)
        {
            pProgressBar->setValue(77);
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

            if(pProgressBar)
            {
                pProgressBar->setValue(78);
            }

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

            if(pProgressBar)
            {
                pProgressBar->setValue(79);
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
                pProgressBar->setValue(80);
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

    if(pProgressBar)
    {
        pProgressBar->setValue(81);
    }

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
//        if(comp.portNodeTypes[i] != "NodeSignal" && (comp.cqsType == "C" || comp.cqsType == "S"))
//        {
//            varNames << getQVariables(comp.portNodeTypes[i]);
//        }
//        if(comp.portNodeTypes[i] != "NodeSignal" && (comp.cqsType == "Q" || comp.cqsType == "S"))
//        {
//            varNames << getCVariables(comp.portNodeTypes[i]);
//        }
        else
        {
            varNames << getQVariables(comp.portNodeTypes[i]);       //Always create both C- and Q-type variables, regaradless of component type (they may be needed)
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

    if(pProgressBar)
    {
        pProgressBar->setValue(82);
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

            if(pProgressBar)
            {
                pProgressBar->setValue(83);
            }

    fileStream << "        }\n\n";
    fileStream << "        void finalize()\n";
    fileStream << "        {\n";
    for(int i=0; i<comp.finalEquations.size(); ++i)
    {
        fileStream << "            " << comp.finalEquations[i] << "\n";
    }
    fileStream << "        }\n\n";
    fileStream << "    };\n";
    fileStream << "}\n\n";

    fileStream << "#endif // " << comp.typeName.toUpper() << "_HPP_INCLUDED\n";
    file.close();


    if(pProgressBar)
    {
        pProgressBar->setLabelText("Creating tempLib.cc");
        pProgressBar->setValue(84);
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

    QFile clBatchFile;
    clBatchFile.setFileName(QString(DATAPATH)+"compile.bat");
    if(!clBatchFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to open compile.bat for writing.");
        return;
    }
    QTextStream clBatchStream(&clBatchFile);
    QString choppedIncludePath = QString(COREINCLUDEPATH);
    choppedIncludePath.chop(1);
    QString choppedExecPath = gExecPath;
    choppedExecPath.chop(1);
    clBatchStream << "g++.exe -shared tempLib.cc -o " << comp.typeName << ".dll -I\"" << choppedIncludePath<< "\"  -I\"" << QString(COREINCLUDEPATH) << "\" -L\""+gExecPath+"\" -L\""+choppedExecPath+"\" -lHopsanCore\n";
    clBatchFile.close();

    if(pProgressBar)
    {
        pProgressBar->setLabelText("Creating appearance file");
        pProgressBar->setValue(85);
    }

    QDir componentsDir(QString(DOCUMENTSPATH));
    QDir generatedDir(QString(DOCUMENTSPATH) + "Generated Componentes/");
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
    xmlStream << "      <icon type=\"user\" path=\""+appearance.getIconPath(USERGRAPHICS, AbsoluteRelativeT(0))+"\" iconrotation=\""+appearance.getIconRotationBehaviour()+"\" scale=\"1\"/>\n";
    xmlStream << "    </icons>\n";
    xmlStream << "    <ports>\n";
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        PortAppearance portApp = appearance.getPortAppearanceMap().find(comp.portNames[i]).value();
        xmlStream << "      <port name=\"" << comp.portNames[i] << "\" x=\"" << portApp.x << "\" y=\"" << portApp.y << "\" a=\"" << portApp.rot << "\"/>\n";
    }
    xmlStream << "    </ports>\n";
    xmlStream << "  </modelobject>\n";
    xmlStream << "</hopsanobjectappearance>\n";
    xmlFile.close();

    if(pProgressBar)
    {
        pProgressBar->setLabelText("Compiling component library");
        pProgressBar->setValue(86);
    }

    //Execute HopsanFMU compile script
#ifdef WIN32
    QProcess p;
    p.start("cmd.exe", QStringList() << "/c" << "cd " + QString(DATAPATH) + " & compile.bat");
    p.waitForFinished();
#else
    QString command = "cd "+QString(DATAPATH)+" && g++ -shared -fPIC tempLib.cc -o " + comp.typeName + ".so -I" + COREINCLUDEPATH + " -L"+gExecPath+" -lHopsanCore\n";
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

    if(pProgressBar)
    {
        pProgressBar->setLabelText("Moving files");
        pProgressBar->setValue(96);
    }

    QString libPath = QDir().cleanPath(generatedDir.path());
    if(gConfig.hasUserLib(libPath))
    {
        //qDebug() << "Updated user libs: " << gConfig.getUserLibs();
        gpMainWindow->mpLibrary->unloadExternalLibrary(libPath);
    }

    if(pProgressBar) { pProgressBar->setValue(97); }

    QFile::remove(generatedDir.path() + "/" + xmlFile.fileName());
    xmlFile.copy(generatedDir.path() + "/" + xmlFile.fileName());

    QFile dllFile(QString(DATAPATH)+comp.typeName+".dll");
    if(QFile::remove(generatedDir.path() + "/" + comp.typeName + ".dll"))
    {
        qDebug() << "Successfully copied library file!";
    }
    else
    {
        qDebug() << "Failed to copy library file to new directory!";
    }
    dllFile.copy(generatedDir.path() + "/" + comp.typeName + ".dll");

    if(pProgressBar) { pProgressBar->setValue(98); }

    QFile soFile(QString(DATAPATH)+comp.typeName+".so");
    QFile::remove(generatedDir.path() + "/" + comp.typeName + ".so");
    soFile.copy(generatedDir.path() + "/" + comp.typeName + ".so");

    if(pProgressBar) { pProgressBar->setValue(99); }

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
        pProgressBar->setValue(100);
    }

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


ModelObjectAppearance generateAppearance(QList<PortSpecification> portList, QString cqsType)
{
    ModelObjectAppearance appearance = ModelObjectAppearance();

    appearance.setIconPath(QString(OBJECTICONPATH)+"generatedcomponenticon.svg", USERGRAPHICS, AbsoluteRelativeT(0));

    QStringList leftPortNames, rightPortNames, topPortNames;
    QList<PortAppearance> leftPorts, rightPorts, topPorts;

    for(int p=0; p<portList.size(); ++p)
    {
        PortAppearanceMapT portMap = appearance.getPortAppearanceMap();
        if(!portMap.contains(portList.at(p).name))
        {
            PortAppearance PortApp;
            PortApp.selectPortIcon(cqsType, portList.at(p).porttype.toUpper(), portList.at(p).nodetype);

            if(portList.at(p).nodetype == "NodeSignal" && portList.at(p).porttype == "ReadPort")
            {
                leftPorts << PortApp;
                leftPortNames << portList.at(p).name;
            }
            else if(portList.at(p).nodetype == "NodeSignal" && portList.at(p).porttype == "WritePort")
            {
                rightPorts << PortApp;
                rightPortNames << portList.at(p).name;
            }
            else
            {
                topPorts << PortApp;
                topPortNames << portList.at(p).name;
            }
        }
    }

    PortAppearanceMapT portMap = appearance.getPortAppearanceMap();
    PortAppearanceMapT::iterator it;
    QStringList keysToRemove;
    for(it=portMap.begin(); it!=portMap.end(); ++it)
    {
        bool exists=false;
        for(int j=0; j<portList.size(); ++j)
        {
            if(portList.at(j).name == it.key())
                exists=true;
        }
        if(!exists)
            keysToRemove << it.key();
    }
    for(int i=0; i<keysToRemove.size(); ++i)
    {
        appearance.getPortAppearanceMap().remove(keysToRemove.at(i));
    }

    for(int i=0; i<leftPorts.size(); ++i)
    {
        leftPorts[i].x = 0.0;
        leftPorts[i].y = (double(i)+1)/(double(leftPorts.size())+1.0);
        leftPorts[i].rot = 180;
        appearance.addPortAppearance(leftPortNames[i], &leftPorts[i]);
    }
    for(int i=0; i<rightPorts.size(); ++i)
    {
        rightPorts[i].x = 1.0;
        rightPorts[i].y = (double(i)+1)/(double(rightPorts.size())+1.0);
        rightPorts[i].rot = 0;
        appearance.addPortAppearance(rightPortNames[i], &rightPorts[i]);
    }
    for(int i=0; i<topPorts.size(); ++i)
    {
        topPorts[i].x = (double(i)+1)/(double(topPorts.size())+1.0);
        topPorts[i].y = 0.0;
        topPorts[i].rot = 270;
        appearance.addPortAppearance(topPortNames[i], &topPorts[i]);
    }

    return appearance;
}




void showOutputDialog(QList<QList<Expression> > jacobian, QList<Expression> equations, QList<Expression> variables)
{
    QDialog *pDialog = new QDialog(gpMainWindow);

    //Description label
    QLabel *pDescription = new QLabel("Resulting equations:", pDialog);

    //Jacobian matrix
    QLabel *pJacobianLabel = new QLabel("J", pDialog);
    pJacobianLabel->setAlignment(Qt::AlignCenter);

    QGridLayout *pJacobianLayout = new QGridLayout(pDialog);
    for(int i=0; i<equations.size(); ++i)
    {
        for(int j=0; j<equations.size(); ++j)
        {
            QString element = jacobian[i][j].toString();
            QString shortElement = element;
            shortElement.truncate(17);
            if(shortElement<element)
            {
                shortElement.append("...");
            }
            QLabel *pElement = new QLabel(shortElement, pDialog);
            pElement->setToolTip(element);
            pElement->setAlignment(Qt::AlignCenter);
            pJacobianLayout->addWidget(pElement, j, i+1);
        }
    }
    QGroupBox *pJacobianBox = new QGroupBox(pDialog);
    pJacobianBox->setLayout(pJacobianLayout);

    //Variables vector
    QLabel *pVariablesLabel = new QLabel("x", pDialog);
    pVariablesLabel->setAlignment(Qt::AlignCenter);

    QGridLayout *pVariablesLayout = new QGridLayout(pDialog);
    for(int i=0; i<variables.size(); ++i)
    {
        QLabel *pElement = new QLabel(variables[i].toString(), pDialog);
        pElement->setAlignment(Qt::AlignCenter);
        pVariablesLayout->addWidget(pElement, i, 0);
    }
    QGroupBox *pVariablesBox = new QGroupBox(pDialog);
    pVariablesBox->setLayout(pVariablesLayout);

    //Equality sign
    QLabel *pEqualityLabel = new QLabel("=", pDialog);
    pEqualityLabel->setAlignment(Qt::AlignCenter);

    //Equations vector
    QLabel *pEquationsLabel = new QLabel("b", pDialog);
    pEquationsLabel->setAlignment(Qt::AlignCenter);

    QGridLayout *pEquationsLayout = new QGridLayout(pDialog);
    for(int i=0; i<equations.size(); ++i)
    {
        QString element = equations[i].toString();
        QString shortElement = element;
        shortElement.truncate(17);
        if(shortElement<element)
        {
            shortElement.append("...");
        }
        QLabel *pElement = new QLabel(shortElement, pDialog);
        pElement->setToolTip(element);
        pElement->setAlignment(Qt::AlignCenter);
        pEquationsLayout->addWidget(pElement, i, 0);
    }
    QGroupBox *pEquationsBox = new QGroupBox(pDialog);
    pEquationsBox->setLayout(pEquationsLayout);

    QPushButton *pOkButton = new QPushButton("Okay", pDialog);
    QDialogButtonBox *pButtonGroup = new QDialogButtonBox(pDialog);
    pButtonGroup->addButton(pOkButton, QDialogButtonBox::AcceptRole);
    pDialog->connect(pOkButton, SIGNAL(pressed()), pDialog, SLOT(close()));

    QGridLayout *pDialogLayout = new QGridLayout(pDialog);
    pDialogLayout->addWidget(pDescription,      0, 0);
    pDialogLayout->addWidget(pJacobianLabel,    1, 0);
    pDialogLayout->addWidget(pJacobianBox,      2, 0);
    pDialogLayout->addWidget(pVariablesLabel,   1, 1);
    pDialogLayout->addWidget(pVariablesBox,     2, 1);
    pDialogLayout->addWidget(pEqualityLabel,    2, 2);
    pDialogLayout->addWidget(pEquationsLabel,   1, 3);
    pDialogLayout->addWidget(pEquationsBox,     2, 3);
    pDialogLayout->addWidget(pButtonGroup,      3, 0, 1, 4);

    pDialog->setLayout(pDialogLayout);

    pDialog->show();
}



//! @brief Verifies that a system of equations is solveable (number of equations = number of unknowns etc)
bool verifyEquationSystem(QList<Expression> equations, QList<Expression> stateVars)
{
    bool retval = true;

    if(equations.size() != stateVars.size())
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Number of equations = " + QString().setNum(equations.size()) + ", number of state variables = " + QString().setNum(stateVars.size()));
        retval = false;
    }

    return retval;
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


//! @brief Parses a modelica model code to Hopsan classes
//! @param code Input Modelica code
//! @param typeName Type name of new component
//! @param displayName Display name of new component
//! @param initAlgorithms Initial algorithms for new component
//! @param equations Equations for new component
//! @param portList List of port specifications for new component
//! @param parametersList List of parameter specifications for new component
void parseModelicaModel(QString code, QString &typeName, QString &displayName, QString &cqsType, QStringList &initAlgorithms, QStringList &equations,
                        QStringList &finalAlgorithms, QList<PortSpecification> &portList, QList<ParameterSpecification> &parametersList)
{
    QStringList lines = code.split("\n");
    QStringList portNames;
    bool initialAlgorithmPart = false;  //Are we in the intial "algorithms" part?
    bool equationPart = false;          //Are we in the "equations" part?
    bool finalAlgorithmPart = false;    //Are we in the final "algorithms" part?
    for(int l=0; l<lines.size(); ++l)
    {
        if(!initialAlgorithmPart && !equationPart && !finalAlgorithmPart)
        {
            //qDebug() << l << " - not in algorithms or equations";
            QStringList words = lines.at(l).trimmed().split(" ");
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
            else if(words.at(0).startsWith("annotation("))        //"annotation" keyword
            {
                QString tempLine = lines[l];
                tempLine.remove(" ");
                int idx = tempLine.indexOf("hopsanCqsType=");
                cqsType = tempLine.at(idx+15);
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

                ParameterSpecification par(name, name, parDisplayName, unit, init);
                parametersList.append(par);
            }
            else if(words.at(0) == "NodeSignalOut")                //Signal connector (output)
            {
                for(int i=0; i<lines.at(l).count(",")+1; ++i)
                {
                    QString name = lines.at(l).trimmed().section(" ", 1).section(",",i,i).section(";",0,0).trimmed();
                    PortSpecification port("WritePort", "NodeSignal", name);
                    portList.append(port);
                    portNames << name;
                }
            }
            else if(words.at(0) == "NodeSignalIn")                //Signal connector (input)
            {
                for(int i=0; i<lines.at(l).count(",")+1; ++i)
                {
                    QString name = lines.at(l).trimmed().section(" ", 1).section(",",i,i).section(";",0,0).trimmed();
                    PortSpecification port("ReadPort", "NodeSignal", name);
                    portList.append(port);
                    portNames << name;
                }
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
            else if(words.at(0) == "algorithm" && !equationPart)    //Initial algorithm part begins!
            {
                initialAlgorithmPart = true;
            }
            else if(words.at(0) == "equation")                      //Equation part begins!
            {
                initialAlgorithmPart = false;
                equationPart = true;
            }
            else if(words.at(0) == "algorithm" && equationPart)     //Final algorithm part begins!
            {
                equationPart = false;
                finalAlgorithmPart = true;
            }
            else if(words.at(0) == "end" && (initialAlgorithmPart || equationPart || finalAlgorithmPart))       //We are finished
            {
                break;
            }
        }
        else if(initialAlgorithmPart)
        {
            //qDebug() << l << " - in algorithms";
            QStringList words = lines.at(l).trimmed().split(" ");
            if(words.at(0) == "end")       //We are finished
            {
                break;
            }
            if(words.at(0) == "equation")       //Equation part begings, end of algorithm section
            {
                initialAlgorithmPart = false;
                equationPart = true;
                continue;
            }
            initAlgorithms << lines.at(l).trimmed();
            initAlgorithms.last().replace(":=", "=");
            //Replace variables with Hopsan syntax, i.e. P2.q => q2
            for(int i=0; i<portNames.size(); ++i)
            {
                QString temp = portNames.at(i)+".";
                while(initAlgorithms.last().contains(temp))
                {
                    if(portList.at(i).nodetype == "NodeSignal")     //Signal nodes are special, they use the port name as the variable name
                    {
                        int idx = initAlgorithms.last().indexOf(temp)+temp.size()-1;
                        if(portList.at(i).porttype == "WritePort")
                        {
                            initAlgorithms.last().remove(idx, 4);
                        }
                        else if(portList.at(i).porttype == "ReadPort")
                        {
                            initAlgorithms.last().remove(idx, 3);
                        }
                    }
                    else
                    {
                        int idx = initAlgorithms.last().indexOf(temp);
                        int idx2=idx+temp.size()+1;
                        while(idx2 < initAlgorithms.last().size()+1 && initAlgorithms.last().at(idx2).isLetterOrNumber())
                            ++idx2;
                        initAlgorithms.last().insert(idx2, QString().setNum(i+1));
                        initAlgorithms.last().remove(idx, temp.size());
                    }
                }
            }
            initAlgorithms.last().chop(1);
        }
        else if(equationPart)
        {
           // qDebug() << l << " - in equations";
            QStringList words = lines.at(l).trimmed().split(" ");
            if(words.at(0) == "end")       //We are finished
            {
                break;
            }
            if(words.at(0) == "algorithm")       //Final algorithm section begins
            {
                equationPart = false;
                finalAlgorithmPart = true;
                continue;
            }
            equations << lines.at(l).trimmed();
            //Replace variables with Hopsan syntax, i.e. P2.q => q2
            for(int i=0; i<portNames.size(); ++i)
            {
                QString temp = portNames.at(i)+".";
                while(equations.last().contains(temp))
                {
                    if(portList.at(i).nodetype == "NodeSignal")     //Signal nodes are special, they use the port name as the variable name
                    {
                        int idx = equations.last().indexOf(temp)+temp.size()-1;
                        if(portList.at(i).porttype == "WritePort")
                        {
                            equations.last().remove(idx, 4);
                        }
                        else if(portList.at(i).porttype == "ReadPort")
                        {
                            equations.last().remove(idx, 3);
                        }
                    }
                    else
                    {
                        int idx = equations.last().indexOf(temp);
                        int idx2=idx+temp.size()+1;
                        while(idx2 < equations.last().size()+1 && equations.last().at(idx2).isLetterOrNumber())
                            ++idx2;
                        equations.last().insert(idx2, QString().setNum(i+1));
                        equations.last().remove(idx, temp.size());
                    }
                }
            }
            equations.last().chop(1);
        }
        else if(finalAlgorithmPart)
        {
           // qDebug() << l << " - in algorithms";
            QStringList words = lines.at(l).trimmed().split(" ");
            if(words.at(0) == "end")       //We are finished
            {
                break;
            }
            finalAlgorithms << lines.at(l).trimmed();
            finalAlgorithms.last().replace(":=", "=");
            //Replace variables with Hopsan syntax, i.e. P2.q => q2
            for(int i=0; i<portNames.size(); ++i)
            {
                QString temp = portNames.at(i)+".";
                while(finalAlgorithms.last().contains(temp))
                {
                    if(portList.at(i).nodetype == "NodeSignal")     //Signal nodes are special, they use the port name as the variable name
                    {
                        int idx = finalAlgorithms.last().indexOf(temp)+temp.size()-1;
                        if(portList.at(i).porttype == "WritePort")
                        {
                            finalAlgorithms.last().remove(idx, 4);
                        }
                        else if(portList.at(i).porttype == "ReadPort")
                        {
                            finalAlgorithms.last().remove(idx, 3);
                        }
                    }
                    else
                    {
                        int idx = finalAlgorithms.last().indexOf(temp);
                        int idx2=idx+temp.size()+1;
                        while(idx2 < finalAlgorithms.last().size()+1 && finalAlgorithms.last().at(idx2).isLetterOrNumber())
                            ++idx2;
                        finalAlgorithms.last().insert(idx2, QString().setNum(i+1));
                        finalAlgorithms.last().remove(idx, temp.size());
                    }
                }
            }
            finalAlgorithms.last().chop(1);
        }
    }

    initAlgorithms.removeAll("\n");
    initAlgorithms.removeAll("");
    equations.removeAll("\n");
    equations.removeAll("");
    finalAlgorithms.removeAll("\n");
    finalAlgorithms.removeAll("");

    //Remove extra boundary equations (assume that they are the last ones)
//    if(cqsType == "Q")
//    {
//        for(int i=0; i<portList.size(); ++i)
//        {
//            //qDebug() << "Port " << i << " has nodetype " << portList.at(i).nodetype;
//            if(portList.at(i).nodetype != "NodeSignal")
//            {
//                equations.removeLast();
//                //qDebug() << "Removing last equation!";
//            }
//        }
//    }
}



//! @note First and last q-type variable must represent intensity and flow
QStringList getQVariables(QString nodeType)
{
    QStringList retval;
    if(nodeType == "NodeMechanic")
    {
        retval << "F" << "x" <<  "me" << "v";
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


//! @brief Returns list of variable enum names for specified node type
//! @param nodeType Node type to use
//! @note c must come first and Zc last
QStringList getVariableLabels(QString nodeType)
{
    QStringList retval;
    if(nodeType == "NodeMechanic")
    {
        retval << "FORCE" << "POSITION" << "EQMASS"  << "VELOCITY"<< "WAVEVARIABLE" << "CHARIMP";
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
