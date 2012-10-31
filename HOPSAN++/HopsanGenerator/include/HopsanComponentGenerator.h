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
//$Id$


#ifndef COMPONENTGENERATORUTILITIES_H
#define COMPONENTGENERATORUTILITIES_H

#include <QPointF>
#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>
#include <QDialog>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QDomElement>

#include "SymHop.h"

//class ModelObjectAppearance;

namespace hopsan {
class ComponentSystem;
}


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
    ComponentSpecification(QString typeName="", QString displayName="", QString cqsType="");
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

    QString plainCode;
};


class HopsanComponentGenerator
{
public:
    HopsanComponentGenerator(QString coreIncludePath, QString binPath, bool showDialog=false);
    void printMessage(QString msg);
    void printErrorMessage(QString msg);
    void generateFromModelica(QString code);
    void generateFromFmu(QString code);
    void generateToFmu(QString savePath, hopsan::ComponentSystem *pSystem);
    void compileFromComponentObject(QString outputFile, ComponentSpecification comp, bool overwriteStartValues=false);

private:
    void parseModelicaModel(QString code, QString &typeName, QString &displayName, QString &cqsType, QStringList &initAlgorithms, QStringList &equations, QStringList &finalAlgorithms, QList<PortSpecification> &portList, QList<ParameterSpecification> &parametersList);
    void generateComponentObject(ComponentSpecification &comp, QString &typeName, QString &displayName, QString &cqsType, QStringList &initAlgorithms, QStringList &equations, QStringList &finalAlgorithms, QList<PortSpecification> &portList, QList<ParameterSpecification> &parametersList);
    QString generateSourceCodefromComponentObject(ComponentSpecification comp, bool overwriteStartValues=false);

    bool verifyParameteres(QList<ParameterSpecification> parameters);
    bool verifyPorts(QList<PortSpecification> ports);
    bool verifyUtilities(QList<UtilitySpecification> utilities);
    bool verifyStaticVariables(QList<StaticVariableSpecification> variables);
    bool verifyEquationSystem(QList<SymHop::Expression> equations, QList<SymHop::Expression> stateVars);
    QStringList getQVariables(QString nodeType);
    QStringList getCVariables(QString nodeType);
    QStringList getVariableLabels(QString nodeType);

    QString mOutputPath;
    QString mTempPath;
    QString mCoreIncludePath;
    QString mBinPath;

    QTextEdit *mpTextEdit;
    QVBoxLayout *mpLayout;
    QPushButton *mpDoneButton;
    QWidget *mpDialog;

    bool mShowDialog;
};



QDomElement loadXMLDomDocument(QFile &rFile, QDomDocument &rDomDocument, QString rootTagName);
void removeDir(QString path);
void copyDir(const QString fromPath, QString toPath);
void copyIncludeFilesToDir(QString path);



//! @brief This utility class wraps a QTextStream and have stream operators to write whole lines. You do not need to add the newline char yourself.
class QTextLineStream
{
public:
    QTextLineStream(QTextStream &rTextStream)
    {
        mpQTextSream = &rTextStream;
    }
    friend QTextLineStream& operator <<(QTextLineStream &rLineStream, const char* input);

private:
    QTextStream* mpQTextSream;
};


#endif // COMPONENTGENERATORUTILITIES_H
