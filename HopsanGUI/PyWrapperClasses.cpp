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
//! @file   PyWrapperClasses.cpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-09-21
//!
//! @brief Contains a wrapper classes to expose parts of HopsanGUI to Python
//!
//$Id$

#include <QProgressDialog>

#include "PyWrapperClasses.h"

#include "global.h"
#include "common.h"

#include "Configuration.h"
#include "GUIConnector.h"
#include "GUIPort.h"
#include "PlotWindow.h"
#include "PlotTab.h"
#include "Widgets/ModelWidget.h"
#include "Widgets/PlotWidget.h"
#include "Widgets/SystemParametersWidget.h"
#include "GUIObjects/GUIModelObject.h"
#include "GUIObjects/GUISystem.h"
#include "Widgets/QuickNavigationWidget.h"

#include "LibraryHandler.h"
#include "PlotHandler.h"
#include "ModelHandler.h"
#include "MessageHandler.h"

// Help functions
ContainerObject *getCurrentViewcontainerObject()
{
    return gpModelHandler->getCurrentViewContainerObject();
}


void PyPortClassWrapper::plot(Port* o, const QString& rDataName)
{
    o->plot(rDataName);
}

double PyPortClassWrapper::lastData(Port* o, const QString& rDataName)
{
    double data;
    if(!o->getLastNodeData(rDataName, data))
        return  -1.0; //!< @todo this is not very smart
    else
        return data;
}

QVector<double> PyPortClassWrapper::data(Port* o, const QString& rDataName)
{
    SharedVectorVariableT pVar = o->getParentContainerObject()->getLogDataHandler()->
            getVectorVariable(makeFullVariableName(o->getParentModelObject()->getParentSystemNameHieararchy(), o->getParentModelObjectName(),
                                                   o->getName(), rDataName),-1);
    if (pVar)
    {
        return pVar->getDataVectorCopy();
    }
    return QVector<double>();
}

QVector<double> PyPortClassWrapper::time(Port* o)
{
    QStringList vars = variableNames(o);
    if (!vars.empty())
    {
        SharedVectorVariableT pVar = o->getParentContainerObject()->getLogDataHandler()->
                getVectorVariable(makeFullVariableName(o->getParentModelObject()->getParentSystemNameHieararchy(), o->getParentModelObjectName(),
                                                       o->getName(), vars.first()), -1);
        if (pVar)
        {
            if (pVar->getSharedTimeOrFrequencyVector())
            {
                return pVar->getSharedTimeOrFrequencyVector()->getDataVectorCopy();
            }
        }
    }
    return QVector<double>();
}

VectorVariable *PyPortClassWrapper::variable(Port *o, const QString &rDataName)
{
    return getCurrentViewcontainerObject()->getLogDataHandler()->
            getVectorVariable(makeFullVariableName(o->getParentModelObject()->getParentSystemNameHieararchy(), o->getParentModelObjectName(),
                                                   o->getName(), rDataName), -1).data();
}

QStringList PyPortClassWrapper::variableNames(Port* o)
{
    return o->getVariableNames();
}

double PyModelObjectClassWrapper::parameter(ModelObject* o, const QString& parName)
{
    QString strParValue = o->getParameterValue(parName);

    return strParValue.toDouble(); //! @todo This is not good if parameter is not double
}

void PyModelObjectClassWrapper::setParameter(ModelObject* o, const QString& parName, const double& value)
{
    o->setParameterValue(parName, QString::number(value));
}

void PyModelObjectClassWrapper::setParameter(ModelObject* o, const QString& parName, const QString& value)
{
    o->setParameterValue(parName, value);
}

Port* PyModelObjectClassWrapper::port(ModelObject* o, const QString& portName)
{
    return o->getPort(portName);
}

QStringList PyModelObjectClassWrapper::portNames(ModelObject* o)
{
    QStringList retval;
    for(int i=0; i<o->getPortListPtrs().size(); ++i)
    {
        retval.append(o->getPortListPtrs().at(i)->getName());
    }
    return retval;
}


void PythonHopsanInterface::setStartTime(const QString start)
{
    ModelWidget *pM = gpModelHandler->getCurrentModel();
    if (pM)
    {
        pM->setTopLevelSimulationTime(start, pM->getTimeStep(), pM->getStopTime());
    }
}

void PythonHopsanInterface::setTimeStep(const QString timestep)
{
    ModelWidget *pM = gpModelHandler->getCurrentModel();
    if (pM)
    {
        pM->setTopLevelSimulationTime(pM->getStartTime(), timestep, pM->getStopTime());
    }
}

void PythonHopsanInterface::setFinishTime(const QString stop)
{
    ModelWidget *pM = gpModelHandler->getCurrentModel();
    if (pM)
    {
        pM->setTopLevelSimulationTime(pM->getStartTime(), pM->getTimeStep(), stop);
    }
}

double PythonHopsanInterface::startTime()
{
    ModelWidget *pM = gpModelHandler->getCurrentModel();
    if (pM)
    {
        pM->getStartTime().toDouble();
    }
    return 0;
}

double PythonHopsanInterface::timeStep()
{
    ModelWidget *pM = gpModelHandler->getCurrentModel();
    if (pM)
    {
        return pM->getTimeStep().toDouble();
    }
    return 0;
}

double PythonHopsanInterface::finishTime()
{
    ModelWidget *pM = gpModelHandler->getCurrentModel();
    if (pM)
    {
        return pM->getStopTime().toDouble();
    }
    return 0;
}

bool PythonHopsanInterface::simulate()
{
    ModelWidget *pM = gpModelHandler->getCurrentModel();
    if (pM)
    {
        return pM->simulate_blocking();
    }
    return false;
}

int PythonHopsanInterface::getSimulationTime()
{
    ModelWidget *pM = gpModelHandler->getCurrentModel();
    if (pM)
    {
        return pM->getLastSimulationTime();
    }
    return 0;
}

void PythonHopsanInterface::openAbortDialog(const QString &text)
{
    mAbort=false;
    if (!mpAbortDialog)
    {
        mpAbortDialog = new QProgressDialog(text, "Abort", 0, 0, gpMainWindowWidget);
        QObject::connect(mpAbortDialog, SIGNAL(canceled()), this, SLOT(abort()));
        mpAbortDialog->setModal(false);
    }
    else
    {
        mpAbortDialog->setWindowTitle(text);
    }
    mpAbortDialog->show();
}

bool PythonHopsanInterface::isAborted()
{
    qApp->processEvents();
    return mAbort;
}

void PythonHopsanInterface::abort()
{
    mAbort=true;
    if(mpAbortDialog)
    {
        mpAbortDialog->close();
    }
}

void PythonHopsanInterface::useMultiCore(const bool tf)
{
    gpConfig->setBoolSetting(CFG_MULTICORE, tf);
}

void PythonHopsanInterface::setNumberOfThreads(const int numThreads)
{
    gpConfig->setIntegerSetting(CFG_NUMBEROFTHREADS, numThreads);
}

void PythonHopsanInterface::enableProgressBar(const bool tf)
{
    gpConfig->setBoolSetting(CFG_PROGRESSBAR, tf);
}

void PythonHopsanInterface::newModel()
{
    gpModelHandler->addNewModel();
}

void PythonHopsanInterface::loadModel(const QString &rModelFileName)
{
    gpModelHandler->loadModel(rModelFileName);
}

void PythonHopsanInterface::closeAllModels()
{
    gpModelHandler->closeAllModels();
}

void PythonHopsanInterface::gotoTab(int tab)
{
    gpModelHandler->setCurrentModel(tab);
}

void PythonHopsanInterface::enterSystem(const QString &rSysName)
{
    SystemContainer *pSystem = qobject_cast<SystemContainer*>(getCurrentViewcontainerObject()->getModelObject(rSysName));
    if (pSystem)
    {
        pSystem->enterContainer();
    }
    else
    {
        printError("No such sub-system to enter: "+rSysName);
    }
}

void PythonHopsanInterface::exitSystem()
{
    ContainerObject *pContainer = getCurrentViewcontainerObject();
    // We do not want to exit the top-level system
    if (!pContainer->isTopLevelContainer())
    {
        //pContainer->exitContainer();
        QuickNavigationWidget *pQNW = gpModelHandler->getCurrentModel()->getQuickNavigationWidget();
        int id = pQNW->getCurrentId();
        pQNW->gotoContainerAndCloseSubcontainers(id-1);
    }
}

ModelObject *PythonHopsanInterface::component(const QString &rCompName)
{
    return getCurrentViewcontainerObject()->getModelObject(rCompName);
}

QStringList PythonHopsanInterface::componentNames()
{
    return getCurrentViewcontainerObject()->getModelObjectNames();
}

double PythonHopsanInterface::parameter(const QString &rCompName, const QString &rParName)
{
    ModelObject *pObject = getCurrentViewcontainerObject()->getModelObject(rCompName);
    if(pObject)
    {
        QString strParValue = pObject->getParameterValue(rParName);
        return strParValue.toDouble(); //! @todo Not good if parameter not double
    }
    return -1;
}

void PythonHopsanInterface::setParameter(const QString &rCompName, const QString &rParName, const double value)
{
    setParameter(rCompName, rParName, QString::number(value));
}

void PythonHopsanInterface::setParameter(const QString &rCompName, const QString &rParName, const QString &value)
{
    ModelObject *pObject = getCurrentViewcontainerObject()->getModelObject(rCompName);
    if(pObject)
    {
        pObject->setParameterValue(rParName, value);
    }
}

void PythonHopsanInterface::setSystemParameter(const QString &rSysParName, const double value)
{
    CoreParameterData paramData(rSysParName, "", "double");
    paramData.mValue.setNum(value);
    getCurrentViewcontainerObject()->setOrAddParameter(paramData);
    gpSystemParametersWidget->update(); //!< @todo this should not be needed, container should signal widget (do not know if it does that right now)
}

ModelObject *PythonHopsanInterface::addComponent(const QString &rName, const QString &rTypeName, const int x, const int y, const int rot)
{
    return addComponent(rName, rTypeName, "", x, y, rot);
}

ModelObject *PythonHopsanInterface::addComponent(const QString &rName, const QString &rTypeName, const QString &rSubTypeName, const int x, const int y, const int rot)
{
    ModelObjectAppearance *pAppearance = gpLibraryHandler->getModelObjectAppearancePtr(rTypeName, rSubTypeName);
    if(!pAppearance)
    {
        printError("Could not find component type");
        return 0;
    }
    pAppearance->setDisplayName(rName);
    ModelObject *pObj = getCurrentViewcontainerObject()->addModelObject(pAppearance, QPointF(x,y), rot);
    if(!pObj)
    {
       printError("Could not create component.");
    }
    return pObj;
}

bool PythonHopsanInterface::connect(const QString &rComp1, const QString &rPort1, const QString &rComp2, const QString &rPort2)
{
    ModelObject *pComp1 = component(rComp1);
    ModelObject *pComp2 = component(rComp2);
    if (pComp1 && pComp2)
    {
        Port* pPort1 = pComp1->getPort(rPort1);
        Port* pPort2 = pComp2->getPort(rPort2);
        if (pPort1 && pPort2)
        {
            Connector *pConn = getCurrentViewcontainerObject()->createConnector(pPort1, pPort2);
            if (pConn)
            {
                QVector<QPointF> pointVector;
                pointVector.append(pPort1->pos());
                pointVector.append(pPort2->pos());

                QStringList geometryList;
                geometryList.append("diagonal");

                pConn->setPointsAndGeometries(pointVector, geometryList);
                pConn->refreshConnectorAppearance();

                //! @todo Register undo!
                return true;
            }
        }
        else
        {
            printError("At least one port was not found");
        }
    }
    else
    {
        printError("At least one component was not found");
    }
    printError("Failed to create connector");
    return false;
}

void PythonHopsanInterface::clearComponents()
{
    QStringList names = componentNames();
    Q_FOREACH(QString name, names)
    {
        getCurrentViewcontainerObject()->deleteModelObject(name);
    }
}

void PythonHopsanInterface::plot(const QString &rCompName, const QString &rPortName, const QString &rDataName, const int gen)
{
    plot(makeFullVariableName(getCurrentViewcontainerObject()->getParentSystemNameHieararchy(), rCompName, rPortName, rDataName), gen);
}

void PythonHopsanInterface::plot(const QString &rName, const int gen)
{
    getCurrentViewcontainerObject()->getLogDataHandler()->plotVariable(mpPlotWindow, rName, gen, 0);
    qApp->processEvents();
}

void PythonHopsanInterface::plot(const VectorVariable *pVariable)
{
    if (pVariable)
    {
        plot(pVariable->getFullVariableName(), pVariable->getGeneration());
    }
}

void PythonHopsanInterface::figure(const QString &rName)
{
    mpPlotWindow = gpPlotHandler->createNewPlotWindowOrGetCurrentOne(rName);
}

void PythonHopsanInterface::savePlotDataPLO(const QString &rFileName)
{
    if (mpPlotWindow)
    {
        mpPlotWindow->getCurrentPlotTab()->exportToPLO();
    }
}

void PythonHopsanInterface::savePlotDataCSV(const QString &rFileName)
{
    if (mpPlotWindow)
    {
        mpPlotWindow->getCurrentPlotTab()->exportToCsv(rFileName);
    }
}

VectorVariable *PythonHopsanInterface::getVariable(const QString &rCompName, const QString &rPortName, const QString &rDataName, const int gen)
{
    return getVariable(makeFullVariableName(getCurrentViewcontainerObject()->getParentSystemNameHieararchy(), rCompName, rPortName, rDataName), gen);
}

VectorVariable *PythonHopsanInterface::getVariable(const QString &rName, const int gen)
{
    return getCurrentViewcontainerObject()->getLogDataHandler()->getVectorVariable(rName, gen).data();
}

VectorVariable *PythonHopsanInterface::addVectorVariable(const QString &rName, QVector<double> &rData)
{
    SharedVectorVariableT pVar = getCurrentViewcontainerObject()->getLogDataHandler()->defineNewVectorVariable(rName);
    pVar->assignFrom(rData);
    return pVar.data();
}

VectorVariable *PythonHopsanInterface::addTimeVariable(const QString &rName, QVector<double> &rTime, QVector<double> &rData)
{
    SharedVectorVariableT pVar = getCurrentViewcontainerObject()->getLogDataHandler()->defineNewVectorVariable(rName, TimeDomainType);
    pVar->assignFrom(rTime, rData);
    return pVar.data();
}

VectorVariable *PythonHopsanInterface::addFrequencyVariable(const QString &rName, QVector<double> &rFrequency, QVector<double> &rData)
{
    SharedVectorVariableT pVar = getCurrentViewcontainerObject()->getLogDataHandler()->defineNewVectorVariable(rName, FrequencyDomainType);
    pVar->assignFrom(rFrequency, rData);
    return pVar.data();
}

PythonHopsanInterface::PythonHopsanInterface(GUIMessageHandler *pPythonMessageHandler) :
    QObject(0)
{
    mpPlotWindow = 0;
    mpAbortDialog = 0;
    mAbort = false;
    mpPythonMessageHandler = pPythonMessageHandler;
}

void PythonHopsanInterface::printMessage(const QString& message)
{
    if (mpPythonMessageHandler)
    {
        mpPythonMessageHandler->addInfoMessage(QString("(Python)  ").append(message));
        mpPythonMessageHandler->collectHopsanCoreMessages();
    }
}

void PythonHopsanInterface::printInfo(const QString& message)
{
    if (mpPythonMessageHandler)
    {
        mpPythonMessageHandler->addInfoMessage(QString("(Python)  ").append(message));
        mpPythonMessageHandler->collectHopsanCoreMessages();
    }
}

void PythonHopsanInterface::printWarning(const QString& message)
{
    if (mpPythonMessageHandler)
    {
        mpPythonMessageHandler->addWarningMessage(QString("(Python)  ").append(message));
        mpPythonMessageHandler->collectHopsanCoreMessages();
    }
}

void PythonHopsanInterface::printError(const QString& message)
{
    if (mpPythonMessageHandler)
    {
        mpPythonMessageHandler->addErrorMessage(QString("(Python)  ").append(message));
        mpPythonMessageHandler->collectHopsanCoreMessages();
    }
}


QString PyVectorVariableClassWrapper::variableType(VectorVariable* o) const
{
    return variableTypeAsString(o->getVariableType());
}

QVector<double> PyVectorVariableClassWrapper::time(VectorVariable *o)
{
    if (o->getVariableType() == TimeDomainType)
    {
        return o->getSharedTimeOrFrequencyVector()->getDataVectorCopy();
    }
    else
    {
        return QVector<double>();
    }
}

QVector<double> PyVectorVariableClassWrapper::frequency(VectorVariable *o)
{
    if (o->getVariableType() == FrequencyDomainType)
    {
        return o->getSharedTimeOrFrequencyVector()->getDataVectorCopy();
    }
    else
    {
        return QVector<double>();
    }
}

QVector<double> PyVectorVariableClassWrapper::data(VectorVariable *o)
{
    return o->getDataVectorCopy();
}

double PyVectorVariableClassWrapper::peek(VectorVariable *o, const int index)
{
    return o->peekData(index);
}

bool PyVectorVariableClassWrapper::poke(VectorVariable *o, const int index, const double value)
{
    QString err;
    return o->pokeData(index, value, err);
}

void PyVectorVariableClassWrapper::assign(VectorVariable *o, const QVector<double> &rSrc)
{
    o->assignFrom(rSrc);
}

void PyVectorVariableClassWrapper::assign(VectorVariable *o, const QVector<double> &rSrcX, const QVector<double> &rSrcY)
{
    o->assignFrom(rSrcX, rSrcY);
}
