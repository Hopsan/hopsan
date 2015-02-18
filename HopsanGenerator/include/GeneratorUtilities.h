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
//! @brief Contains component generation utility functions
//!
//$Id$


#ifndef GENERATORUTILITIES_H
#define GENERATORUTILITIES_H

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
#include "win32dll.h"

#include "SymHop.h"

#include "JM/jm_portability.h"

//class ModelObjectAppearance;

namespace hopsan {
class ComponentSystem;
}

class HopsanGenerator;


class ComponentAppearanceSpecification
{
public:
    ComponentAppearanceSpecification(QString typeName)
    {
        mTypeName = typeName;
        mDisplayName = typeName;
        mRecompilable = false;
        mUserIconRotation = true;
        mIsoIconRotation = true;
        mUserIconScale = 1.0;
        mIsoIconScale = 1.0;
    }

    void addPort(QString name, double x, double y, double a)
    {
        mPortNames.append(name);
        mPortX.append(x);
        mPortY.append(y);
        mPortA.append(a);
    }

    QString mTypeName;
    QString mDisplayName;
    QString mLibPath;
    QString mSourceCode;
    bool mRecompilable;

    QString mUserIconPath;
    bool mUserIconRotation;
    QString mIsoIconPath;
    bool mIsoIconRotation;
    double mUserIconScale;
    double mIsoIconScale;

    QStringList mPortNames;
    QList<double> mPortX;
    QList<double> mPortY;
    QList<double> mPortA;
};






class PortSpecification
{
public:
    PortSpecification(QString porttype = "ReadPortType", QString nodetype = "NodeSignal", QString name = QString(), bool notrequired=false, QString defaultvalue=QString());
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

class VariableSpecification
{
public:
    VariableSpecification(QString name = QString(), QString init = QString());
    QString name;
    QString init;
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
    QStringList varInits;
    QStringList portNames;
    QStringList portTypes;
    QStringList portNodeTypes;
    QStringList portDefaults;
    QList<bool> portNotReq;
    QStringList confEquations;
    QStringList initEquations;
    QStringList simEquations;
    QStringList finalEquations;
    QStringList deconfEquations;
    QStringList auxiliaryFunctions;

    QString plainCode;
};



QDomElement loadXMLDomDocument(QFile &rFile, QDomDocument &rDomDocument, QString rootTagName);
bool removeDir(QString path);
void copyDir(const QString fromPath, QString toPath);
bool compileComponentLibrary(QString path, HopsanGenerator *pGenerator, QString extraLFlags="", QString extraIncludes="");
bool compile(QString wdPath, QString gccPath, QString o, QString srcFiles, QString inclPaths, QString cflags, QString lflags, QString &output);
QString toVarName(const QString org);
QString extractTaggedSection(QString str, QString tag);
void replaceTaggedSection(QString &str, QString tag, QString replacement);
QString replaceTag(QString str, QString tag, QString replacement);
QString replaceTags(QString str, QStringList tags, QStringList replacements);

bool verifyEquationSystem(QList<SymHop::Expression> equations, QList<SymHop::Expression> stateVars, HopsanGenerator *pGenerator);

void findAllFilesInFolderAndSubFolders(QString path, QString ext, QStringList &files);
QStringList getHopsanCoreSourceFiles();
QStringList getHopsanCoreIncludeFiles(bool skipDependencies=false);
QStringList getHopsanCoreIncludePaths(bool skipDependencies=false);

void hopsanLogger(jm_callbacks* c, jm_string module, jm_log_level_enu_t log_level, jm_string message);

class GeneratorNodeInfo
{
    public:
        GeneratorNodeInfo(QString nodeType);
        static void getNodeTypes(QStringList &nodeTypes);

        QString niceName;
        QStringList qVariables;
        QStringList cVariables;
        QStringList variableLabels;
        QStringList shortNames;
        QList<size_t> varIdx;
};


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


class InterfaceVarSpec
{
public:
    enum CausalityEnumT {Input, Output};

    InterfaceVarSpec(QString dataName, int dataId, CausalityEnumT causality);

    QString dataName;
    int dataId;
    CausalityEnumT causality;
};


class InterfacePortSpec
{
public:
    enum InterfaceTypesEnumT {Input, Output, MechanicC, MechanicQ, MechanicRotationalC, MechanicRotationalQ, HydraulicC, HydraulicQ, ElectricC, ElectricQ, PneumaticC, PneumaticQ};

    InterfacePortSpec(InterfaceTypesEnumT type, QString component, QString port, QStringList path);

    InterfaceTypesEnumT type;
    QStringList path;
    QString component;
    QString port;

    QList<InterfaceVarSpec> vars;
};





void getInterfaces(QList<InterfacePortSpec> &interfaces, hopsan::ComponentSystem *pSystem, QStringList &path);

#endif // GENERATORUTILITIES_H
