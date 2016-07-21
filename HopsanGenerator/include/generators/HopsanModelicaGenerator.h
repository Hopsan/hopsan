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

#ifndef HOPSANMODELICAGENERATOR_H
#define HOPSANMODELICAGENERATOR_H

#include "HopsanGenerator.h"

extern SymHop::Expression gTempExpr;


SymHop::Expression concurrentDiff(SymHop::Expression expr);

class HopsanModelicaGenerator : public HopsanGenerator
{
public:
    HopsanModelicaGenerator(QString coreIncludePath, QString binPath, QString gccPath, bool showDialog=false);
    void generateFromModelica(QString code, SolverT solver=NumericalIntegration);

private:
    void parseModelicaModel(QString code, QString &typeName, QString &displayName, QString &cqsType, QStringList &initAlgorithms, QStringList &preAlgorithms, QStringList &equations, QStringList &finalAlgorithms, QList<PortSpecification> &portList, QList<ParameterSpecification> &parametersList, QList<VariableSpecification> &variablesList);
    void generateComponentObject(ComponentSpecification &comp, QString &typeName, QString &displayName, QString &cqsType, /*QStringList &initAlgorithms,*/ QStringList &preAlgorithms, QStringList &equations, QStringList &finalAlgorithms, QList<PortSpecification> &portList, QList<ParameterSpecification> &parametersList, QList<VariableSpecification> &variables, QTextStream &logStream);
    void generateComponentObjectNumericalIntegration(ComponentSpecification &comp, QString &typeName, QString &displayName, QString &cqsType, QStringList &initAlgorithms, QStringList &preAlgorithms, QStringList &equations, QStringList &finalAlgorithms, QList<PortSpecification> &portList, QList<ParameterSpecification> &parametersList, QList<VariableSpecification> &variables, QTextStream &logStream);
    bool sortEquationByVariables(QList<SymHop::Expression> &equations, QList<SymHop::Expression> &variables, QList<SymHop::Expression> &knowns);
};

#endif // HOPSANMODELICAGENERATOR_H
