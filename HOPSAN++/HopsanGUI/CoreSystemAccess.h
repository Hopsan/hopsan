#ifndef GUIROOTSYSTEM_H
#define GUIROOTSYSTEM_H

#include "HopsanCore.h"
#include "common.h"

#include <QString>
#include <qdebug.h>

class CoreSystemAccess
{
public:
    CoreSystemAccess(QString name=QString(), CoreSystemAccess* pParentCoreSystemAccess=0);
    ~CoreSystemAccess();
    hopsan::ComponentSystem *getCoreSubSystemPtr(QString name);
    void deleteRootSystemPtr(); //!< @todo This is completely stuipid, temporary extreamly ugly hack, need to solve this (the subsystem core ptrs) in a better way

    bool connect(QString compname1, QString portname1, QString compname2, QString portname2);
    bool disconnect(QString compname1, QString portname1, QString compname2, QString portname2);

    void setDesiredTimeStep(double timestep);
    double getDesiredTimeStep();

    void setRootTypeCQS(const QString cqs_type);
    void setSubSystemTypeCQS(const QString systemName, const QString cqs_type);
    QString getRootSystemTypeCQS();
    QString getSubComponentTypeCQS(QString componentName);
    //QString getSystemTypeCQS(QString systemName); //!< @todo dont think that we need this one the component specifik one should do

    QString setRootSystemName(QString name);
    QString getRootSystemName();
    //QString setSystemName(QString systemname, QString name, bool doOnlyLocalRename=false); //!< @todo This might not be necessary, should be able to use the component base class specifik one
    QString renameSubComponent(QString componentName, QString name);

    QString getPortType(QString componentName, QString portName);
    QString getNodeType(QString componentName, QString portName);

    void getStartValueDataNamesValuesAndUnits(QString componentName, QString portName, QVector<QString> &rNames, QVector<double> &rStartDataValues, QVector<QString> &rUnits);
    void setStartValueDataByNames(QString componentName, QString portName, QVector<QString> names, QVector<double> startDataValues);

    QVector<QString> getParameterNames(QString componentName);
    QString getParameterUnit(QString componentName, QString parameterName);
    QString getParameterDescription(QString componentName, QString parameterName);
    double getParameterValue(QString componentName, QString parameterName);
    void setParameter(QString componentName, QString parameterName, double value); //!< @todo maybe call this set parameter value

    QString createComponent(QString type, QString name="");
    QString createSubSystem(QString name="");
    void removeSubComponent(QString componentName, bool doDelete);

    void loadSystemFromFileCoreOnly(QString sysname, QString filepath);

    bool isSimulationOk();
    void initialize(double mStartTime, double mFinishTime, size_t nSamples=2048);
    void simulate(double mStartTime, double mFinishTime, simulationMethod type);
    void simulate(double mStartTime, double mFinishTime);
    void finalize(double mStartTime, double mFinishTime);
    double getCurrentTime();
    void stop();

    void deleteSystemPort(QString portname);
    QString addSystemPort(QString portname);
    QString renameSystemPort(QString oldname, QString newname);

    std::vector<double> getTimeVector(QString componentName, QString portName);
    void getPlotDataNamesAndUnits(const QString compname, const QString portname, QVector<QString> &rNames, QVector<QString> &rUnits);
    QString getPlotDataUnit(const QString compname, const QString portname, const QString dataname);
    //void getPlotDataUnit(const QString compname, const QString portname, const string dataname, QString &rUnit);
    //QVector<QString> getPlotDataUnits();
    void getPlotData(const QString compname, const QString portname, const QString dataname, QVector<double> &rData);
    bool getLastNodeData(const QString compname, const QString portname, const QString dataname, double& rData);
    bool isPortConnected(QString componentName, QString portName);

private:
    hopsan::Port* getPortPtr(QString componentName, QString portName);

    //*****Core Interaction*****
    hopsan::ComponentSystem *mpCoreComponentSystem;
    //**************************
};

class CoreAccess
{
public:

};

#endif // GUIROOTSYSTEM_H
