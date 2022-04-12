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

#ifndef GENERATORTYPES_H
#define GENERATORTYPES_H

#include "hopsangenerator_win32dll.h"

#include <QString>
#include <QStringList>
#include <QVector>

namespace hopsan {

namespace os_strings {
    constexpr auto win = "win";
    constexpr auto win32 = "win32";
    constexpr auto win64 = "win64";
    constexpr auto apple = "apple";
    constexpr auto Linux = "linux";
}

namespace compiler_strings {
    constexpr auto gcc = "gcc";
    constexpr auto gpp = "g++";
    constexpr auto clang = "clang";
    constexpr auto msvc = "cl";
}

namespace sharedlibrary_suffixes {
    constexpr auto dll = "dll";
    constexpr auto so = "so";
    constexpr auto dylib = "dylib";
}

class ComponentSystem;

}

class HOPSANGENERATOR_DLLAPI BuildFlags
{
public:
    enum class Platform {notset, win, win32, win64, Linux, apple};
    enum class Compiler {Any, GCC, Clang, MSVC};
    enum class Language {C, Cpp, Modelica};
    static QString platformString(Platform platform);
    static Platform platformFromString(const QString& platformString);
    static QString compilerString(Compiler compiler, Language language);

    QString platformString() const;
    //QString compilerString() const;

    BuildFlags() = default;
    BuildFlags(const QStringList& cflags, const QStringList& lflags);
    BuildFlags(const Platform platform, const QStringList& cflags, const QStringList& lflags);
    BuildFlags(const Compiler compiler, const QStringList& cflags, const QStringList& lflags);

    QStringList mCompilerFlags;
    QStringList mLinkerFlags;
    Platform mPlatform = Platform::notset;
    Compiler mCompiler = Compiler::Any;
};

class HOPSANGENERATOR_DLLAPI CompilerSelection
{
public:
    CompilerSelection() = default;
    CompilerSelection(const QString& p) : path(p) {}
    CompilerSelection(const BuildFlags::Compiler c, const QString& p) : compiler(c), path(p) {}
    BuildFlags::Compiler compiler = BuildFlags::Compiler::GCC;
    QString path;
};

class HOPSANGENERATOR_DLLAPI ComponentLibrary
{
public:

    QString mLoadFilePath;
    QString mId;
    QString mName;
    QStringList mSourceFiles;
    QStringList mExtraSourceFiles;
    QString mSharedLibraryName;
    QString mSharedLibraryDebugExtension;
    QStringList mComponentCodeFiles;
    QStringList mComponentXMLFiles;
    QStringList mAuxFiles;
    QVector<BuildFlags> mBuildFlags;
    QStringList mIncludePaths;
    QStringList mLinkPaths;
    QStringList mLinkLibraries;

    void clear();
    bool saveToXML(QString filepath) const;
    bool loadFromXML(QString filepath);
    bool generateRegistrationCode(const QString& libraryRootPath, QString& rIncludeCode, QString& rRegisterCode, QString& rGeneratorError) const;

    QStringList checkSourceXMLConsistency() const;
};

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
    PortSpecification(QString porttype = "ReadPortType", QString nodetype = "NodeSignal", QString name = QString(), bool notrequired=false, QString defaultvalue=QString(), QString description = QString(), QString unit = QString());
    QString porttype;
    QString nodetype;
    QString name;
    QString description;
    QString unit;
    bool notrequired;
    QString defaultvalue;
};

class GeneratorNodeInfo
{
    public:
        GeneratorNodeInfo(QString nodeType);
        static void getNodeTypes(QStringList &nodeTypes);

        bool isValidNode;
        QString niceName;
        QStringList qVariables;
        QStringList cVariables;
        QStringList variableLabels;
        QStringList shortNames;
        QList<int> varIdx;
        QList<int> qVariableIds;
        QList<int> cVariableIds;
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
    QString type;
};

class VariableSpecification
{
public:
    VariableSpecification(QString name = QString(), QString init = QString());
    QString name;
    QString init;
};

class VariableLimitation {
public:
    QString var;        //Name of variable to limit
    QString der;        //Name of derivative of variable
    QString min;         //Minimum value
    QString max;         //Maximum value
    int varEquation;    //Equation where variable is defined
    int derEquation;    //Equation where derivative is defined
};

class CavitationCheck {
public:
    QString pressure;
    QString wave;
    QString impedance;
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
    QStringList portDescriptions;
    QStringList portUnits;
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
    QString transform;

    QString plainCode;
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
    enum InterfaceTypesEnumT {Input, Output, MechanicC, MechanicQ, MechanicRotationalC, MechanicRotationalQ, HydraulicC, HydraulicQ, ElectricC, ElectricQ, PneumaticC, PneumaticQ, PetriNetC, PetriNetQ};

    InterfacePortSpec(InterfaceTypesEnumT type, QString component, QString port, QStringList path);

    InterfaceTypesEnumT type;
    QStringList path;
    QString component;
    QString port;
    double start;

    QList<InterfaceVarSpec> vars;
};

enum ModelVariableCausality {Input, Output};

class ModelVariableSpecification
{
public:
    ModelVariableSpecification(QStringList systemHierarchy, QString componentName, QString portName, QString dataName, int dataId, double startValue, ModelVariableCausality causality);
    QString getName() const;
    QString getCausalityStr() const;
    QStringList systemHierarchy;
    QString componentName;
    QString portName;
    QString dataName;
    int dataId;
    double startValue;
    ModelVariableCausality causality;
};

void getInterfaces(QList<InterfacePortSpec> &interfaces, hopsan::ComponentSystem *pSystem, QStringList &path);
void getModelVariables(hopsan::ComponentSystem *pSystem, QList<ModelVariableSpecification> &vars, QStringList &systemHierarchy);
void getParameters(QList<ParameterSpecification> &parameters, hopsan::ComponentSystem *pSystem);

#endif // GENERATORTYPES_H
