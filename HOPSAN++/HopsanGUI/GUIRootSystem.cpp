#include "GUIRootSystem.h"

GUIRootSystem::GUIRootSystem()
{

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

void GUIRootSystem::setTypeCQS(const string cqs_type, bool doOnlyLocalSet)
{
    mpCoreComponentSystem->setTypeCQS(cqs_type, doOnlyLocalSet);
}

double GUIRootSystem::getDesiredTimeStep()
{
    return mpCoreComponentSystem->getDesiredTimeStep();
}

void GUIRootSystem::setName(string name, bool doOnlyLocalRename)
{
    mpCoreComponentSystem->setName(name, doOnlyLocalRename);
}

string GUIRootSystem::getName()
{
    return mpCoreComponentSystem->getName();
}

double GUIRootSystem::getCurrentTime()
{
    return *(mpCoreComponentSystem->getTimePtr());
}

void GUIRootSystem::stop()
{
    mpCoreComponentSystem->stop();
}
