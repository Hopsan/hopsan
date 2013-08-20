#ifndef HOPSANMODELICAGENERATOR_H
#define HOPSANMODELICAGENERATOR_H

#include "HopsanGenerator.h"

extern SymHop::Expression gTempExpr;


SymHop::Expression concurrentDiff(SymHop::Expression expr);

class HopsanModelicaGenerator : public HopsanGenerator
{
public:
    HopsanModelicaGenerator(QString coreIncludePath, QString binPath, bool showDialog=false);
    void generateFromModelica(QString code);

private:
    void parseModelicaModel(QString code, QString &typeName, QString &displayName, QString &cqsType, QStringList &initAlgorithms, QStringList &equations, QStringList &finalAlgorithms, QList<PortSpecification> &portList, QList<ParameterSpecification> &parametersList, QList<VariableSpecification> &variablesList);
    void generateComponentObject(ComponentSpecification &comp, QString &typeName, QString &displayName, QString &cqsType, QStringList &initAlgorithms, QStringList &equations, QStringList &finalAlgorithms, QList<PortSpecification> &portList, QList<ParameterSpecification> &parametersList, QList<VariableSpecification> &variables);
};

#endif // HOPSANMODELICAGENERATOR_H
