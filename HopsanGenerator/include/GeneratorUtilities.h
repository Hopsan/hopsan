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


bool matchOSString(QString os);
QDomElement loadXMLDomDocument(QFile &rFile, QDomDocument &rDomDocument, QString rootTagName);
bool removeDir(QString path);
bool copyDir(const QString fromPath, QString toPath, QString &rErrorMessage);
bool copyFile(const QString &source, const QString &target, QString &rErrorMessage);
bool compileComponentLibrary(QString path, HopsanGenerator *pGenerator, QString extraLFlags="", QString extraIncludes="");
bool compile(QString wdPath, QString gccPath, QString o, QString srcFiles, QString inclPaths, QString cflags, QString lflags, QString &output);
void callProcess(const QString &name, const QStringList &args, const QString workingDirectory, QString &rStdOut, QString &rStdErr);
QString toValidVarName(const QString &rOrg);
QString extractTaggedSection(QString str, QString tag);
void replaceTaggedSection(QString &str, QString tag, QString replacement);
QString replaceTag(QString str, QString tag, QString replacement);
QString replaceTags(QString str, QStringList tags, QStringList replacements);
bool replacePattern(const QString &rPattern, const QString &rReplacement, QString &rText);

bool verifyEquationSystem(QList<SymHop::Expression> equations, QList<SymHop::Expression> stateVars, HopsanGenerator *pGenerator);

void findAllFilesInFolderAndSubFolders(QString path, QString ext, QStringList &files);
void getHopsanCoreDependecyFiles(QStringList &rSrcFiles, QStringList &rIncludeFiles);
QStringList listHopsanCoreSourceFiles(const QString rootPath);
QStringList listDefaultLibrarySourceFiles(const QString rootPath);
QStringList listHopsanCoreIncludeFiles(const QString rootPath);
QStringList getHopsanCoreIncludePaths();

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
        QList<size_t> qVariableIds;
        QList<size_t> cVariableIds;
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
    friend QTextLineStream& operator <<(QTextLineStream &rLineStream, const QString &input);

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
