#ifndef HOPSANMODELICAGENERATOR_H
#define HOPSANMODELICAGENERATOR_H

#include "HopsanGenerator.h"

extern SymHop::Expression gTempExpr;


SymHop::Expression concurrentDiff(SymHop::Expression expr);

class HopsanModelicaGenerator : public HopsanGenerator
{
public:
    HopsanModelicaGenerator(QString coreIncludePath, QString binPath, bool showDialog=false);
    void generateFromModelica(QString code, SolverT solver=BilinearTransform);

private:
    void parseModelicaModel(QString code, QString &typeName, QString &displayName, QString &cqsType, QStringList &initAlgorithms, QStringList &equations, QStringList &finalAlgorithms, QList<PortSpecification> &portList, QList<ParameterSpecification> &parametersList, QList<VariableSpecification> &variablesList);
    void generateComponentObject(ComponentSpecification &comp, QString &typeName, QString &displayName, QString &cqsType, QStringList &initAlgorithms, QStringList &equations, QStringList &finalAlgorithms, QList<PortSpecification> &portList, QList<ParameterSpecification> &parametersList, QList<VariableSpecification> &variables);
    void generateComponentObjectNumericalIntegration(ComponentSpecification &comp, QString &typeName, QString &displayName, QString &cqsType, QStringList &initAlgorithms, QStringList &equations, QStringList &finalAlgorithms, QList<PortSpecification> &portList, QList<ParameterSpecification> &parametersList, QList<VariableSpecification> &variables);
    bool sortEquationByVariables(QList<SymHop::Expression> &equations, QList<SymHop::Expression> &variables, QList<SymHop::Expression> &knowns);
};

#endif // HOPSANMODELICAGENERATOR_H
