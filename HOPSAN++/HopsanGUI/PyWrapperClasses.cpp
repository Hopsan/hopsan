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

#include "PyWrapperClasses.h"
#include "Widgets/HcomWidget.h"
#include "PlotWindow.h"
#include "PlotTab.h"
#include "PlotHandler.h"

//Implementations are done in h-file right now
QString PyPortClassWrapper::plot(Port* o, const QString& dataName)
{
    QString res;
    if(!(o->plot(dataName)))
        res = "Sorry, for some reason this couldn't be plotted";
    else
    {
        res = "Plotted '"; //Kanske inte ska skriva massa skit till Pythonkonsollen
        res.append(dataName);
        res.append("' at component '");
        res.append(o->getParentModelObjectName());
        res.append("' and port '");
        res.append(o->getName());
        res.append("'.");
    }
    return res;
}
double PyPortClassWrapper::lastData(Port* o, const QString& dataName)
{
    double data;

    if(!o->getLastNodeData(dataName, data))
        return  -1.0; //! @todo this is not very smart
    else
        return data;
}
QVector<double> PyPortClassWrapper::data(Port* o, const QString& dataName)
{
    QVector<double> yData;
    std::vector<double> *pTime;
    o->getParentContainerObject()->getCoreSystemAccessPtr()->getPlotData(o->getParentModelObject()->getName(),o->getName(),dataName,pTime,yData);

    return yData;
}
QVector<double> PyPortClassWrapper::time(Port* o)
{
    QVector<double> tVector = QVector<double>::fromStdVector(o->getParentContainerObject()->getCoreSystemAccessPtr()->getTimeVector(o->getParentModelObject()->getName(),o->getName()));

    return tVector;
}
QStringList PyPortClassWrapper::variableNames(Port* o)
{
    QStringList retval;
    QString portName = o->getName();
    QString componentName = o->getParentModelObject()->getName();
    QVector<QString> dataNames;
    QVector<QString> dataUnits;
    o->getParentModelObject()->getParentContainerObject()->getCoreSystemAccessPtr()->getPlotDataNamesAndUnits(componentName, portName, dataNames, dataUnits);
    for(int i=0; i<dataNames.size(); ++i)
    {
        retval.append(dataNames.at(i));
    }
    return retval;
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
void PyMainWindowClassWrapper::newModel(MainWindow* o)
{
    o->mpProjectTabs->addNewProjectTab();
}
void PyMainWindowClassWrapper::loadModel(MainWindow* o, const QString& modelFileName)
{
    o->mpProjectTabs->loadModel(modelFileName, true);
}
void PyMainWindowClassWrapper::closeAllModels(MainWindow* o)
{
    o->mpProjectTabs->closeAllProjectTabs();
}
void PyMainWindowClassWrapper::gotoTab(MainWindow* o, int tab)
{
    o->mpProjectTabs->setCurrentIndex(tab);
}
void PyMainWindowClassWrapper::printMessage(MainWindow* o, const QString& message)
{
    o->mpTerminalWidget->mpConsole->printInfoMessage(QString("pyMessage: ").append(message));
    o->mpTerminalWidget->mpConsole->checkMessages();
}
void PyMainWindowClassWrapper::printInfo(MainWindow* o, const QString& message)
{
    o->mpTerminalWidget->mpConsole->printInfoMessage(QString("pyInfo: ").append(message));
    o->mpTerminalWidget->mpConsole->checkMessages();
}
void PyMainWindowClassWrapper::printWarning(MainWindow* o, const QString& message)
{
    o->mpTerminalWidget->mpConsole->printWarningMessage(QString("pyWarning: ").append(message));
    o->mpTerminalWidget->mpConsole->checkMessages();
}
void PyMainWindowClassWrapper::printError(MainWindow* o, const QString& message)
{
    o->mpTerminalWidget->mpConsole->printErrorMessage(QString("pyError: ").append(message));
    o->mpTerminalWidget->mpConsole->checkMessages();
}
ModelObject* PyMainWindowClassWrapper::component(MainWindow* o, const QString& compName)
{
    return o->mpProjectTabs->getCurrentContainer()->getModelObject(compName);
}
void PyMainWindowClassWrapper::setStartTime(MainWindow* o, const double& start)
{
    o->setStartTimeInToolBar(start);
}
void PyMainWindowClassWrapper::setTimeStep(MainWindow* o, const double& timestep)
{
    o->setTimeStepInToolBar(timestep);
}
void PyMainWindowClassWrapper::setFinishTime(MainWindow* o, const double& stop)
{
    o->setStopTimeInToolBar(stop);
}
double PyMainWindowClassWrapper::getStartTime(MainWindow* o)
{
    return o->getStartTimeFromToolBar();
}

double PyMainWindowClassWrapper::getTimeStep(MainWindow* o)
{
    return o->getTimeStepFromToolBar();
}

double PyMainWindowClassWrapper::getFinishTime(MainWindow* o)
{
    return o->getFinishTimeFromToolBar();
}

bool PyMainWindowClassWrapper::simulate(MainWindow* o)
{
    //bool previousProgressBarSetting = o->mpConfig->getEnableProgressBar();
    //o->mpConfig->setEnableProgressBar(false);
    bool success = o->mpProjectTabs->getCurrentTab()->simulate_blocking();
    //o->mpConfig->setEnableProgressBar(previousProgressBarSetting);
    //qApp->processEvents();
    return success;
}


bool PyMainWindowClassWrapper::simulateAllOpenModels(MainWindow* o, bool modelsHaveNotChanged)
{
    //bool previousProgressBarSetting = o->mpConfig->getEnableProgressBar();
    //o->mpConfig->setEnableProgressBar(false);
    bool success = o->mpProjectTabs->simulateAllOpenModels_blocking(modelsHaveNotChanged);
    //o->mpConfig->setEnableProgressBar(previousProgressBarSetting);
    //qApp->processEvents();
    return success;
}


double PyMainWindowClassWrapper::getParameter(MainWindow* o, const QString& compName, const QString& parName)
{
    if(o->mpProjectTabs->getCurrentContainer()->hasModelObject(compName))
    {
        QString strParValue = o->mpProjectTabs->getCurrentContainer()->getModelObject(compName)->getParameterValue(parName);
        return strParValue.toDouble(); //! @todo Not good if parameter not double
    }
    //assert(false);
    return 0;
}

void PyMainWindowClassWrapper::setParameter(MainWindow* o, const QString& compName, const QString& parName, const double& value)
{
    if(o->mpProjectTabs->getCurrentContainer()->hasModelObject(compName))
    {
        o->mpProjectTabs->getCurrentContainer()->getModelObject(compName)->setParameterValue(parName, QString::number(value));
    }
}

void PyMainWindowClassWrapper::setParameter(MainWindow* o, const QString& compName, const QString& parName, const QString& value)
{
    if(o->mpProjectTabs->getCurrentContainer()->hasModelObject(compName))
    {
        o->mpProjectTabs->getCurrentContainer()->getModelObject(compName)->setParameterValue(parName, value);
    }
}

void PyMainWindowClassWrapper::setSystemParameter(MainWindow* o, const QString& parName, const double& value)
{
    CoreParameterData paramData(parName, "", "double");
    paramData.mValue.setNum(value);
    o->mpProjectTabs->getCurrentContainer()->setOrAddParameter(paramData);
    o->mpSystemParametersWidget->update();
}

QString PyMainWindowClassWrapper::addComponent(MainWindow* o, const QString& name, const QString& typeName, const int& x, const int& y, const int& rot)
{
    ModelObjectAppearance *pAppearance = o->mpLibrary->getAppearanceData(typeName);
    if(!pAppearance)
        return "Could not find component type.";
    pAppearance->setDisplayName(name);
    ModelObject *pObj = o->mpProjectTabs->getCurrentContainer()->addModelObject(pAppearance, QPointF(x,y),rot);
    if(!pObj)
        return "Could not create component.";
    return pObj->getName();
}

QString PyMainWindowClassWrapper::addComponent(MainWindow* o, const QString& name, const QString& typeName, const QString& subTypeName, const int& x, const int& y, const int& rot)
{
    ModelObjectAppearance *pAppearance = o->mpLibrary->getAppearanceData(typeName, subTypeName);
    if(!pAppearance)
        return "Could not find component type.";
    pAppearance->setDisplayName(name);
    ModelObject *pObj = o->mpProjectTabs->getCurrentContainer()->addModelObject(pAppearance, QPointF(x,y),rot);
    if(!pObj)
        return "Could not create component.";
    return pObj->getName();
}


bool PyMainWindowClassWrapper::connect(MainWindow* o, const QString& comp1, const QString& port1, const QString& comp2, const QString& port2)
{
    Port *pPort1 = o->mpProjectTabs->getCurrentContainer()->getModelObject(comp1)->getPort(port1);
    Port *pPort2 = o->mpProjectTabs->getCurrentContainer()->getModelObject(comp2)->getPort(port2);
    Connector *pConn = o->mpProjectTabs->getCurrentContainer()->createConnector(pPort1, pPort2);

    if (pConn != 0)
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
    return false;
}


void PyMainWindowClassWrapper::enterSystem(MainWindow* o, const QString& sysName)
{
    ModelObject *sysObj = o->mpProjectTabs->getCurrentContainer()->getModelObject(sysName);
    SystemContainer *system = dynamic_cast<SystemContainer *>(sysObj);
    system->enterContainer();
}


void PyMainWindowClassWrapper::exitSystem(MainWindow* o)
{
    //o->mpProjectTabs->getCurrentContainer()->exitContainer();
    int id = o->mpProjectTabs->getCurrentTab()->getQuickNavigationWidget()->getCurrentId();
    o->mpProjectTabs->getCurrentTab()->getQuickNavigationWidget()->gotoContainerAndCloseSubcontainers(id-1);
}


void PyMainWindowClassWrapper::clear(MainWindow* o)
{
    while(!o->mpProjectTabs->getCurrentContainer()->getModelObjectNames().isEmpty())
    {
        o->mpProjectTabs->getCurrentContainer()->deleteModelObject(o->mpProjectTabs->getCurrentContainer()->getModelObjectNames().first());
    }
}


void PyMainWindowClassWrapper::plot(MainWindow* o, const QString& compName, const QString& portName, const QString& dataName)
{
    o->mpProjectTabs->getCurrentContainer()->getModelObject(compName)->getPort(portName)->plot(dataName, "");
    qApp->processEvents();
}

void PyMainWindowClassWrapper::plot(MainWindow* o, const QString &portAlias)
{
    QString fullName = o->mpProjectTabs->getCurrentContainer()->getLogDataHandler()->getFullNameFromAlias(portAlias);
    if (!fullName.isEmpty())
    {
        o->mpProjectTabs->getCurrentContainer()->getLogDataHandler()->plotVariable("", fullName, -1, 0);
    }
    qApp->processEvents();
}

//! @todo maybe need a version for alias too,
void PyMainWindowClassWrapper::plotToWindow(MainWindow* o, const int& generation, const QString& compName, const QString& portName, const QString& dataName, const QString& windowName)
{
    QString fullName = makeConcatName(compName, portName, dataName);
    o->mpProjectTabs->getCurrentContainer()->getLogDataHandler()->plotVariable(windowName, fullName, generation, 0);
}

void  PyMainWindowClassWrapper::offset(MainWindow* o, const QString varName, const double value, const int gen)
{
    SharedLogVariableDataPtrT pData = o->mpProjectTabs->getCurrentContainer()->getLogDataHandler()->getPlotData(varName, gen);
    if (pData)
    {
        pData->setValueOffset(value);
    }
    //! @todo what about error message if not found
}

void PyMainWindowClassWrapper::savePlotData(MainWindow* o, const QString& fileName, const QString& windowName)
{
    PlotWindow *pPW = gpPlotHandler->getPlotWindow(windowName);
    if (pPW)
    {
        pPW->getCurrentPlotTab()->exportToCsv(fileName);
    }
    //! @todo error message if not found
}

int PyMainWindowClassWrapper::getSimulationTime(MainWindow* o)
{
    return o->mpProjectTabs->getCurrentTab()->getLastSimulationTime();
}

void PyMainWindowClassWrapper::useMultiCore(MainWindow* o)
{
    o->mpConfig->setUseMultiCore(true);
}

void PyMainWindowClassWrapper::useSingleCore(MainWindow* o)
{
    o->mpConfig->setUseMultiCore(false);
}

void PyMainWindowClassWrapper::setNumberOfThreads(MainWindow* o, const int& value)
{
    o->mpConfig->setNumberOfThreads(value);
}

void PyMainWindowClassWrapper::turnOnProgressBar(MainWindow* o)
{
    o->mpConfig->setEnableProgressBar(true);
}

void PyMainWindowClassWrapper::turnOffProgressBar(MainWindow* o)
{
    o->mpConfig->setEnableProgressBar(false);
}

QStringList PyMainWindowClassWrapper::componentNames(MainWindow* o)
{
    return o->mpProjectTabs->getCurrentContainer()->getModelObjectNames();
}

LogDataHandler *PyMainWindowClassWrapper::getLogDataHandler(MainWindow *o)
{
    return o->mpProjectTabs->getCurrentContainer()->getLogDataHandler();
}

void PyMainWindowClassWrapper::openAbortDialog(MainWindow *o, const QString &text)
{
    mAbort=false;
    mpDialog = new QProgressDialog(text, "Abort",0,0, o);
    o->connect(mpDialog, SIGNAL(canceled()), this, SLOT(abort()));
    mpDialog->setModal(false);
    mpDialog->show();
}

bool PyMainWindowClassWrapper::isAborted(MainWindow *o)
{
    qApp->processEvents();
    return mAbort;
}

void PyMainWindowClassWrapper::abort(MainWindow* o)
{
    mAbort=true;
    if(mpDialog)
    {
        mpDialog->close();
    }
}

QString PyLogDataHandlerClassWrapper::addVariables(LogDataHandler* o, const QString &a, const QString &b)
{
    QString tempStr = o->addVariables(a,b);
    return tempStr;
}

QString PyLogDataHandlerClassWrapper::subVariables(LogDataHandler *o, const QString &a, const QString &b)
{
    QString tempStr = o->subVariables(a,b);
    return tempStr;
}

QString PyLogDataHandlerClassWrapper::multVariables(LogDataHandler *o, const QString &a, const QString &b)
{
    QString tempStr = o->multVariables(a,b);
    return tempStr;
}

QString PyLogDataHandlerClassWrapper::divVariables(LogDataHandler *o, const QString &a, const QString &b)
{
    QString tempStr = o->divVariables(a,b);
    return tempStr;
}

QString PyLogDataHandlerClassWrapper::assignVariables(LogDataHandler *o, const QString &a, const QString &b)
{
    QString tempStr = o->assignVariable(a,b);
    return tempStr;
}

bool PyLogDataHandlerClassWrapper::pokeVariables(LogDataHandler *o, const QString &a, const int index, const double value)
{
    return o->pokeVariable(a,index,value);
}

bool PyLogDataHandlerClassWrapper::delVariables(LogDataHandler *o, const QString &a)
{
    return o->deleteVariable(a);
}

QString PyLogDataHandlerClassWrapper::saveVariables(LogDataHandler *o, const QString &currName, const QString &newName)
{
    QString tempStr = o->saveVariable(currName, newName);
    return tempStr;
}

QString PyLogDataHandlerClassWrapper::addVariablesWithScalar(LogDataHandler *o, const QString &VarName, const int &ScaName)
{
    QString tempStr = o->addVariableWithScalar(VarName, ScaName);
    return tempStr;
}

QString PyLogDataHandlerClassWrapper::subVariablesWithScalar(LogDataHandler *o, const QString &VarName, const int &ScaName)
{
    QString tempStr = o->subVariableWithScalar(VarName, ScaName);
    return tempStr;
}

QString PyLogDataHandlerClassWrapper::multVariablesWithScalar(LogDataHandler *o, const QString &VarName, const int &ScaName)
{
    QString tempStr = o->mulVariableWithScalar(VarName, ScaName);
    return tempStr;
}

QString PyLogDataHandlerClassWrapper::divVariablesWithScalar(LogDataHandler *o, const QString &VarName, const int &ScaName)
{
    QString tempStr = o->divVariableWithScalar(VarName, ScaName);
    return tempStr;
}

double PyLogDataHandlerClassWrapper::peekVariables(LogDataHandler *o,const QString &varName, const int index)
{
    return o->peekVariable(varName,index);
}

QVector<double> PyLogDataHandlerClassWrapper::data(LogDataHandler *o, const QString fullName)
{
    return o->getPlotDataValues(fullName, -1);
}
