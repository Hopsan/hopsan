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

#ifndef HOPSANMODELICAGENERATOR_H
#define HOPSANMODELICAGENERATOR_H

#include "HopsanGeneratorBase.h"
#include "SymHop.h"

class HopsanModelicaGenerator : public HopsanGeneratorBase
{
public:
    HopsanModelicaGenerator(const QString &hopsanInstallPath, const QString &compilerPath, const QString &tempPath="");
    bool generateFromModelica(QString code, SolverT solver=NumericalIntegration);

private:
    bool replaceCustomFunctions(SymHop::Expression &expr);
    void parseModelicaModel(QString code, QString &typeName, QString &displayName, QString &cqsType, QStringList &initAlgorithms, QStringList &preAlgorithms, QStringList &equations, QStringList &finalAlgorithms, QList<PortSpecification> &portList, QList<ParameterSpecification> &parametersList, QList<VariableSpecification> &variablesList);
    bool generateComponentObject(ComponentSpecification &comp, QString &typeName, QString &displayName, QString &cqsType, /*QStringList &initAlgorithms,*/ QStringList &preAlgorithms, QStringList &equations, QStringList &finalAlgorithms, QList<PortSpecification> &portList, QList<ParameterSpecification> &parametersList, QList<VariableSpecification> &variables, QTextStream &logStream);
    bool generateComponentObjectNumericalIntegration(ComponentSpecification &comp, QString &typeName, QString &displayName, QString &cqsType, QStringList &initAlgorithms, QStringList &preAlgorithms, QStringList &equations, QStringList &finalAlgorithms, QList<PortSpecification> &portList, QList<ParameterSpecification> &parametersList, QList<VariableSpecification> &variables, QTextStream &logStream);
    bool generateComponentObjectKinsol(ComponentSpecification &comp, QString &typeName, QString &displayName, QString &cqsType, /*QStringList &initAlgorithms,*/ QStringList &preAlgorithms, QStringList &plainEquations, QStringList &finalAlgorithms, QList<PortSpecification> &ports, QList<ParameterSpecification> &parameters, QList<VariableSpecification> &variables, QTextStream &logStream);
    bool sortEquationByVariables(QList<SymHop::Expression> &equations, QList<SymHop::Expression> &variables, QList<SymHop::Expression> &knowns);
};

#endif // HOPSANMODELICAGENERATOR_H
