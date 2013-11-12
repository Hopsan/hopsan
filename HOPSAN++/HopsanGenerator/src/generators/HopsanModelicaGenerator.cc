#include "generators/HopsanModelicaGenerator.h"
#include "GeneratorUtilities.h"
#include "SymHop.h"
#include <QtConcurrentMap>


using namespace SymHop;


Expression gTempExpr;

HopsanModelicaGenerator::HopsanModelicaGenerator(QString coreIncludePath, QString binPath, bool showDialog)
    : HopsanGenerator(coreIncludePath, binPath, showDialog)
{

}


void HopsanModelicaGenerator::generateFromModelica(QString path, SolverT solver)
{
    qDebug() << "SOLVER: " << solver;

    QFile moFile(path);
    moFile.open(QFile::ReadOnly);
    QString code = moFile.readAll();
    moFile.close();

    QString typeName, displayName, cqsType;
    QStringList initAlgorithms, equations, finalAlgorithms;
    QList<PortSpecification> portList;
    QList<ParameterSpecification> parametersList;
    QList<VariableSpecification> variablesList;
    ComponentSpecification comp;

    //qDebug() << "Parsing!";
    printMessage("Parsing Modelica code...");

    //Parse Modelica code and generate equation system
    parseModelicaModel(code, typeName, displayName, cqsType, initAlgorithms, equations, finalAlgorithms, portList, parametersList, variablesList);

    //qDebug() << "Transforming!";
    printMessage("Transforming...");

    if(solver == BilinearTransform)
    {
        //Transform equation system using Bilinear Transform method
        generateComponentObject(comp, typeName, displayName, cqsType, initAlgorithms, equations, finalAlgorithms, portList, parametersList, variablesList);
    }
    else /*if(solver == NumericalIntegration)*/
    {
        //Transform equation system using numerical integration methods
        generateComponentObjectNumericalIntegration(comp, typeName, displayName, cqsType, initAlgorithms, equations, finalAlgorithms, portList, parametersList, variablesList);
    }

    //qDebug() << "Compiling!";
    printMessage("Generating component...");

    //Compile component
    QString cppCode = generateSourceCodefromComponentObject(comp, false);

    //Write output file
    QString moPath = path;
    QFile hppFile(path.replace(".mo", ".hpp"));
    hppFile.open(QFile::WriteOnly | QFile::Truncate);
    hppFile.write(cppCode.toUtf8());
    hppFile.close();

    //Generate or update appearance file
    generateOrUpdateComponentAppearanceFile(path.replace(".hpp",".xml"), comp, QFileInfo(moPath).fileName());

    //qDebug() << "Finished!";
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
                                                 QList<ParameterSpecification> &parametersList, QList<VariableSpecification> &variablesList)
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
            else if(words.at(0) == "Real")              //"Real" keyword (local variable)
            {
                for(int i=0; i<lines.at(l).count(",")+1; ++i)
                {
                    QString var = lines.at(l).trimmed().section(" ", 1).section(",",i,i).section(";",0,0).trimmed();
                    QString name, init;
                    if(var.contains("("))
                    {
                        name = var.section("(",0,0);
                        init = var.section("=",1,1).section(")",0,0);
                    }
                    else
                    {
                        name = var;
                        init = "";
                    }
                    VariableSpecification varSpec(name, init);
                    variablesList.append(varSpec);
                }
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
                GeneratorNodeInfo::getNodeTypes(nodeTypes);
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
void HopsanModelicaGenerator::generateComponentObject(ComponentSpecification &comp, QString &typeName, QString &displayName, QString &cqsType, QStringList &plainInitAlgorithms, QStringList &plainEquations, QStringList &plainFinalAlgorithms, QList<PortSpecification> &ports, QList<ParameterSpecification> &parameters, QList<VariableSpecification> &variables)
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
        //qDebug() << "EQUATION: " << equations[e].toString();
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
    //qDebug() << "All symbols: " << allSymbolsList;


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
        if(ports[i].porttype == "ReadPort")
        {
            nonStateVars.append(Expression(ports[i].name));
        }
        else if(ports[i].porttype == "PowerPort" && cqsType == "C")
        {
            QStringList qVars;
            qVars << GeneratorNodeInfo(ports[i].nodetype).qVariables;
            for(int v=0; v<qVars.size(); ++v)
            {
                nonStateVars.append(Expression(qVars[v]+num));
            }
        }
        else if(ports[i].porttype == "PowerPort" && cqsType == "Q")
        {
            QStringList cVars;
            cVars << GeneratorNodeInfo(ports[i].nodetype).cVariables;
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
            qVars << GeneratorNodeInfo(ports[i].nodetype).qVariables;
            cVars << GeneratorNodeInfo(ports[i].nodetype).cVariables;
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
    for(int i=0; i<variables.size(); ++i)
    {
        if(!localVars.contains(Expression(variables[i].name)))
        {
            localVars.append(Expression(variables[i].name));
        }
    }

    //Make all equations left-sided
    for(int e=0; e<equations.size(); ++e)
    {
        equations[e].toLeftSided();
        //qDebug() << "LEFT SIDED: " << equations[e].toString();
    }

    //Generate a preferred path for sorting, based on the location of derivatives of state variables
    //This must be done before the bilinear transform, so that we can identify derivatives
    QList< QList<int> > preferredPath;
    Q_FOREACH(const Expression &var, stateVars)
    {
        preferredPath.append(QList<int>());
        Q_FOREACH(const Expression &equation, equations)
        {
            if(equation.contains(Expression::fromFunctionArguments("der", QList<Expression>() << var)) ||
               equation.getLeft()->getTerms().contains(var))
            {
                preferredPath.last().append(equations.indexOf(equation));
            }
        }
    }
    //qDebug() << preferredPath;

    QList<int> preferredOrder;
    findPath(preferredOrder, preferredPath, 0);
    //qDebug() << preferredOrder;


    //Apply bilinear transform
    for(int e=0; e<equations.size(); ++e)
    {
        equations[e] = equations[e].bilinearTransform();
        equations[e]._simplify(Expression::FullSimplification, Expression::Recursive);
        //qDebug() << "BILINEAR TRANSFORM: " << equations[e].toString();
    }


    //Linearize equations
    for(int e=0; e<equations.size(); ++e)
    {
        equations[e].linearize();
        equations[e]._simplify(Expression::FullSimplification, Expression::Recursive);
        //qDebug() << "LINEARIZED: " << equations[e].toString();
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
        //qDebug() << "TRANSFORMED TO DELAYS: " << equations[e].toString();
    }


    for(int i=0; i<limitedVariableEquations.size(); ++i)
    {
        equations[limitedVariableEquations[i]].factor(limitedVariables[i]);

        Expression rem = equations[limitedVariableEquations[i]];
        rem.replace(limitedVariables[i], Expression(0));
        rem._simplify(Expression::FullSimplification, Expression::Recursive);

        //qDebug() << "REM: " << rem.toString();

        Expression div = equations[limitedVariableEquations[i]];
        div.subtractBy(rem);
        div.replace(limitedVariables[i], Expression(1));
        div.expand();
       // div._simplify(Expression::FullSimplification, Expression::Recursive);

        //qDebug() << "DIV: " << div.toString();

        rem = Expression::fromFactorDivisor(rem, div);
        rem.changeSign();
        rem._simplify(Expression::FullSimplification, Expression::Recursive);

        //qDebug() << "REM AGAIN: " << rem.toString();

        //qDebug() << "Limit string: -limit(("+rem.toString()+"),"+limitMinValues[i].toString()+","+limitMaxValues[i].toString()+")";
        equations[limitedVariableEquations[i]] = Expression::fromTwoTerms(limitedVariables[i], Expression("-limit(("+rem.toString()+"),"+limitMinValues[i].toString()+","+limitMaxValues[i].toString()+")"));

        //qDebug() << "Limited: " << equations[limitedVariableEquations[i]].toString();

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
        gTempExpr = equations[e];
        gTempExpr._simplify(Expression::FullSimplification, Expression::Recursive);

        QList<Expression> result = QtConcurrent::blockingMapped(stateVars, concurrentDiff);

        jacobian.append(result);
    }

    //Sort equation system so that each equation contains its corresponding state variable
    if(!sortEquationSystem(equations, jacobian, stateVars, limitedVariableEquations, limitedDerivativeEquations, preferredOrder))
    {
        printErrorMessage("Could not sort equations. System is probably under-determined.");
        //qDebug() << "Could not sort equations. System is probably under-determined.";
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
        comp.varInits << "" << "" << "" << "" << "";
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
        bool foundInit=false;
        for(int v=0; v<variables.size(); ++v)
        {
            if(variables[v].name == localVars[i].toString())
            {
                comp.varInits << variables[v].init;
                foundInit=true;
            }
        }
        if(!foundInit)
        {
            comp.varInits << "";
        }
    }
}


void HopsanModelicaGenerator::generateComponentObjectNumericalIntegration(ComponentSpecification &comp, QString &typeName, QString &displayName, QString &cqsType, QStringList &initAlgorithms, QStringList &plainEquations, QStringList &finalAlgorithms, QList<PortSpecification> &ports, QList<ParameterSpecification> &parameters, QList<VariableSpecification> &variables)
{
    //Q_UNUSED(initAlgorithms);
    //Q_UNUSED(finalAlgorithms);

    //Create list of equqtions
    QList<Expression> equations;
    for(int e=0; e<plainEquations.size(); ++e)
    {
        equations.append(Expression(plainEquations.at(e)));
        if(!equations[e].isEquation())
        {
            printErrorMessage("Equation is not an equation.");
            return;
        }
    }

    QMap<QString, QString> varToStateVarMap;
    QMap<QString, QString> derToStateDerMap;
    int s=0;
    Q_FOREACH(const QString &equation, plainEquations)
    {
        if(!equation.contains("der(")) continue;
        int nDer = equation.count("der(");
        for(int i=0; i<nDer; ++i)
        {
            if(equation.section("der(",i, i).right(1)[0].isLetterOrNumber()) continue;
            QString var = equation.section("der(",i+1,i+1).section(")",0,0);
            if(!varToStateVarMap.contains(var))
            {
                varToStateVarMap.insert(var, "STATEVAR"+QString::number(s));
                derToStateDerMap.insert("der("+var+")", "der(STATEVAR"+QString::number(s)+")");
                ++s;
            }
        }
    }

    for(int e=0; e<equations.size(); ++e)
    {
        QMapIterator<QString, QString> it(varToStateVarMap);
        while(it.hasNext())
        {
            it.next();
            equations[e].replace(Expression(it.key()), Expression(it.value()));
        }
        QMapIterator<QString, QString> itd(derToStateDerMap);
        while(itd.hasNext())
        {
            itd.next();
            equations[e].replace(Expression(itd.key()), Expression(itd.value()));
        }
    }

    //Generate list of dependencies (i.e. which state variable derivatives exist in each equation)
    QList<QList<Expression> > dependencies;
    Q_FOREACH(const Expression &equation, equations)
    {
        dependencies.append(QList<Expression>());
        Q_FOREACH(const QString &var, derToStateDerMap.values())
        {
            if(equation.contains(var))
            {
                dependencies[equations.indexOf(equation)].append(Expression(var));
            }
        }
    }

    //Sort equations by dependencies
    QList<Expression> systemEquations;                     //System equations, not required to resolve state variables
    QList<Expression> afterSolverEquations;                     //Equations calculated after system is solved
    QList<Expression> stateEquations;                       //State equations, used to resolve state variables
    QList<Expression> stateEquationsDerivatives;            //Derivatives of state equations, used for root-finding if necessary
    QList<Expression> resolvedDependencies;                 //State variables that has been resolved
    QMap<Expression*, Expression*> stateDerToEquationMap;   //Map between state equations and corresponding state variable
    QMap<Expression*, Expression*> stateDerDerToEquationMap;   //Map between state equations and corresponding state variable
    int iterationCounter=0;
    int e=0;
    while(!equations.isEmpty())
    {
        //Abort if no success after very many iterations
        ++iterationCounter;
        if(iterationCounter > plainEquations.size()*100)
        {
            printErrorMessage("Unable to resolve dependencies in equation system.");
            break;
        }

        //Restart counter
        if(e >= equations.size())
            e=0;

        //Remove resolved dependencies
        for(int i=0; i<resolvedDependencies.size(); ++i)
        {
            dependencies[e].removeAll(resolvedDependencies[i]);
        }

        //No dependencies, equation is trivial
        if(dependencies[e].isEmpty())
        {
            systemEquations.append(equations[e]);
            equations.removeAt(e);
            dependencies.removeAt(e);
            e=0;
            continue;
        }

        //Single dependency, resolve it
        else if(dependencies[e].size() == 1)
        {
            resolvedDependencies.append(dependencies[e][0]);
            stateEquations.append(equations[e]);
            stateDerToEquationMap.insert(&resolvedDependencies.last(), &stateEquations.last());
            equations.removeAt(e);
            dependencies.removeAt(e);
            e=0;
            continue;
        }
        ++e;
    }

    //Break out the derivative of the state variable in each corresponding equation
    QMapIterator<Expression*, Expression*> it(stateDerToEquationMap);
    while(it.hasNext())
    {
        it.next();

        Expression stateVar = *it.key();
        Expression *equation = it.value();

        equation->linearize();
        equation->toLeftSided();
        *equation = *equation->getLeft();
        equation->factor(stateVar);
        if(equation->getTerms().size() == 1)
        {
            *equation = Expression(0);   //Only one term, state var must equal zero
        }
        else
        {
            Expression term = equation->getTerms()[0];
            equation->removeTerm(term);
            term.replace(stateVar, Expression(1));
            term._simplify(Expression::FullSimplification, Expression::Recursive);
            equation->divideBy(term);
            equation->changeSign();
            equation->_simplify(Expression::FullSimplification, Expression::Recursive);
        }
    }



    for(int e=0; e<stateEquations.size(); ++e)
    {
        Expression equation = stateEquations[e];

        QMap<Expression*, Expression*> renameMap;
        for(int v=0; v<resolvedDependencies.size(); ++v)
        {
            equation.replace(resolvedDependencies[v].getArgument(0), Expression(resolvedDependencies[v].getArgument(0).toString()+"(TIME)"));
            renameMap.insert(new Expression("D"+resolvedDependencies[v].getArgument(0).toString()+"(TIME)"), &stateEquations[v]);       //! @todo MEMORY LEAK
        }

        bool ok;
        equation = equation.derivative(Expression("TIME"), ok);

        QMapIterator<Expression*, Expression*> itr(renameMap);
        while(itr.hasNext())
        {
            itr.next();
            equation.replace(*itr.key(), *itr.value());
        }

        stateEquationsDerivatives.append(equation);
    }


    // DEBUG DEBUG DEBUG

//    //Sort remaining equations so that they can be solved
//    QList<Expression> trivialVariables;
//    QList<Expression> knowns;
//    knowns.append(resolvedDependencies);
//    for(int v=0; v<varToStateVarMap.values().size(); ++v)
//    {
//        knowns.append(Expression(varToStateVarMap.values()[v]));
//    }
//    for(int p=0; p<parameters.size(); ++p)
//    {
//        knowns.append(Expression(parameters[p].name));
//    }
//    for(int i=0; i<ports.size(); ++i)
//    {
//        QString num = QString::number(i+1);
//        if(ports[i].porttype == "ReadPort")
//        {
//            knowns.append(Expression(ports[i].name));
//        }
//        else if(ports[i].porttype == "PowerPort" && cqsType == "C")
//        {
//            QStringList qVars;
//            qVars << GeneratorNodeInfo(ports[i].nodetype).qVariables;
//            for(int v=0; v<qVars.size(); ++v)
//            {
//                knowns.append(Expression(qVars[v]+num));
//            }
//        }
//        else if(ports[i].porttype == "PowerPort" && cqsType == "Q")
//        {
//            QStringList cVars;
//            cVars << GeneratorNodeInfo(ports[i].nodetype).cVariables;
//            for(int v=0; v<cVars.size(); ++v)
//            {
//                knowns.append(Expression(cVars[v]+num));
//            }
//        }
//    }
//    if(!sortEquationByVariables(trivialEquations, trivialVariables, knowns))
//    {
//        return;
//    }

//    //Break out the variable defined by each trivial equation
//    for(int e=0; e<trivialEquations.size(); ++e)
//    {
//        trivialEquations[e].linearize();
//        trivialEquations[e].toLeftSided();
//        trivialEquations[e] = *trivialEquations[e].getLeft();
//        trivialEquations[e].factor(trivialVariables[e]);
//        Expression term = trivialEquations[e].getTerms()[0];
//        trivialEquations[e].removeTerm(term);
//        term.replace(trivialVariables[e], Expression(1));
//        term._simplify(Expression::FullSimplification, Expression::Recursive);
//        trivialEquations[e].divideBy(term);
//        trivialEquations[e].changeSign();
//        trivialEquations[e]._simplify(Expression::FullSimplification, Expression::Recursive);
//        trivialEquations[e] = Expression::fromEquation(trivialVariables[e], trivialEquations[e]);
//    }

    printMessage("DEBUG1");

    //Generate list of known variables
    QList<Expression> knowns;
    knowns.append(resolvedDependencies);
    for(int v=0; v<varToStateVarMap.values().size(); ++v)
    {
        knowns.append(Expression(varToStateVarMap.values()[v]));
    }
    for(int p=0; p<parameters.size(); ++p)
    {
        knowns.append(Expression(parameters[p].name));
    }
    for(int i=0; i<ports.size(); ++i)
    {
        QString num = QString::number(i+1);
        if(ports[i].porttype == "ReadPort")
        {
            knowns.append(Expression(ports[i].name));
        }
        else if(ports[i].porttype == "PowerPort" && cqsType == "C")
        {
            QStringList qVars;
            qVars << GeneratorNodeInfo(ports[i].nodetype).qVariables;
            for(int v=0; v<qVars.size(); ++v)
            {
                knowns.append(Expression(qVars[v]+num));
            }
        }
        else if(ports[i].porttype == "PowerPort" && cqsType == "Q")
        {
            QStringList cVars;
            cVars << GeneratorNodeInfo(ports[i].nodetype).cVariables;
            for(int v=0; v<cVars.size(); ++v)
            {
                knowns.append(Expression(cVars[v]+num));
            }
        }
    }
    for(int i=0; i<initAlgorithms.size(); ++i)
    {
        if(initAlgorithms[i].contains("="))
        {
            QString var = initAlgorithms[i].section("=",0,0).trimmed();
            knowns.append(var);
        }
    }

    printMessage("DEBUG2");

    for(int e=0; e<systemEquations.size(); ++e)
    {
        systemEquations[e].toLeftSided();
        systemEquations[e] = *systemEquations[e].getLeft();
    }

    //Generate list of variables needed to be solved
    QList<Expression> systemVariables;
    for(int e=0; e<systemEquations.size(); ++e)
    {
        QList<Expression> newVars = systemEquations[e].getVariables();
        for(int n=0; n<newVars.size(); ++n)
        {
            if(!systemVariables.contains(newVars[n]))
            {
                systemVariables.append(newVars[n]);
            }
        }
    }
    for(int k=0; k<knowns.size(); ++k)
    {
        systemVariables.removeAll(knowns[k]);
    }

    //! @todo Exclude trivial equations (only depending on known variables) from Jacobian and solve them before the Jacobian
    //! @todo Exclude equations that contains unique variablse (existing only in one equation) and solve them after the Jacobian

    printMessage("DEBUG3");

    //Differentiate each equation for each state variable to generate the Jacobian matrix
    QList<QList<Expression> > jacobian;
    for(int e=0; e<systemEquations.size(); ++e)
    {
         //Remove all delay operators, since they shall not be in the Jacobian anyway
        gTempExpr = systemEquations[e];
        gTempExpr._simplify(Expression::FullSimplification, Expression::Recursive);

        QList<Expression> result = QtConcurrent::blockingMapped(systemVariables, concurrentDiff);

        jacobian.append(result);
    }

    printMessage("DEBUG4");

    QList< QList<int> > preferredPath;
    for(int i=0; i<systemEquations.size(); ++i)
    {
        preferredPath.append(QList<int>());
    }

    QList<int> preferredOrder;
    findPath(preferredOrder, preferredPath, 0);

    qDebug() << "Equations: " << systemEquations.size();
    for(int i=0; i<systemVariables.size(); ++i)
    {
        qDebug() << "Variable: " << systemVariables[i].toString();
    }
    qDebug() << "order: " << preferredOrder.size();

    if(systemEquations.size() != systemVariables.size())
    {
        printErrorMessage("Number of system equations does not equal number of system variables.");
        printErrorMessage("Number of system equations: " + QString::number(systemEquations.size()));
        printErrorMessage("Number of system variables: " + QString::number(systemVariables.size()));
        return;

    }

    //Sort equation system so that each equation contains its corresponding state variable
    QList<int> dummy1, dummy2;
    if(!sortEquationSystem(systemEquations, jacobian, systemVariables, dummy1, dummy2, preferredOrder))
    {
        printErrorMessage("Could not sort equations. System is probably under-determined.");
        qDebug() << "Could not sort equations. System is probably under-determined.";
        return;
    }

    QList<Expression> systemVars = systemVariables;

    printMessage("DEBUG5");


    //Add call to solver
    QList<Expression> beforeSolverEquations;
    beforeSolverEquations.prepend(Expression("CALLSOLVER"));

    printMessage("DEBUG6");

    //Initialize state variables, and convert them back at end
    QMapIterator<QString, QString> itv(varToStateVarMap);
    while(itv.hasNext())
    {
        itv.next();
        beforeSolverEquations.prepend(itv.value()+"="+itv.key());
        afterSolverEquations.append(itv.key()+"="+itv.value());
    }

    printMessage("DEBUG6");

    for(int e=0; e<stateEquations.size(); ++e)
    {
        //trivialEquations.prepend(Expression::fromEquation(resolvedDependencies[e].getArgument(0), stateEquations[e]));
    }


    //DEBUG OUTPUT
    Q_FOREACH(const Expression &var, resolvedDependencies)
    {
        qDebug() << "STATE VARIABLE: " << var.toString();
    }

    Q_FOREACH(const Expression &equation, stateEquations)
    {
        qDebug() << "STATE EQUATION: " << equation.toString();
    }

    Q_FOREACH(const Expression &equation, stateEquationsDerivatives)
    {
        qDebug() << "STATE EQUATION DERIVATIVE: " << equation.toString();
    }

    Q_FOREACH(const Expression &var, systemVariables)
    {
        qDebug() << "TRIVIAL VARIABLE: " << var.toString();
    }

    Q_FOREACH(const Expression &equation, systemEquations)
    {
        qDebug() << "TRIVIAL EQUATION: " << equation.toString();
    }

    double apa=3;
    (void)apa;


    //Generate component object
    comp.typeName = typeName;
    comp.displayName = displayName;
    comp.cqsType = cqsType;
    if(comp.cqsType == "S") { comp.cqsType = "Signal"; }

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

    for(int i=0; i<variables.size(); ++i)
    {
        comp.varNames.append(variables[i].name);
        comp.varInits.append(variables[i].init);
        comp.varTypes.append("double");
    }

    comp.varInits.append("");
    comp.varNames.append("STATEVARS");
    comp.varTypes.append("std::vector<double>");

    comp.varInits.append("2");
    comp.varNames.append("nStateVars");
    comp.varTypes.append("int");

    comp.varInits.append("");
    comp.varNames.append("solverType");
    comp.varTypes.append("int");

    comp.confEquations.append("std::vector<HString> availableSolvers = NumericalIntegrationSolver::getAvailableSolverTypes();");
    comp.confEquations.append("addConditionalConstant(\"solverType\", \"Solver Type\", availableSolvers, solverType);");

    comp.initEquations.append("STATEVARS.resize(2);");
    comp.initEquations.append("mpSolver = new NumericalIntegrationSolver(this, &STATEVARS);");

    if(!jacobian.isEmpty())
    {
        comp.varNames << "order["+QString::number(systemVars.size())+"]" << "jacobianMatrix" << "systemEquations" << "stateVariables" << "mpSystemSolver";
        comp.varInits << "" << "" << "" << "" << "";
        comp.varTypes << "double" << "Matrix" << "Vec" << "Vec" << "EquationSystemSolver*";

        comp.initEquations << "jacobianMatrix.create("+QString::number(systemEquations.size())+","+QString::number(systemVars.size())+");";
        comp.initEquations << "systemEquations.create("+QString::number(systemEquations.size())+");";
        comp.initEquations << "stateVariables.create("+QString::number(systemEquations.size())+");";
        comp.initEquations << "";
    }

    if(!jacobian.isEmpty())
    {
        comp.initEquations << "";
        //comp.initEquations << "mpSystemSolver = new EquationSystemSolver(this, "+QString::number(sysEquations.size())+");";
        comp.initEquations << "mpSystemSolver = new EquationSystemSolver(this, "+QString::number(systemEquations.size())+", &jacobianMatrix, &systemEquations, &stateVariables);";
        comp.finalEquations << "delete mpSystemSolver;";
    }

    //Initial algorithm section
    for(int i=0; i<initAlgorithms.size(); ++i)
    {
        //! @todo Convert everything to C++ syntax
        QString initEq = initAlgorithms[i];
        initEq.replace(":=", "=");
        initEq.append(";");
        comp.simEquations.append(initEq);
    }

    Q_FOREACH(const Expression &equation, beforeSolverEquations)
    {
        QString equationStr = equation.toString();
        for(int s=0; s<stateEquations.size(); ++s)      //State vars must be renamed, because SymHop does not consider "STATEVARS[i]" an acceptable variable name
        {
            equationStr.replace("STATEVAR"+QString::number(s), "STATEVARS["+QString::number(s)+"]");
        }
        if(equation.toString() == "CALLSOLVER")
        {
            comp.simEquations.append("mpSolver->solve(solverType);");
        }
        else
        {
            comp.simEquations.append(equationStr+";");
        }
    }

    Q_FOREACH(const Expression &equation, afterSolverEquations)
    {
        QString equationStr = equation.toString();
        for(int s=0; s<stateEquations.size(); ++s)      //State vars must be renamed, because SymHop does not consider "STATEVARS[i]" an acceptable variable name
        {
            equationStr.replace("STATEVAR"+QString::number(s), "STATEVARS["+QString::number(s)+"]");
        }
        if(equation.toString() == "CALLSOLVER")
        {
            comp.simEquations.append("mpSolver->solve(solverType);");
        }
        else
        {
            comp.simEquations.append(equationStr+";");
        }
    }

    for(int i=0; i<finalAlgorithms.size(); ++i)
    {
        //! @todo Convert everything to C++ syntax
        QString finalEq = finalAlgorithms[i];
        finalEq.replace(":=", "=");
        comp.simEquations.append(finalEq);
    }

    comp.finalEquations.append("delete mpSolver;");

    comp.auxiliaryFunctions.append("");
    comp.auxiliaryFunctions.append("void reInitializeValuesFromNodes()");
    comp.auxiliaryFunctions.append("{");
    QString readInputs;
    int portId=1;
    for(int i=0; i<comp.portNames.size(); ++i)
    {
        QStringList varNames;
        if(comp.portNodeTypes[i] == "NodeSignal")
        {
            varNames << comp.portNames[i];
        }
        else
        {
            varNames << GeneratorNodeInfo(comp.portNodeTypes[i]).qVariables << GeneratorNodeInfo(comp.portNodeTypes[i]).cVariables;
        }

        for(int v=0; v<varNames.size(); ++v)
        {
            QString varName;
            if(comp.portNodeTypes[i] == "NodeSignal")
                varName = varNames[v];
            else
                varName = varNames[v] + QString::number(portId);
            comp.auxiliaryFunctions.append("    "+varName+" = (*mpND_"+varName+");");
        }
        ++portId;
    }
    comp.auxiliaryFunctions.append("}");

    comp.auxiliaryFunctions.append("");
    comp.auxiliaryFunctions.append("void solveSystem()");
    comp.auxiliaryFunctions.append("{");

    if(!jacobian.isEmpty())
    {
        for(int i=0; i<systemVars.size(); ++i)
        {
            comp.auxiliaryFunctions << "    stateVariables["+QString::number(i)+"] = "+systemVars[i].toString()+";";
        }

        comp.auxiliaryFunctions << "";
        comp.auxiliaryFunctions << "    //System Equations";
        for(int i=0; i<systemEquations.size(); ++i)
        {
            QString equationStr = systemEquations[i].toString();
            for(int s=0; s<systemEquations.size(); ++s)      //State vars must be renamed, because SymHop does not consider "STATEVARS[i]" an acceptable variable name
            {
                equationStr.replace("STATEVAR"+QString::number(s), "STATEVARS["+QString::number(s)+"]");
            }
            comp.auxiliaryFunctions << "    systemEquations["+QString::number(i)+"] = "+equationStr+";";
   //         comp.auxiliaryFunctions << "    "+systemVars[i]+" = " + resEquations[i]+";";
        }
        comp.auxiliaryFunctions << "";
        comp.auxiliaryFunctions << "    //Jacobian Matrix";
        for(int i=0; i<systemEquations.size(); ++i)
        {
            for(int j=0; j<systemVars.size(); ++j)
            {
                QString elementStr = jacobian[i][j].toString();
                for(int s=0; s<systemEquations.size(); ++s)      //State vars must be renamed, because SymHop does not consider "STATEVARS[i]" an acceptable variable name
                {
                    elementStr.replace("STATEVAR"+QString::number(s), "STATEVARS["+QString::number(s)+"]");
                }
                comp.auxiliaryFunctions << "    jacobianMatrix["+QString::number(i)+"]["+QString::number(j)+"] = "+elementStr+";";
            }
        }

        comp.auxiliaryFunctions << "";
        comp.auxiliaryFunctions << "    //Solving equation using LU-faktorisation";
        comp.auxiliaryFunctions << "    mpSystemSolver->solve();";
        comp.auxiliaryFunctions << "";
        for(int i=0; i<systemVars.size(); ++i)
        {
            comp.auxiliaryFunctions << "    "+systemVars[i].toString()+"=stateVariables["+QString::number(i)+"];";
        }
    }
    comp.auxiliaryFunctions.append("}");

    comp.auxiliaryFunctions.append("");
    comp.auxiliaryFunctions.append("double getStateVariableDerivative(int i)");
    comp.auxiliaryFunctions.append("{");
    comp.auxiliaryFunctions.append("    switch(i)");
    comp.auxiliaryFunctions.append("    {");
    for(int e=0; e<stateEquations.size(); ++e)
    {
        comp.auxiliaryFunctions.append("        case "+QString::number(e)+" :");
        QString stateEqStr = stateEquations[e].toString();
        for(int s=0; s<stateEquations.size(); ++s)      //State vars must be renamed, because SymHop does not consider "STATEVARS[i]" an acceptable variable name
        {
            stateEqStr.replace("STATEVAR"+QString::number(s), "STATEVARS["+QString::number(s)+"]");
        }
        comp.auxiliaryFunctions.append("            return "+stateEqStr+";");
    }
    comp.auxiliaryFunctions.append("    }");
    comp.auxiliaryFunctions.append("}");
    comp.auxiliaryFunctions.append("");

    comp.auxiliaryFunctions.append("double getStateVariableSecondDerivative(int i)");
    comp.auxiliaryFunctions.append("{");
    comp.auxiliaryFunctions.append("    switch(i)");
    comp.auxiliaryFunctions.append("    {");
    for(int e=0; e<stateEquations.size(); ++e)
    {
        comp.auxiliaryFunctions.append("        case "+QString::number(e)+" :");
        QString stateEqStr = stateEquationsDerivatives[e].toString();
        for(int s=0; s<stateEquationsDerivatives.size(); ++s)      //State vars must be renamed, because SymHop does not consider "STATEVARS[i]" an acceptable variable name
        {
            stateEqStr.replace("STATEVAR"+QString::number(s), "STATEVARS["+QString::number(s)+"]");
        }
        comp.auxiliaryFunctions.append("            return "+stateEqStr+";");
    }
    comp.auxiliaryFunctions.append("    }");
    comp.auxiliaryFunctions.append("}");
    comp.auxiliaryFunctions.append("");

    comp.utilityNames.append("mpSolver");
    comp.utilities.append("NumericalIntegrationSolver*");


    //comp.simEquations.append(stateEquations);

    equations.clear();
}




Expression concurrentDiff(Expression expr)
{
    Expression tempExpr = gTempExpr;

    bool ok = true;
    Expression derExpr = tempExpr.derivative(expr, ok);
    if(!ok)
    {
        //printErrorMessage("Failed to differentiate expression: " + tempExpr.toString() + " for variable " + expr.toString());
        return Expression(0);
    }
    derExpr._simplify(Expression::FullSimplification);
    return derExpr;
}


//! @brief Sorts and equation system by its depending variables, so that it can be solved equation-by-equation
//! @param equations List with equations to sort
//! @param variables Empty list that will contain corresponding variables
//! @param knowns List of already known parameter, that does not need to be taken into consideration
bool HopsanModelicaGenerator::sortEquationByVariables(QList<Expression> &equations, QList<Expression> &variables, QList<Expression> &knowns)
{
    //Generate list of variables in equations
    Q_FOREACH(const Expression &equation, equations)
    {
        QList<Expression> tempVars = equation.getVariables();
        Q_FOREACH(const Expression var, tempVars)
        {
            if(!variables.contains(var) && !knowns.contains(var))
                variables.append(var);
        }
    }

    if(variables.size() != equations.size())
    {
        printErrorMessage("Wrong number of equations! Number of equations: "+QString::number(equations.size())+", number of variables: "+QString::number(variables.size()));
        return false;
    }

    //Generate list of dependencies (i.e. which state variable derivatives exist in each equation)
    QList<QList<Expression> > dependencies;
    Q_FOREACH(const Expression &equation, equations)
    {
        dependencies.append(QList<Expression>());
        Q_FOREACH(const Expression &var, variables)
        {
            if(equation.contains(var))
            {
                dependencies[equations.indexOf(equation)].append(Expression(var));
            }
        }
    }

    //Sort equations by dependencies
    QList<Expression> resolvedEquations;                       //State equations, used to resolve state variables
    QList<Expression> resolvedVariables;                 //State variables that has been resolved

    int iterationCounter=0;
    int e=0;
    while(!equations.isEmpty())
    {
        //Abort if no success after very many iterations
        ++iterationCounter;
        if(iterationCounter > equations.size()*100)
        {
            printErrorMessage("Unable to resolve dependencies in equation system.");
            return false;
        }

        //Restart counter
        if(e >= equations.size())
            e=0;

        //Remove resolved dependencies
        for(int i=0; i<resolvedVariables.size(); ++i)
        {
            dependencies[e].removeAll(resolvedVariables[i]);
        }

        //No dependencies, equation is trivial
        if(dependencies[e].isEmpty())
        {
            resolvedEquations.append(equations[e]);
            equations.removeAt(e);
            dependencies.removeAt(e);
            e=0;
            continue;
        }

        //Single dependency, resolve it
        else if(dependencies[e].size() <= 1)
        {
            resolvedVariables.append(dependencies[e][0]);
            resolvedEquations.append(equations[e]);
            equations.removeAt(e);
            dependencies.removeAt(e);
            e=0;
            continue;
        }
        ++e;
    }

    equations = resolvedEquations;
    variables = resolvedVariables;

    return true;
}
