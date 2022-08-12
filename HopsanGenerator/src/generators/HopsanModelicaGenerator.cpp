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

#include "generators/HopsanModelicaGenerator.h"
#include "GeneratorUtilities.h"
#include <QTime>
#include <QFileInfo>

namespace {

SymHop::Expression gTempExpr;

//! @brief Verifies that a system of equations is solvable (number of equations = number of unknowns etc)
bool verifyEquationSystem(QList<SymHop::Expression> equations, QList<SymHop::Expression> unknowns, HopsanGeneratorBase *pGenerator)
{
    bool retval = true;

    if(equations.size() != unknowns.size())
    {
        QStringList equationStrs;
        for(int s=0; s<equations.size(); ++s)
        {
            equationStrs.append(equations[s].toString());
        }

        QStringList unknownStrs;
        for(int s=0; s<unknowns.size(); ++s)
        {
            unknownStrs.append(unknowns[s].toString());
        }

        pGenerator->printErrorMessage("Number of equations = " + QString::number(equations.size()) + ", number of unknowns = " + QString::number(unknowns.size()));
        pGenerator->printErrorMessage("Unknowns: "+unknownStrs.join(", "));
        retval = false;
    }

    return retval;
}

SymHop::Expression concurrentDiff(SymHop::Expression expr)
{
    SymHop::Expression tempExpr = gTempExpr;

    bool ok = true;
    SymHop::Expression derExpr = tempExpr.derivative(expr, ok);
    if(!ok)
    {
        //printErrorMessage("Failed to differentiate expression: " + tempExpr.toString() + " for variable " + expr.toString());
        return SymHop::Expression(0);
    }
    derExpr._simplify(SymHop::Expression::FullSimplification);
    return derExpr;
}

} // End anon namespace

using namespace SymHop;

HopsanModelicaGenerator::HopsanModelicaGenerator(const QString &hopsanInstallPath, const QString &compilerPath, const QString &tempPath)
    : HopsanGeneratorBase(hopsanInstallPath, compilerPath, tempPath)
{

}


bool HopsanModelicaGenerator::generateFromModelica(QString path)
{
    QFile moFile(path);
    moFile.open(QFile::ReadOnly);
    QString code = moFile.readAll();
    moFile.close();

    QString typeName, displayName, cqsType, transform;
    QStringList initAlgorithms, algorithms, equations, finalAlgorithms;
    QList<PortSpecification> portList;
    QList<ParameterSpecification> parametersList;
    QList<VariableSpecification> variablesList;
    ComponentSpecification comp;

    //qDebug() << "Parsing!";
    printMessage("Parsing "+moFile.fileName()+"...");

    //Parse Modelica code and generate equation system
    if(!parseModelicaModel(code, typeName, displayName, cqsType, initAlgorithms, algorithms, equations, portList, parametersList, variablesList, transform)) {
        printErrorMessage("Failed to parse Modelica model.");
        return false;
    }

    //qDebug() << "Transforming!";
    printMessage("Transforming...");

    QFile logFile(QFileInfo(path).absolutePath()+"/generatorlog.txt");
    logFile.open(QFile::WriteOnly | QFile::Text | QFile::Truncate);
    QTextStream logStream(&logFile);

    bool success = generateComponentObject(comp,typeName,displayName,cqsType,transform,initAlgorithms,algorithms,equations,portList,parametersList,variablesList,logStream);

    logFile.close();

    printMessage("Generating component...");

    if(!success) {
        printErrorMessage("Translation from Modelica to C++ failed.");
        return false;
    }

    //Compile component
    QString cppCode = generateSourceCodefromComponentSpec(comp, false);

    //Write output file
    QString moPath = path;
    QFile hppFile(path.replace(".mo", ".hpp"));
    hppFile.open(QFile::WriteOnly | QFile::Truncate);
    hppFile.write(cppCode.toUtf8());
    hppFile.close();

    //Generate or update appearance file
    bool genOK = generateOrUpdateComponentAppearanceFile(path.replace(".hpp",".xml"), comp, QFileInfo(moPath).fileName());
    if (!genOK) {
        printErrorMessage("Could not generate component appearance file");
        return false;
    }

    //qDebug() << "Finished!";
    printMessage("HopsanGenerator finished!");
    return true;
}


enum ModelicaCodePartT { UndefinedPart, HeaderSection, InitialAlgorithmSection, AlgorithmSection, EquationSection };

//! @brief Parses a modelica model code to Hopsan classes
//! @param code Input Modelica code
//! @param typeName Type name of new component
//! @param displayName Display name of new component
//! @param initAlgorithms Initial algorithms for new component
//! @param equations Equations for new component
//! @param portList List of port specifications for new component
//! @param parametersList List of parameter specifications for new component
bool HopsanModelicaGenerator::parseModelicaModel(QString code, QString &typeName, QString &displayName, QString &cqsType,
                                                 QStringList &initAlgorithms, QStringList &algorithms, QStringList &equations,
                                                 QList<PortSpecification> &portList, QList<ParameterSpecification> &parametersList,
                                                 QList<VariableSpecification> &variablesList, QString &transform)
{
    QStringList lines = code.split(QRegExp("[\r\n]"),QString::SkipEmptyParts);
    QStringList portNames;
    QStringList nodeTypes;
    GeneratorNodeInfo::getNodeTypes(nodeTypes);
    ModelicaCodePartT section = UndefinedPart;
    bool foundHeader = false;
    bool foundInitialAlgorithms = false;
    for(int l=0; l<lines.size(); ++l)
    {
        if(lines[l].trimmed().startsWith("//")) continue;   //Skip comments
        if(lines[l].trimmed().isEmpty()) continue;          //Skip blank lines
        QStringList words = lines.at(l).trimmed().split(QRegExp("\\s(?=(?:[^'\"`]*(['\"`])[^'\"`]*\\1)*[^'\"`]*$)"), QString::SkipEmptyParts);
        if(section == UndefinedPart)
        {
            if(!verifyModelicaLine(lines[l], ModelDeclaration)) {
                return false;
            }
            if(words.at(0) == "model")              //"model" keyword
            {
                section = HeaderSection;
                foundHeader = true;
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
        }
        else if(section != UndefinedPart && words.at(0) == "initial" && words.at(1) == "algorithm")
        {
            if(foundInitialAlgorithms) {
                printErrorMessage("Only one initial algorithm section allowed.");
                return false;
            }
            section = InitialAlgorithmSection;
        }
        else if(section != UndefinedPart && words.at(0) == "algorithm")    //Algorithm section begins
        {
            section = AlgorithmSection;
        }
        else if(section != UndefinedPart && words.at(0) == "equation")    //Equation section begins
        {
            section = EquationSection;
        }
        else if(section != UndefinedPart && words.at(0) == "end")       //We are finished
        {
            break;
        }
        else if(section == HeaderSection) {
            if(!verifyModelicaLine(lines[l], Annotation | VariableDeclaration)) {
                return false;
            }
            if(words.at(0).startsWith("annotation("))        //"annotation" keyword
            {
                QString tempLine = lines[l];
                tempLine.remove(" ");
                int idx = tempLine.indexOf("hopsanCqsType=");
                cqsType = tempLine.at(idx+15);
                if(tempLine.contains("linearTransform=")) {
                    transform = tempLine.section("linearTransform=",1,1).section("\"",1,1);
                }
            }
            else if(words.at(0) == "parameter")         //"parameter" keyword
            {
                QString name = words.at(2).section("(",0,0).section("=",0,0);
                QString unit = lines.at(l).section("unit=",1,1).section("\"",1,1);
                QString init;
                //Default value can be written with white spaces in different way, test them all
                if(words.size() == 3 || (words.size() == 4 && words.at(2).contains(")=")))
                    init = words.at(2).section("=", -1,-1);                         //"...)=x"
                else if(words.size() == 4 && !words.at(3).startsWith("="))
                    init = words.at(3);                                             //"...)= x"
                else if(words.size() == 4 && words.at(3).startsWith("="))
                    init = words.at(3).section("=", 1);                             //"...) =x"
                else if(words.size() == 5)
                    init = words.at(4);                                             //"...) = x"
                init.remove(";");


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
            else if(words.at(0) == "output" && words.at(1) == "Real")                //Signal connector (output)
            {
                for(int i=0; i<lines.at(l).count(",")+1; ++i)
                {
                    QString name = lines.at(l).trimmed().section(" ", 2).section(",",i,i).section(";",0,0).trimmed();
                    PortSpecification port("WritePort", "NodeSignal", name);
                    portList.append(port);
                    portNames << name;
                }
            }
            else if(words.at(0) == "input" && words.at(1) == "Real")                //Signal connector (input)
            {
                for(int i=0; i<lines.at(l).count(",")+1; ++i)
                {
                    QString name = lines.at(l).trimmed().section(" ", 2).section(",",i,i).section(";",0,0).trimmed();
                    QString defaultValue;
                    if(name.contains("(start=")) {
                        defaultValue = name.section("(start=",1,1).section(")",0,0);
                        name.remove("(start="+defaultValue+")");
                    }
                    PortSpecification port("ReadPort", "NodeSignal", name, false, defaultValue);
                    portList.append(port);
                    portNames << name;
                }
            }
            else if(nodeTypes.contains(words[0]))                                            //Power connector
            {
                for(const auto &type : nodeTypes) {
                    if(words.at(0) == type) {
                        for(int i=0; i<lines.at(l).count(",")+1; ++i) {
                            QString name = lines.at(l).trimmed().section(" ", 1).section(",",i,i).section(";",0,0).trimmed();
                            PortSpecification port("PowerPort", type, name);
                            portList.append(port);
                            portNames << name;
                        }
                    }
                }
            }
        }
        else if(section == InitialAlgorithmSection || section == HeaderSection)
        {
            if(!verifyModelicaLine(lines[l], Assignment)) {
                return false;
            }

            foundInitialAlgorithms = true;
            QStringList words = lines.at(l).trimmed().split(" ");
            if(words.at(0) == "end")       //We are finished
            {
                break;
            }
            else if(words.at(0) == "algorithm") //New algorithm section begins
            {
                section = AlgorithmSection;
                continue;
            }
            else if(words.at(0) == "equation") //New equation section beginning
            {
                section = EquationSection;
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
        else if(section == AlgorithmSection)
        {
            if(!verifyModelicaLine(lines[l], Assignment)) {
                return false;
            }

            QStringList words = lines.at(l).trimmed().split(" ");
            if(words.at(0) == "end")       //We are finished
            {
                break;
            }
            else if(words.at(0) == "initial" && words.at(1) == "algorithm") //Initial algorithm section begins
            {
                if(foundInitialAlgorithms) {
                    printErrorMessage("Only one initial algorithm section allowed.");
                    return false;
                }
                section = InitialAlgorithmSection;
                continue;
            }
            else if(words.at(0) == "equation") //New equation section begins
            {
                section = EquationSection;
                continue;
            }
            if(words.at(0) == "if") {
                QString condition = lines[l].remove("if").remove("then").trimmed();
                QStringList vars;
                QStringList ifExpressions;
                QStringList elseExpressions;
                ++l;
                while(!lines.at(l).trimmed().startsWith("end if") && !lines.at(l).trimmed().startsWith("else")) {
                    QString lhs = lines.at(l).section(":=",0,0).trimmed();
                    QString rhs = lines.at(l).section(":=",1,1).trimmed();
                    vars << lhs;
                    rhs.remove(";");
                    ifExpressions << rhs;
                    elseExpressions << "";  //Populate in case it is needed below
                    ++l;
                }
                if(lines.at(l).trimmed().startsWith("else")) {

                    ++l;
                    while(!lines.at(l).trimmed().startsWith("end if")) {
                        QString lhs = lines.at(l).section(":=",0,0).trimmed();
                        QString rhs = lines.at(l).section(":=",1,1).trimmed();
                        rhs.remove(";");
                        elseExpressions[vars.indexOf(lhs)] = rhs;
                        ++l;
                    }
                }
                for(int i=0; i<vars.size(); ++i) {
                    if(elseExpressions.isEmpty()) {
                        algorithms << vars[i]+"=ifElse("+condition+","+ifExpressions[i]+","+vars[i]+")";
                    }
                    else {
                        algorithms << vars[i]+"=ifElse("+condition+","+ifExpressions[i]+","+elseExpressions[i]+")";
                    }
                }
            }
            else {
                algorithms << lines.at(l).trimmed();
                algorithms.last().replace(":=", "=");
            }
            if(algorithms.endsWith(";")) {
                algorithms.last().chop(1);
            }
        }
        else if(section == EquationSection)
        {
            if(!verifyModelicaLine(lines[l], Equation)) {
                return false;
            }

           // qDebug() << l << " - in equations";
            QStringList words = lines.at(l).trimmed().split(" ");
            if(words.at(0) == "end")       //We are finished
            {
                break;
            }
            else if(words.at(0) == "initial" && words.at(1) == "algorithm") //Initial algorithm section begins
            {
                if(foundInitialAlgorithms) {
                    printErrorMessage("Only one initial algorithm section allowed.");
                    return false;
                }
                section = InitialAlgorithmSection;
                continue;
            }
            else if(words.at(0) == "algorithm") //New algorithm section beginning
            {
                section = EquationSection;
                continue;
            }
            else if(words.at(0) == "if") {
                QString condition = lines[l].remove("if").remove("then").trimmed();
                QStringList vars;
                QStringList ifExpressions;
                QStringList elseExpressions;
                ++l;
                while(!lines.at(l).trimmed().startsWith("end if") && !lines.at(l).trimmed().startsWith("else")) {
                    QString lhs = lines.at(l).section("=",0,0).trimmed();
                    QString rhs = lines.at(l).section("=",1,1).trimmed();
                    vars << lhs;
                    rhs.remove(";");
                    ifExpressions << rhs;
                    ++l;
                }
                if(lines.at(l).trimmed().startsWith("else")) {

                    ++l;
                    while(!lines.at(l).trimmed().startsWith("end if")) {
                        QString lhs = lines.at(l).section("=",0,0).trimmed();
                        QString rhs = lines.at(l).section("=",1,1).trimmed();
                        elseExpressions << rhs;
                        rhs.remove(";");
                        ++l;
                    }
                }
                for(int i=0; i<vars.size(); ++i) {
                    if(elseExpressions.isEmpty()) {
                        printErrorMessage("If statements in equation sections must have an \"else\" section.");
                        return false;
                    }
                    else if(elseExpressions.size() != ifExpressions.size()) {
                        printErrorMessage("\"If\" section and \"else\" section must have same number of equations.");
                        return false;
                    }
                    else {
                        equations << vars[i]+"=ifElse("+condition+","+ifExpressions[i]+","+elseExpressions[i]+");";
                    }
                }
            }
            else {
                equations << lines.at(l).trimmed();
            }
            //Replace variables with Hopsan syntax, i.e. P2.q => q2
            for(int i=0; i<portNames.size(); ++i)
            {
                QString temp = portNames.at(i)+".";
                while(!equations.isEmpty() && equations.last().contains(temp))
                {
                    if(portList.at(i).nodetype == "NodeSignal")     //Signal nodes are special, they use the port name as the variable name
                    {
                        equations.last().replace(".","__");
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
            if(!equations.isEmpty()) {
                equations.last().chop(1);
            }
        }
    }

    for(auto &algorithm : algorithms) {
        //Replace variables with Hopsan syntax, i.e. P2.q => q2
        for(int i=0; i<portNames.size(); ++i)
        {
            QString temp = portNames.at(i)+".";
            while(algorithm.contains(temp))
            {
                if(portList.at(i).nodetype == "NodeSignal")     //Signal nodes are special, they use the port name as the variable name
                {
                    int idx = algorithm.indexOf(temp)+temp.size()-1;
                    if(portList.at(i).porttype == "WritePort")
                    {
                        algorithm.remove(idx, 4);
                    }
                    else if(portList.at(i).porttype == "ReadPort")
                    {
                        algorithm.remove(idx, 3);
                    }
                }
                else
                {
                    int idx = algorithm.indexOf(temp);
                    int idx2=idx+temp.size()+1;
                    while(idx2 < algorithm.size()+1 && algorithm.at(idx2).isLetterOrNumber())
                        ++idx2;
                    algorithm.insert(idx2, QString::number(i+1));
                    algorithm.remove(idx, temp.size());
                }
            }
        }
    }

    initAlgorithms.removeAll("\n");
    initAlgorithms.removeAll("");
    algorithms.removeAll("\n");
    algorithms.removeAll("");
    equations.removeAll("\n");
    equations.removeAll("");

    if(!foundHeader) {
        printErrorMessage("Syntax error in Modelica code: Could not parse model header.");
        return false;
    }
    else if(algorithms.isEmpty() && equations.isEmpty()) {
        printErrorMessage("Modelica model contains no algorithms or equations.");
        return false;
    }
    printMessage("Successfully parsed Modelica model.");
    return true;
}






bool HopsanModelicaGenerator::replaceCustomFunctions(Expression &expr) {
    Expression *pFuncExpr = expr.findFunction("turbulentFlow");
    while(pFuncExpr != nullptr) {
        if(pFuncExpr->getArguments().size() != 5) {
            printErrorMessage("turbulentFlow() function takes exactly 5 arguments.");
            return false;
        }
        QString c1 = "("+pFuncExpr->getArguments().at(0).toString()+")";
        QString c2 = "("+pFuncExpr->getArguments().at(1).toString()+")";
        QString Zc1 = "("+pFuncExpr->getArguments().at(2).toString()+")";
        QString Zc2 = "("+pFuncExpr->getArguments().at(3).toString()+")";
        QString Ks = "("+pFuncExpr->getArguments().at(4).toString()+")";
        Expression newExpr = Expression(QString("onPositive(%1-%2)*(%5*(sqrt(abs(%1-%2)+(%3+%4)*(%3+%4)*%5*%5/4.0)-%5*(%3+%4)/2.0)) + onNegative(%1-%2)*(%5*(%5*(%3+%4)/2.0-sqrt(abs(%1-%2)+(%3+%4)*(%3+%4)*%5*%5/4.0)))").arg(c1).arg(c2).arg(Zc1).arg(Zc2).arg(Ks));
        printMessage("Replacing: "+pFuncExpr->toString());
        printMessage("With: "+newExpr.toString());
        *pFuncExpr = newExpr;

        pFuncExpr = expr.findFunction("turbulentFlow");
    }

    return true;
    //Ks = Cq*d*3.1415*xv*sqrt(2.0/rho);
        //P2.q =   onPositive(P1.c-P2.c)*(Ks*(sqrt(abs(P1.c-P2.c)+(P1.Zc+P2.Zc)*(P1.Zc+P2.Zc)*Ks*Ks/4.0)-Ks*(P1.Zc+P2.Zc)/2.0)) + onNegative(P1.c-P2.c)*(Ks*(Ks*(P1.Zc+P2.Zc)/2.0-sqrt(abs(P1.c-P2.c)+(P1.Zc+P2.Zc)*(P1.Zc+P2.Zc)*Ks*Ks/4.0)));
}





bool HopsanModelicaGenerator::generateComponentObject(ComponentSpecification &comp, QString &typeName, QString &displayName, QString &cqsType, QString &transform, QStringList &initAlgorithms, QStringList &algorithms, QStringList &plainEquations, QList<PortSpecification> &ports, QList<ParameterSpecification> &parameters, QList<VariableSpecification> &variables, QTextStream &logStream)
{
    printMessage("Initializing Modelica generator for Kinsol solver.");

    //Create list of equations
    QList<Expression> systemEquations;
    QList<VariableLimitation> limitedVariables;
    QList<CavitationCheck> cavitationChecks;
    for(int e=0; e<plainEquations.size(); ++e) {
        if(plainEquations[e].trimmed().startsWith("//")) {
            continue;   //Ignore comments
        }
        bool ok;
        systemEquations.append(Expression(plainEquations.at(e), &ok));
        if(!ok) {
            QStringList errors = systemEquations.last().readErrorMessages();
            for(const QString &error : errors) {
                printErrorMessage(error);
            }
            return false;
        }
        if(systemEquations.last().getFunctionName() == "limitVariable") {
            int nArgs = systemEquations.last().getArguments().size();
            VariableLimitation limit;
            if(nArgs == 3) {
                limit.var = systemEquations.last().getArgument(0).toString();
                limit.min = systemEquations.last().getArgument(1).toString();
                limit.max = systemEquations.last().getArgument(2).toString();
            }
            else if(nArgs == 4) {
                limit.var = systemEquations.last().getArgument(0).toString();
                limit.der = systemEquations.last().getArgument(1).toString();
                limit.min = systemEquations.last().getArgument(2).toString();
                limit.max = systemEquations.last().getArgument(3).toString();
            }
            else {
                printErrorMessage("Wrong number of arguments to limitVariable() (should be 3 or 4).");
                return false;
            }
            limitedVariables.append(limit);
            systemEquations.removeLast();
            continue;
        }
        if(systemEquations.last().getFunctionName() == "preventCavitation") {
            if(systemEquations.last().getArguments().size() != 3) {
                printErrorMessage("Wrong number of arguments to preventCavitation() (should be 3).");
                return false;
            }
            CavitationCheck cav;
            cav.pressure = systemEquations.last().getArguments()[0].toString();
            cav.wave = systemEquations.last().getArguments()[1].toString();
            cav.impedance = systemEquations.last().getArguments()[2].toString();
            cavitationChecks.append(cav);
            systemEquations.removeLast();
            continue;
        }
        if(!replaceCustomFunctions(systemEquations.last())) {
            printErrorMessage("Failed to replace custom functions.");
            return false;
        }
        logStream << systemEquations.last().toString() << "\n";
        if(!systemEquations.last().isEquation()) {
            printErrorMessage("Expected an equation: "+systemEquations[e].toString());
            return false;
        }
    }

    //Verify each equation
    for(const auto &equation : systemEquations) {
        if(!equation.verifyExpression()) {
            printErrorMessage("Illegal function(s) found in "+equation.toString());
            return false;
        }
    }

    //Identify and replace pure derivatives ("der(x) = y")
    for(const auto &equation : systemEquations) {
        if(equation.getLeft()->isSymbol() && equation.getRight()->getFunctionName() == "der" && equation.getRight()->getArgument(0).isSymbol()) {
            for(auto &equation2 : systemEquations) {
                if(equation != equation2) {
                    equation2.replace(*equation.getLeft(), *equation.getRight());
                }
            }
        }
        else if(equation.getRight()->isSymbol() && equation.getLeft()->getFunctionName() == "der" && equation.getLeft()->getArgument(0).isSymbol()) {
            for(auto &equation2 : systemEquations) {
                if(equation != equation2) {
                    equation2.replace(*equation.getRight(), *equation.getLeft());
                }
            }
        }
    }

    //Sum up all used variables to a single list
    QList<Expression> unknowns;
    for(const auto &variable : variables) {
        unknowns.append(Expression(variable.name));
    }
    for(const auto &port : ports) {
        if(port.porttype == "WritePort") {
            unknowns.append(Expression(port.name));
        }
    }
    for(const auto &equation : systemEquations) {
        unknowns.append(equation.getVariables());
    }

    //Add variables defined in initial algorithms to list of known variables
    QList<Expression> knowns;
    for(auto &algorithm : initAlgorithms) {
        algorithm.replace(":=","=");
        if(algorithm.contains("=")) {
            QString var = algorithm.section("=",0,0).trimmed();
            knowns.append(var);
        }
    }

    for(auto &algorithm : algorithms) {
        algorithm.replace(":=","=");
        while(algorithm.endsWith(";")) {
            algorithm.chop(1);
        }
        Expression algExpr = Expression(algorithm);
        if(!algExpr.isAssignment()) {
            printErrorMessage("Only assignments are allowed in algorithm sections.");
            return false;
        }
        if(!algExpr.verifyExpression()) {
            printErrorMessage("Illegal function(s) found in "+algExpr.toString());
            return false;
        }

        for(auto &equation : systemEquations) {
            equation.replace(*algExpr.getLeft(), *algExpr.getRight());
        }
        systemEquations.append(algExpr);
        unknowns.append(algExpr.getVariables());
    }

    //Add parameters to list of known variables
    for(const auto  &par : parameters) {
        knowns.append(Expression(par.name));
    }

    //Add TLM input variables to list of known variables
    for(int i=0; i<ports.size(); ++i) {
        QString num = QString::number(i+1);
        if(ports[i].porttype == "ReadPort") {
            knowns.append(Expression(ports[i].name));
        }
        else if(ports[i].porttype == "PowerPort" && cqsType == "C") {
            QStringList qVars;
            qVars << GeneratorNodeInfo(ports[i].nodetype).qVariables;
            for(int v=0; v<qVars.size(); ++v) {
                knowns.append(Expression(qVars[v]+num));
            }
        }
        else if(ports[i].porttype == "PowerPort" && cqsType == "Q") {
            QStringList cVars;
            cVars << GeneratorNodeInfo(ports[i].nodetype).cVariables;
            for(int v=0; v<cVars.size(); ++v) {
                knowns.append(Expression(cVars[v]+num));
            }
        }
    }

    //Remove known variables from list of unknowns
    for(const auto &known : knowns) {
        unknowns.removeAll(known);
    }
    removeDuplicates(unknowns);
    unknowns.removeAll(Expression("mTime"));
    unknowns.removeAll(Expression("mTimestep"));
    unknowns.removeAll(Expression("s"));

    //Verify equation system
    printMessage("Found "+QString::number(systemEquations.size())+" equations, "+QString::number(knowns.size())+" known variables and "+QString::number(unknowns.size())+" unknown variables.");
    if(!verifyEquationSystem(systemEquations, unknowns, this)) {
        printErrorMessage("Verification of equation system failed.");
        return false;
    }

    //Verify limitations
    for(VariableLimitation &limit : limitedVariables) {
        if(!unknowns.contains(limit.var) || (!limit.der.isEmpty() && !unknowns.contains(limit.der))) {
            printErrorMessage("Limited variables not found in model, or are not unknown.");
            return false;
        }
    }

    //Verify cavitation
    for(const auto &cav : cavitationChecks) {
        bool foundPressure = unknowns.contains(cav.pressure) || knowns.contains(cav.pressure);
        bool foundWave= unknowns.contains(cav.wave) || knowns.contains(cav.wave);
        bool foundImpedance= unknowns.contains(cav.impedance) || knowns.contains(cav.impedance);
        if(!foundPressure || !foundWave || !foundImpedance) {
            printErrorMessage("All cavitation variables not found in model.");
            return false;
        }
    }

    //Map limitations to corresponding equations
    for(VariableLimitation &limit : limitedVariables) {
        Expression firstDerExpr = Expression::fromFunctionArgument("der", Expression(limit.var));
        for(const auto &equation : systemEquations) {
            if(equation.contains(firstDerExpr)) {
                limit.varEquation = systemEquations.indexOf(equation);
            }
        }
        if(limit.der.isEmpty()) {
            continue;
        }
        Expression secondDerExpr = Expression::fromFunctionArgument("der", Expression(limit.der));
        for(const auto &equation : systemEquations) {
            if(equation.contains(secondDerExpr)) {
                limit.derEquation = systemEquations.indexOf(equation);
            }
        }
    }

    //Make all equations left-sided
    for(int e=0; e<systemEquations.size(); ++e) {
        systemEquations[e].toLeftSided();
    }

    //Apply inline transform
    for(int e=0; e<systemEquations.size(); ++e)
    {
        bool ok;
        systemEquations[e] = systemEquations[e].inlineTransform(strToTransform(transform), ok);
        if(!ok) {
            QStringList errors = systemEquations[e].readErrorMessages();
            for(const QString &error : errors) {
                printErrorMessage(error);
            }
            return false;
        }
        systemEquations[e]._simplify(Expression::FullSimplification, Expression::Recursive);
    }

    //Linearize equations
    for(int e=0; e<systemEquations.size(); ++e)
    {
        systemEquations[e].linearize();
        systemEquations[e]._simplify(Expression::FullSimplification, Expression::Recursive);
        systemEquations[e].replaceBy((*systemEquations[e].getLeft()));
    }

    //Transform delay operators to delay functions and store delay terms separately
    QList<Expression> delayTerms;
    QStringList delaySteps;
    for(int e=0; e<systemEquations.size(); ++e)
    {
        systemEquations[e]._simplify();
        systemEquations[e].expand();
        systemEquations[e].factor(Expression("Z"));
        systemEquations[e].factor(Expression("Z*Z"));
        systemEquations[e].factor(Expression("Z*Z*Z"));
        systemEquations[e].factor(Expression("Z*Z*Z*Z"));
        systemEquations[e].factor(Expression("Z*Z*Z*Z*Z"));
        systemEquations[e].factor(Expression("Z*Z*Z*Z*Z*Z"));
        systemEquations[e].factor(Expression("Z*Z*Z*Z*Z*Z*Z"));
        systemEquations[e].factor(Expression("Z*Z*Z*Z*Z*Z*Z*Z"));
        systemEquations[e].factor(Expression("Z*Z*Z*Z*Z*Z*Z*Z*Z"));
        systemEquations[e].factor(Expression("Z*Z*Z*Z*Z*Z*Z*Z*Z*Z"));
        systemEquations[e].toDelayForm(delayTerms, delaySteps);
        systemEquations[e]._simplify(Expression::FullSimplification);
    }

    //Simplify delay terms
    for(Expression &term : delayTerms) {
        term._simplify(Expression::FullSimplification, Expression::Recursive);
        term.expandPowers();
    }

    //Identify system equations containing only one unknown (can be resolved before the rest of the system)
    QStringList preAlgorithms;
    bool didSomething = true;
    while(didSomething) {
        didSomething = false;
        for(int e=0; e<systemEquations.size(); ++e) {
            QList<Expression> usedUnknowns;
            for(const auto &unknown : unknowns) {
                if(systemEquations[e].contains(unknown)) {
                    usedUnknowns.append(unknown);
                }
            }

            if(usedUnknowns.size() == 1) {
                //Found only one unknown, try to break it out of the equation
                Expression tempExpr = Expression::fromEquation(systemEquations[e],Expression(0));
                tempExpr.linearize();
                tempExpr.expand();
                tempExpr = (*tempExpr.getLeft());
                tempExpr.factor(usedUnknowns[0]);
                if(tempExpr.getTerms().size() == 1) {
                    tempExpr = Expression(0.0);
                }
                else {
                    Expression term = tempExpr.getTerms()[0];
                    tempExpr.removeTerm(term);
                    term.replace(usedUnknowns[0], Expression(1));
                    term._simplify(Expression::FullSimplification, Expression::Recursive);
                    tempExpr.divideBy(term);
                    tempExpr.changeSign();
                    tempExpr._simplify(Expression::FullSimplification, Expression::Recursive);
                }
                if(!tempExpr.contains(usedUnknowns[0])) {
                    Expression algorithm = Expression::fromEquation(usedUnknowns[0], tempExpr);
                    algorithm._simplify(Expression::FullSimplification, Expression::Recursive);
                    algorithm.expandPowers();
                    printMessage("Moving the following equations to intial algorithm section:");
                    printMessage("  "+algorithm.toString());

                    preAlgorithms.append(algorithm.toString());
                    systemEquations.removeAt(e);
                    --e;
                    unknowns.removeAll(usedUnknowns[0]);
                    didSomething = true;
                }
            }
        }
    }


    //Identify system equations containing a unique variable (can be resolved after the rest of the system)
    QStringList finalAlgorithms;
    for(int u=0; u<unknowns.size(); ++u) {
        size_t count=0;
        int lastFound=-1;
        for(int e=0; e<systemEquations.size(); ++e)
        {
            if(systemEquations[e].contains(unknowns[u]))
            {
                ++count;
                lastFound=e;
            }
        }
        if(count==1)
        {
            //Found the unknown if only one equation, try to break it out and prepend on final algorithms
            Expression tempExpr = systemEquations[lastFound];
            tempExpr.factor(unknowns[u]);
            if(tempExpr.getTerms().size() == 1) {
                tempExpr = Expression(0.0);
            }
            else {
                Expression term = tempExpr.getTerms()[0];
                tempExpr.removeTerm(term);
                term.replace(unknowns[u], Expression(1));
                term._simplify(Expression::FullSimplification, Expression::Recursive);
                tempExpr.divideBy(term);
                tempExpr.changeSign();
                tempExpr._simplify(Expression::FullSimplification, Expression::Recursive);
            }
            if(!tempExpr.contains(unknowns[u]))
            {
                Expression algExpr = Expression::fromEquation(unknowns[u], tempExpr);
                algExpr._simplify(Expression::FullSimplification, Expression::Recursive);
                printMessage("Moving the following equations to final algorithms:");
                printMessage("  "+algExpr.toString());
                algExpr.expandPowers();
                finalAlgorithms.prepend(algExpr.toString());
                systemEquations.removeAt(lastFound);
                unknowns.removeAt(u);
                u = 0;  //Restart checking
            }
        }
    }

    //Differentiate each equation for each state variable to generate the Jacobian matrix
    QList<QList<Expression> > jacobian;
    for(int e=0; e<systemEquations.size(); ++e)
    {
         //Remove all delay operators, since they shall not be in the Jacobian anyway
        gTempExpr = systemEquations[e];
        gTempExpr._simplify(Expression::FullSimplification, Expression::Recursive);

        QList<Expression> result;
        for(int u=0; u<unknowns.size(); ++u)
        {
            result.append(concurrentDiff(unknowns[u]));
            result.last().expandPowers();
        }

        jacobian.append(result);
    }

    //Expand power functions for performance
    for(auto &equation : systemEquations) {
        equation.expandPowers();
    }

    logStream << "\n--- Initial Algorithms ---\n";
    printMessage("Initial algorithms:");
    for(int i=0; i<initAlgorithms.size(); ++i) {
        logStream << initAlgorithms[i] << "\n";
        printMessage("  "+initAlgorithms[i]);
    }

    logStream << "\n--- Equation System ---\n";
    printMessage("Equation system:");
    for(int i=0; i<systemEquations.size(); ++i) {
            logStream << systemEquations[i].toString() << " = 0\n";
            printMessage("  "+systemEquations[i].toString()+" = 0");
    }

    logStream << "\n--- Final Algorithms ---\n";
    printMessage("Final algorithms:");
    for(int i=0; i<finalAlgorithms.size(); ++i) {
        printMessage("  "+finalAlgorithms[i]);
        logStream << finalAlgorithms[i] << "\n";
    }

    //Generate component specification object

    comp.typeName = typeName;
    comp.displayName = displayName;
    comp.cqsType = cqsType;
    if(comp.cqsType == "S") { comp.cqsType = "Signal"; }

    for(int i=0; i<ports.size(); ++i)
    {
        comp.portNames << ports[i].name;
        comp.portDescriptions << ports[i].description;
        comp.portUnits << ports[i].unit;
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

    comp.parNames << "mTolerance";
    comp.parDisplayNames << "tolerance";
    comp.parDescriptions << "Solver tolerance";
    comp.parUnits << "";
    comp.parInits << "1e-5";

    for(int i=0; i<delayTerms.size(); ++i)
    {
        comp.utilities << "Delay";
        comp.utilityNames << "mDelay"+QString::number(i);
    }

    for(int i=0; i<variables.size(); ++i)
    {
        comp.varNames.append(variables[i].name);
        comp.varInits.append(variables[i].init);
        comp.varTypes.append("double");
    }

    if(!unknowns.isEmpty()) {
        comp.varNames << "mpSolver";
        comp.varInits << "";
        comp.varTypes << "KinsolSolver*";
    }

    if(!unknowns.isEmpty()) {
        QString solverMethod = "KinsolSolver::NewtonIteration";
        comp.initEquations << "mpSolver = new KinsolSolver(this, mTolerance, "+QString::number(systemEquations.size())+", "+solverMethod+");";
    }

    for(int i=0; i<delayTerms.size(); ++i)
    {
        comp.initEquations << "mDelay"+QString::number(i)+".initialize("+QString::number(int(delaySteps.at(i).toDouble()))+", "+delayTerms[i].toString()+");";
    }

    if(!comp.initEquations.isEmpty()) {
        comp.initEquations << "";
        comp.initEquations << "//Initial algorithm section";
        for(const auto &algorithm : initAlgorithms) {
            comp.initEquations << algorithm+";";
        }
    }

    if(!cavitationChecks.isEmpty()) {
        comp.simEquations << "while(true) {";
        comp.simEquations << "";
    }

    if(!preAlgorithms.isEmpty()) {
        comp.simEquations << "";
        comp.simEquations << "//Pre-algorithm section";
        for(int i=0; i<preAlgorithms.size(); ++i)
        {
            comp.simEquations << preAlgorithms[i]+";";
        }
        comp.simEquations << "";
    }

    if(!unknowns.isEmpty()) {
        comp.simEquations << "//Provide Kinsol with updated state variables";
        for(int u=0; u<unknowns.size(); ++u) {
            comp.simEquations << "mpSolver->setState("+QString::number(u)+","+unknowns[u].toString()+");";
        }
        comp.simEquations << "";
        comp.simEquations << "//Solve algebraic equation system";
        comp.simEquations << "mpSolver->solve();";
        comp.simEquations << "";
        comp.simEquations << "//Obtain new state variables from Kinsol";
        for(int u=0; u<unknowns.size(); ++u) {
            comp.simEquations << unknowns[u].toString()+" = mpSolver->getState("+QString::number(u)+");";
        }
        comp.simEquations << "";
    }

    if(!finalAlgorithms.isEmpty()) {
        comp.simEquations << "//Final algorithm section";
        for(int i=0; i<finalAlgorithms.size(); ++i)
        {
            comp.simEquations << finalAlgorithms[i]+";";
        }
        comp.simEquations << "";
    }

    if(!cavitationChecks.isEmpty()) {
        comp.simEquations << "//Handle cavitation checks";
        comp.simEquations << "bool cav = false;";
        for(const auto &cav : cavitationChecks) {
            comp.simEquations << "if("+cav.pressure+" < 0.0)";
            comp.simEquations << "{";
            comp.simEquations << "    "+cav.wave+" = 0.0;";
            comp.simEquations << "    "+cav.impedance+" = 0.0;";
            comp.simEquations << "    cav = true;";
            comp.simEquations << "}";
        }
        comp.simEquations << "if(!cav) {";
        comp.simEquations << "    break;";
        comp.simEquations << "}";
        comp.simEquations << "}";
        comp.simEquations << "";
    }

    comp.simEquations << "//Handle variable limitations";
    if(true) {
        comp.simEquations << "bool reachedLimit = false;";
        for(const auto &limit : limitedVariables) {
            comp.simEquations << "if("+limit.var+" < "+limit.min+") {";
            comp.simEquations << "    "+limit.var+" = "+limit.min+";";
            if(!limit.der.isEmpty()) {
                comp.simEquations << "    "+limit.der+" = max(0.0,"+limit.der+");";
            }
            comp.simEquations << "    reachedLimit = true;";
            comp.simEquations << "}";
            comp.simEquations << "if("+limit.var+" > "+limit.max+") {";
            comp.simEquations << "    "+limit.var+" = "+limit.max+";";
            if(!limit.der.isEmpty()) {
                comp.simEquations << "    "+limit.der+" = min(0.0,"+limit.der+");";
            }
            comp.simEquations << "    reachedLimit = true;";
            comp.simEquations << "}";
        }

        comp.simEquations << "if(reachedLimit) {";
        for(int i=0; i<delayTerms.size(); ++i)
        {
            comp.simEquations << "    mDelay"+QString::number(i)+".initialize("+QString::number(int(delaySteps.at(i).toDouble()))+", "+delayTerms[i].toString()+");";
        }
        comp.simEquations << "}";
        comp.simEquations << "";
    }
    for(int i=0; i<delayTerms.size(); ++i)
    {
        comp.simEquations << "mDelay"+QString::number(i)+".update("+delayTerms[i].toString()+");";
    }


    if(!unknowns.isEmpty()) {
        comp.auxiliaryFunctions << "//! @brief Returns the residuals for speed and position";
        comp.auxiliaryFunctions << "//! @param [in] y Array of state variables from previous iteration";
        comp.auxiliaryFunctions << "//! @param [out] res Array of residuals or new state variables";
        comp.auxiliaryFunctions << "void getResiduals(double *y, double *res)";
        comp.auxiliaryFunctions << "{";
        for(int u=0; u<unknowns.size(); ++u) {
            comp.auxiliaryFunctions << "    double "+unknowns[u].toString()+" = y["+QString::number(u)+"];";
        }
        comp.auxiliaryFunctions << "    ";
        for(int e=0; e<systemEquations.size(); ++e) {
            comp.auxiliaryFunctions << "    res["+QString::number(e)+"] = "+systemEquations[e].toString()+";";
        }
        comp.auxiliaryFunctions << "}";

        comp.auxiliaryFunctions << "";
        comp.auxiliaryFunctions << "//! @brief Returns the residuals for speed and position";
        comp.auxiliaryFunctions << "//! @param [in] y Array of state variables from previous iteration";
        comp.auxiliaryFunctions << "//! @param [in] f Array of function values (f(y))";
        comp.auxiliaryFunctions << "//! @param [out] J Array of Jacobian elements, stored column-wise";
        comp.auxiliaryFunctions << "void getJacobian(double *y, double *f, double *J)";
        comp.auxiliaryFunctions << "{";
        for(int u=0; u<unknowns.size(); ++u) {
            comp.auxiliaryFunctions << "    double "+unknowns[u].toString()+" = y["+QString::number(u)+"];";
        }
        comp.auxiliaryFunctions << "    ";

        //Only compute Jacobian elements that are non-zero for best performance
        for(int i=0; i<jacobian.size(); ++i) {
            for(int j=0; j<jacobian[i].size(); ++j) {
                if(jacobian[i][j] != Expression(0)) {
                    comp.auxiliaryFunctions << QString("    J[%2*%3+%1] = ").arg(i).arg(j).arg(unknowns.size()) + jacobian[i][j].toString() + ";";
                }
            }
        }
        comp.auxiliaryFunctions << "}";
    }

    printMessage("Component specification succesfully generated!");

    return true;
}



//! @brief Sorts and equation system by its depending variables, so that it can be solved equation-by-equation
//! @param equations List with equations to sort
//! @param variables Empty list that will contain corresponding variables
//! @param knowns List of already known parameter, that does not need to be taken into consideration
bool HopsanModelicaGenerator::sortEquationByVariables(QList<Expression> &equations, QList<Expression> &variables, QList<Expression> &knowns)
{
    //Generate list of variables in equations
    for(const Expression &equation : equations) {
        QList<Expression> tempVars = equation.getVariables();
        for(const Expression var : tempVars) {
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
    for(const Expression &equation : equations) {
        dependencies.append(QList<Expression>());
        for(const Expression &var : variables) {
            if(equation.contains(var)) {
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

bool HopsanModelicaGenerator::verifyModelicaLine(const QString &line, int flags)
{
    //Split line at spaces unless space is within quotation marks
    QStringList split = line.split(QRegExp("\\s(?=(?:[^'\"`]*(['\"`])[^'\"`]*\\1)*[^'\"`]*$)"), QString::SkipEmptyParts);

    if(flags & ModelDeclaration) {
        if(split.size() == 2 &&
                split.at(0) == "model") {
            return true;
        }
        if(split.size() == 3 &&
                split.at(0) == "model" &&
                split.at(2).startsWith("\"") &&
                split.at(2).endsWith("\"")) {
            return true;
        }
    }
    if(flags & VariableDeclaration) {
        QStringList split = line.trimmed().split(" ");

        QStringList dataTypes;
        GeneratorNodeInfo::getNodeTypes(dataTypes);
        dataTypes.append("Real");

        QStringList specifiers = {"parameter", "input", "output"};

        if(split.size() > 1 && dataTypes.contains(split.at(0))) {
            return true;
        }
        if(split.size() > 2 && specifiers.contains(split.at(0)) && dataTypes.contains(split.at(1))) {
            return true;
        }
    }
    if(flags & Annotation) {
        //! @todo Handle multi-line annotations?
        if(line.trimmed().startsWith("annotation(") && (line.endsWith(")") || line.endsWith(");"))) {
            return true;
        }
    }
    if(flags & Assignment) {
        QString temp = line.trimmed();
        temp.replace(":=","=");
        if(SymHop::Expression(line).isAssignment()) {
            return true;
        }
        if(split.size() == 3 && split.at(0) == "if" && split.at(2) == "then") {
            return true;
        }
        if(line.trimmed() == "else") {
            return true;
        }
        if(line.trimmed() == "end if" || line.trimmed() == "end if;") {
            return true;
        }
    }
    if(flags & Equation) {
        QString temp = line.trimmed();
        if(!line.contains(":=") && SymHop::Expression(line).isEquation()) {
            return true;
        }
        SymHop::Expression expr(line.trimmed().remove(";"));
        if(expr.getFunctionName() == "limitVariable") {
            return true;
        }
        if(expr.getFunctionName() == "preventCavitation") {
            return true;
        }
        if(split.size() == 3 && split.at(0) == "if" && split.at(2) == "then") {
            return true;
        }
        if(line.trimmed() == "else") {
            return true;
        }
        if(line.trimmed() == "end if" || line.trimmed() == "end if;") {
            return true;
        }
    }
    //! @todo Add more checks

    //All checks failed, print error and return false
    QString errorMessage = "Verification of line failed:\n-> "+ line.trimmed()+"\n(expected ";
    if(flags & ModelDeclaration) {
        errorMessage.append("model declaration/");
    }
    if(flags & VariableDeclaration) {
        errorMessage.append("variable declaration/");
    }
    if(flags & Annotation) {
        errorMessage.append("annotation");
    }
    if(flags & Assignment) {
        errorMessage.append("assignment");
    }
    if(flags & Equation) {
        errorMessage.append("equation");
    }
    if(errorMessage.endsWith("/")) {
        errorMessage.chop(1);
    }
    errorMessage.append(")");
    printErrorMessage(errorMessage);
    return false;
}
