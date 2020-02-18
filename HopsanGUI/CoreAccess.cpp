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
//! @file CoreAccess.cpp
//!
//! @brief Contains the HopsanCore Qt API classes for communication with the HopsanCore
//!
//$Id$

#include <QDebug>
#include <QDir>
#include <QMessageBox>

#include "CoreAccess.h"
#include "global.h"
#include "GUIObjects/GUISystem.h"
#include "Configuration.h"
#include "DesktopHandler.h"
#include "LibraryHandler.h"
#include "common.h"
#include "Utilities/GUIUtilities.h"

// HopsanCore includes
#include "HopsanCore.h"
#include "Node.h"
#include "ComponentSystem.h"
#include "ComponentUtilities/CSVParser.h"
#include "CoreUtilities/NumHopHelper.h"
#include "CoreUtilities/HmfLoader.h"
#include "CoreUtilities/SaveRestoreSimulationPoint.h"
#include "compiler_info.h"

// Here the HopsanCore object is created
hopsan::HopsanEssentials gHopsanCore;

void initializaHopsanCore(QString logPath)
{
    if (logPath.isEmpty()) {
        logPath = gpDesktopHandler->getTempPath();
    }
    QFileInfo fi(logPath);
    if (fi.isDir()) {
        logPath.append("hopsan_logfile.txt");
    }
    gHopsanCore.openCoreLogFile(logPath.toStdString().c_str());
}

//! @brief Help function to copy parameter data from core to GUI class
void copyParameterData(const hopsan::ParameterEvaluator *pCoreParam, CoreParameterData &rGUIParam)
{
    rGUIParam.mName = QString(pCoreParam->getName().c_str());
    rGUIParam.mType = QString(pCoreParam->getType().c_str());
    rGUIParam.mValue = QString(pCoreParam->getValue().c_str());
    rGUIParam.mQuantity = QString::fromStdString(pCoreParam->getQuantity().c_str());
    rGUIParam.mUnit = QString(pCoreParam->getUnit().c_str());
    rGUIParam.mDescription = QString::fromStdString(pCoreParam->getDescription().c_str());
    for(size_t c=0; c<pCoreParam->getConditions().size(); ++c)
    {
        rGUIParam.mConditions.append(QString(pCoreParam->getConditions()[c].c_str()));
    }
}


bool CoreLibraryAccess::hasComponent(const QString &rComponentName)
{
    return gHopsanCore.hasComponent(rComponentName.toStdString().c_str());
}


bool CoreLibraryAccess::loadComponentLib(const QString &rFileName)
{
    return gHopsanCore.loadExternalComponentLib(rFileName.toStdString().c_str());
}

bool CoreLibraryAccess::unLoadComponentLib(const QString &rFileName)
{
    return gHopsanCore.unLoadExternalComponentLib(rFileName.toStdString().c_str());
}

//! @brief Reserves a type name in the Hopsan Core, to prevent external libs from loading components with that specific typename
bool CoreLibraryAccess::reserveComponentTypeName(const QString &rTypeName)
{
    return gHopsanCore.reserveComponentTypeName(rTypeName.toStdString().c_str());
}

void CoreLibraryAccess::getLoadedLibNames(QVector<QString> &rLibNames)
{
    std::vector<hopsan::HString> names;
    gHopsanCore.getExternalComponentLibNames(names);

    rLibNames.clear();
    rLibNames.reserve(names.size());
    for (unsigned int i=0; i<names.size(); ++i)
    {
        rLibNames.push_back(names[i].c_str());
    }
}

void CoreLibraryAccess::getLibPathForComponent(QString typeName, QString &rLibPath)
{
    hopsan::HString typeNameStr = typeName.toStdString().c_str();
    hopsan::HString libStr;
    gHopsanCore.getLibPathForComponentType(typeNameStr, libStr);
    rLibPath = QString(libStr.c_str());
}

void CoreLibraryAccess::getLibraryContents(QString libPath, QStringList &rComponents, QStringList &rNodes)
{
    std::vector<hopsan::HString> components, nodes;
    gHopsanCore.getExternalLibraryContents(libPath.toStdString().c_str(), components, nodes);

    //rComponents.clear();
    rComponents.reserve(components.size());
    for (unsigned int i=0; i<components.size(); ++i)
    {
        rComponents.push_back(components[i].c_str());
    }

    //rNodes.clear();
    rNodes.reserve(nodes.size());
    for (unsigned int i=0; i<nodes.size(); ++i)
    {
        rNodes.push_back(nodes[i].c_str());
    }
}


unsigned int CoreMessagesAccess::getNumberOfMessages()
{
    return gHopsanCore.checkMessage();
}

void CoreMessagesAccess::getMessage(QString &rMessage, QString &rType, QString &rTag)
{
    hopsan::HString msg, type, tag;
    gHopsanCore.getMessage(msg, type, tag);
    rMessage = msg.c_str();
    rTag = tag.c_str();
    rType = type.c_str();
}

bool CoreSimulationHandler::initialize(const double startTime, const double stopTime, const double logStartTime, const int nLogSamples, CoreSystemAccess* pCoreSystemAccess)
{
    //! @todo write get set wrappers for n log samples, and use only value in core instead of duplicate in gui
    pCoreSystemAccess->getCoreSystemPtr()->setNumLogSamples(nLogSamples);
    pCoreSystemAccess->getCoreSystemPtr()->setLogStartTime(logStartTime);
    return gHopsanCore.getSimulationHandler()->initializeSystem(startTime, stopTime, pCoreSystemAccess->getCoreSystemPtr());
}

bool CoreSimulationHandler::initialize(const double startTime, const double stopTime, const double logStartTime, const int nLogSamples, QVector<CoreSystemAccess*> &rvCoreSystemAccess)
{
    std::vector<hopsan::ComponentSystem*> coreSystems;
    for (int i=0; i<rvCoreSystemAccess.size(); ++i)
    {
        //! @todo write get set wrappers for n log samples, and use only value in core instead of duplicate in gui
        rvCoreSystemAccess[i]->getCoreSystemPtr()->setNumLogSamples(nLogSamples);
        rvCoreSystemAccess[i]->getCoreSystemPtr()->setLogStartTime(logStartTime);
        coreSystems.push_back(rvCoreSystemAccess[i]->getCoreSystemPtr());
    }
    return gHopsanCore.getSimulationHandler()->initializeSystem(startTime, stopTime, coreSystems);
}

bool CoreSimulationHandler::simulate(const double startTime, const double stopTime, const int nThreads, CoreSystemAccess* pCoreSystemAccess, bool modelHasNotChanged)
{
    hopsan::ParallelAlgorithmT algorithm = hopsan::ParallelAlgorithmT(gpConfig->getParallelAlgorithm());
    return gHopsanCore.getSimulationHandler()->simulateSystem(startTime, stopTime, nThreads, pCoreSystemAccess->getCoreSystemPtr(), modelHasNotChanged, algorithm);
}

bool CoreSimulationHandler::simulate(const double startTime, const double stopTime, const int nThreads, QVector<CoreSystemAccess*> &rvCoreSystemAccess, bool modelHasNotChanged)
{
    std::vector<hopsan::ComponentSystem*> coreSystems;
    for (int i=0; i<rvCoreSystemAccess.size(); ++i)
    {
        coreSystems.push_back(rvCoreSystemAccess[i]->getCoreSystemPtr());
    }
    hopsan::ParallelAlgorithmT algorithm = hopsan::ParallelAlgorithmT(gpConfig->getParallelAlgorithm());
    return gHopsanCore.getSimulationHandler()->simulateSystem(startTime, stopTime, nThreads, coreSystems, modelHasNotChanged, algorithm);
}

void CoreSimulationHandler::finalize(CoreSystemAccess* pCoreSystemAccess)
{
    gHopsanCore.getSimulationHandler()->finalizeSystem(pCoreSystemAccess->getCoreSystemPtr());
}

void CoreSimulationHandler::finalize(QVector<CoreSystemAccess*> &rvCoreSystemAccess)
{
    std::vector<hopsan::ComponentSystem*> coreSystems;
    for (int i=0; i<rvCoreSystemAccess.size(); ++i)
    {
        coreSystems.push_back(rvCoreSystemAccess[i]->getCoreSystemPtr());
    }
    gHopsanCore.getSimulationHandler()->finalizeSystem(coreSystems);
}


CoreSystemAccess::CoreSystemAccess(QString name, CoreSystemAccess* pParentCoreSystemAccess)
{
    //Create new Core system component
    if (pParentCoreSystemAccess == 0)
    {
        mpCoreComponentSystem = gHopsanCore.createComponentSystem();
    }
    else
    {
        //Creating a subsystem, setting internal pointer
        mpCoreComponentSystem = pParentCoreSystemAccess->getCoreSubSystemPtr(name);
    }
}

hopsan::ComponentSystem* CoreSystemAccess::getCoreSystemPtr()
{
    return mpCoreComponentSystem;
}

hopsan::ComponentSystem* CoreSystemAccess::getCoreSubSystemPtr(QString name)
{
    //qDebug() << " corecomponentsystemname: " <<  QString::fromStdString(mpCoreComponentSystem->getName()) << "  Subname: " << name;
    return mpCoreComponentSystem->getSubComponentSystem(name.toStdString().c_str());
}

CoreSystemAccess::~CoreSystemAccess()
{
    //Dont remove the mpCoreComponentSystem here you must do that manually until we have found a smarter way to do all of this
    //see deleteRootSystemPtr()
    //delete mpCoreComponentSystem;
}

//! @todo This is very strange, needed because core systems are deleted from parent if they are subsystems (not if root systems), this is the only way to safely delete the core object
void CoreSystemAccess::deleteRootSystemPtr()
{
    delete mpCoreComponentSystem;
}

bool CoreSystemAccess::connect(QString compname1, QString portname1, QString compname2, QString portname2)
{
    //*****Core Interaction*****
    return mpCoreComponentSystem->connect(compname1.toStdString().c_str(), portname1.toStdString().c_str(), compname2.toStdString().c_str(), portname2.toStdString().c_str());
    //**************************
}

bool CoreSystemAccess::disconnect(QString compname1, QString portname1, QString compname2, QString portname2)
{
    //*****Core Interaction*****
    return mpCoreComponentSystem->disconnect(compname1.toStdString().c_str(), portname1.toStdString().c_str(), compname2.toStdString().c_str(), portname2.toStdString().c_str());
    //**************************
}

void CoreSystemAccess::setDesiredTimeStep(double timestep)
{
    mpCoreComponentSystem->setDesiredTimestep(timestep);
}

void CoreSystemAccess::setDesiredTimeStep(QString compname, double timestep)
{
    mpCoreComponentSystem->getSubComponent(compname.toStdString().c_str())->setDesiredTimestep(timestep);
}


void CoreSystemAccess::setInheritTimeStep(bool inherit)
{
    mpCoreComponentSystem->setInheritTimestep(inherit);
}

void CoreSystemAccess::setInheritTimeStep(QString compname, bool inherit)
{
    mpCoreComponentSystem->getSubComponent(compname.toStdString().c_str())->setInheritTimestep(inherit);
}


bool CoreSystemAccess::doesInheritTimeStep()
{
    return mpCoreComponentSystem->doesInheritTimestep();
}

bool CoreSystemAccess::doesInheritTimeStep(QString compname)
{
    return mpCoreComponentSystem->getSubComponent(compname.toStdString().c_str())->doesInheritTimestep();
}


double CoreSystemAccess::getDesiredTimeStep()
{
    return mpCoreComponentSystem->getDesiredTimeStep();
}


size_t CoreSystemAccess::getNumLogSamples()
{
    return mpCoreComponentSystem->getNumLogSamples();
}


QString CoreSystemAccess::getSystemTypeCQS()
{
    //qDebug() << "getRootTypeCQS: " << componentName;
    return mpCoreComponentSystem->getTypeCQSString().c_str();
}

QString CoreSystemAccess::getSubComponentTypeCQS(const QString componentName)
{
    //qDebug() << "getSubComponentTypeCQS: " << componentName << " in " << QString::fromStdString(mpCoreComponentSystem->getName());
    QString ans = mpCoreComponentSystem->getSubComponent(componentName.toUtf8().constData())->getTypeCQSString().c_str();
    //qDebug() << "cqs answer: " << ans;
    return ans;
}

void CoreSystemAccess::setExternalModelFilePath(const QString path)
{
    mpCoreComponentSystem->setExternalModelFilePath(path.toStdString().c_str());
}

// Commented by Peter, maybe should be used in the future
//QString CoreSystemAccess::getSubComponentSubTypeName(const QString componentName) const
//{
//    hopsan::Component *pComp = mpCoreComponentSystem->getSubComponent(componentName.toStdString());
//    if (pComp)
//    {
//        return QString::fromStdString(pComp->getSubTypeName());
//    }
//    return QString();
//}
//void CoreSystemAccess::setSubComponentSubTypeName(const QString componentName, const QString subTypeName)
//{
//    hopsan::Component *pComp = mpCoreComponentSystem->getSubComponent(componentName.toStdString());
//    if (pComp)
//    {
//        pComp->setSubTypeName(subTypeName.toStdString());
//    }
//}


QString CoreSystemAccess::setSystemName(QString name)
{
    //qDebug() << "setting root system name to: " << name;
    mpCoreComponentSystem->setName(name.toStdString().c_str());
    //qDebug() << "root system name after rename: " << QString::fromStdString(mpCoreComponentSystem->getName());
    return mpCoreComponentSystem->getName().c_str();
}


QString CoreSystemAccess::renameSubComponent(QString componentName, QString name)
{
    qDebug() << "rename subcomponent from " << componentName << " to: " << name;
    hopsan::Component *pTempComponent = mpCoreComponentSystem->getSubComponent(componentName.toStdString().c_str());
    pTempComponent->setName(name.toStdString().c_str());
    qDebug() << "name after: " << pTempComponent->getName().c_str();
    return pTempComponent->getName().c_str();
}


void CoreSystemAccess::setSubComponentDisabled(QString componentName, bool disabled)
{
    hopsan::Component* pComp =  mpCoreComponentSystem->getSubComponent(componentName.toStdString().c_str());
    if (pComp)
    {
        pComp->setDisabled(disabled);
    }
}

bool CoreSystemAccess::isSubComponentDisabled(QString componentName)
{
    hopsan::Component* pComp =  mpCoreComponentSystem->getSubComponent(componentName.toStdString().c_str());
    if (pComp)
    {
        return pComp->isDisabled();
    }
    return false;
}


bool CoreSystemAccess::hasParameter(const QString &rComponentName, const QString &rParameterName)
{
    hopsan::Component* pComp =  mpCoreComponentSystem->getSubComponent(rComponentName.toStdString().c_str());
    if (pComp)
    {
        return pComp->hasParameter(rParameterName.toStdString().c_str());
    }
    return false;
}

QString CoreSystemAccess::getSystemName()
{
   // qDebug() << "getNAme from core root: " << QString::fromStdString(mpCoreComponentSystem->getName());
    return mpCoreComponentSystem->getName().c_str();
}

double CoreSystemAccess::getCurrentTime() const
{
    return mpCoreComponentSystem->getTime();
}

void CoreSystemAccess::stop()
{
    mpCoreComponentSystem->stopSimulation();
}


QString CoreSystemAccess::getPortType(const QString componentName, const QString portName, const PortTypeIndicatorT portTypeIndicator)
{
    //qDebug() << "name for port fetch " << componentName << " " << portName;

    hopsan::Port *pPort = this->getCorePortPtr(componentName, portName);
    if(pPort)
    {
        switch (portTypeIndicator)
        {
        case InternalPortType:
            return portTypeToString(pPort->getInternalPortType()).c_str();
            break;
        case ActualPortType:
            return portTypeToString(pPort->getPortType()).c_str();
            break;
        case ExternalPortType:
            return portTypeToString(pPort->getExternalPortType()).c_str();
            break;
        default:
            return QString("Invalid  portTypeIndicator specified");
        }
    }
    else
    {
        qDebug() <<  "======== ERROR ========= Could not find Port in getPortType: " << componentName << " " << portName << " in: " << this->getSystemName();
        return QString(); //Empty
    }
}

QString CoreSystemAccess::getNodeType(QString componentName, QString portName)
{
    hopsan::Port *pPort = this->getCorePortPtr(componentName, portName);
    if(pPort)
    {
        return QString(pPort->getNodeType().c_str());
    }
    else
    {
        qDebug() <<  "======================================== EMPTY nodetype: " << componentName << " " << portName << " in: " << this->getSystemName();
        return QString(); //Empty
    }
}

QString CoreSystemAccess::getPortDescription(QString componentName, QString portName)
{
    hopsan::Port *pPort = this->getCorePortPtr(componentName, portName);
    if (pPort)
    {
        return pPort->getDescription().c_str();
    }
    return QString();
}


bool CoreSystemAccess::setParameterValue(QString componentName, QString parameterName, QString value, bool force)
{
    hopsan::Component *pComponent = mpCoreComponentSystem->getSubComponent(componentName.toStdString().c_str());
    bool retval = pComponent->setParameterValue(parameterName.toStdString().c_str(), value.toStdString().c_str(), force);
    if(pComponent->getTypeName() == HOPSAN_BUILTIN_TYPENAME_MODELICACOMPONENT)
    {
        pComponent->configure();
    }
    return retval;
}

void CoreSystemAccess::getVariameters(QString componentName, QVector<CoreVariameterDescription> &rVariameterDescriptions)
{
    rVariameterDescriptions.clear();
    hopsan::Component* pComp = mpCoreComponentSystem->getSubComponent(componentName.toStdString().c_str());
    if (pComp)
    {
        const std::vector<hopsan::VariameterDescription>* pDescs = pComp->getVariameters();
        for (size_t i=0; i<pDescs->size(); ++i)
        {
            CoreVariameterDescription data;
            data.mName = pDescs->at(i).mName.c_str();
            data.mShortName = pDescs->at(i).mShortName.c_str();
            data.mPortName = pDescs->at(i).mPortName.c_str();
            data.mNodeType = pDescs->at(i).mNodeType.c_str();
            data.mUnit = pDescs->at(i).mUnit.c_str();
            data.mQuantity = pDescs->at(i).mQuantity.c_str();
            data.mUserModifiableQuantity = pDescs->at(i).mUserModifiableQuantity;
            data.mDescription = pDescs->at(i).mDescription.c_str();
            data.mDataType = pDescs->at(i).mDataType.c_str();
            data.mAlias = pDescs->at(i).mAlias.c_str();
            data.mVariabelId = pDescs->at(i).mVariableId;
            data.mVariameterType = pDescs->at(i).mVariameterType;
            data.mVariabelType = hopsan::nodeDataVariableTypeAsString(pDescs->at(i).mVarType).c_str();
            rVariameterDescriptions.push_back(data);
        }
    }
}

QStringList CoreSystemAccess::getModelAssets() const
{
    std::list<hopsan::HString> assets = mpCoreComponentSystem->getModelAssets();
    assets.sort();
    assets.unique();
    QStringList ret;
    for (auto it=assets.begin(); it!=assets.end(); ++it)
    {
        ret << it->c_str();
    }
    return ret;
}

void CoreSystemAccess::runNumHopScript(const QString &rScript, bool printOutput, QString &rOutput)
{
    hopsan::HString output;
    mpCoreComponentSystem->runNumHopScript(rScript.toStdString().c_str(), printOutput, output);
    rOutput = QString::fromLocal8Bit(output.c_str());
}

void CoreSystemAccess::setNumHopScript(const QString &rScript)
{
   mpCoreComponentSystem->setNumHopScript(rScript.toStdString().c_str());
}

bool CoreSystemAccess::setVariableAlias(QString compName, QString portName, QString varName, QString alias)
{
    return mpCoreComponentSystem->getAliasHandler().setVariableAlias(alias.toStdString().c_str(), compName.toStdString().c_str(),
                                                                     portName.toStdString().c_str(), varName.toStdString().c_str());
}

QString CoreSystemAccess::getVariableAlias(QString compName, QString portName, QString varName)
{
    return mpCoreComponentSystem->getAliasHandler().getVariableAlias(compName.toStdString().c_str(), portName.toStdString().c_str(), varName.toStdString().c_str()).c_str();
}

void CoreSystemAccess::setParameterAlias(QString compName, QString paramName, QString alias)
{
    mpCoreComponentSystem->getAliasHandler().setParameterAlias(alias.toStdString().c_str(), compName.toStdString().c_str(), paramName.toStdString().c_str());
}

void CoreSystemAccess::getFullVariableNameByAlias(QString alias, QString &rCompName, QString &rPortName, QString &rVarName)
{
    hopsan::HString comp, port, var;
    mpCoreComponentSystem->getAliasHandler().getVariableFromAlias(alias.toStdString().c_str(), comp, port, var);
    rCompName = comp.c_str();
    rPortName = port.c_str();
    rVarName = var.c_str();
}

QStringList CoreSystemAccess::getAliasNames() const
{
    std::vector<hopsan::HString> str_vec = mpCoreComponentSystem->getAliasHandler().getAliases();
    QStringList qvec;
    qvec.reserve(str_vec.size());
    for (size_t i=0; i<str_vec.size(); ++i)
    {
        qvec.push_back(str_vec[i].c_str());
    }
    return qvec;
}

bool CoreSystemAccess::setModifyableSignalQuantity(QString compPortVar, QString quantity)
{
   QStringList systems;
   QString c,p,v;
   splitFullVariableName(compPortVar,systems,c,p,v);
   hopsan::Component *pComp =  mpCoreComponentSystem->getSubComponent(c.toStdString().c_str());
   if(pComp)
   {
       hopsan::Port *pPort = pComp->getPort(p.toStdString().c_str());
       if (pPort && pPort->getSignalNodeQuantityModifyable())
       {
           pPort->setSignalNodeQuantityOrUnit(quantity.toStdString().c_str());
           return true;
       }
   }
   return false;
}

QString CoreSystemAccess::getModifyableSignalQuantity(QString compPortVar)
{
    QStringList systems;
    QString c,p,v;
    splitFullVariableName(compPortVar,systems,c,p,v);
    hopsan::Component *pComp =  mpCoreComponentSystem->getSubComponent(c.toStdString().c_str());
    if(pComp)
    {
        hopsan::Port *pPort = pComp->getPort(p.toStdString().c_str());
        if (pPort)
        {
            // Only return a Custom quantity fior those nodes where quantity is changeable
            if (pPort->getSignalNodeQuantityModifyable())
            {
                return pPort->getSignalNodeQuantity().c_str();
            }
        }
    }
    return "";
}


void CoreSystemAccess::removeSubComponent(QString componentName, bool doDelete)
{
    mpCoreComponentSystem->removeSubComponent(componentName.toStdString().c_str(), doDelete);
}


std::vector<double> CoreSystemAccess::getTimeVector(QString componentName, QString portName)
{
    //qDebug() << "getTimeVector, " << componentName << ", " << portName;
    hopsan::Component* pComp = mpCoreComponentSystem->getSubComponentOrThisIfSysPort(componentName.toStdString().c_str());
    if (pComp != 0)
    {
        hopsan::Port* pPort = pComp->getPort(portName.toStdString().c_str());
        if (pPort != 0)
        {
            std::vector<double> *ptr = pPort->getLogTimeVectorPtr();
            if (ptr != 0)
            {
                return *ptr; //Return a copy of the vector
            }
        }
    }

    // Else Return empty dummy
    return std::vector<double>();
}


bool CoreSystemAccess::doesKeepStartValues()
{
    return mpCoreComponentSystem->keepsValuesAsStartValues();
}


void CoreSystemAccess::setKeepValuesAsStartValues(bool load)
{
    mpCoreComponentSystem->setKeepValuesAsStartValues(load);
}

//! @deprecated maybe
bool CoreSystemAccess::isSimulationOk()
{
    return mpCoreComponentSystem->checkModelBeforeSimulation();
}

//! @deprecated use the coresimulation access class instead
bool CoreSystemAccess::initialize(double mStartTime, double mFinishTime, int nSamples)
{
    //! @todo write get set wrappers for n log samples, and use only value in core instead of duplicate in gui
    mpCoreComponentSystem->setNumLogSamples(nSamples);
    return mpCoreComponentSystem->initialize(mStartTime, mFinishTime);
}

//! @deprecated use the coresimulation access class instead
void CoreSystemAccess::simulate(double mStartTime, double mFinishTime, int nThreads, bool modelHasNotChanged)
{
    //qDebug() << "simulate(), nThreads = " << nThreads;

    if(nThreads >= 0)
    {
        qDebug() << "Starting multicore simulation";
        hopsan::ParallelAlgorithmT algorithm = hopsan::ParallelAlgorithmT(gpConfig->getParallelAlgorithm());
        mpCoreComponentSystem->simulateMultiThreaded(mStartTime, mFinishTime, nThreads, modelHasNotChanged, algorithm);
        qDebug() << "Finished multicore simulation";
        //mpCoreComponentSystem->simulateMultiThreadedOld(mStartTime, mFinishTime);
    }
    else
    {
        //qDebug() << "Starting singlecore simulation";
        mpCoreComponentSystem->simulate(mFinishTime);
    }
}

//! @deprecated use the coresimulation access class instead
void CoreSystemAccess::finalize()
{
    mpCoreComponentSystem->finalize();
}

QString CoreSystemAccess::createComponent(QString type, QString name)
{
    //qDebug() << "createComponent: " << "type: " << type << " desired name:  " << name << " in system: " << this->getRootSystemName();
    hopsan::Component *pCoreComponent = gHopsanCore.createComponent(type.toStdString().c_str());
    if (pCoreComponent != 0)
    {
        mpCoreComponentSystem->addComponent(pCoreComponent);
        if (!name.isEmpty())
        {
            pCoreComponent->setName(name.toStdString().c_str());
        }
        //qDebug() << "createComponent: name after add: " << QString::fromStdString(pCoreComponent->getName()) << " added to: " << QString::fromStdString(mpCoreComponentSystem->getName());
        return pCoreComponent->getName().c_str();
    }
    else
    {
        qDebug() << "failed to create component of type: " << type << " maybe it is not registered in the core";
        return QString();
    }
}

QString CoreSystemAccess::createSubSystem(QString name)
{
    hopsan::ComponentSystem *pTempComponentSystem;
    pTempComponentSystem = gHopsanCore.createComponentSystem();
    mpCoreComponentSystem->addComponent(pTempComponentSystem);
    if (!name.isEmpty())
    {
        pTempComponentSystem->setName(name.toStdString().c_str());
    }
    return pTempComponentSystem->getName().c_str();
}

QString CoreSystemAccess::createConditionalSubSystem(QString name)
{
    hopsan::ComponentSystem *pTempComponentSystem;

    pTempComponentSystem = gHopsanCore.createConditionalComponentSystem();
    mpCoreComponentSystem->addComponent(pTempComponentSystem);
    if (!name.isEmpty())
    {
        pTempComponentSystem->setName(name.toStdString().c_str());
    }
    return pTempComponentSystem->getName().c_str();
}

void CoreSystemAccess::getParameters(QString componentName, QVector<CoreParameterData> &rParameterDataVec)
{
    rParameterDataVec.clear();
    hopsan::Component* pComp =  mpCoreComponentSystem->getSubComponent(componentName.toStdString().c_str());
    if (pComp!=0)
    {
        const std::vector<hopsan::ParameterEvaluator*> *pParams = pComp->getParametersVectorPtr();

        rParameterDataVec.resize(pParams->size()); //preAllocate storage
        for(size_t i=0; i<pParams->size(); ++i)
        {
            CoreParameterData data;
            copyParameterData(pParams->at(i), data);
            rParameterDataVec[i] = data;
        }
    }
}

void CoreSystemAccess::getParameter(QString componentName, QString parameterName, CoreParameterData &rData)
{
    hopsan::Component* pComp =  mpCoreComponentSystem->getSubComponent(componentName.toStdString().c_str());
    if (pComp!=0)
    {
        const hopsan::ParameterEvaluator *pParam = pComp->getParameter(parameterName.toStdString().c_str());
        if (pParam!=0)
        {
            copyParameterData(pParam, rData);
        }
    }
}


QStringList CoreSystemAccess::getParameterNames(QString componentName)
{
    QStringList qParameterNames;
    std::vector<hopsan::HString> parameterNames;
    hopsan::Component* pComp =  mpCoreComponentSystem->getSubComponent(componentName.toStdString().c_str());
    if (pComp!=0)
    {
        pComp->getParameterNames(parameterNames);
        for(size_t i=0; i<parameterNames.size(); ++i)
        {
            qParameterNames.push_back(QString(parameterNames[i].c_str()));
        }
    }

    return qParameterNames;
}


size_t CoreSystemAccess::loadParameterFile(QString fileName)
{
    return mpCoreComponentSystem->loadParameterValues(fileName.toStdString().c_str());
}

size_t CoreSystemAccess::loadParameterFile(QString componentName, QString fileName)
{
    hopsan::Component* pComp = mpCoreComponentSystem->getSubComponent(qPrintable(componentName));
    if (pComp!=0) {
        return pComp->loadParameterValues(qPrintable(fileName));
    }
    return 0;
}


QStringList CoreSystemAccess::getSystemParameterNames()
{
    std::vector<hopsan::HString> parameterNames;
    mpCoreComponentSystem->getParameterNames(parameterNames);
    QStringList qParameterNames;
    for(size_t i=0; i<parameterNames.size(); ++i)
    {
        qParameterNames.push_back(QString(parameterNames[i].c_str()));
    }
    return qParameterNames;
}


QString CoreSystemAccess::getParameterValue(QString componentName, QString parameterName)
{
    hopsan::HString parameterValue="";
    hopsan::Component* pComp = mpCoreComponentSystem->getSubComponent(componentName.toStdString().c_str());
    if (pComp != 0)
    {
        hopsan::HString value;
        pComp->getParameterValue(parameterName.toStdString().c_str(), value);
        parameterValue = value.c_str();
    }

    return parameterValue.c_str();
}

void CoreSystemAccess::deleteSystemPort(QString portname)
{
    mpCoreComponentSystem->deleteSystemPort(portname.toStdString().c_str());
}

QString CoreSystemAccess::addSystemPort(QString portname)
{
    //qDebug() << "add system port: " << portname;
    return mpCoreComponentSystem->addSystemPort(portname.toUtf8().constData())->getName().c_str();
}

QString CoreSystemAccess::renameSystemPort(QString oldname, QString newname)
{
    QByteArray ba_old = oldname.toUtf8();
    QByteArray ba_new = newname.toUtf8();
    return mpCoreComponentSystem->renameSystemPort(ba_old.data(), ba_new.data()).c_str();
}

QString CoreSystemAccess::reserveUniqueName(QString desiredName)
{
    QByteArray ba = desiredName.toUtf8();
    return mpCoreComponentSystem->reserveUniqueName(ba.data()).c_str();
}

void CoreSystemAccess::unReserveUniqueName(QString name)
{
    QByteArray ba = name.toUtf8();
    mpCoreComponentSystem->unReserveUniqueName(ba.data());
}


//! @todo how to handle fetching from systemports, component names will not be found
void CoreSystemAccess::getPlotDataNamesAndUnits(const QString compname, const QString portname, QVector<QString> &rNames, QVector<QString> &rUnits)
{
    rNames.clear();
    rUnits.clear();

    hopsan::Port* pPort = this->getCorePortPtr(compname, portname);
    if (pPort && pPort->getPortType() < hopsan::MultiportType)
    {
        const std::vector<hopsan::NodeDataDescription>* pDescs = pPort->getNodeDataDescriptions();
        if (pDescs != 0)
        {
            //Copy into QT datatype vector
            for (size_t i=0; i<pDescs->size(); ++i)
            {
                rNames.push_back(pDescs->at(i).name.c_str());
                rUnits.push_back(pDescs->at(i).unit.c_str());
            }
        }
    }
}

void CoreSystemAccess::getPlotData(const QString compname, const QString portname, const QString dataname, std::vector<double> *&rpTimeVector, QVector<double> &rData)
{
    int dataId = -1;
    hopsan::Port* pPort = this->getCorePortPtr(compname, portname);
    if (pPort)
    {
        dataId = pPort->getNodeDataIdFromName(dataname.toStdString().c_str());
        if (dataId > -1)
        {
            std::vector< std::vector<double> > *pData = pPort->getLogDataVectorPtr();
            rpTimeVector = pPort->getLogTimeVectorPtr();

            // Instead of pData.size() lets ask for latest logsample, this way we can avoid coping log slots that have not bee written and contains junk
            // This is useful when a simulation has been aborted
            size_t nElements;
            if (pPort->getNodePtr())
            {
                nElements = qMin(pPort->getNodePtr()->getOwnerSystem()->getNumActuallyLoggedSamples(), pData->size());
            }
            else
            {
                // this should never happen i think
                nElements = qMin(pData->size(), rpTimeVector->size());
            }
            //size_t nElements = min(pPort->getNodegetComponent()->getSystemParent()->getNumActuallyLoggedSamples(), pData->size());
            //qDebug() << "pData.size(): " << pData->size() << " nElements: " << nElements;

            //Ok lets copy all of the data to a Qt vector
            rData.resize(nElements); //Allocate memory for data
            for (size_t i=0; i<nElements; ++i)
            {
                rData[i] = pData->at(i).at(dataId);
            }
        }
    }
}

std::vector<double> *CoreSystemAccess::getLogTimeData() const
{
    return mpCoreComponentSystem->getLogTimeVector();
}

bool CoreSystemAccess::havePlotData(const QString compname, const QString portname, const QString dataname)
{
    hopsan::Port* pPort = this->getCorePortPtr(compname, portname);
    if (pPort)
    {
        bool exists=false;
        for(size_t i=0; i<pPort->getNodeDataDescriptions()->size(); ++i)
        {
            if(pPort->getNodeDataDescriptions()->at(i).name == dataname.toStdString().c_str())
            {
                exists = true;
            }
        }
        if(!exists)
        {
            return false;
        }

        if(pPort->isConnected())
        {
            return pPort->haveLogData();
        }
    }
    return false;
}


bool CoreSystemAccess::getLastNodeData(const QString compname, const QString portname, const QString dataname, double& rData) const
{
    int dataId = -1;
    hopsan::Port* pPort = this->getCorePortPtr(compname, portname);
    if (pPort)
    {
        dataId = pPort->getNodeDataIdFromName(dataname.toStdString().c_str());

        if (dataId >= 0)
        {
            std::vector<double> *pData = pPort->getDataVectorPtr();
            rData = pData->at(dataId);
            return true;
        }
    }
    return false;
}


double *CoreSystemAccess::getNodeDataPtr(const QString compname, const QString portname, const QString dataname)
{
    int dataId = -1;
    hopsan::Port* pPort = this->getCorePortPtr(compname, portname);
    if (pPort)
    {
        dataId = pPort->getNodeDataIdFromName(dataname.toStdString().c_str());

        if (dataId >= 0)
        {
            return pPort->getNodeDataPtr(dataId);
        }
    }
    return 0;
}


void CoreSystemAccess::measureSimulationTime(QStringList &rComponentNames, QList<double> &rTimes, int nSteps)
{
    if(!getCoreSystemPtr()->checkModelBeforeSimulation())
    {
        return; //! @todo Give user a message?
    }

    getCoreSystemPtr()->initialize(0, 10);

    if(!getCoreSystemPtr()->simulateAndMeasureTime(nSteps))
    {
        return;     //! @todo Do something better, so the user understands why this won't work (compiled without C++11 support)
    }

    getCoreSystemPtr()->finalize();

    std::vector<hopsan::HString> names = getCoreSystemPtr()->getSubComponentNames();
    for(size_t i=0; i<names.size(); ++i)
    {
        rComponentNames.append(names.at(i).c_str());
        rTimes.append(getCoreSystemPtr()->getSubComponent(names.at(i))->getMeasuredTime());
    }
}


bool CoreSystemAccess::isPortConnected(QString componentName, QString portName)
{
    hopsan::Port* pPort = this->getCorePortPtr(componentName, portName);
    if(pPort)
    {
        return pPort->isConnected();
    }
    else
    {
        return false;
    }
}

void CoreSystemAccess::setLoggingEnabled(const QString &componentName, const QString &portName, bool enable)
{
    hopsan::Port* pPort = this->getCorePortPtr(componentName, portName);
    if(pPort)
    {
        pPort->setEnableLogging(enable);
    }
}

bool CoreSystemAccess::isLoggingEnabled(const QString &componentName, const QString &portName)
{
    hopsan::Port *pPort = this->getCorePortPtr(componentName, portName);
    if(pPort)
    {
        return pPort->isLoggingEnabled();
    }
    else
    {
        return false;
    }
}


bool CoreSystemAccess::writeNodeData(const QString compname, const QString portname, const QString dataname, double data)
{
    hopsan::Port* pPort = getCorePortPtr(compname,portname);
    int dataId = -1;
    if(pPort)
    {
        dataId = pPort->getNodeDataIdFromName(dataname.toStdString().c_str());

        if(dataId >= 0)
        {
            pPort->writeNode(dataId, data);
            return 1;
        }
    }
    return 0;
}


//! @brief Helpfunction that tries to fetch a port pointer
//! @param [in] componentName The name of the component to which the port belongs
//! @param [in] portName The name of the port
//! @returns A pointer to the port or a 0 ptr if component or port not found
hopsan::Port* CoreSystemAccess::getCorePortPtr(QString componentName, QString portName) const
{
    //We must use getcomponent here if we want to be able to find root system ptr
    //! @todo see if we can reduce the number f public get functions one, the one which only searches subcomponents make function in core to solve the other access type like bellow
    hopsan::Component* pComp = mpCoreComponentSystem->getSubComponentOrThisIfSysPort(componentName.toStdString().c_str());
    if (pComp)
    {
        return pComp->getPort(portName.toStdString().c_str());
    }
    return 0;
}


bool CoreSystemAccess::setSystemParameter(const CoreParameterData &rParameter, bool doAdd, bool force)
{
    if (doAdd)
    {
        return mpCoreComponentSystem->setOrAddSystemParameter(
                    rParameter.mName.toStdString().c_str(),
                    rParameter.mValue.toStdString().c_str(),
                    rParameter.mType.toStdString().c_str(),
                    rParameter.mDescription.toStdString().c_str(),
                    rParameter.mQuantity.toStdString().c_str(),
                    force);
    }
    else
    {
        return mpCoreComponentSystem->setSystemParameter(
                    rParameter.mName.toStdString().c_str(),
                    rParameter.mValue.toStdString().c_str(),
                    rParameter.mType.toStdString().c_str(),
                    rParameter.mDescription.toStdString().c_str(),
                    rParameter.mQuantity.toStdString().c_str(),
                    force);
    }
}


bool CoreSystemAccess::setSystemParameterValue(QString name, QString value, bool force)
{
    return mpCoreComponentSystem->setParameterValue(name.toStdString().c_str(), value.toStdString().c_str(), force);
}

//! @brief Get the value of a parameter in the system
//! @returns The parameter value as a QString or "" if parameter not found
QString CoreSystemAccess::getSystemParameterValue(const QString name)
{
    hopsan::HString value;
    mpCoreComponentSystem->getParameterValue(name.toStdString().c_str(), value);
    return QString(value.c_str());
}


bool CoreSystemAccess::hasSystemParameter(const QString name)
{
    return mpCoreComponentSystem->hasParameter(name.toStdString().c_str());
}

//! @brief Rename a system parameter
bool CoreSystemAccess::renameSystemParameter(const QString oldName, const QString newName)
{
    return mpCoreComponentSystem->renameParameter(oldName.toStdString().c_str(), newName.toStdString().c_str());
}

//! @brief Removes the parameter with given name
//! @param [in] name The name of the system parameter to remove
void CoreSystemAccess::removeSystemParameter(const QString name)
{
    mpCoreComponentSystem->unRegisterParameter(name.toStdString().c_str());
}

//! @todo how to handle fetching from systemports, component names will not be found
void CoreSystemAccess::getVariableDescriptions(const QString compname, const QString portname, QVector<CoreVariableData> &rVarDescriptions)
{
    rVarDescriptions.clear();

    hopsan::Port* pPort = this->getCorePortPtr(compname, portname);
    if (pPort && pPort->getPortType() < hopsan::MultiportType)
    {
        const std::vector<hopsan::NodeDataDescription>* pDescs = pPort->getNodeDataDescriptions();
        if (pDescs != 0)
        {
            //Copy into QT datatype vector
            for (size_t i=0; i<pDescs->size(); ++i)
            {
                CoreVariableData data;
                data.mName = pDescs->at(i).name.c_str();
                data.mUnit = pDescs->at(i).unit.c_str();
                data.mQuantity = pDescs->at(i).quantity.c_str();
                data.mAlias = pPort->getVariableAlias(i).c_str();
                //data.mDescription = pDescs->at(i).description.c_str();
                data.mNodeDataVariableType = nodeDataVariableTypeAsString(pDescs->at(i).varType).c_str();
                rVarDescriptions.push_back(data);
            }
        }
    }
}

void CoreSystemAccess::getSystemParameter(const QString name, CoreParameterData &rParameterData)
{
    const hopsan::ParameterEvaluator *pParam = mpCoreComponentSystem->getParameter(name.toStdString().c_str());
    if (pParam!=0)
    {
        copyParameterData(pParam, rParameterData);
    }
}


void CoreSystemAccess::getSystemParameters(QVector<CoreParameterData> &rParameterDataVec)
{
    rParameterDataVec.clear();
    const std::vector<hopsan::ParameterEvaluator*> *pParams = mpCoreComponentSystem->getParametersVectorPtr();
    rParameterDataVec.resize(pParams->size()); //preAllocate storage
    for(size_t i=0; i<pParams->size(); ++i)
    {
        CoreParameterData data;
        copyParameterData(pParams->at(i), data);
        rParameterDataVec[i] = data;
    }
}



//! @brief Adds a search path to the Component system in Core that can be used by its components to look for external files, e.g. area curves
//! @param searchPath the search path to be added
void CoreSystemAccess::addSearchPath(QString searchPath)
{
    mpCoreComponentSystem->addSearchPath(searchPath.toStdString().c_str());
}

QStringList CoreSystemAccess::getSearchPaths() const
{
    QStringList ret;
    std::vector<hopsan::HString> paths = mpCoreComponentSystem->getSearchPaths();
    for (auto &path : paths)
    {
        ret << path.c_str();
    }
    return ret;
}

void CoreSystemAccess::saveSimulationState(const QString &filePath)
{
    hopsan::saveSimulationPoint(filePath.toStdString().c_str(), mpCoreComponentSystem);
}

void CoreSystemAccess::loadSimulationState(const QString &filePath, double &rTimeOffset)
{
    hopsan::restoreSimulationPoint(filePath.toStdString().c_str(), mpCoreComponentSystem, rTimeOffset);
}



NodeInfo::NodeInfo(QString nodeType)
{
    hopsan::Node *pNode = 0;
    if (!nodeType.isEmpty())
    {
        pNode = gHopsanCore.createNode(nodeType.toStdString().c_str());
    }
    if(!pNode) return;

    niceName = pNode->getNiceName().c_str();
    for(size_t i=0; i<pNode->getDataDescriptions()->size(); ++i)
    {
        if(pNode->getDataDescription(i)->varType == hopsan::DefaultType ||
           pNode->getDataDescription(i)->varType == hopsan::IntensityType ||
           pNode->getDataDescription(i)->varType == hopsan::FlowType)        //Q variable
        {
            qVariables << pNode->getDataDescription(i)->shortname.c_str();
            variableLabels << pNode->getDataDescription(i)->name.c_str();
            varIdx << pNode->getDataDescription(i)->id;
        }
        if(pNode->getDataDescription(i)->varType == hopsan::IntensityType)
        {
            intensity = pNode->getDataDescription(i)->name.c_str();
        }
        if(pNode->getDataDescription(i)->varType == hopsan::FlowType)
        {
            flow = pNode->getDataDescription(i)->name.c_str();
        }
    }
    for(size_t i=0; i<pNode->getDataDescriptions()->size(); ++i)
    {
        if(pNode->getDataDescription(i)->varType == hopsan::TLMType)        //C variable
        {
            cVariables << pNode->getDataDescription(i)->shortname.c_str();
            variableLabels << pNode->getDataDescription(i)->name.c_str();
            varIdx << pNode->getDataDescription(i)->id;
        }
    }

    shortNames << cVariables << qVariables;

    gHopsanCore.removeNode(pNode);
    //delete(pNode);
}

void NodeInfo::getNodeTypes(QStringList &nodeTypes)
{
    //! @todo this should not be hardcoded
    nodeTypes << "NodeMechanic" << "NodeMechanicRotational" << "NodeHydraulic" << "NodePneumatic" << "NodeElectric" << "NodePetriNet";
}

QString getHopsanCoreVersion()
{
    return QString::fromStdString(gHopsanCore.getCoreVersion());
}


CoreCSVParserAccess::CoreCSVParserAccess(QString file, QChar separator, int linesToSkip)
{
    mpParser = new hopsan::CSVParserNG(separator.toLatin1(), linesToSkip);
    bool openOK = mpParser->openFile(hopsan::HString(file.toStdString().c_str()));
    if (openOK)
    {
        mpParser->indexFile();
    }
    else
    {
        delete mpParser;
        mpParser = 0;
    }
}

CoreCSVParserAccess::~CoreCSVParserAccess()
{
    if (mpParser)
    {
        delete mpParser;
    }
}

bool CoreCSVParserAccess::isOk()
{
    return mpParser != 0;
}

int CoreCSVParserAccess::getNumberOfRows()
{
    if(mpParser)
        return this->mpParser->getNumDataRows();
    else
        return 0;
}

int CoreCSVParserAccess::getNumberOfColumns()
{
    if(mpParser)
        return this->mpParser->getNumDataCols();
    else
        return 0;
}

bool CoreCSVParserAccess::getColumn(int col, QVector<double> &rVector)
{
    if(mpParser)
    {
        std::vector<double> vec;
        bool rc = mpParser->copyColumn(col, vec);
        rVector = QVector<double>::fromStdVector(vec);
        return rc;
    }
    else
    {
        rVector = QVector<double>();
        return false;
    }
}

bool CoreCSVParserAccess::getRow(int row, QVector<double> &rVector)
{
    if(mpParser)
    {
        std::vector<double> vec;
        bool rc = mpParser->copyRow(row, vec);
        rVector = QVector<double>::fromStdVector(vec);
        return rc;
    }
    else
    {
        rVector = QVector<double>();
        return false;
    }
}

QString getHopsanCoreCompiler()
{
    return QString::fromStdString(gHopsanCore.getCoreCompiler());
}


QString getHopsanCoreArchitecture()
{
    if (gHopsanCore.isCore64Bit())
    {
        return "64-bit";
    }
    else
    {
        return "32-bit";
    }
}


QString getHopsanCoreBuildTime()
{
    return QString::fromStdString(gHopsanCore.getCoreBuildTime());
}


bool CoreQuantityAccess::haveQuantity(const QString &rQuantity)
{
    return gHopsanCore.haveQuantity(rQuantity.toStdString().c_str());
}

double evalWithNumHop(const QString &rExpression)
{
    hopsan::HString dummy;
    hopsan::NumHopHelper numhop;
    double value;
    numhop.evalNumHopScript(rExpression.toStdString().c_str(), value, false, dummy);
    return value;
}

QStringList getEmbeddedSriptVariableNames(const QString& expression, CoreSystemAccess* pCoreSystem)
{
    hopsan::NumHopHelper numhop;
    numhop.setSystem(pCoreSystem->getCoreSystemPtr());
    auto hnames = numhop.extractVariableNames(qPrintable(expression));
    QStringList names;
    names.reserve(hnames.size());
    for (size_t i=0; i<hnames.size(); ++i) {
        names.append(hnames[i].c_str());
    }
    return names;
}

void prependSelfToParameterExpresions(CoreSystemAccess *pCoreSystem)
{
    std::function<void(hopsan::ComponentSystem*)> processSystem;
    processSystem = [&processSystem](hopsan::ComponentSystem* pSystem) {
        hopsan::autoPrependSelfToParameterExpressions(pSystem);
        auto childComponents = pSystem->getSubComponents();
        for (auto pChild : childComponents) {
            hopsan::autoPrependSelfToParameterExpressions(pChild);
            if (pChild->isComponentSystem()) {
                processSystem(dynamic_cast<hopsan::ComponentSystem*>(pChild));
            }
        }
    };

    auto pTopSystem = pCoreSystem->getCoreSystemPtr();
    if (pTopSystem != nullptr) {
        processSystem(pTopSystem);
    }
}
