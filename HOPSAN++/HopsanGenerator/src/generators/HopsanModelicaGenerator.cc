#include "generators/HopsanModelicaGenerator.h"
#include "GeneratorUtilities.h"
#include "symhop/SymHop.h"


using namespace SymHop;

HopsanModelicaGenerator::HopsanModelicaGenerator(QString coreIncludePath, QString binPath, bool showDialog)
    : HopsanGenerator(coreIncludePath, binPath, showDialog)
{

}


void HopsanModelicaGenerator::generateFromModelica(QString code)
{
    validateFunctions();

    QString typeName, displayName, cqsType;
    QStringList initAlgorithms, equations, finalAlgorithms;
    QList<PortSpecification> portList;
    QList<ParameterSpecification> parametersList;
    ComponentSpecification comp;

    qDebug() << "Parsing!";
    printMessage("Parsing Modelica code...");

    //Parse Modelica code and generate equation system
    parseModelicaModel(code, typeName, displayName, cqsType, initAlgorithms, equations, finalAlgorithms, portList, parametersList);

    qDebug() << "Transforming!";
    printMessage("Transforming...");

    //Transform equation system, generate Jacobian
    generateComponentObject(comp, typeName, displayName, cqsType, initAlgorithms, equations, finalAlgorithms, portList, parametersList);

    qDebug() << "Compiling!";
    printMessage("Generating component...");

    QString target;
    if(mTarget.isEmpty())
        target = typeName;
    else
        target = mTarget;

    //Compile component
    compileFromComponentObject(target, comp, false, typeName+".mo");

    QFile moFile(mOutputPath+typeName+".mo");
    moFile.open(QFile::WriteOnly | QFile::Text);
    moFile.write(code.toUtf8());
    moFile.close();

    qDebug() << "Finished!";
    printMessage("HopsanGenerator finished!");
}




//! @brief Parses a modelica model code to Hopsan classes
//! @param code Input Modelica code
//! @param typeName Type name of new component
//! @param displayName Display name of new component
//! @param initAlgorithms Initial algorithms for new component
//! @param equations Equations for new component
//! @param portList List of port specifications for new component
//! @param parametersList List of parameter specifications for new component
void HopsanModelicaGenerator::parseModelicaModel(QString code, QString &typeName, QString &displayName, QString &cqsType,
                                                 QStringList &initAlgorithms, QStringList &equations,
                                                 QStringList &finalAlgorithms, QList<PortSpecification> &portList,
                                                 QList<ParameterSpecification> &parametersList)
{
    QStringList lines = code.split("\n");
    QStringList portNames;
    bool initialAlgorithmPart = false;  //Are we in the intial "algorithms" part?
    bool equationPart = false;          //Are we in the "equations" part?
    bool finalAlgorithmPart = false;    //Are we in the final "algorithms" part?
    for(int l=0; l<lines.size(); ++l)
    {
        if(lines[l].startsWith("//")) continue;
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
            else                                                //Power connector
            {
                QStringList nodeTypes;
                NodeInfo::getNodeTypes(nodeTypes);
                Q_FOREACH(const QString &type, nodeTypes)
                {
                    if(words.at(0) == type)
                    {
                        for(int i=0; i<lines.at(l).count(",")+1; ++i)
                        {
                            QString name = lines.at(l).trimmed().section(" ", 1).section(",",i,i).section(";",0,0).trimmed();
                            PortSpecification port("PowerPort", type, name);
                            portList.append(port);
                            portNames << name;
                        }
                    }
                }
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
                        initAlgorithms.last().insert(idx2, QString::number(i+1));
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
                        equations.last().insert(idx2, QString::number(i+1));
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
                        if(portList.at(i).porttype == "WritePortType")
                        {
                            finalAlgorithms.last().remove(idx, 4);
                        }
                        else if(portList.at(i).porttype == "ReadPortType")
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
                        finalAlgorithms.last().insert(idx2, QString::number(i+1));
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
}





//! @brief Generates XML and compiles the new component
void HopsanModelicaGenerator::generateComponentObject(ComponentSpecification &comp, QString &typeName, QString &displayName, QString &cqsType, QStringList &plainInitAlgorithms, QStringList &plainEquations, QStringList &plainFinalAlgorithms, QList<PortSpecification> &ports, QList<ParameterSpecification> &parameters)
{

    //Create list of initial algorithms
    QList<Expression> initAlgorithms;
    for(int i=0; i<plainInitAlgorithms.size(); ++i)
    {
        initAlgorithms.append(Expression(plainInitAlgorithms.at(i)));
        if(!initAlgorithms[i].isAssignment())
        {
            printErrorMessage("Initial algorithm is not an assignment.");
            return;
        }
    }

    //Create list of equqtions
    QList<Expression> equations;
    for(int e=0; e<plainEquations.size(); ++e)
    {
        equations.append(Expression(plainEquations.at(e)));
        qDebug() << "EQUATION: " << equations[e].toString();
        if(!equations[e].isEquation())
        {
            printErrorMessage("Equation is not an equation.");
            return;
        }
    }

    //Create list of final algorithms
    QList<Expression> finalAlgorithms;
    for(int i=0; i<plainFinalAlgorithms.size(); ++i)
    {
        finalAlgorithms.append(Expression(plainFinalAlgorithms.at(i)));
        if(!initAlgorithms[i].isAssignment())
        {
            printErrorMessage("Final algorithm is not an assignment.");
            return;
        }
    }

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
                printErrorMessage("VariableLimits not preceeded by equations defining variable.");
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
                printErrorMessage("Variable2Limits not preeded by equations defining variable and derivative.");
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
    }

    //Verify each equation
    for(int i=0; i<equations.size(); ++i)
    {
        if(!equations[i].verifyExpression())
        {
            printErrorMessage("Component generation failed: Verification of variables failed.");
            return;
        }
    }

    QList<QList<Expression> > leftSymbols2, rightSymbols2;
    for(int i=0; i<equations.size(); ++i)
    {
        leftSymbols2.append(equations[i].getLeft()->getVariables());
        rightSymbols2.append(equations[i].getRight()->getVariables());
    }

    //Sum up all used variables to a single list
    QList<Expression> allSymbols;
    for(int i=0; i<equations.size(); ++i)
    {
        allSymbols.append(leftSymbols2.at(i));
        allSymbols.append(rightSymbols2.at(i));
    }

    QStringList allSymbolsList;
    for(int a=0; a<allSymbols.size(); ++a)
    {
        allSymbolsList.append(allSymbols[a].toString());
    }
    qDebug() << "All symbols: " << allSymbolsList;


    QList<Expression> initSymbols2;
    for(int i=0; i<initAlgorithms.size(); ++i)
    {
        if(!initAlgorithms[i].isAssignment())
        {
            printErrorMessage("Component generation failed: Initial algorithms section contains non-algorithms.");
            return;
        }
        initSymbols2.append((*initAlgorithms[i].getLeft()));
    }

    QList<Expression> finalSymbols2;
    for(int i=0; i<finalAlgorithms.size(); ++i)
    {
        //! @todo We must check that all algorithms are actually algorithms before doing this!
        if(!finalAlgorithms[i].isAssignment())
        {
            printErrorMessage("Component generation failed: Final algorithms section contains non-algorithms.");
            return;
        }
        finalSymbols2.append((*finalAlgorithms[i].getLeft()));
    }

    for(int i=0; i<parameters.size(); ++i)
    {
        allSymbols.append(Expression(parameters[i].name));
    }
    allSymbols.append(initSymbols2);
    allSymbols.append(finalSymbols2);
    removeDuplicates(allSymbols);

    //Generate a list of state variables (= "output" variables & local variables)
    QList<Expression> nonStateVars;

    for(int i=0; i<ports.size(); ++i)
    {
        QString num = QString::number(i+1);
        if(ports[i].porttype == "ReadPortType")
        {
            nonStateVars.append(Expression(ports[i].name));
        }
        else if(ports[i].porttype == "PowerPort" && cqsType == "C")
        {
            QStringList qVars;
            qVars << NodeInfo(ports[i].nodetype).qVariables;
            for(int v=0; v<qVars.size(); ++v)
            {
                nonStateVars.append(Expression(qVars[v]+num));
            }
        }
        else if(ports[i].porttype == "PowerPort" && cqsType == "Q")
        {
            QStringList cVars;
            cVars << NodeInfo(ports[i].nodetype).cVariables;
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

    //Verify equation system
    if(!verifyEquationSystem(equations, stateVars, this))
    {
//        printErrorMessage("Verification of equation system failed.");
        return;
    }


    //Generate list of local variables (variables that are neither input nor output)
    QList<Expression> nonLocals;

    for(int i=0; i<ports.size(); ++i)
    {
        QString num = QString::number(i+1);
        if(ports[i].porttype == "ReadPort" || ports[i].porttype == "WritePort")
        {
            nonLocals.append(Expression(ports[i].name));     //Remove all readport/writeport varibles
        }
        else if(ports[i].porttype == "PowerPort")
        {
            QStringList qVars;
            QStringList cVars;
            qVars << NodeInfo(ports[i].nodetype).qVariables;
            cVars << NodeInfo(ports[i].nodetype).cVariables;
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

    QList<Expression> localVars = allSymbols;
    for(int i=0; i<nonLocals.size(); ++i)
    {
        localVars.removeAll(nonLocals[i]);
    }
    for(int i=0; i<localVars.size(); ++i)
    {
        allSymbols.removeAll(localVars[i]);
    }

    //Make all equations left-sided
    for(int e=0; e<equations.size(); ++e)
    {
        equations[e].toLeftSided();
        qDebug() << "LEFT SIDED: " << equations[e].toString();
    }

    //Apply bilinear transform
    for(int e=0; e<equations.size(); ++e)
    {
        equations[e] = equations[e].bilinearTransform();
        equations[e]._simplify(Expression::FullSimplification, Expression::Recursive);
        qDebug() << "BILINEAR TRANSFORM: " << equations[e].toString();
    }


    //Linearize equations
    for(int e=0; e<equations.size(); ++e)
    {
        equations[e].linearize();
        equations[e]._simplify(Expression::FullSimplification, Expression::Recursive);
        qDebug() << "LINEARIZED: " << equations[e].toString();
        equations[e].replaceBy((*equations[e].getLeft()));
    }


    //Transform delay operators to delay functions and store delay terms separately
    QList<Expression> delayTerms;
    QStringList delaySteps;
    for(int e=0; e<equations.size(); ++e)
    {
        equations[e].expand();
        equations[e].toDelayForm(delayTerms, delaySteps);
        equations[e]._simplify(Expression::FullSimplification);
        qDebug() << "TRANSFORMED TO DELAYS: " << equations[e].toString();
    }


    for(int i=0; i<limitedVariableEquations.size(); ++i)
    {
        equations[limitedVariableEquations[i]].factor(limitedVariables[i]);

        Expression rem = equations[limitedVariableEquations[i]];
        rem.replace(limitedVariables[i], Expression(0));
        rem._simplify(Expression::FullSimplification, Expression::Recursive);

        qDebug() << "REM: " << rem.toString();

        Expression div = equations[limitedVariableEquations[i]];
        div.subtractBy(rem);
        div.replace(limitedVariables[i], Expression(1));
        div.expand();
       // div._simplify(Expression::FullSimplification, Expression::Recursive);

        qDebug() << "DIV: " << div.toString();

        rem = Expression::fromFactorDivisor(rem, div);
        rem.changeSign();
        rem._simplify(Expression::FullSimplification, Expression::Recursive);

        qDebug() << "REM AGAIN: " << rem.toString();

        qDebug() << "Limit string: -limit(("+rem.toString()+"),"+limitMinValues[i].toString()+","+limitMaxValues[i].toString()+")";
        equations[limitedVariableEquations[i]] = Expression::fromTwoTerms(limitedVariables[i], Expression("-limit(("+rem.toString()+"),"+limitMinValues[i].toString()+","+limitMaxValues[i].toString()+")"));

        qDebug() << "Limited: " << equations[limitedVariableEquations[i]].toString();

        if(!limitedDerivatives[i].toString().isEmpty())      //Variable2Limits (has a derivative)
        {
            equations[limitedDerivativeEquations[i]].factor(limitedDerivatives[i]);

            Expression rem = equations[limitedDerivativeEquations[i]];
            rem.replace(limitedDerivatives[i], Expression(0));
            rem._simplify(Expression::FullSimplification, Expression::Recursive);

            Expression div = equations[limitedDerivativeEquations[i]];
            div.subtractBy(rem);
            div.replace(limitedDerivatives[i], Expression(1));
            div.expand();
            div.factorMostCommonFactor();

            rem = Expression::fromFactorDivisor(rem, div);
            rem.changeSign();

            equations[limitedDerivativeEquations[i]] = Expression::fromTwoTerms(limitedDerivatives[i], Expression::fromTwoFactors(Expression("-dxLimit("+limitedVariables[i].toString()+","+limitMinValues[i].toString()+","+limitMaxValues[i].toString()+")"), rem));
        }
    }

    //Differentiate each equation for each state variable to generate the Jacobian matrix
    QList<QList<Expression> > jacobian;
    for(int e=0; e<equations.size(); ++e)
    {
        //Remove all delay operators, since they shall not be in the Jacobian anyway
        Expression tempExpr = equations[e];

       // tempExpr.replace(Expression("Z", Expression::NoSimplifications), Expression("0.0", Expression::NoSimplifications));
       // tempExpr.replace(Expression("-Z",Expression::NoSimplifications), Expression("0.0", Expression::NoSimplifications));

        tempExpr._simplify(Expression::FullSimplification, Expression::Recursive);

        //Now differentiate all jacobian elements
        jacobian.append(QList<Expression>());
        for(int j=0; j<stateVars.size(); ++j)
        {
            bool ok = true;
            //Expression negExpr = stateVars[j];
            //negExpr.changeSign();
            //tempExpr.replace(negExpr, Expression::fromFactorsDivisors(QList<Expression>() << stateVars[j] << Expression("-1.0", Expression::NoSimplifications), QList<Expression>()));
            Expression derExpr = tempExpr.derivative(stateVars[j], ok);
            qDebug() << "Derivating \""+tempExpr.toString()+"\" with respect to \""+stateVars[j].toString();
            qDebug() << "Result: \""+derExpr.toString()+"\"";
            jacobian[e].append(derExpr);
            if(!ok)
            {
                printErrorMessage("Failed to differentiate expression: " + equations[e].toString() + " for variable " + stateVars[j].toString());
                return;
            }
            jacobian[e].last()._simplify(Expression::FullSimplification);
        }
    }



    //Sort equation system so that each equation contains its corresponding state variable
    if(!sortEquationSystem(equations, jacobian, stateVars, limitedVariableEquations, limitedDerivativeEquations))
    {
        printErrorMessage("Could not sort equations. System is probably under-determined.");
        qDebug() << "Could not sort equations. System is probably under-determined.";
        return;
    }

    for(int e=0; e<equations.size(); ++e)
    {
        equations[e]._simplify(Expression::FullSimplification);
    }

    //Generate appearance object
    //ModelObjectAppearance appearance = generateAppearance(ports, cqsType);

    QList<Expression> yExpressions;
    QList<Expression> xExpressions;
    QList<Expression> vExpressions;

//    QList<QList<Expression> > L;
//    for(int i=0; i<jacobian.size(); ++i)
//    {
//        L.append(QList<Expression>());
//        for(int j=0; j<jacobian.size(); ++j)
//        {
//            if(i == j)
//            {
//                L[i].append(Expression(1));
//            }
//            else
//            {
//                L[i].append(Expression(0));
//            }
//        }
//    }

//    QList<QList<Expression> > U = jacobian;

//    qDebug() << "\nJacobian matrix:";
//    for(int i=0; i<L.size(); ++i)
//    {
//        QString line;
//        for(int j=0; j<L.size(); ++j)
//        {
//            line.append(jacobian[i][j].toString());
//            line.append(",     ");
//        }
//        line.chop(6);
//        qDebug() << line;
//    }


//    for(int i=0; i<jacobian.size()-1; ++i)
//    {
//        for(int j=i+1; j<jacobian.size(); ++j)
//        {
//            Expression factor = Expression(1);
//            factor.divideBy(U[i][i]);
//            factor.multiplyBy(U[j][i]);
//            L[j][i] = factor;
//            L[j][i].expandParentheses();
//            L[j][i]._simplify(Expression::FullSimplification, Expression::Recursive);
//            for(int k=i; k<jacobian.size(); ++k)
//            {
//                if(j==1 && k==1)
//                {
//                    qDebug() << "factor: " << factor.toString();

//                    int apa=3;
//                }

//                Expression temp = U[i][k];
//                temp.multiplyBy(factor);
//                U[j][k].subtractBy(temp);
//                U[j][k].expandParentheses();
//                U[j][k]._simplify(Expression::FullSimplification, Expression::Recursive);
//                           }
//            U[j][i] = Expression(0);
//        }
//    }


//    qDebug() << "U matrix:";
//    for(int l=0; l<U.size(); ++l)
//    {
//        QString line;
//        for(int m=0; m<U.size(); ++m)
//        {
//            line.append(U[l][m].toString());
//            line.append(",     ");
//        }
//        line.chop(6);
//        qDebug() << line;
//    }

//    qDebug() << "\nL matrix:";
//    for(int i=0; i<L.size(); ++i)
//    {
//        QString line;
//        for(int j=0; j<L.size(); ++j)
//        {
//            line.append(L[i][j].toString());
//            line.append(",     ");
//        }
//        line.chop(6);
//        qDebug() << line;
//    }
//    qDebug() << "\n";

//    for(int r=0; r<L.size(); ++r)
//    {
//        Expression temp = Expression(0);
//        for(int c=0; c<L.size(); ++c)
//        {
//            Expression y = Expression("y_LU"+QString::number(c));
//            Expression element = L[r][c];
//            element.multiplyBy(y);
//            temp.addBy(element);
//        }
//        Expression y = Expression("y_LU"+QString::number(r));

//        temp.replace(y, Expression("0"));
//        Expression right = equations[r];
//        right.subtractBy(temp);
//        right.expand();
//        right._simplify(Expression::FullSimplification, Expression::Recursive);

//        yExpressions.append(Expression(y, "=", right));

//        qDebug() << "Y expression: " << yExpressions.last().toString();
//    }
//    qDebug() << "\n";

//    for(int r=U.size()-1; r>=0; --r)
//    {
//        Expression temp = Expression(0);
//        for(int c=0; c<U.size(); ++c)
//        {
//            Expression x = Expression("x_LU"+QString::number(c));
//            Expression element = L[r][c];
//            element.multiplyBy(x);
//            temp.addBy(element);
//        }
//        Expression x = Expression("x_LU"+QString::number(r));
//        temp.replace(x, Expression("0"));
//        Expression right = Expression("y_LU"+QString::number(r));
//        right.subtractBy(temp);
//        right.expand();
//        right._simplify(Expression::FullSimplification, Expression::Recursive);

//        xExpressions.append(Expression(x, "=", right));

//        qDebug() << "X expression: " << xExpressions.last().toString();
//    }
//    qDebug() << "\n";

//    for(int v=0; v<stateVars.size(); ++v)
//    {
//        Expression x = Expression("-x_LU"+QString::number(v));
//        Expression right = Expression(stateVars[v], "+", x);
//        vExpressions.append(Expression(stateVars[v], "=", right));
//        qDebug() << "V Expression: " << vExpressions.last().toString();
//    }
//    qDebug() << "\n";

//    for(int i=0; i<yExpressions.size(); ++i)
//    {
//        localVars.append(Expression("y_LU"+QString::number(i)));
//        localVars.append(Expression("x_LU"+QString::number(i)));
//    }


//    jacobian.clear();


    comp.typeName = typeName;
    comp.displayName = displayName;
    comp.cqsType = cqsType;
    if(comp.cqsType == "S") { comp.cqsType = "Signal"; }

    for(int i=0; i<delayTerms.size(); ++i)
    {
        comp.utilities << "Delay";
        comp.utilityNames << "mDelay"+QString::number(i);
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

    if(!jacobian.isEmpty())
    {
        comp.varNames << "order["+QString::number(stateVars.size())+"]" << "jacobianMatrix" << "systemEquations" << "stateVariables" << "mpSolver";
        comp.varTypes << "double" << "Matrix" << "Vec" << "Vec" << "EquationSystemSolver*";

        comp.initEquations << "jacobianMatrix.create("+QString::number(equations.size())+","+QString::number(stateVars.size())+");";
        comp.initEquations << "systemEquations.create("+QString::number(equations.size())+");";
        comp.initEquations << "stateVariables.create("+QString::number(equations.size())+");";
        comp.initEquations << "";
    }

    for(int i=0; i<delayTerms.size(); ++i)
    {
        comp.initEquations << "mDelay"+QString::number(i)+".initialize("+QString::number(delaySteps.at(i).toInt())+", "+delayTerms[i].toString()+");";
    }

    if(!jacobian.isEmpty())
    {
        comp.initEquations << "";
        //comp.initEquations << "mpSolver = new EquationSystemSolver(this, "+QString::number(sysEquations.size())+");";
        comp.initEquations << "mpSolver = new EquationSystemSolver(this, "+QString::number(equations.size())+", &jacobianMatrix, &systemEquations, &stateVariables);";
        comp.finalEquations << "delete mpSolver;";
    }

    comp.simEquations << "//Initial algorithm section";
    for(int i=0; i<initAlgorithms.size(); ++i)
    {
        comp.simEquations << initAlgorithms[i].toString()+";";
    }
    comp.simEquations << "";

    if(!yExpressions.isEmpty())
    {
        //Forwards substitution
        for(int i=0; i<yExpressions.size(); ++i)
        {
            comp.simEquations << yExpressions[i].toString()+";";
        }

        //Backwards substitution
        for(int i=xExpressions.size()-1; i>=0; --i)
        {
            comp.simEquations << xExpressions[i].toString()+";";
        }

        //Newton-Rhapson
        for(int i=0; i<vExpressions.size(); ++i)
        {
            comp.simEquations << vExpressions[i].toString()+";";
        }
    }



        //! @todo Add support for using more than one iteration

    if(!jacobian.isEmpty())
    {
        for(int i=0; i<stateVars.size(); ++i)
        {
            comp.simEquations << "stateVariables["+QString::number(i)+"] = "+stateVars[i].toString()+";";
        }

        comp.simEquations << "";
        comp.simEquations << "    //System Equations";
        for(int i=0; i<equations.size(); ++i)
        {
            comp.simEquations << "    systemEquations["+QString::number(i)+"] = "+equations[i].toString()+";";
   //         comp.simEquations << "    "+stateVars[i]+" = " + resEquations[i]+";";
        }
        comp.simEquations << "";
        comp.simEquations << "    //Jacobian Matrix";
        for(int i=0; i<equations.size(); ++i)
        {
            for(int j=0; j<stateVars.size(); ++j)
            {
                comp.simEquations << "    jacobianMatrix["+QString::number(i)+"]["+QString::number(j)+"] = "+jacobian[i][j].toString()+";";
            }
        }

        comp.simEquations << "";
        comp.simEquations << "    //Solving equation using LU-faktorisation";
        comp.simEquations << "    mpSolver->solve();";
        comp.simEquations << "";
        for(int i=0; i<stateVars.size(); ++i)
        {
            comp.simEquations << "    "+stateVars[i].toString()+"=stateVariables["+QString::number(i)+"];";
        }
    }

    //Update delays
    comp.simEquations << "";
    for(int i=0; i<delayTerms.size(); ++i)
    {
        comp.simEquations << "mDelay"+QString::number(i)+".update("+delayTerms[i].toString()+");";
    }

    comp.simEquations << "";
    comp.simEquations << "//Final algorithm section";
    for(int i=0; i<finalAlgorithms.size(); ++i)
    {
        comp.simEquations << finalAlgorithms[i].toString()+";";
    }

    for(int i=0; i<localVars.size(); ++i)
    {
        comp.varNames << localVars[i].toString();
        comp.varTypes << "double";
    }
}



