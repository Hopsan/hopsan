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
//! @file   CoreAccess.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the CoreAccess class, The API class for communication with the HopsanCore
//!
//$Id$

#include "CoreAccess.h"
#include "MainWindow.h"
#include "Widgets/MessageWidget.h"
#include <QString>
#include <QVector>

//HopsanCore includes
#include "HopsanCore.h"
#include "ComponentSystem.h"

using namespace std;
using namespace hopsan;


bool CoreLibraryAccess::hasComponent(QString componentName)
{
    return HopsanEssentials::getInstance()->hasComponent(componentName.toStdString());
}


bool CoreLibraryAccess::loadComponent(QString fileName)
{
    return HopsanEssentials::getInstance()->loadExternalComponent(fileName.toStdString());
}


size_t CoreMessagesAccess::getNumberOfMessages()
{
    return HopsanEssentials::getInstance()->checkMessage();
}

void CoreMessagesAccess::getMessage(QString &rMessage, QString &rType, QString &rTag)
{
    std::string msg, tag, type;
    HopsanEssentials::getInstance()->getMessage(msg, type, tag);
    rMessage = QString::fromStdString(msg);
    rTag = QString::fromStdString(tag);
    rType = QString::fromStdString(type);
}


CoreSystemAccess::CoreSystemAccess(QString name, CoreSystemAccess* pParentCoreSystemAccess)
{
    //Create new Core system component
    if (pParentCoreSystemAccess == 0)
    {
        //Create new root system
        mpCoreComponentSystem = HopsanEssentials::getInstance()->CreateComponentSystem();
    }
    else
    {
        //Creating a subsystem, setting internal pointer
        mpCoreComponentSystem = pParentCoreSystemAccess->getCoreSubSystemPtr(name);
    }
}

ComponentSystem* CoreSystemAccess::getCoreSubSystemPtr(QString name)
{
    qDebug() << " corecomponentsystemname: " <<  QString::fromStdString(mpCoreComponentSystem->getName()) << "  Subname: " << name;
    return mpCoreComponentSystem->getSubComponentSystem(name.toStdString());
}

CoreSystemAccess::~CoreSystemAccess()
{
    //Dont remove the mpCoreComponentSystem here you must do that manually until we have found a samrter way to do all of this
    //see deleteRootSystemPtr()
    //delete mpCoreComponentSystem;
}

//! @todo This is very strange, needed becouse core systems are deleted from parent if they are subsystems (not if root systems), this is the only way to safely delete the ore object
void CoreSystemAccess::deleteRootSystemPtr()
{
    delete mpCoreComponentSystem;
}

bool CoreSystemAccess::connect(QString compname1, QString portname1, QString compname2, QString portname2)
{
    //*****Core Interaction*****
    return mpCoreComponentSystem->connect(compname1.toStdString(), portname1.toStdString(), compname2.toStdString(), portname2.toStdString());
    //**************************
}

bool CoreSystemAccess::disconnect(QString compname1, QString portname1, QString compname2, QString portname2)
{
    //*****Core Interaction*****
    return mpCoreComponentSystem->disconnect(compname1.toStdString(), portname1.toStdString(), compname2.toStdString(), portname2.toStdString());
    //**************************
}

void CoreSystemAccess::setDesiredTimeStep(double timestep)
{
    mpCoreComponentSystem->setDesiredTimestep(timestep);
}


void CoreSystemAccess::setInheritTimeStep(bool inherit)
{
    mpCoreComponentSystem->setInheritTimestep(inherit);
}


bool CoreSystemAccess::doesInheritTimeStep()
{
    return mpCoreComponentSystem->doesInheritTimestep();
}


double CoreSystemAccess::getDesiredTimeStep()
{
    return mpCoreComponentSystem->getDesiredTimeStep();
}


QString CoreSystemAccess::getRootSystemTypeCQS()
{
    //qDebug() << "getRootTypeCQS: " << componentName;
    return QString::fromStdString(mpCoreComponentSystem->getTypeCQSString());
}

QString CoreSystemAccess::getSubComponentTypeCQS(QString componentName)
{
    //qDebug() << "getSubComponentTypeCQS: " << componentName << " in " << QString::fromStdString(mpCoreComponentSystem->getName());
    QString ans = QString::fromStdString(mpCoreComponentSystem->getSubComponent(componentName.toStdString())->getTypeCQSString());
    //qDebug() << "cqs answer: " << ans;
    return ans;
}


QString CoreSystemAccess::setRootSystemName(QString name)
{
    //qDebug() << "setting root system name to: " << name;
    mpCoreComponentSystem->setName(name.toStdString());
    //qDebug() << "root system name after rename: " << QString::fromStdString(mpCoreComponentSystem->getName());
    return QString::fromStdString(mpCoreComponentSystem->getName());
}


QString CoreSystemAccess::renameSubComponent(QString componentName, QString name)
{
    qDebug() << "rename subcomponent from " << componentName << " to: " << name;
    Component *pTempComponent = mpCoreComponentSystem->getSubComponent(componentName.toStdString());
    pTempComponent->setName(name.toStdString());
    qDebug() << "name after: " << QString::fromStdString(pTempComponent->getName());
    return QString::fromStdString(pTempComponent->getName());
}

QString CoreSystemAccess::getRootSystemName()
{
   // qDebug() << "getNAme from core root: " << QString::fromStdString(mpCoreComponentSystem->getName());
    return QString::fromStdString(mpCoreComponentSystem->getName());
}

double CoreSystemAccess::getCurrentTime()
{
    return *(mpCoreComponentSystem->getTimePtr());
}

void CoreSystemAccess::stop()
{
    mpCoreComponentSystem->stop();
}

QString CoreSystemAccess::getPortType(const QString componentName, const QString portName, const PortTypeIndicatorT portTypeIndicator)
{
    //qDebug() << "name for port fetch " << componentName << " " << portName;

    Port *pPort = this->getPortPtr(componentName, portName);
    if(pPort)
    {
        switch (portTypeIndicator)
        {
        case INTERNALPORTTYPE:
            return QString::fromStdString( portTypeToString(pPort->getInternalPortType()) );
            break;
        case ACTUALPORTTYPE:
            return QString::fromStdString( portTypeToString(pPort->getPortType()) );
            break;
        case EXTERNALPORTTYPE:
            return QString::fromStdString( portTypeToString(pPort->getExternalPortType()) );
            break;
        default:
            return QString("Invalid  portTypeIndicator specified");
        }
    }
    else
    {
        qDebug() <<  "======== ERROR ========= Could not find Port in getPortType: " << componentName << " " << portName << " in: " << this->getRootSystemName();
        return QString(); //Empty
    }
}

QString CoreSystemAccess::getNodeType(QString componentName, QString portName)
{
    Port *pPort = this->getPortPtr(componentName, portName);
    if(pPort)
    {
        return QString(pPort->getNodeType().c_str());
    }
    else
    {
        qDebug() <<  "======================================== EMPTY nodetype: " << componentName << " " << portName << " in: " << this->getRootSystemName();
        return QString(); //Empty
    }
}


void CoreSystemAccess::getStartValueDataNamesValuesAndUnits(QString componentName, QString portName, QVector<QString> &rNames, QVector<double> &rValues, QVector<QString> &rUnits)
{
    std::vector<std::string> stdNames, stdUnits;
    std::vector<double> stdValues;
    Port *pPort = this->getPortPtr(componentName, portName);
    if(pPort)
    {
        pPort->getStartValueDataNamesValuesAndUnits(stdNames, stdValues, stdUnits);
    }
    rNames.resize(stdNames.size());
    rValues.resize(stdValues.size());
    rUnits.resize(stdUnits.size());
    for(size_t i=0; i < stdNames.size(); ++i) //! @todo Make a nicer conversion fron std::vector<std::string> --> QVector<QString>
    {
        rNames[i] = QString::fromStdString(stdNames[i]);
        rValues[i] = stdValues[i];
        rUnits[i] = QString::fromStdString(stdUnits[i]);
    }
}


void CoreSystemAccess::getStartValueDataNamesValuesAndUnits(QString componentName, QString portName, QVector<QString> &rNames, QVector<QString> &rValuesTxt, QVector<QString> &rUnits)
{
    std::vector<std::string> stdNames, stdUnits;
    std::vector<std::string> stdValuesTxt;
    Port *pPort = this->getPortPtr(componentName, portName);
    if(pPort)
    {
        pPort->getStartValueDataNamesValuesAndUnits(stdNames, stdValuesTxt, stdUnits);
    }
    rNames.resize(stdNames.size());
    rValuesTxt.resize(stdValuesTxt.size());
    rUnits.resize(stdUnits.size());
    for(size_t i=0; i < stdNames.size(); ++i) //! @todo Make a nicer conversion fron std::vector<std::string> --> QVector<QString>
    {
        rNames[i] = QString::fromStdString(stdNames[i]);
        rValuesTxt[i] = QString::fromStdString(stdValuesTxt[i]);
        rUnits[i] = QString::fromStdString(stdUnits[i]);
    }
}


bool CoreSystemAccess::setStartValueDataByNames(QString componentName, QString portName, QVector<QString> names, QVector<double> values)
{
    bool success = true;
    std::vector<std::string> stdNames;
    std::vector<double> stdValues;
    stdNames.resize(names.size());
    stdValues.resize(values.size());
    for(int i=0; i < names.size(); ++i) //! @todo Make a nicer conversion fron std::vector<std::string> --> QVector<QString>
    {
        stdNames[i] = names[i].toStdString();
    }
    stdValues = values.toStdVector();

    Port *pPort = this->getPortPtr(componentName, portName);
    if(pPort)
    {
        success = pPort->setStartValueDataByNames(stdNames, stdValues);
    }
    return success;
}


bool CoreSystemAccess::setStartValueDataByNames(QString componentName, QString portName, QVector<QString> names, QVector<QString> valuesTxt)
{
    bool success = false;
    //Used for plain values
    QVector<QString> startDataNamesStr;
    QVector<double> startDataValuesDbl;
    //Used for mapped to system parameters start values
    QVector<QString> startDataNamesStrSysPar;
    QVector<QString> startDataValuesTxtSysPar;

    for(int i=0; i < names.size(); ++i)
    {
        bool isDbl;
        //Check if the start value is convertible to a double, if so assume that it is just a plain value that should be used,
        //if not assume that it should be mapped to a System parameter
        double valueDbl = valuesTxt[i].toDouble(&isDbl);
        if(isDbl)
        {
            //Save the start values that should be set to just plain values (e.g. 17.0) into tmp vectors
            startDataNamesStr.append(names[i]);
            startDataValuesDbl.append(valueDbl);
        }
        else
        {
            //Save the System parameter name (e.g. "myparameter") that should be mapped with the start value
            startDataNamesStrSysPar.append(names[i]);
            startDataValuesTxtSysPar.append(valuesTxt[i]);
        }
    }
    //Set this plain start values to the ports
    success = this->setStartValueDataByNames(componentName, portName, startDataNamesStr, startDataValuesDbl);

    //Set the System parameters to the ports
    std::vector<std::string> stdNames, stdValuesTxt;
    stdNames.resize(startDataNamesStrSysPar.size());
    stdValuesTxt.resize(startDataValuesTxtSysPar.size());
    for(int i=0; i < startDataNamesStrSysPar.size(); ++i) //! @todo Make a nicer conversion fron std::vector<std::string> --> QVector<QString>
    {
        stdNames[i] = startDataNamesStrSysPar[i].toStdString();
        stdValuesTxt[i] = startDataValuesTxtSysPar[i].toStdString();
    }

    Port *pPort = this->getPortPtr(componentName, portName);
    if(pPort)
    {
        success *= pPort->setStartValueDataByNames(stdNames, stdValuesTxt);
    }
    return success;
}


bool CoreSystemAccess::setParameter(QString componentName, QString parameterName, QString value, bool force)
{
    return mpCoreComponentSystem->getSubComponent(componentName.toStdString())->setParameterValue(parameterName.toStdString(), value.toStdString(), force);
}


void CoreSystemAccess::removeSubComponent(QString componentName, bool doDelete)
{
    mpCoreComponentSystem->removeSubComponent(componentName.toStdString(), doDelete);
}


vector<double> CoreSystemAccess::getTimeVector(QString componentName, QString portName)
{
    //qDebug() << "getTimeVector, " << componentName << ", " << portName;

    Component* pComp = mpCoreComponentSystem->getComponent(componentName.toStdString());
    Port* pPort = 0;
    if (pComp != 0)
    {
        pPort = pComp->getPort(portName.toStdString());
    }

    if (pPort != 0)
    {
        vector<double> *ptr = pPort->getTimeVectorPtr();
        if (ptr != 0)
        {
            return *ptr; //Return a copy of the vector
        }
    }

    //Return empty dummy
    vector<double> dummy;
    return dummy;
}


bool CoreSystemAccess::isSimulationOk()
{
    return mpCoreComponentSystem->isSimulationOk();
}


void CoreSystemAccess::initialize(double mStartTime, double mFinishTime, size_t nSamples)
{
    mpCoreComponentSystem->initialize(mStartTime, mFinishTime, nSamples);
}


void CoreSystemAccess::simulate(double mStartTime, double mFinishTime, simulationMethod type, size_t nThreads)
{
    if(type == MULTICORE)
    {
        qDebug() << "Starting multicore simulation";
        mpCoreComponentSystem->simulateMultiThreaded(mStartTime, mFinishTime, nThreads);
        //mpCoreComponentSystem->simulateMultiThreadedOld(mStartTime, mFinishTime);
    }
    else
    {
        qDebug() << "Starting singlecore simulation";
        mpCoreComponentSystem->simulate(mStartTime, mFinishTime);
    }
}


void CoreSystemAccess::finalize(double mStartTime, double mFinishTime)
{
    mpCoreComponentSystem->finalize(mStartTime, mFinishTime);
}

QString CoreSystemAccess::createComponent(QString type, QString name)
{
    //qDebug() << "createComponent: " << "type: " << type << " desired name:  " << name << " in system: " << this->getRootSystemName();
    Component *pCoreComponent = HopsanEssentials::getInstance()->CreateComponent(type.toStdString());
    if (pCoreComponent != 0)
    {
        mpCoreComponentSystem->addComponent(pCoreComponent);
        if (!name.isEmpty())
        {
            pCoreComponent->setName(name.toStdString());
        }
        //qDebug() << "createComponent: name after add: " << QString::fromStdString(pCoreComponent->getName()) << " added to: " << QString::fromStdString(mpCoreComponentSystem->getName());
        return QString::fromStdString(pCoreComponent->getName());
    }
    else
    {
        qDebug() << "failed to create component of type: " << type << " maybe it is not registered in the core";
        return QString();
    }
}

QString CoreSystemAccess::createSubSystem(QString name)
{
    ComponentSystem *pTempComponentSystem = HopsanEssentials::getInstance()->CreateComponentSystem();
    mpCoreComponentSystem->addComponent(pTempComponentSystem);
    if (!name.isEmpty())
    {
        pTempComponentSystem->setName(name.toStdString());
    }
    return QString::fromStdString(pTempComponentSystem->getName());
}

void CoreSystemAccess::getParameters(QString componentName, QVector<QString> &qParameterNames, QVector<QString> &qParameterValues, QVector<QString> &qDescriptions, QVector<QString> &qUnits, QVector<QString> &qTypes)
{
    std::vector<std::string> parameterNames, parameterValues, descriptions, units, types;
    mpCoreComponentSystem->getSubComponent(componentName.toStdString())->getParameters(parameterNames, parameterValues, descriptions, units, types);
    for(size_t i=0; i<parameterNames.size(); ++i)
    {
        qParameterNames.push_back(QString::fromStdString(parameterNames[i]));
        qParameterValues.push_back(QString::fromStdString(parameterValues[i]));
        qDescriptions.push_back(QString::fromStdString(descriptions[i]));
        qUnits.push_back(QString::fromStdString(units[i]));
        qTypes.push_back(QString::fromStdString(types[i]));
    }
}

QVector<QString> CoreSystemAccess::getParameterNames(QString componentName)
{
    std::vector<std::string> parameterNames, parameterValues, descriptions, units, types;
    mpCoreComponentSystem->getSubComponent(componentName.toStdString())->getParameters(parameterNames, parameterValues, descriptions, units, types);
    QVector<QString> qParameterNames, qParameterValues, qDescriptions, qUnits;
    for(size_t i=0; i<parameterNames.size(); ++i)
    {
        qParameterNames.push_back(QString::fromStdString(parameterNames[i]));
        qParameterValues.push_back(QString::fromStdString(parameterValues[i]));
        qDescriptions.push_back(QString::fromStdString(descriptions[i]));
        qUnits.push_back(QString::fromStdString(units[i]));
    }
    return qParameterNames;
}

QString CoreSystemAccess::getParameterUnit(QString componentName, QString parameterName)
{
    return QString("");
}

QString CoreSystemAccess::getParameterDescription(QString componentName, QString parameterName)
{
    return QString("");
}

QString CoreSystemAccess::getParameterValue(QString componentName, QString parameterName)
{
    QString parameterValue = "";
    std::vector<std::string> parameterNames, parameterValues, descriptions, units, types;
    mpCoreComponentSystem->getSubComponent(componentName.toStdString())->getParameters(parameterNames, parameterValues, descriptions, units, types);
    for(size_t i=0; i<parameterNames.size(); ++i)
    {
        if(parameterNames[i] == parameterName.toStdString())
        {
            parameterValue = QString::fromStdString(parameterValues[i]);
        }
    }
    return parameterValue;
}

void CoreSystemAccess::deleteSystemPort(QString portname)
{
    mpCoreComponentSystem->deleteSystemPort(portname.toStdString());
}

QString CoreSystemAccess::addSystemPort(QString portname)
{
    //qDebug() << "add system port: " << portname;
    return QString::fromStdString(mpCoreComponentSystem->addSystemPort(portname.toStdString())->getPortName());
}

QString CoreSystemAccess::renameSystemPort(QString oldname, QString newname)
{
    return QString::fromStdString(mpCoreComponentSystem->renameSystemPort(oldname.toStdString(), newname.toStdString()));
}

QString CoreSystemAccess::reserveUniqueName(QString desiredName)
{
    return QString::fromStdString(mpCoreComponentSystem->reserveUniqueName(desiredName.toStdString()));
}

void CoreSystemAccess::unReserveUniqueName(QString name)
{
    mpCoreComponentSystem->unReserveUniqueName(name.toStdString());
}

QString CoreSystemAccess::getPlotDataUnit(const QString compname, const QString portname, const QString dataname)
{
    std::string dummy, unit;
    Port* pPort = this->getPortPtr(compname, portname);
    if(pPort)
    {
        int idx = pPort->getNodeDataIdFromName(dataname.toStdString());
        if(idx >= 0)
            pPort->getNodeDataNameAndUnit(idx,dummy,unit);
    }
    return QString::fromStdString(unit);
}

//! @todo how to handle fetching from systemports, component names will not be found
void CoreSystemAccess::getPlotDataNamesAndUnits(const QString compname, const QString portname, QVector<QString> &rNames, QVector<QString> &rUnits)
{
    vector<string> corenames, coreunits;
    rNames.clear();
    rUnits.clear();

    Port* pPort = this->getPortPtr(compname, portname);
    if (pPort && pPort->getPortType() < MULTIPORT)
    {
        pPort->getNodeDataNamesAndUnits(corenames, coreunits);
        //Copy into QT datatype vector (assumes bothe received vectors same length (should always be same)
        for (size_t i=0; i<corenames.size(); ++i)
        {
            rNames.push_back(QString::fromStdString(corenames[i]));
            rUnits.push_back(QString::fromStdString(coreunits[i]));
        }
    }
}

void CoreSystemAccess::getPlotData(const QString compname, const QString portname, const QString dataname, QPair<QVector<double>, QVector<double> > &rData)
{
    int dataId = -1;
    Port* pPort = this->getPortPtr(compname, portname);
    if (pPort)
        if(pPort->isConnected())
        {
        dataId = pPort->getNodeDataIdFromName(dataname.toStdString());

        if (dataId >= 0)
        {
            vector< vector<double> > *pData = pPort->getDataVectorPtr();
            vector<double> *pTime = pPort->getTimeVectorPtr();

            //Ok lets copy all of the data to a Qt vector
            rData.first.clear();
            rData.first.resize(pTime->size());    //Allocate memory for time
            rData.second.clear();           //Allocate memory for data
            rData.second.resize(pData->size());
            for (size_t i=0; i<pData->size() && i<pTime->size(); ++i)
            {
                rData.first[i] = pTime->at(i);
                rData.second[i] = pData->at(i).at(dataId);
            }
        }
    }
}


bool CoreSystemAccess::getLastNodeData(const QString compname, const QString portname, const QString dataname, double& rData)
{
    int dataId = -1;
    Port* pPort = this->getPortPtr(compname, portname);
    if (pPort)
    {
        dataId = pPort->getNodeDataIdFromName(dataname.toStdString());

        if (dataId >= 0)
        {
            vector< vector<double> > *pData = pPort->getDataVectorPtr();
            rData = pData->back().at(dataId);
            return 1;
        }
    }
    return 0;
}


bool CoreSystemAccess::isPortConnected(QString componentName, QString portName)
{
    Port* pPort = this->getPortPtr(componentName, portName);
    if(pPort)
    {
        return pPort->isConnected();
    }
    else
    {
        return false;
    }
}

//! @brief Helpfunction that tries to fetch a port pointer
//! @param [in] componentName The name of the component to which the port belongs
//! @param [in] portName The name of the port
//! @returns A pointer to the port or a 0 ptr if component or port not found
hopsan::Port* CoreSystemAccess::getPortPtr(QString componentName, QString portName)
{
    //We must use getcomponent here if we want to be able to find root system ptr
    //! @todo see if we can reduce the number f public get functions one, the one which only searches subcomponents make function in core to solve the other access type like bellow
    Component* pComp = mpCoreComponentSystem->getComponent(componentName.toStdString());
    if (pComp)
    {
        return pComp->getPort(portName.toStdString());
    }
    return 0;
}



bool CoreSystemAccess::setSystemParameter(QString name, QString value, QString description, QString unit, QString type)
{
    bool success = true;
//    //Makes sure name is not a number and not empty
//    bool isDbl;
//    name.toDouble(&isDbl);
//    if(isDbl || name.isEmpty())
//    {
//        success *= false;
//    }
//    else
    {
        if(!(success *= mpCoreComponentSystem->getSystemParameters().setParameter(name.toStdString(), value.toStdString(), description.toStdString(), unit.toStdString(), type.toStdString())))
        {
            success += mpCoreComponentSystem->getSystemParameters().addParameter(name.toStdString(), value.toStdString(), description.toStdString(), unit.toStdString(), type.toStdString());
        }
    }
    return success;
}


QString CoreSystemAccess::getSystemParameter(QString name)
{
    std::string value;
    //mpCoreComponentSystem->getSystemParameters().getValue(name.toStdString(), value);
    return QString::fromStdString(value);
}


bool CoreSystemAccess::hasSystemParameter(QString name)
{
    std::string dummy;
    return true;//mpCoreComponentSystem->getSystemParameters().getValue(name.toStdString(), dummy);
}


void CoreSystemAccess::removeSystemParameter(QString name)
{
    mpCoreComponentSystem->getSystemParameters().deleteParameter(name.toStdString());
}


//! @todo delete this methode and use getParmeters instead!
QMap<std::string, std::string> CoreSystemAccess::getSystemParametersMap()
{
    std::vector<std::string> parameterNames, parameterValues, descriptions, units, types;
    mpCoreComponentSystem->getSystemParameters().getParameters(parameterNames, parameterValues, descriptions, units, types);
    QMap<std::string, std::string> tmpMap;
    for(size_t i=0; i < parameterNames.size(); ++i)
    {
        tmpMap.insert(parameterNames[i], parameterValues[i]);
    }
    return tmpMap;//QMap<std::string, std::string>()(mpCoreComponentSystem->getSystemParameters().getSystemParameterMap());
}


void CoreSystemAccess::getSystemParameters(QVector<QString> &qParameterNames, QVector<QString> &qParameterValues, QVector<QString> &qDescriptions, QVector<QString> &qUnits, QVector<QString> &qTypes)
{
    std::vector<std::string> parameterNames, parameterValues, descriptions, units, types;
    mpCoreComponentSystem->getParameters(parameterNames, parameterValues, descriptions, units, types);
    for(size_t i=0; i<parameterNames.size(); ++i)
    {
        qParameterNames.push_back(QString::fromStdString(parameterNames[i]));
        qParameterValues.push_back(QString::fromStdString(parameterValues[i]));
        qDescriptions.push_back(QString::fromStdString(descriptions[i]));
        qUnits.push_back(QString::fromStdString(units[i]));
        qTypes.push_back(QString::fromStdString(types[i]));
    }
}
