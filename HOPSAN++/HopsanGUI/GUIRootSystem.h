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
    void setTypeCQS(const string cqs_type, bool doOnlyLocalSet=false);
    QString getTypeCQS();
    void setSystemName(string name, bool doOnlyLocalRename=false);
    void setName(string componentName, string name, bool doOnlyLocalRename=false);
    string getName();
    double getCurrentTime();
    void stop();
    QString getPortType(QString componentName, QString portName);
    QString getNodeType(QString componentName, QString portName);
    void setParameter(QString componentName, QString parameterName, double value);
    void removeSubComponent(QString componentName, bool doDelete);
    void removeSystem();

//private:
    //*****Core Interaction*****
    ComponentSystem *mpCoreComponentSystem;
    //**************************
};

#endif // GUIROOTSYSTEM_H
