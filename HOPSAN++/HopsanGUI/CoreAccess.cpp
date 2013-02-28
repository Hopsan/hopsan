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
//! @brief Contains the HopsanCore Qt API classes for communication with the HopsanCore
//!
//$Id$

#include "CoreAccess.h"
#include <QDebug>
#include <QDir>
#include "GUIObjects/GUISystem.h"

//HopsanCore includes
#include "HopsanCore.h"
#include "Node.h"
#include "ComponentSystem.h"
#include "CoreUtilities/GeneratorHandler.h"
#include "MainWindow.h"
#include "Widgets/LibraryWidget.h"
#include "Widgets/MessageWidget.h"
#include "common.h"
#include "Utilities/GUIUtilities.h"

using namespace std;

hopsan::HopsanEssentials gHopsanCore;

//! @brief Help function to copy parameter data from core to GUI class
void copyParameterData(const hopsan::Parameter *pCoreParam, CoreParameterData &rGUIParam)
{
    rGUIParam.mName = QString::fromStdString(pCoreParam->getName());
    rGUIParam.mType = QString::fromStdString(pCoreParam->getType());
    rGUIParam.mValue = QString::fromStdString(pCoreParam->getValue());
    rGUIParam.mUnit = QString::fromStdString(pCoreParam->getUnit());
    rGUIParam.mDescription = QString::fromStdString(pCoreParam->getDescription());
    rGUIParam.mIsDynamic = pCoreParam->isDynamic();
    rGUIParam.mIsEnabled = pCoreParam->isEnabled();
}


bool CoreGeneratorAccess::generateFromModelica(QString code)
{
    hopsan::GeneratorHandler *pHandler = new hopsan::GeneratorHandler();
    if(pHandler->isLoadedSuccessfully())
    {
        pHandler->callModelicaGenerator(code.toStdString(), QString(COREINCLUDEPATH).toStdString(), gExecPath.toStdString(), true);
        return true;
    }
    return false;
}


//! @todo Return false if compilation fails!
bool CoreGeneratorAccess::generateFromCpp(QString code, bool showOutputDialog)
{
    hopsan::GeneratorHandler *pHandler = new hopsan::GeneratorHandler();
    if(pHandler->isLoadedSuccessfully())
    {
        pHandler->callCppGenerator(code.toStdString(), QString(COREINCLUDEPATH).toStdString(), gExecPath.toStdString(), showOutputDialog);
        return true;
    }
    return false;
}


bool CoreGeneratorAccess::generateFromFmu(QString path)
{
    hopsan::GeneratorHandler *pHandler = new hopsan::GeneratorHandler();
    if(pHandler->isLoadedSuccessfully())
    {
        QFileInfo fmuFileInfo = QFileInfo(path);
        fmuFileInfo.setFile(path);
        QString fmuName = fmuFileInfo.fileName();
        fmuName.chop(4);
        if(QDir().exists(QString(FMUPATH) + fmuName))
        {
            QMessageBox existWarningBox(QMessageBox::Warning, "Warning","Another FMU with same name exist. Do you want unload this library and then overwrite it?", 0, 0);
            existWarningBox.addButton("Yes", QMessageBox::AcceptRole);
            existWarningBox.addButton("No", QMessageBox::RejectRole);
            existWarningBox.setWindowIcon(gpMainWindow->windowIcon());
            bool doIt = (existWarningBox.exec() == QMessageBox::AcceptRole);

            if(doIt)
            {
                gpMainWindow->mpLibrary->unloadExternalLibrary(fmuName, "FMU");
                removeDir(QDir::cleanPath(QString(FMUPATH)+fmuName));
            }
            else
            {
                return false;
            }
        }

        pHandler->callFmuImportGenerator(path.toStdString(), QString(COREINCLUDEPATH).toStdString(), gExecPath.toStdString(), true);

        if(QDir().exists(QString(FMUPATH) + fmuName))
        {
            //Copy component icon
            QFile fmuIcon;
            fmuIcon.setFileName(QString(GRAPHICSPATH)+"/objecticons/fmucomponent.svg");
            fmuIcon.copy(QString(FMUPATH)+fmuName+"/fmucomponent.svg");
            fmuIcon.close();
            fmuIcon.setFileName(QString(FMUPATH)+fmuName+"/fmucomponent.svg");
            fmuIcon.setPermissions(QFile::WriteUser | QFile::ReadUser);
            fmuIcon.close();

            //Load library
            gpMainWindow->mpLibrary->loadAndRememberExternalLibrary(QString(FMUPATH) + fmuName, "FMU");
            return true;
        }
    }
    return false;
}


bool CoreGeneratorAccess::generateToFmu(QString path, SystemContainer *pSystem)
{
    hopsan::GeneratorHandler *pHandler = new hopsan::GeneratorHandler();
    if(pHandler->isLoadedSuccessfully())
    {
        pHandler->callFmuExportGenerator(path.toStdString(), pSystem->getCoreSystemAccessPtr()->getCoreSystemPtr(), QString(COREINCLUDEPATH).toStdString(), gExecPath.toStdString(), true);
        return true;
    }
    return false;
}


bool CoreGeneratorAccess::generateToSimulink(QString path, SystemContainer *pSystem, bool disablePortLabels, int compiler)
{
    hopsan::GeneratorHandler *pHandler = new hopsan::GeneratorHandler();
    if(pHandler->isLoadedSuccessfully())
    {
        pHandler->callSimulinkExportGenerator(path.toStdString(), pSystem->getCoreSystemAccessPtr()->getCoreSystemPtr(), disablePortLabels, compiler, QString(COREINCLUDEPATH).toStdString(), gExecPath.toStdString(), true);
        return true;
    }
    return false;
}


bool CoreGeneratorAccess::generateToSimulinkCoSim(QString path, SystemContainer *pSystem, bool disablePortLabels, int compiler)
{
    hopsan::GeneratorHandler *pHandler = new hopsan::GeneratorHandler();
    if(pHandler->isLoadedSuccessfully())
    {
        pHandler->callSimulinkCoSimExportGenerator(path.toStdString(), pSystem->getCoreSystemAccessPtr()->getCoreSystemPtr(), disablePortLabels, compiler, QString(COREINCLUDEPATH).toStdString(), gExecPath.toStdString(), true);
        return true;
    }
    return false;
}


bool CoreGeneratorAccess::generateToLabViewSIT(QString path, SystemContainer *pSystem)
{
    hopsan::GeneratorHandler *pHandler = new hopsan::GeneratorHandler();
    if(pHandler->isLoadedSuccessfully())
    {
        pHandler->callLabViewSITGenerator(path.toStdString(), pSystem->getCoreSystemAccessPtr()->getCoreSystemPtr(), QString(COREINCLUDEPATH).toStdString(), gExecPath.toStdString(), true);
        return true;
    }
    return false;
}


bool CoreLibraryAccess::hasComponent(QString componentName)
{
    return gHopsanCore.hasComponent(componentName.toStdString());
}


bool CoreLibraryAccess::loadComponentLib(QString fileName)
{
    return gHopsanCore.loadExternalComponentLib(fileName.toStdString());
}

bool CoreLibraryAccess::unLoadComponentLib(QString fileName)
{
    return gHopsanCore.unLoadExternalComponentLib(fileName.toStdString());
}

//! @brief Reserves a type name in the Hopsan Core, to prevent external libs from loading components with that specific typename
bool CoreLibraryAccess::reserveComponentTypeName(const QString typeName)
{
    return gHopsanCore.reserveComponentTypeName(typeName.toStdString());
}

void CoreLibraryAccess::getLoadedLibNames(QVector<QString> &rLibNames)
{
    std::vector<std::string> names;
    gHopsanCore.getExternalComponentLibNames(names);

    rLibNames.clear();
    rLibNames.reserve(names.size());
    for (unsigned int i=0; i<names.size(); ++i)
    {
        rLibNames.push_back(QString::fromStdString(names[i]));
    }
}

void CoreLibraryAccess::getLibraryContents(QString libPath, QStringList &rComponents, QStringList &rNodes)
{
    std::vector<std::string> components, nodes;
    gHopsanCore.getExternalLibraryContents(libPath.toStdString(), components, nodes);

    rComponents.clear();
    rComponents.reserve(components.size());
    for (unsigned int i=0; i<components.size(); ++i)
    {
        rComponents.push_back(QString::fromStdString(components[i]));
    }

    rNodes.clear();
    rNodes.reserve(nodes.size());
    for (unsigned int i=0; i<nodes.size(); ++i)
    {
        rNodes.push_back(QString::fromStdString(nodes[i]));
    }
}


unsigned int CoreMessagesAccess::getNumberOfMessages()
{
    return gHopsanCore.checkMessage();
}

void CoreMessagesAccess::getMessage(QString &rMessage, QString &rType, QString &rTag)
{
    std::string msg, tag, type;
    gHopsanCore.getMessage(msg, type, tag);
    rMessage = QString::fromStdString(msg);
    rTag = QString::fromStdString(tag);
    rType = QString::fromStdString(type);
}

bool CoreSimulationHandler::initialize(const double startTime, const double stopTime, const int nLogSamples, CoreSystemAccess* pCoreSystemAccess)
{
    //! @todo write get set wrappers for n log samples, and use only value in core instead of duplicate in gui
    pCoreSystemAccess->getCoreSystemPtr()->setNumLogSamples(nLogSamples);
    return gHopsanCore.getSimulationHandler()->initializeSystem(startTime, stopTime, pCoreSystemAccess->getCoreSystemPtr());
}

bool CoreSimulationHandler::initialize(const double startTime, const double stopTime, const int nLogSamples, QVector<CoreSystemAccess*> &rvCoreSystemAccess)
{
    std::vector<hopsan::ComponentSystem*> coreSystems;
    for (int i=0; i<rvCoreSystemAccess.size(); ++i)
    {
        //! @todo write get set wrappers for n log samples, and use only value in core instead of duplicate in gui
        rvCoreSystemAccess[i]->getCoreSystemPtr()->setNumLogSamples(nLogSamples);
        coreSystems.push_back(rvCoreSystemAccess[i]->getCoreSystemPtr());
    }
    return gHopsanCore.getSimulationHandler()->initializeSystem(startTime, stopTime, coreSystems);
}

bool CoreSimulationHandler::simulate(const double startTime, const double stopTime, const int nThreads, CoreSystemAccess* pCoreSystemAccess, bool modelHasNotChanged)
{
    return gHopsanCore.getSimulationHandler()->simulateSystem(startTime, stopTime, nThreads, pCoreSystemAccess->getCoreSystemPtr(), modelHasNotChanged);
}

void CoreSimulationHandler::runCoSimulation(CoreSystemAccess* pCoreSystemAccess)
{
    gHopsanCore.getSimulationHandler()->runCoSimulation(pCoreSystemAccess->getCoreSystemPtr());
}

bool CoreSimulationHandler::simulate(const double startTime, const double stopTime, const int nThreads, QVector<CoreSystemAccess*> &rvCoreSystemAccess, bool modelHasNotChanged)
{
    std::vector<hopsan::ComponentSystem*> coreSystems;
    for (int i=0; i<rvCoreSystemAccess.size(); ++i)
    {
        coreSystems.push_back(rvCoreSystemAccess[i]->getCoreSystemPtr());
    }
    return gHopsanCore.getSimulationHandler()->simulateSystem(startTime, stopTime, nThreads, coreSystems, modelHasNotChanged);
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
        //Create new root system
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
    return mpCoreComponentSystem->getSubComponentSystem(name.toStdString());
}

CoreSystemAccess::~CoreSystemAccess()
{
    //Dont remove the mpCoreComponentSystem here you must do that manually until we have found a samrter way to do all of this
    //see deleteRootSystemPtr()
    //delete mpCoreComponentSystem;
}

//! @todo This is very strange, needed becouse core systems are deleted from parent if they are subsystems (not if root systems), this is the only way to safely delete the core object
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

QString CoreSystemAccess::getHopsanCoreVersion()
{
    return QString::fromStdString(gHopsanCore.getCoreVersion());
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


size_t CoreSystemAccess::getNSamples()
{
    return mpCoreComponentSystem->getNumLogSamples();
}


QString CoreSystemAccess::getSystemTypeCQS()
{
    //qDebug() << "getRootTypeCQS: " << componentName;
    return QString::fromStdString(mpCoreComponentSystem->getTypeCQSString());
}

QString CoreSystemAccess::getSubComponentTypeCQS(const QString componentName)
{
    //qDebug() << "getSubComponentTypeCQS: " << componentName << " in " << QString::fromStdString(mpCoreComponentSystem->getName());
    QString ans = QString::fromStdString(mpCoreComponentSystem->getSubComponent(componentName.toStdString())->getTypeCQSString());
    //qDebug() << "cqs answer: " << ans;
    return ans;
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
    mpCoreComponentSystem->setName(name.toStdString());
    //qDebug() << "root system name after rename: " << QString::fromStdString(mpCoreComponentSystem->getName());
    return QString::fromStdString(mpCoreComponentSystem->getName());
}


QString CoreSystemAccess::renameSubComponent(QString componentName, QString name)
{
    qDebug() << "rename subcomponent from " << componentName << " to: " << name;
    hopsan::Component *pTempComponent = mpCoreComponentSystem->getSubComponent(componentName.toStdString());
    pTempComponent->setName(name.toStdString());
    qDebug() << "name after: " << QString::fromStdString(pTempComponent->getName());
    return QString::fromStdString(pTempComponent->getName());
}

QString CoreSystemAccess::getSystemName()
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


//void CoreSystemAccess::getStartValueDataNamesValuesAndUnits(QString componentName, QString portName, QVector<QString> &rNames, QVector<QString> &rValuesTxt, QVector<QString> &rUnits)
//{
//    std::vector<std::string> stdNames, stdUnits;
//    std::vector<std::string> stdValuesTxt;
//    hopsan::Port *pPort = this->getCorePortPtr(componentName, portName);
//    if(pPort)
//    {
//        pPort->getStartValueDataNamesValuesAndUnits(stdNames, stdValuesTxt, stdUnits);
//    }
//    rNames.resize(stdNames.size());
//    rValuesTxt.resize(stdValuesTxt.size());
//    rUnits.resize(stdUnits.size());
//    for(size_t i=0; i < stdNames.size(); ++i) //! @todo Make a nicer conversion fron std::vector<std::string> --> QVector<QString>
//    {
//        rNames[i] = QString::fromStdString(stdNames[i]);
//        rValuesTxt[i] = QString::fromStdString(stdValuesTxt[i]);
//        rUnits[i] = QString::fromStdString(stdUnits[i]);
//    }
//}


bool CoreSystemAccess::setParameterValue(QString componentName, QString parameterName, QString value, bool force)
{
    return mpCoreComponentSystem->getSubComponent(componentName.toStdString())->setParameterValue(parameterName.toStdString(), value.toStdString(), force);
}

void CoreSystemAccess::setVariableAlias(QString compName, QString portName, QString varName, QString alias)
{
    mpCoreComponentSystem->getAliasHandler().setVariableAlias(alias.toStdString(), compName.toStdString(),
                                                              portName.toStdString(), varName.toStdString());
}

void CoreSystemAccess::setParameterAlias(QString compName, QString paramName, QString alias)
{
    mpCoreComponentSystem->getAliasHandler().setParameterAlias(alias.toStdString(), compName.toStdString(), paramName.toStdString());
}

void CoreSystemAccess::getFullVariableNameByAlias(QString alias, QString &rCompName, QString &rPortName, QString &rVarName)
{
    std::string comp, port, var;
    mpCoreComponentSystem->getAliasHandler().getVariableFromAlias(alias.toStdString(), comp, port, var);
    rCompName = QString::fromStdString(comp);
    rPortName = QString::fromStdString(port);
    rVarName = QString::fromStdString(var);
}

QStringList CoreSystemAccess::getAliasNames() const
{
    std::vector<std::string> str_vec = mpCoreComponentSystem->getAliasHandler().getAliases();
    QStringList qvec;
    qvec.reserve(str_vec.size());
    for (size_t i=0; i<str_vec.size(); ++i)
    {
        qvec.push_back(QString::fromStdString(str_vec[i]));
    }
    return qvec;
}


void CoreSystemAccess::removeSubComponent(QString componentName, bool doDelete)
{
    mpCoreComponentSystem->removeSubComponent(componentName.toStdString(), doDelete);
}


vector<double> CoreSystemAccess::getTimeVector(QString componentName, QString portName)
{
    //qDebug() << "getTimeVector, " << componentName << ", " << portName;
    hopsan::Component* pComp = mpCoreComponentSystem->getSubComponentOrThisIfSysPort(componentName.toStdString());
    if (pComp != 0)
    {
        hopsan::Port* pPort = pComp->getPort(portName.toStdString());
        if (pPort != 0)
        {
            vector<double> *ptr = pPort->getLogTimeVectorPtr();
            if (ptr != 0)
            {
                return *ptr; //Return a copy of the vector
            }
        }
    }

    // Else Return empty dummy
    return vector<double>();
}


bool CoreSystemAccess::doesKeepStartValues()
{
    return mpCoreComponentSystem->doesKeepStartValues();
}


void CoreSystemAccess::setLoadStartValues(bool load)
{
    mpCoreComponentSystem->setLoadStartValues(load);
}

//! @deprectaed maybe
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
        mpCoreComponentSystem->simulateMultiThreaded(mStartTime, mFinishTime, nThreads, modelHasNotChanged);
        qDebug() << "Finished multicore simulation";
        //mpCoreComponentSystem->simulateMultiThreadedOld(mStartTime, mFinishTime);
    }
    else
    {
        //qDebug() << "Starting singlecore simulation";
        mpCoreComponentSystem->simulate(mStartTime, mFinishTime);
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
    hopsan::Component *pCoreComponent = gHopsanCore.createComponent(type.toStdString());
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
    hopsan::ComponentSystem *pTempComponentSystem = gHopsanCore.createComponentSystem();
    mpCoreComponentSystem->addComponent(pTempComponentSystem);
    if (!name.isEmpty())
    {
        pTempComponentSystem->setName(name.toStdString());
    }
    return QString::fromStdString(pTempComponentSystem->getName());
}

void CoreSystemAccess::getParameters(QString componentName, QVector<CoreParameterData> &rParameterDataVec)
{
    rParameterDataVec.clear();
    hopsan::Component* pComp =  mpCoreComponentSystem->getSubComponent(componentName.toStdString());
    if (pComp!=0)
    {
        const std::vector<hopsan::Parameter*> *pParams = pComp->getParametersVectorPtr();

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
    hopsan::Component* pComp =  mpCoreComponentSystem->getSubComponent(componentName.toStdString());
    if (pComp!=0)
    {
        const hopsan::Parameter *pParam = pComp->getParameter(parameterName.toStdString());
        if (pParam!=0)
        {
            copyParameterData(pParam, rData);
        }
    }
}


QStringList CoreSystemAccess::getParameterNames(QString componentName)
{
    QStringList qParameterNames;
    std::vector<std::string> parameterNames;
    hopsan::Component* pComp =  mpCoreComponentSystem->getSubComponent(componentName.toStdString());
    if (pComp!=0)
    {
        pComp->getParameterNames(parameterNames);
        for(size_t i=0; i<parameterNames.size(); ++i)
        {
            qParameterNames.push_back(QString::fromStdString(parameterNames[i]));
        }
    }

    return qParameterNames;
}


void CoreSystemAccess::loadParameterFile(QString fileName)
{
    mpCoreComponentSystem->loadParameters(fileName.toStdString());
}


QStringList CoreSystemAccess::getSystemParameterNames()
{
    std::vector<std::string> parameterNames;
    mpCoreComponentSystem->getParameterNames(parameterNames);
    QStringList qParameterNames;
    for(size_t i=0; i<parameterNames.size(); ++i)
    {
        qParameterNames.push_back(QString::fromStdString(parameterNames[i]));
    }
    return qParameterNames;
}


QString CoreSystemAccess::getParameterValue(QString componentName, QString parameterName)
{
    std::string parameterValue="";
    hopsan::Component* pComp = mpCoreComponentSystem->getSubComponent(componentName.toStdString());
    if (pComp != 0)
    {
        pComp->getParameterValue(parameterName.toStdString(), parameterValue);
    }

    return QString::fromStdString(parameterValue);
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


//! @todo how to handle fetching from systemports, component names will not be found
void CoreSystemAccess::getPlotDataNamesAndUnits(const QString compname, const QString portname, QVector<QString> &rNames, QVector<QString> &rUnits)
{
    rNames.clear();
    rUnits.clear();

    hopsan::Port* pPort = this->getCorePortPtr(compname, portname);
    if (pPort && pPort->getPortType() < hopsan::MULTIPORT)
    {
        const std::vector<hopsan::NodeDataDescription>* pDescs = pPort->getNodeDataDescriptions();
        if (pDescs != 0)
        {
            //Copy into QT datatype vector
            for (size_t i=0; i<pDescs->size(); ++i)
            {
                rNames.push_back(QString::fromStdString(pDescs->at(i).name));
                rUnits.push_back(QString::fromStdString(pDescs->at(i).unit));
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
        if(pPort->isConnected())
        {
            dataId = pPort->getNodeDataIdFromName(dataname.toStdString());
            if (dataId > -1)
            {
                vector< vector<double> > *pData = pPort->getLogDataVectorPtr();
                rpTimeVector = pPort->getLogTimeVectorPtr();

                // Instead of pData.size() lets ask for latest logsample, this way we can avoid coping log slots that have not bee written and contains junk
                // This is usefull when a simulation has been aborted
                size_t nElements = pPort->getComponent()->getSystemParent()->getNumActuallyLoggedSamples();
                qDebug() << "pData.size(): " << pData->size() << " nElements: " << nElements;

                //Ok lets copy all of the data to a Qt vector
                rData.resize(nElements); //Allocate memory for data
                for (size_t i=0; i<nElements; ++i)
                {
                     rData[i] = pData->at(i).at(dataId);
                }
            }
        }
    }
}

bool CoreSystemAccess::havePlotData(const QString compname, const QString portname, const QString dataname)
{
    hopsan::Port* pPort = this->getCorePortPtr(compname, portname);
    if (pPort)
    {
        bool exists=false;
        for(size_t i=0; i<pPort->getNodeDataDescriptions()->size(); ++i)
        {
            if(pPort->getNodeDataDescriptions()->at(i).name == dataname.toStdString())
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


bool CoreSystemAccess::getLastNodeData(const QString compname, const QString portname, const QString dataname, double& rData)
{
    int dataId = -1;
    hopsan::Port* pPort = this->getCorePortPtr(compname, portname);
    if (pPort)
    {
        dataId = pPort->getNodeDataIdFromName(dataname.toStdString());

        if (dataId >= 0)
        {
            vector<double> *pData = pPort->getDataVectorPtr();
            rData = pData->at(dataId);
            return 1;
        }
    }
    return 0;
}


double *CoreSystemAccess::getNodeDataPtr(const QString compname, const QString portname, const QString dataname)
{
    int dataId = -1;
    hopsan::Port* pPort = this->getCorePortPtr(compname, portname);
    if (pPort)
    {
        dataId = pPort->getNodeDataIdFromName(dataname.toStdString());

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
        return;     //! @todo Do something better, so the user understands why this won't work (TBB is not installed)
    }

    getCoreSystemPtr()->finalize();

    std::vector<std::string> names = getCoreSystemPtr()->getSubComponentNames();
    for(size_t i=0; i<names.size(); ++i)
    {
        rComponentNames.append(QString(names.at(i).c_str()));
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


bool CoreSystemAccess::writeNodeData(const QString compname, const QString portname, const QString dataname, double data)
{
    hopsan::Port* pPort = getCorePortPtr(compname,portname);
    int dataId = -1;
    if(pPort)
    {
        dataId = pPort->getNodeDataIdFromName(dataname.toStdString());

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
hopsan::Port* CoreSystemAccess::getCorePortPtr(QString componentName, QString portName)
{
    //We must use getcomponent here if we want to be able to find root system ptr
    //! @todo see if we can reduce the number f public get functions one, the one which only searches subcomponents make function in core to solve the other access type like bellow
    hopsan::Component* pComp = mpCoreComponentSystem->getSubComponentOrThisIfSysPort(componentName.toStdString());
    if (pComp)
    {
        return pComp->getPort(portName.toStdString());
    }
    return 0;
}


bool CoreSystemAccess::setSystemParameter(const CoreParameterData &rParameter, bool force)
{
    return mpCoreComponentSystem->setSystemParameter(rParameter.mName.toStdString(),
                                                     rParameter.mValue.toStdString(),
                                                     rParameter.mType.toStdString(),
                                                     rParameter.mDescription.toStdString(),
                                                     rParameter.mUnit.toStdString(),
                                                     force);
}


bool CoreSystemAccess::setSystemParameterValue(QString name, QString value, bool force)
{
    return mpCoreComponentSystem->setParameterValue(name.toStdString(), value.toStdString(), force);
}

//! @brief Get the value of a parameter in the system
//! @returns The aprameter value as a QString or "" if parameter not found
QString CoreSystemAccess::getSystemParameterValue(const QString name)
{
    std::string value;
    mpCoreComponentSystem->getParameterValue(name.toStdString(), value);
    return QString::fromStdString(value);
}


//! @todo Dont know if this is actually used
bool CoreSystemAccess::hasSystemParameter(const QString name)
{
    return mpCoreComponentSystem->hasParameter(name.toStdString());
}

//! @brief Rename a system parameter
bool CoreSystemAccess::renameSystemParameter(const QString oldName, const QString newName)
{
    return mpCoreComponentSystem->renameParameter(oldName.toStdString(), newName.toStdString());
}

//! @brief Removes the parameter with given name
//! @param [in] name The name of the system parameter to remove
void CoreSystemAccess::removeSystemParameter(const QString name)
{
    mpCoreComponentSystem->unRegisterParameter(name.toStdString());
}

//! @todo how to handle fetching from systemports, component names will not be found
void CoreSystemAccess::getVariableDescriptions(const QString compname, const QString portname, QVector<CoreVariableData> &rVarDescriptions)
{
    rVarDescriptions.clear();

    hopsan::Port* pPort = this->getCorePortPtr(compname, portname);
    if (pPort && pPort->getPortType() < hopsan::MULTIPORT)
    {
        const std::vector<hopsan::NodeDataDescription>* pDescs = pPort->getNodeDataDescriptions();
        if (pDescs != 0)
        {
            //Copy into QT datatype vector
            for (size_t i=0; i<pDescs->size(); ++i)
            {
                CoreVariableData data;
                data.mName = QString::fromStdString(pDescs->at(i).name);
                data.mUnit = QString::fromStdString(pDescs->at(i).unit);
                data.mAlias = QString::fromStdString(pPort->getVariableAlias(i));
                rVarDescriptions.push_back(data);
            }
        }
    }
}

void CoreSystemAccess::getSystemParameter(const QString name, CoreParameterData &rParameterData)
{
    const hopsan::Parameter *pParam = mpCoreComponentSystem->getParameter(name.toStdString());
    if (pParam!=0)
    {
        copyParameterData(pParam, rParameterData);
    }
}


void CoreSystemAccess::getSystemParameters(QVector<CoreParameterData> &rParameterDataVec)
{
    rParameterDataVec.clear();
    const std::vector<hopsan::Parameter*> *pParams = mpCoreComponentSystem->getParametersVectorPtr();
    rParameterDataVec.resize(pParams->size()); //preAllocate storage
    for(size_t i=0; i<pParams->size(); ++i)
    {
        CoreParameterData data;
        copyParameterData(pParams->at(i), data);
        rParameterDataVec[i] = data;
    }
}
