/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

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

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   CoreAccess.h
//!
//! @brief Contains the HopsanCore Qt API classes for communication with the HopsanCore
//!
//$Id$

#ifndef GUIROOTSYSTEM_H
#define GUIROOTSYSTEM_H

#include <QString>
#include <QVector>
#include <QPair>
#include <QStringList>
#include <QMultiMap>
#include <QSharedPointer>

//Forward declarations of HopsanGUI classes
class LibraryWidget;
class SystemContainer;
class CoreSystemAccess;

//Forward declaration of HopsanCore classes
namespace hopsan {
class ComponentSystem;
class Port;
class SimulationHandler;
class CSVParserNG;
}

void initializaHopsanCore(QString logPath);

class CoreCSVParserAccess
{
public:
    CoreCSVParserAccess(QString file, QChar separator=',', int linesToSkip=0);
    ~CoreCSVParserAccess();
    bool isOk();
    int getNumberOfRows();
    int getNumberOfColumns();
    bool getColumn(int col, QVector<double> &rVector);
    bool getRow(int row, QVector<double> &rVector);
private:
    hopsan::CSVParserNG *mpParser;
};


class CoreLibraryAccess
{
public:
    bool hasComponent(const QString &rComponentName);
    bool loadComponentLib(const QString &rFileName);
    bool unLoadComponentLib(const QString &rFileName);
    bool reserveComponentTypeName(const QString &rTypeName);
    void getLoadedLibNames(QVector<QString> &rLibNames);
    void getLibraryContents(QString libPath, QStringList &rComponents, QStringList &rNodes);
    void getLibPathForComponent(QString typeName, QString &rLibPath);
};

class CoreMessagesAccess
{
public:
    unsigned int getNumberOfMessages();
    void getMessage(QString &rMessage, QString &rType, QString &rTag);
};


class CoreQuantityAccess
{
public:
    bool haveQuantity(const QString &rQuantity);
};

double evalWithNumHop(const QString &rExpression);

QStringList getEmbeddedSriptVariableNames(const QString& expression, CoreSystemAccess* pCoreSystem);

void prependSelfToParameterExpresions(CoreSystemAccess* pCoreSystem);


class CoreParameterData
{
public:
    CoreParameterData() {}
    CoreParameterData(const QString name, const QString value, const QString type, const QString quantity="", const QString unit="", const QString desc="")
        : mName(name), mValue(value), mType(type), mQuantity(quantity), mUnit(unit), mDescription(desc) {}

    QString mName;
    QString mAlias;
    QString mValue;
    QString mType;
    QString mQuantity;
    QString mUnit;
    QString mDescription;
    QStringList mConditions;
};

class CoreVariableData
{
public:
    QString mName;
    QString mAlias;
    QString mValue;
    QString mType;
    QString mQuantity;
    QString mUnit;
    QString mDescription;
    QString mNodeDataVariableType;
};

class CoreVariameterDescription
{
public:
    QString mName;
    QString mShortName;
    QString mPortName;
    QString mNodeType;
    QString mAlias;
    QString mDataType;
    QString mUnit;
    QString mDescription;
    QString mQuantity;
    bool mUserModifiableQuantity;
    QString mVariabelType;
    QStringList mConditions;
    int mVariameterType;
    int mVariabelId;
};

//Forward declaration
class CoreSimulationHandler;

class CoreSystemAccess
{
    friend class CoreSimulationHandler;
    friend class CoreGeneratorAccess;
public:
    enum PortTypeIndicatorT {InternalPortType, ActualPortType, ExternalPortType};

    CoreSystemAccess(QString name=QString(), CoreSystemAccess* pParentCoreSystemAccess=0);
    ~CoreSystemAccess();
    void deleteRootSystemPtr(); //!< @todo This is very strange, needed because core systems are deleted from parent if they are subsystems (not if root systems), this is the only way to safely delete the core object
    hopsan::ComponentSystem *getCoreSystemPtr();

    // Name and type functions
    //! @todo maybe we should use name="" (empty) to indicate root system instead, to cut down on the number of functions
    QString getSystemTypeCQS();
    QString getSubComponentTypeCQS(const QString componentName);
    void setExternalModelFilePath(const QString path);

    // Commented by Peter, maybe should be used in the future
    // QString getSubComponentSubTypeName(const QString componentName) const;
    // void setSubComponentSubTypeName(const QString componentName, const QString subTypeName);

    QString setSystemName(QString name);
    QString getSystemName();
    QString renameSubComponent(QString componentName, QString name);

    //Disable components
    void setSubComponentDisabled(QString componentName, bool disabled);
    bool isSubComponentDisabled(QString componentName);

    // Parameters and start values
    bool hasParameter(const QString &rComponentName, const QString &rParameterName);
    QStringList getParameterNames(QString componentName);
    void getParameters(QString componentName, QVector<CoreParameterData> &rParameterDataVec);
    void getParameter(QString componentName, QString parameterName, CoreParameterData &rData);
    QString getParameterValue(QString componentName, QString parameterName);
    //void getStartValueDataNamesValuesAndUnits(QString componentName, QString portName, QVector<QString> &rNames, QVector<QString> &rStartDataValuesTxt, QVector<QString> &rUnits);
    bool setParameterValue(QString componentName, QString parameterName, QString value, bool force=0);

    void getVariameters(QString componentName, QVector<CoreVariameterDescription>& rVariameterDescriptions);

    QStringList getModelAssets() const;

    void runNumHopScript(const QString &rScript, bool printOutput, QString &rOutput);
    void setNumHopScript(const QString &rScript);

    // Alias functions
    bool setVariableAlias(QString compName, QString portName, QString varName, QString alias);
    QString getVariableAlias(QString compName, QString portName, QString varName);
    void setParameterAlias(QString compName, QString paramName, QString alias);
    void getFullVariableNameByAlias(QString alias, QString &rCompName, QString &rPortName, QString &rVarName);
    QStringList getAliasNames() const;

    // Custom Quantities
    bool setModifyableSignalQuantity(QString compPortVar, QString quantity);
    QString getModifyableSignalQuantity(QString compPortVar);

    // Port Functions
    //! @todo maybe need some get allport info function (instead of separate type nodetype description)
    QString getPortType(const QString componentName, const QString portName, const PortTypeIndicatorT portTypeIndicator=ActualPortType);
    QString getNodeType(QString componentName, QString portName);
    QString getPortDescription(QString componentName, QString portName);
    bool isPortConnected(QString componentName, QString portName);
    void setLoggingEnabled(const QString &componentName, const QString &portName, bool enable);
    bool isLoggingEnabled(const QString &componentName, const QString &portName);

    // Component creation and removal
    QString createComponent(QString type, QString name="");
    QString createSubSystem(QString name="");
    QString createConditionalSubSystem(QString name="");
    void removeSubComponent(QString componentName, bool doDelete);
    QString reserveUniqueName(QString desiredName);
    void unReserveUniqueName(QString name);

    // Component connection and disconnection
    bool connect(QString compname1, QString portname1, QString compname2, QString portname2);
    bool disconnect(QString compname1, QString portname1, QString compname2, QString portname2);

    // Simulation functions
    bool isSimulationOk();
    bool initialize(double mStartTime, double mFinishTime, int nSamples=2048);
    void simulate(double mStartTime, double mFinishTime, int nThreads=-1, bool modelHasNotChanged=false);
    void finalize();
    double getCurrentTime() const;
    void stop();
    bool writeNodeData(const QString compname, const QString portname, const QString dataname, double data);

    // System settings
    bool doesKeepStartValues();
    void setKeepValuesAsStartValues(bool load);

    void setDesiredTimeStep(double timestep);
    void setDesiredTimeStep(QString compname, double timestep);
    void setInheritTimeStep(bool inherit);
    void setInheritTimeStep(QString compname, bool inherit);
    bool doesInheritTimeStep();
    bool doesInheritTimeStep(QString compname);

    double getDesiredTimeStep();
    size_t getNumLogSamples();

    // System Port Functions
    void deleteSystemPort(QString portname);
    QString addSystemPort(QString portname);
    QString renameSystemPort(QString oldname, QString newname);

    // System Parameter Functions
    size_t loadParameterFile(QString fileName);
    size_t loadParameterFile(QString componentName, QString fileName);
    QStringList getSystemParameterNames();
    void getSystemParameters(QVector<CoreParameterData> &rParameterDataVec);
    void getSystemParameter(const QString name, CoreParameterData &rParameterData);
    QString getSystemParameterValue(const QString name);
    bool setSystemParameter(const CoreParameterData &rParameter, bool doAdd, bool force=false);
    bool setSystemParameterValue(QString name, QString value, bool force=false);
    bool hasSystemParameter(const QString name);
    bool renameSystemParameter(const QString oldName, const QString newName);
    void removeSystemParameter(const QString name);

    // Simulation results data retrieval
    void getVariableDescriptions(const QString compname, const QString portname, QVector<CoreVariableData> &rVarDescriptions);
    void getPlotDataNamesAndUnits(const QString compname, const QString portname, QVector<QString> &rNames, QVector<QString> &rUnits); //!< @deprecated
    std::vector<double> getTimeVector(QString componentName, QString portName);
    void getPlotData(const QString compname, const QString portname, const QString dataname, std::vector<double> *&rpTimeVector, QVector<double> &rData);
    std::vector<double> *getLogTimeData() const;
    bool havePlotData(const QString compname, const QString portname, const QString dataname);
    bool getLastNodeData(const QString compname, const QString portname, const QString dataname, double& rData) const;
    double *getNodeDataPtr(const QString compname, const QString portname, const QString dataname);

    //Time measurements
    void measureSimulationTime(QStringList &rComponentNames, QList<double> &rTimes, int nSteps=5);

    // Search path
    void addSearchPath(QString searchPath);
    QStringList getSearchPaths() const;

    // Save and loaf state
    void saveSimulationState(const QString &filePath);
    void loadSimulationState(const QString &filePath, double &rTimeOffset);

private:

    hopsan::ComponentSystem *getCoreSubSystemPtr(QString name);
    hopsan::Port* getCorePortPtr(QString componentName, QString portName) const;

    hopsan::ComponentSystem *mpCoreComponentSystem;
};


// Version functions
QString getHopsanCoreVersion();
QString getHopsanCoreCompiler();
QString getHopsanCoreArchitecture();
QString getHopsanCoreBuildTime();

class CoreSimulationHandler
{
public:
    //! @todo a doitall function
    bool initialize(const double startTime, const double stopTime, const double logStartTime, const int nLogSamples, CoreSystemAccess* pCoreSystemAccess);
    bool initialize(const double startTime, const double stopTime, const double logStartTime, const int nLogSamples, QVector<CoreSystemAccess*> &rvCoreSystemAccess);

    bool simulate(const double startTime, const double stopTime, const int nThreads, CoreSystemAccess* pCoreSystemAccess, bool modelHasNotChanged=false);
    bool simulate(const double startTime, const double stopTime, const int nThreads, QVector<CoreSystemAccess*> &rvCoreSystemAccess, bool modelHasNotChanged=false);

    void finalize(CoreSystemAccess* pCoreSystemAccess);
    void finalize(QVector<CoreSystemAccess*> &rvCoreSystemAccess);

private:


};

class NodeInfo
{
    public:
        NodeInfo(QString nodeType);
        static void getNodeTypes(QStringList &nodeTypes);

        QString niceName;
        QStringList qVariables;
        QStringList cVariables;
        QStringList variableLabels;
        QStringList shortNames;
        QList<size_t> varIdx;
        QString intensity;
        QString flow;
};


#endif // GUIROOTSYSTEM_H
