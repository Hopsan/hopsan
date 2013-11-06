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
#include "global.h"
#include "GUIObjects/GUISystem.h"
#include "Configuration.h"

//HopsanCore includes
#include "HopsanCore.h"
#include "Node.h"
#include "ComponentSystem.h"
#include "CoreUtilities/GeneratorHandler.h"
#include "DesktopHandler.h"
#include "LibraryHandler.h"
#include "Widgets/MessageWidget.h"
#include "common.h"
#include "Utilities/GUIUtilities.h"
#include "ComponentUtilities/CSVParser.h"

using namespace std;

hopsan::HopsanEssentials gHopsanCore;

//! @brief Help function to copy parameter data from core to GUI class
void copyParameterData(const hopsan::ParameterEvaluator *pCoreParam, CoreParameterData &rGUIParam)
{
    rGUIParam.mName = QString(pCoreParam->getName().c_str());
    rGUIParam.mType = QString(pCoreParam->getType().c_str());
    rGUIParam.mValue = QString(pCoreParam->getValue().c_str());
    rGUIParam.mUnit = QString(pCoreParam->getUnit().c_str());
    rGUIParam.mDescription = QString::fromStdString(pCoreParam->getDescription().c_str());
    rGUIParam.mIsEnabled = pCoreParam->isEnabled();
    for(size_t c=0; c<pCoreParam->getConditions().size(); ++c)
    {
        rGUIParam.mConditions.append(QString(pCoreParam->getConditions()[c].c_str()));
    }
}


bool CoreGeneratorAccess::generateFromModelica(QString path, bool showDialog, int solver, bool compile)
{
    qDebug() << "SOLVER: " << solver;

    hopsan::GeneratorHandler *pHandler = new hopsan::GeneratorHandler();

    if(pHandler->isLoadedSuccessfully())
    {
        pHandler->callModelicaGenerator(path.toStdString().c_str(), showDialog, solver, compile, gpDesktopHandler->getCoreIncludePath().toStdString().c_str(), gpDesktopHandler->getExecPath().toStdString().c_str());
        return true;
    }
    delete(pHandler);
    return false;
}


//! @todo Return false if compilation fails!
bool CoreGeneratorAccess::generateFromCpp(QString hppFile, bool compile)
{
    hopsan::GeneratorHandler *pHandler = new hopsan::GeneratorHandler();
    if(pHandler->isLoadedSuccessfully())
    {
        pHandler->callCppGenerator(hppFile.toStdString().c_str(), compile, gpDesktopHandler->getCoreIncludePath().toStdString().c_str(), gpDesktopHandler->getExecPath().toStdString().c_str());
        return true;
    }
    delete(pHandler);
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
        if(QDir().exists(gpDesktopHandler->getFMUPath() + fmuName))
        {
            QMessageBox existWarningBox(QMessageBox::Warning, "Warning","Another FMU with same name exist. Do you want unload this library and then overwrite it?", 0, 0);
            existWarningBox.addButton("Yes", QMessageBox::AcceptRole);
            existWarningBox.addButton("No", QMessageBox::RejectRole);
            existWarningBox.setWindowIcon(QIcon(QString(QString(ICONPATH) + "hopsan.png")));
            bool doIt = (existWarningBox.exec() == QMessageBox::AcceptRole);

            if(doIt)
            {
                gpLibraryHandler->unloadLibrary(fmuName);
                removeDir(QDir::cleanPath(gpDesktopHandler->getFMUPath()+fmuName));
            }
            else
            {
                return false;
            }
        }

        pHandler->callFmuImportGenerator(path.toStdString().c_str(), gpDesktopHandler->getFMUPath().toStdString().c_str(), gpDesktopHandler->getCoreIncludePath().toStdString().c_str(), gpDesktopHandler->getExecPath().toStdString().c_str(), true);

        if(QDir().exists(gpDesktopHandler->getFMUPath() + fmuName))
        {
            //Copy component icon
            QFile fmuIcon;
            fmuIcon.setFileName(QString(GRAPHICSPATH)+"/objecticons/fmucomponent.svg");
            fmuIcon.copy(gpDesktopHandler->getFMUPath()+fmuName+"/fmucomponent.svg");
            fmuIcon.close();
            fmuIcon.setFileName(gpDesktopHandler->getFMUPath()+fmuName+"/fmucomponent.svg");
            fmuIcon.setPermissions(QFile::WriteUser | QFile::ReadUser);
            fmuIcon.close();

            //Load library
            gpLibraryHandler->loadLibrary(gpDesktopHandler->getFMUPath()+fmuName+"/"+fmuName+"_lib.xml", FMU);
            return true;
        }
    }
    delete(pHandler);
    return false;
}


bool CoreGeneratorAccess::generateToFmu(QString path, SystemContainer *pSystem)
{
    hopsan::GeneratorHandler *pHandler = new hopsan::GeneratorHandler();
    if(pHandler->isLoadedSuccessfully())
    {
        pHandler->callFmuExportGenerator(path.toStdString().c_str(), pSystem->getCoreSystemAccessPtr()->getCoreSystemPtr(), gpDesktopHandler->getCoreIncludePath().toStdString().c_str(), gpDesktopHandler->getExecPath().toStdString().c_str(), true);
        return true;
    }
    delete(pHandler);
    return false;
}


bool CoreGeneratorAccess::generateToSimulink(QString path, SystemContainer *pSystem, bool disablePortLabels, int compiler)
{
    hopsan::GeneratorHandler *pHandler = new hopsan::GeneratorHandler();
    if(pHandler->isLoadedSuccessfully())
    {
        pHandler->callSimulinkExportGenerator(path.toStdString().c_str(), pSystem->getModelFileInfo().fileName().toStdString().c_str(), pSystem->getCoreSystemAccessPtr()->getCoreSystemPtr(), disablePortLabels, compiler, gpDesktopHandler->getCoreIncludePath().toStdString().c_str(), gpDesktopHandler->getExecPath().toStdString().c_str(), true);
        return true;
    }
    delete(pHandler);
    return false;
}


bool CoreGeneratorAccess::generateToSimulinkCoSim(QString path, SystemContainer *pSystem, bool disablePortLabels, int compiler)
{
    hopsan::GeneratorHandler *pHandler = new hopsan::GeneratorHandler();
    if(pHandler->isLoadedSuccessfully())
    {
        pHandler->callSimulinkCoSimExportGenerator(path.toStdString().c_str(), pSystem->getCoreSystemAccessPtr()->getCoreSystemPtr(), disablePortLabels, compiler, gpDesktopHandler->getCoreIncludePath().toStdString().c_str(), gpDesktopHandler->getExecPath().toStdString().c_str(), true);
        return true;
    }
    delete(pHandler);
    return false;
}


bool CoreGeneratorAccess::generateToLabViewSIT(QString path, SystemContainer *pSystem)
{
    hopsan::GeneratorHandler *pHandler = new hopsan::GeneratorHandler();
    if(pHandler->isLoadedSuccessfully())
    {
        pHandler->callLabViewSITGenerator(path.toStdString().c_str(), pSystem->getCoreSystemAccessPtr()->getCoreSystemPtr(), gpDesktopHandler->getCoreIncludePath().toStdString().c_str(), gpDesktopHandler->getExecPath().toStdString().c_str(), true);
        return true;
    }
    delete(pHandler);
    return false;
}

void CoreGeneratorAccess::generateLibrary(QString path, QStringList hppFiles)
{
    hopsan::GeneratorHandler *pHandler = new hopsan::GeneratorHandler();
    if(pHandler->isLoadedSuccessfully())
    {
        std::vector<hopsan::HString> hppList;
        for(int i=0; i<hppFiles.size(); ++i)
        {
            hppList.push_back(hppFiles[i].toStdString().c_str());
        }
        pHandler->callLibraryGenerator(path.toStdString().c_str(), hppList, true);
    }
}


bool CoreGeneratorAccess::compileComponentLibrary(QString path, QString extraLibs, bool showDialog)
{
    hopsan::GeneratorHandler *pHandler = new hopsan::GeneratorHandler();
    if(pHandler->isLoadedSuccessfully())
    {
        pHandler->callComponentLibraryCompiler(path.toStdString().c_str(), extraLibs.toStdString().c_str(), gpDesktopHandler->getCoreIncludePath().toStdString().c_str(), gpDesktopHandler->getExecPath().toStdString().c_str(), showDialog);
        return true;
    }
    delete(pHandler);
    return false;
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

void CoreLibraryAccess::getLibraryContents(QString libPath, QStringList &rComponents, QStringList &rNodes)
{
    std::vector<hopsan::HString> components, nodes;
    gHopsanCore.getExternalLibraryContents(libPath.toStdString().c_str(), components, nodes);

    rComponents.clear();
    rComponents.reserve(components.size());
    for (unsigned int i=0; i<components.size(); ++i)
    {
        rComponents.push_back(components[i].c_str());
    }

    rNodes.clear();
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
    return mpCoreComponentSystem->getSubComponentSystem(name.toStdString().c_str());
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


size_t CoreSystemAccess::getNSamples()
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
    return mpCoreComponentSystem->getSubComponent(componentName.toStdString().c_str())->setParameterValue(parameterName.toStdString().c_str(), value.toStdString().c_str(), force);
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
            data.mUnit = pDescs->at(i).mUnit.c_str();
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

bool CoreSystemAccess::setVariableAlias(QString compName, QString portName, QString varName, QString alias)
{
    return mpCoreComponentSystem->getAliasHandler().setVariableAlias(alias.toStdString().c_str(), compName.toStdString().c_str(),
                                                              portName.toStdString().c_str(), varName.toStdString().c_str());
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


void CoreSystemAccess::removeSubComponent(QString componentName, bool doDelete)
{
    mpCoreComponentSystem->removeSubComponent(componentName.toStdString().c_str(), doDelete);
}


vector<double> CoreSystemAccess::getTimeVector(QString componentName, QString portName)
{
    //qDebug() << "getTimeVector, " << componentName << ", " << portName;
    hopsan::Component* pComp = mpCoreComponentSystem->getSubComponentOrThisIfSysPort(componentName.toStdString().c_str());
    if (pComp != 0)
    {
        hopsan::Port* pPort = pComp->getPort(portName.toStdString().c_str());
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
    hopsan::ComponentSystem *pTempComponentSystem = gHopsanCore.createComponentSystem();
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


void CoreSystemAccess::loadParameterFile(QString fileName)
{
    mpCoreComponentSystem->loadParameters(fileName.toStdString().c_str());
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
            vector< vector<double> > *pData = pPort->getLogDataVectorPtr();
            rpTimeVector = pPort->getLogTimeVectorPtr();

            // Instead of pData.size() lets ask for latest logsample, this way we can avoid coping log slots that have not bee written and contains junk
            // This is usefull when a simulation has been aborted
            size_t nElements = min(pPort->getComponent()->getSystemParent()->getNumActuallyLoggedSamples(), pData->size());
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


bool CoreSystemAccess::getLastNodeData(const QString compname, const QString portname, const QString dataname, double& rData)
{
    int dataId = -1;
    hopsan::Port* pPort = this->getCorePortPtr(compname, portname);
    if (pPort)
    {
        dataId = pPort->getNodeDataIdFromName(dataname.toStdString().c_str());

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
        return;     //! @todo Do something better, so the user understands why this won't work (TBB is not installed)
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
hopsan::Port* CoreSystemAccess::getCorePortPtr(QString componentName, QString portName)
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


bool CoreSystemAccess::setSystemParameter(const CoreParameterData &rParameter, bool force)
{
    return mpCoreComponentSystem->setSystemParameter(rParameter.mName.toStdString().c_str(),
                                                     rParameter.mValue.toStdString().c_str(),
                                                     rParameter.mType.toStdString().c_str(),
                                                     rParameter.mDescription.toStdString().c_str(),
                                                     rParameter.mUnit.toStdString().c_str(),
                                                     force);
}


bool CoreSystemAccess::setSystemParameterValue(QString name, QString value, bool force)
{
    return mpCoreComponentSystem->setParameterValue(name.toStdString().c_str(), value.toStdString().c_str(), force);
}

//! @brief Get the value of a parameter in the system
//! @returns The aprameter value as a QString or "" if parameter not found
QString CoreSystemAccess::getSystemParameterValue(const QString name)
{
    hopsan::HString value;
    mpCoreComponentSystem->getParameterValue(name.toStdString().c_str(), value);
    return QString(value.c_str());
}


//! @todo Dont know if this is actually used
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
                data.mAlias = pPort->getVariableAlias(i).c_str();
                data.mDescription = pDescs->at(i).description.c_str();
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



NodeInfo::NodeInfo(QString nodeType)
{
    hopsan::Node *pNode = gHopsanCore.createNode(nodeType.toStdString().c_str());
    if(!pNode) return;

    niceName = pNode->getNiceName().c_str();
    for(size_t i=0; i<pNode->getDataDescriptions()->size(); ++i)
    {
        if(pNode->getDataDescription(i)->varType == hopsan::DefaultType ||
           pNode->getDataDescription(i)->varType == hopsan::IntensityType ||
           pNode->getDataDescription(i)->varType == hopsan::FlowType)        //Q variable
        {
            qVariables << pNode->getDataDescription(i)->shortname.c_str();
            variableLabels << QString(pNode->getDataDescription(i)->name.c_str())/*.toUpper()*/;
            varIdx << pNode->getDataDescription(i)->id;
        }
        if(pNode->getDataDescription(i)->varType == hopsan::IntensityType)
        {
            intensity = QString(pNode->getDataDescription(i)->name.c_str());
        }
        if(pNode->getDataDescription(i)->varType == hopsan::FlowType)
        {
            flow = QString(pNode->getDataDescription(i)->name.c_str());
        }
    }
    for(size_t i=0; i<pNode->getDataDescriptions()->size(); ++i)
    {
        if(pNode->getDataDescription(i)->varType == hopsan::TLMType)        //C variable
        {
            cVariables << pNode->getDataDescription(i)->shortname.c_str();
            variableLabels << QString(pNode->getDataDescription(i)->name.c_str())/*.toUpper()*/;
            varIdx << pNode->getDataDescription(i)->id;
        }
    }

    gHopsanCore.removeNode(pNode);
    //delete(pNode);
}

void NodeInfo::getNodeTypes(QStringList &nodeTypes)
{
    nodeTypes << "NodeMechanic" << "NodeMechanicRotational" << "NodeHydraulic" << "NodePneumatic" << "NodeElectric";
}

QString getHopsanCoreVersion()
{
    return QString::fromStdString(gHopsanCore.getCoreVersion());
}


CoreCSVParserAccess::CoreCSVParserAccess(QString file)
{
    bool success;
    mpParser = new hopsan::CSVParser(success, hopsan::HString(file.toStdString().c_str()));
    if(!success)
    {
        mpParser = 0;
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

QVector<double> CoreCSVParserAccess::getColumn(int col)
{
    if(mpParser)
        return QVector<double>::fromStdVector(mpParser->getDataColumn(col));
    else
        return QVector<double>();
}

