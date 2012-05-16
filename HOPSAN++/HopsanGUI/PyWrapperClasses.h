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
//! @file   PyWrapperClasses.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-09-21
//!
//! @brief Contains a wrapper classes to expose parts of HopsanGUI to Python
//!
//$Id$

#ifndef PYWRAPPERCLASSES_H
#define PYWRAPPERCLASSES_H

#include <QObject>

#include "Configuration.h"
#include "GUIConnector.h"
#include "GUIPort.h"
#include "MainWindow.h"
#include "PlotWindow.h"
#include "Widgets/LibraryWidget.h"
#include "Widgets/MessageWidget.h"
#include "Widgets/ProjectTabWidget.h"
#include "Widgets/PlotWidget.h"
#include "Widgets/PyDockWidget.h"
#include "Widgets/SystemParametersWidget.h"
#include "GUIObjects/GUIModelObject.h"
#include "GUIObjects/GUISystem.h"

//Just for test purposes
class pyTestClass : public QObject
{
    Q_OBJECT

public:
    pyTestClass()
    {
        mpVector = new QVector<double>;
        mpVector->append(3.34);
        mpVector->append(4.45);
    }

public slots:
    QVector<double> getVector()
    {
        return *mpVector;
    }

private:
    QVector<double> *mpVector;
};


class PyGUIPortClassWrapper : public QObject
{
    Q_OBJECT

public slots:
    QString plot(Port* o, const QString& dataName)
    {
        QString res;
        if(!(o->plot(dataName)))
            res = "Sorry, for some reason this couldn't be plotted";
        else
        {
            res = "Plotted '"; //Kanske inte ska skriva massa skit till Pythonkonsollen
            res.append(dataName);
            res.append("' at component '");
            res.append(o->getGuiModelObjectName());
            res.append("' and port '");
            res.append(o->getPortName());
            res.append("'.");
        }


        return res;
    }

    double getLastValue(Port* o, const QString& dataName)
    {
        double data;

        if(!o->getLastNodeData(dataName, data))
            return  -1.0; //! @todo this is not very smart
        else
            return data;
    }

    QVector<double> getDataVector(Port* o, const QString& dataName)
    {
        QPair<QVector<double>, QVector<double> > yData;
        o->getParentContainerObjectPtr()->getCoreSystemAccessPtr()->getPlotData(o->getGuiModelObject()->getName(),o->getPortName(),dataName,yData);

        return yData.second;
    }

    QVector<double> getTimeVector(Port* o)
    {
        QVector<double> tVector = QVector<double>::fromStdVector(o->getParentContainerObjectPtr()->getCoreSystemAccessPtr()->getTimeVector(o->getGuiModelObject()->getName(),o->getPortName()));

        return tVector;
    }

};


class PyGUIObjectClassWrapper : public QObject
{
    Q_OBJECT

public slots:
    double getParameter(ModelObject* o, const QString& parName)
    {
        QString strParValue = o->getParameterValue(parName);

        return strParValue.toDouble(); //! @todo This is not good if parameter is not double
    }

    void setParameter(ModelObject* o, const QString& parName, const double& value)
    {
        o->setParameterValue(parName, QString::number(value));
    }

    Port* port(ModelObject* o, const QString& portName)
    {
        return o->getPort(portName);
    }
};


class PyHopsanClassWrapper : public QObject
{
    Q_OBJECT

public slots:
//    MainWindow* new_MainWindow(const double& number)
//    {
//        return new MainWindow(0);
//    }

//    void delete_MainWindow(MainWindow* o)
//    {
//        delete o;
//    }

    void newModel(MainWindow* o)
    {
        o->mpProjectTabs->addNewProjectTab();
    }

    void loadModel(MainWindow* o, const QString& modelFileName)
    {
        o->mpProjectTabs->loadModel(modelFileName, true);
    }

    void closeAllModels(MainWindow* o)
    {
        o->mpProjectTabs->closeAllProjectTabs();
    }

    void gotoTab(MainWindow* o, int tab)
    {
        o->mpProjectTabs->setCurrentIndex(tab);
    }

    void printMessage(MainWindow* o, const QString& message)
    {
        o->mpMessageWidget->printGUIInfoMessage(QString("pyMessage: ").append(message));
        o->mpMessageWidget->checkMessages();
    }

    void printInfo(MainWindow* o, const QString& message)
    {
        o->mpMessageWidget->printGUIInfoMessage(QString("pyInfo: ").append(message));
        o->mpMessageWidget->checkMessages();
    }

    void printWarning(MainWindow* o, const QString& message)
    {
        o->mpMessageWidget->printGUIWarningMessage(QString("pyWarning: ").append(message));
        o->mpMessageWidget->checkMessages();
    }

    void printError(MainWindow* o, const QString& message)
    {
        o->mpMessageWidget->printGUIErrorMessage(QString("pyError: ").append(message));
        o->mpMessageWidget->checkMessages();
    }

    ModelObject* component(MainWindow* o, const QString& compName)
    {
        return o->mpProjectTabs->getCurrentTopLevelSystem()->getModelObject(compName);
    }

    void setStartTime(MainWindow* o, const double& start)
    {
        o->setStartTimeInToolBar(start);
    }

    void setTimeStep(MainWindow* o, const double& timestep)
    {
        o->setTimeStepInToolBar(timestep);
    }

    void setFinishTime(MainWindow* o, const double& stop)
    {
        o->setStopTimeInToolBar(stop);
    }

    double getStartTime(MainWindow* o)
    {
        return o->getStartTimeFromToolBar();
    }

    double getTimeStep(MainWindow* o)
    {
        return o->getTimeStepFromToolBar();
    }

    double getFinishTime(MainWindow* o)
    {
        return o->getFinishTimeFromToolBar();
    }

    bool simulate(MainWindow* o)
    {
        bool previousProgressBarSetting = o->mpConfig->getEnableProgressBar();
        o->mpConfig->setEnableProgressBar(false);
        bool success = o->mpProjectTabs->getCurrentTab()->simulate();
        o->mpConfig->setEnableProgressBar(previousProgressBarSetting);
        qApp->processEvents();
        return success;
    }


    bool simulateAllOpenModels(MainWindow* o, bool modelsHaveNotChanged)
    {
        bool previousProgressBarSetting = o->mpConfig->getEnableProgressBar();
        o->mpConfig->setEnableProgressBar(false);
        bool success = o->mpProjectTabs->simulateAllOpenModels(modelsHaveNotChanged);
        o->mpConfig->setEnableProgressBar(previousProgressBarSetting);
        qApp->processEvents();
        return success;
    }


    double getParameter(MainWindow* o, const QString& compName, const QString& parName)
    {
        if(o->mpProjectTabs->getCurrentTopLevelSystem()->hasModelObject(compName))
        {
            QString strParValue = o->mpProjectTabs->getCurrentTopLevelSystem()->getModelObject(compName)->getParameterValue(parName);
            return strParValue.toDouble(); //! @todo Not good if parameter not double
        }
        assert(false);
        return 0;
    }

    void setParameter(MainWindow* o, const QString& compName, const QString& parName, const double& value)
    {
        if(o->mpProjectTabs->getCurrentTopLevelSystem()->hasModelObject(compName))
        {
            o->mpProjectTabs->getCurrentTopLevelSystem()->getModelObject(compName)->setParameterValue(parName, QString::number(value));
        }
    }

    void setSystemParameter(MainWindow* o, const QString& parName, const double& value)
    {
        CoreParameterData paramData(parName, "", "double");
        paramData.mValue.setNum(value);
        o->mpProjectTabs->getCurrentContainer()->setOrAddParameter(paramData);
        o->mpSystemParametersWidget->update();
    }

    QString addComponent(MainWindow* o, const QString& name, const QString& typeName, const int& x, const int& y, const int& rot)
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


    bool connect(MainWindow* o, const QString& comp1, const QString& port1, const QString& comp2, const QString& port2)
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


    void enterSystem(MainWindow* o, const QString& sysName)
    {
        ModelObject *sysObj = o->mpProjectTabs->getCurrentContainer()->getModelObject(sysName);
        SystemContainer *system = dynamic_cast<SystemContainer *>(sysObj);
        system->enterContainer();
    }


    void exitSystem(MainWindow* o)
    {
        //o->mpProjectTabs->getCurrentContainer()->exitContainer();
        int id = o->mpProjectTabs->getCurrentTab()->getQuickNavigationWidget()->getCurrentId();
        o->mpProjectTabs->getCurrentTab()->getQuickNavigationWidget()->gotoContainerAndCloseSubcontainers(id-1);
    }


    void clear(MainWindow* o)
    {
        while(!o->mpProjectTabs->getCurrentContainer()->getModelObjectNames().isEmpty())
        {
            o->mpProjectTabs->getCurrentContainer()->deleteModelObject(o->mpProjectTabs->getCurrentContainer()->getModelObjectNames().first());
        }
    }


    void plot(MainWindow* o, const QString& compName, const QString& portName, const QString& dataName)
    {
        o->mpProjectTabs->getCurrentTopLevelSystem()->getModelObject(compName)->getPort(portName)->plot(dataName, "");
        qApp->processEvents();
    }

    void plot(MainWindow* o, const QString &portAlias)
    {
        QStringList variableDescription = o->mpProjectTabs->getCurrentContainer()->getPlotVariableFromAlias(portAlias);
        if(!variableDescription.isEmpty())
        {
            QString compName = variableDescription.at(0);
            QString portName = variableDescription.at(1);
            QString dataName = variableDescription.at(2);
            o->mpProjectTabs->getCurrentTopLevelSystem()->getModelObject(compName)->getPort(portName)->plot(dataName, "");
        }
        else
        {
            //! @todo Write a message in the python console that the port with specified alias was not found
        }
        qApp->processEvents();
    }

    void plotToWindow(MainWindow* o, const int& generation, const QString& compName, const QString& portName, const QString& dataName, const int& windowNumber)
    {
        o->mpPlotWidget->mpPlotVariableTree->getPlotWindow(windowNumber)->addPlotCurve(generation, compName, portName, dataName);
    }

    void savePlotData(MainWindow* o, const QString& fileName, const int& windowNumber)
    {
        o->mpPlotWidget->mpPlotVariableTree->getPlotWindow(windowNumber)->getCurrentPlotTab()->exportToCsv(fileName);
    }

    void closeLastPlotWindow(MainWindow* o)
    {
        o->mpPlotWidget->mpPlotVariableTree->closeLastPlotWindow();
    }

    void refreshLastPlotWindow(MainWindow* o)
    {
        o->mpPlotWidget->mpPlotVariableTree->refreshLastPlotWindow();
    }

    int getSimulationTime(MainWindow* o)
    {
        return o->mpProjectTabs->getCurrentTab()->getLastSimulationTime();
    }

    void useMultiCore(MainWindow* o)
    {
        o->mpConfig->setUseMultiCore(true);
    }

    void useSingleCore(MainWindow* o)
    {
        o->mpConfig->setUseMultiCore(false);
    }

    void setNumberOfThreads(MainWindow* o, const int& value)
    {
        o->mpConfig->setNumberOfThreads(value);
    }

    void turnOnProgressBar(MainWindow* o)
    {
        o->mpConfig->setEnableProgressBar(true);
    }

    void turnOffProgressBar(MainWindow* o)
    {
        o->mpConfig->setEnableProgressBar(false);
    }


};

#endif // PYWRAPPERCLASSES_H
