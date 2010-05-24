#ifndef GUIROOTSYSTEM_H
#define GUIROOTSYSTEM_H

#include "HopsanCore.h"
#include <QString>

class GUIRootSystem
{
public:
    GUIRootSystem();
    bool connect(QString compname1, QString portname1, QString compname2, QString portname2);
    bool disconnect(QString compname1, QString portname1, QString compname2, QString portname2);
    void setDesiredTimeStep(double timestep);
    double getDesiredTimeStep();
    void setRootTypeCQS(const string cqs_type, bool doOnlyLocalSet=false);
    void setSystemTypeCQS(QString systemName, const string cqs_type, bool doOnlyLocalSet=false);
    QString getSystemTypeCQS(QString systemName);
    QString getTypeCQS(QString componentName);
    void setRootSystemName(string name, bool doOnlyLocalRename=false);
    QString setSystemName(string systemname, string name, bool doOnlyLocalRename=false);
    QString setName(string componentName, string name, bool doOnlyLocalRename=false);
    QString getName();
    double getCurrentTime();
    void stop();
    QString getPortType(QString componentName, QString portName);
    QString getNodeType(QString componentName, QString portName);
    void setParameter(QString componentName, QString parameterName, double value);
    void removeSubComponent(QString componentName, bool doDelete);
    void removeSystem(QString name);
    vector<double> getTimeVector(QString componentName, QString portName);
    void initialize(double mStartTime, double mFinishTime);
    void simulate(double mStartTime, double mFinishTime);
    void finalize(double mStartTime, double mFinishTime);
    QString createComponent(QString type);
    QString createSubSystem();

//private:
    //*****Core Interaction*****
    ComponentSystem *mpCoreComponentSystem;
    //**************************
};

#endif // GUIROOTSYSTEM_H
