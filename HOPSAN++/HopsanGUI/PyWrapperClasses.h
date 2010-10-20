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
#include "MainWindow.h"
#include "MessageWidget.h"
#include "ProjectTabWidget.h"
#include "GUIObject.h"
#include "GUISystem.h"
#include "GUIPort.h"


class PyGUIPortClassWrapper : public QObject
{
    Q_OBJECT

public slots:
    void plot(GUIPort* o, const QString& dataName)
    {
        o->plot(dataName);
    }

    double getLastValue(GUIPort* o, const QString& dataName)
    {
        double data;

        if(!o->getLastNodeData(dataName, data))
            return  -1.0;
        else
            return data;
    }

};


class PyGUIObjectClassWrapper : public QObject
{
    Q_OBJECT

public slots:
    double getParameter(GUIObject* o, const QString& parName)
    {
        return o->getParameterValue(parName);
    }

    void setParameter(GUIObject* o, const QString& parName, const double& value)
    {
        o->setParameterValue(parName, value);
    }

    GUIPort* port(GUIObject* o, const QString& portName)
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

    void loadModel(MainWindow* o, const QString& modelFileName)
    {
        o->mpProjectTabs->loadModel(modelFileName);
    }

    void printMessage(MainWindow* o, const QString& message)
    {
        o->mpMessageWidget->printGUIMessage(QString("pyMessage: ").append(message));
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

    GUIObject* component(MainWindow* o, const QString& compName)
    {
        return o->mpProjectTabs->getCurrentSystem()->getGUIObject(compName);
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
        o->setFinishTimeInToolBar(stop);
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
        return o->mpProjectTabs->getCurrentTab()->simulate();
    }

    double getParameter(MainWindow* o, const QString& compName, const QString& parName)
    {
        return o->mpProjectTabs->getCurrentSystem()->getGUIObject(compName)->getParameterValue(parName);
    }

    void setParameter(MainWindow* o, const QString& compName, const QString& parName, const double& value)
    {
        o->mpProjectTabs->getCurrentSystem()->getGUIObject(compName)->setParameterValue(parName, value);
    }

    void plot(MainWindow* o, const QString& compName, const QString& portName, const QString& dataName)
    {
        o->mpProjectTabs->getCurrentSystem()->getGUIObject(compName)->getPort(portName)->plot(dataName);
    }

};

#endif // PYWRAPPERCLASSES_H
