#include "GUIRootSystem.h"
#include <QString>

GUIRootSystem::GUIRootSystem()
{
    //Create new Core system component
    mpCoreComponentSystem = HopsanEssentials::getInstance()->CreateComponentSystem();
}

bool GUIRootSystem::connect(QString compname1, QString portname1, QString compname2, QString portname2)
{
    //*****Core Interaction*****
    return mpCoreComponentSystem->connect(compname1.toStdString(), portname1.toStdString(), compname2.toStdString(), portname2.toStdString());
    //**************************
}

bool GUIRootSystem::disconnect(QString compname1, QString portname1, QString compname2, QString portname2)
{
    //*****Core Interaction*****
    return mpCoreComponentSystem->disconnect(compname1.toStdString(), portname1.toStdString(), compname2.toStdString(), portname2.toStdString());
    //**************************
}

void GUIRootSystem::setDesiredTimeStep(double timestep)
{
    mpCoreComponentSystem->setDesiredTimestep(timestep);
}

double GUIRootSystem::getDesiredTimeStep()
{
    return mpCoreComponentSystem->getDesiredTimeStep();
}

void GUIRootSystem::setRootTypeCQS(const string cqs_type, bool doOnlyLocalSet)
{
    mpCoreComponentSystem->setTypeCQS(cqs_type, doOnlyLocalSet);
}

void GUIRootSystem::setSystemTypeCQS(QString systemName, const string cqs_type, bool doOnlyLocalSet)
{
    mpCoreComponentSystem->getSubComponentSystem(systemName.toStdString())->setTypeCQS(cqs_type, doOnlyLocalSet);
}

QString GUIRootSystem::getSystemTypeCQS(QString systemName)
{
    return QString::fromStdString(mpCoreComponentSystem->getSubComponentSystem(systemName.toStdString())->getTypeCQSString());
}

QString GUIRootSystem::getTypeCQS(QString componentName)
{
    return QString::fromStdString(mpCoreComponentSystem->getSubComponent(componentName.toStdString())->getTypeCQSString());
}

void GUIRootSystem::setRootSystemName(string name, bool doOnlyLocalRename)
{
    mpCoreComponentSystem->setName(name, doOnlyLocalRename);
}

QString GUIRootSystem::setSystemName(string systemname, string name, bool doOnlyLocalRename)
{
    ComponentSystem *pTempComponentSystem = mpCoreComponentSystem->getSubComponentSystem(systemname);
    pTempComponentSystem->setName(name, doOnlyLocalRename);
    return QString::fromStdString(pTempComponentSystem->getName());
}

QString GUIRootSystem::setName(string componentName, string name, bool doOnlyLocalRename)
{
    Component *pTempComponent = mpCoreComponentSystem->getSubComponent(componentName);
    pTempComponent->setName(name, doOnlyLocalRename);
    return QString::fromStdString(pTempComponent->getName());
}

QString GUIRootSystem::getName()
{
    return QString::fromStdString(mpCoreComponentSystem->getName());
}

double GUIRootSystem::getCurrentTime()
{
    return *(mpCoreComponentSystem->getTimePtr());
}

void GUIRootSystem::stop()
{
    mpCoreComponentSystem->stop();
}

QString GUIRootSystem::getPortType(QString componentName, QString portName)
{
    return QString(mpCoreComponentSystem->getSubComponent(componentName.toStdString())->getPort(portName.toStdString())->getPortTypeString().c_str());
}

QString GUIRootSystem::getNodeType(QString componentName, QString portName)
{
    return QString(mpCoreComponentSystem->getSubComponent(componentName.toStdString())->getPort(portName.toStdString())->getNodeType().c_str());
}

void GUIRootSystem::setParameter(QString componentName, QString parameterName, double value)
{
    mpCoreComponentSystem->getSubComponent(componentName.toStdString())->setParameterValue(parameterName.toStdString(), value);
}

void GUIRootSystem::removeSubComponent(QString componentName, bool doDelete)
{
    mpCoreComponentSystem->removeSubComponent(mpCoreComponentSystem->getSubComponent(componentName.toStdString()), doDelete);
}

void GUIRootSystem::removeSystem(QString name)
{
    mpCoreComponentSystem->removeSubComponent(mpCoreComponentSystem->getSubComponentSystem(name.toStdString()), true);
}


vector<double> GUIRootSystem::getTimeVector(QString componentName, QString portName)
{
    return *(mpCoreComponentSystem->getSubComponent(componentName.toStdString())->getPort(portName.toStdString())->getTimeVectorPtr());
}


void GUIRootSystem::initialize(double mStartTime, double mFinishTime)
{
    mpCoreComponentSystem->initialize(mStartTime, mFinishTime);
}


void GUIRootSystem::simulate(double mStartTime, double mFinishTime)
{
    mpCoreComponentSystem->simulate(mStartTime, mFinishTime);
}


void GUIRootSystem::finalize(double mStartTime, double mFinishTime)
{
    mpCoreComponentSystem->finalize(mStartTime, mFinishTime);
}

QString GUIRootSystem::createComponent(QString type)
{
    Component *pCoreComponent = HopsanEssentials::getInstance()->CreateComponent(type.toStdString());
    mpCoreComponentSystem->addComponent(pCoreComponent);
    return QString::fromStdString(pCoreComponent->getName());
}

QString GUIRootSystem::createSubSystem()
{
    ComponentSystem *pTempComponentSystem = HopsanEssentials::getInstance()->CreateComponentSystem();
    mpCoreComponentSystem->addComponent(pTempComponentSystem);
    return QString::fromStdString(pTempComponentSystem->getName());

}
