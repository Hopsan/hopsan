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
#include "MainWindow.h"
#include "Widgets/MessageWidget.h"
#include "Widgets/ProjectTabWidget.h"
#include "Widgets/PlotWidget.h"
#include "GUIObjects/GUIModelObject.h"
#include "GUIObjects/GUISystem.h"
#include "GUIPort.h"
#include "Configuration.h"
#include "PlotWindow.h"
#include "Widgets/PyDockWidget.h"
#include "Widgets/SystemParametersWidget.h"


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
    QString plot(GUIPort* o, const QString& dataName)
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

    double getLastValue(GUIPort* o, const QString& dataName)
    {
        double data;

        if(!o->getLastNodeData(dataName, data))
            return  -1.0; //! @todo this is not very smart
        else
            return data;
    }

    QVector<double> getDataVector(GUIPort* o, const QString& dataName)
    {
        QPair<QVector<double>, QVector<double> > yData;
        o->getParentContainerObjectPtr()->getCoreSystemAccessPtr()->getPlotData(o->getGuiModelObject()->getName(),o->getPortName(),dataName,yData);

        return yData.second;
    }

    QVector<double> getTimeVector(GUIPort* o)
    {
        QVector<double> tVector = QVector<double>::fromStdVector(o->getParentContainerObjectPtr()->getCoreSystemAccessPtr()->getTimeVector(o->getGuiModelObject()->getName(),o->getPortName()));

        return tVector;
    }

};


class PyGUIObjectClassWrapper : public QObject
{
    Q_OBJECT

public slots:
    double getParameter(GUIModelObject* o, const QString& parName)
    {
        QString strParValue = o->getParameterValue(parName);

        return strParValue.toDouble(); //! @todo This is not good if parameter is not double
    }

    void setParameter(GUIModelObject* o, const QString& parName, const double& value)
    {
        o->setParameterValue(parName, QString::number(value));
    }

    GUIPort* port(GUIModelObject* o, const QString& portName)
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

    GUIObject* component(MainWindow* o, const QString& compName)
    {
        return o->mpProjectTabs->getCurrentTopLevelSystem()->getGUIModelObject(compName);
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
        bool previousProgressBarSetting = o->mpConfig->getEnableProgressBar();
        o->mpConfig->setEnableProgressBar(false);
        bool success = o->mpProjectTabs->getCurrentTab()->simulate();
        o->mpConfig->setEnableProgressBar(previousProgressBarSetting);
        return success;
    }

    double getParameter(MainWindow* o, const QString& compName, const QString& parName)
    {
        if(o->mpProjectTabs->getCurrentTopLevelSystem()->hasGUIModelObject(compName))
        {
            QString strParValue = o->mpProjectTabs->getCurrentTopLevelSystem()->getGUIModelObject(compName)->getParameterValue(parName);
            return strParValue.toDouble(); //! @todo Not good if parameter not double
        }
        assert(false);
        return 0;
    }

    void setParameter(MainWindow* o, const QString& compName, const QString& parName, const double& value)
    {
        if(o->mpProjectTabs->getCurrentTopLevelSystem()->hasGUIModelObject(compName))
        {
            o->mpProjectTabs->getCurrentTopLevelSystem()->getGUIModelObject(compName)->setParameterValue(parName, QString::number(value));
        }
    }

    void setSystemParameter(MainWindow* o, const QString& parName, const double& value)
    {
        QString valueString;
        valueString.setNum(value);
            o->mpProjectTabs->getCurrentContainer()->getCoreSystemAccessPtr()->setSystemParameter(parName, valueString, "", "", "double");
        o->mpSystemParametersWidget->update();
    }

    void plot(MainWindow* o, const QString& compName, const QString& portName, const QString& dataName)
    {
        o->mpProjectTabs->getCurrentTopLevelSystem()->getGUIModelObject(compName)->getPort(portName)->plot(dataName, "");
    }

    void plot(MainWindow* o, const QString &portAlias)
    {
        QStringList variableDescription = o->mpProjectTabs->getCurrentContainer()->getPlotVariableFromAlias(portAlias);
        if(!variableDescription.isEmpty())
        {
            QString compName = variableDescription.at(0);
            QString portName = variableDescription.at(1);
            QString dataName = variableDescription.at(2);
            o->mpProjectTabs->getCurrentTopLevelSystem()->getGUIModelObject(compName)->getPort(portName)->plot(dataName, "");
        }
        else
        {
            //! @todo Write a message in the python console that the port with specified alias was not found
        }
    }

    void plotToWindow(MainWindow* o, const int& generation, const QString& compName, const QString& portName, const QString& dataName, const int& windowNumber)
    {
        o->mpPlotWidget->mpPlotParameterTree->getPlotWindow(windowNumber)->addPlotCurve(generation, compName, portName, dataName);
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
