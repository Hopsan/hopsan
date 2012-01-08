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
//$Id: GUIUtilities.h 3813 2012-01-05 17:11:57Z robbr48 $


#ifndef COMPONENTGENERATORUTILITIES_H
#define COMPONENTGENERATORUTILITIES_H

#include <QPointF>
#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>
#include <QDomElement>

class PortSpecification
{
public:
    PortSpecification(QString porttype = "ReadPort", QString nodetype = "NodeSignal", QString name = QString(), bool notrequired=false, QString defaultvalue=QString());
    QString porttype;
    QString nodetype;
    QString name;
    bool notrequired;
    QString defaultvalue;
};

class ParameterSpecification
{
public:
    ParameterSpecification(QString name = QString(), QString displayName = QString(), QString description = QString(), QString unit = QString(), QString init = QString());
    QString name;
    QString displayName;
    QString description;
    QString unit;
    QString init;
};

class UtilitySpecification
{
public:
    UtilitySpecification(QString utility="FirstOrderTransferFunction", QString name=QString());
    QString utility;
    QString name;
};

class StaticVariableSpecification
{
public:
    StaticVariableSpecification(QString datatype="double", QString name=QString());
    QString datatype;
    QString name;
};
class ComponentSpecification
{
public:
    ComponentSpecification(QString typeName, QString displayName, QString cqsType);
    QString typeName;
    QString displayName;
    QString cqsType;
    QStringList utilities;
    QStringList utilityNames;
    QStringList parNames;
    QStringList parInits;
    QStringList parDisplayNames;
    QStringList parDescriptions;
    QStringList parUnits;
    QStringList varNames;
    QStringList varTypes;
    QStringList portNames;
    QStringList portTypes;
    QStringList portNodeTypes;
    QStringList portDefaults;
    QList<bool> portNotReq;
    QStringList initEquations;
    QStringList simEquations;
    QStringList finalEquations;
};
void generateComponentSourceCode(QString outputFile, QDomElement &rDomElement);
void generateComponentSourceCode(QString typeName, QString displayName, QString cqsType, QList<PortSpecification> ports, QList<ParameterSpecification> parameters, QStringList sysEquations, QStringList stateVars, QStringList jacobian, QStringList delayTerms, QStringList delaySteps);
void generateComponentSourceCode(QString outputFile, ComponentSpecification comp, bool overwriteStartValues=false);
void identifyVariables(QString equation, QStringList &leftSideVariables, QStringList &righrSideVariables);
void identifyFunctions(QString equation, QStringList &functions);
bool verifyEquations(QStringList equations);
bool verifyEquation(QString equation);
void replaceReservedWords(QStringList &equations);
void replaceReservedWords(QString &equation);
void replaceReservedWords(QList<PortSpecification> &ports);
void identifyDerivatives(QStringList &equations);
void translateDelaysFromPython(QStringList &equations, QStringList &delayTerms, QStringList &delaySteps);
void parseModelicaModel(QString code, QString &typeName, QString &displayName, QStringList &equations, QList<PortSpecification> &portList, QList<ParameterSpecification> &parametersList);

#endif // COMPONENTGENERATORUTILITIES_H
