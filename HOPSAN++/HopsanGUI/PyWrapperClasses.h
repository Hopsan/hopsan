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
            res.append(o->getName());
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
        QVector<double> yData;
        o->getParentContainerObjectPtr()->getCoreSystemAccessPtr()->getPlotData(o->getGuiModelObject()->getName(),o->getName(),dataName,yData);

        return yData;
    }

    QVector<double> getTimeVector(GUIPort* o)
    {
        QVector<double> tVector = QVector<double>::fromStdVector(o->getParentContainerObjectPtr()->getCoreSystemAccessPtr()->getTimeVector(o->getGuiModelObject()->getName(),o->getName()));

        return tVector;
    }

};


class PyGUIObjectClassWrapper : public QObject
{
    Q_OBJECT

public slots:
    double getParameter(GUIModelObject* o, const QString& parName)
    {
        return o->getParameterValue(parName);
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
        return o->mpProjectTabs->getCurrentTab()->simulate();
    }

    double getParameter(MainWindow* o, const QString& compName, const QString& parName)
    {
        if(o->mpProjectTabs->getCurrentTopLevelSystem()->hasGUIModelObject(compName))
        {
            return o->mpProjectTabs->getCurrentTopLevelSystem()->getGUIModelObject(compName)->getParameterValue(parName);
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

    void plot(MainWindow* o, const QString& compName, const QString& portName, const QString& dataName)
    {
        o->mpProjectTabs->getCurrentTopLevelSystem()->getGUIModelObject(compName)->getPort(portName)->plot(dataName);
    }

    void plotToWindow(MainWindow* o, const QString& compName, const QString& portName, const QString& dataName, const int& windowNumber)
    {
        o->mpPlotWidget->mpPlotParameterTree->getPlotWindow(windowNumber)->addPlotCurve(compName, portName, dataName);
    }

    void discardPlotGeneration(MainWindow* o, const int& windowNumber)
    {
        o->mpPlotWidget->mpPlotParameterTree->getPlotWindow(windowNumber)->discardGeneration();
    }

    void discardOldestPlotGeneration(MainWindow* o, const int& windowNumber)
    {
        o->mpPlotWidget->mpPlotParameterTree->getPlotWindow(windowNumber)->discardOldestGeneration();
    }

    int getSimulationTime(MainWindow* o)
    {
        return o->mpProjectTabs->getCurrentTab()->mLastSimulationTime;
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
