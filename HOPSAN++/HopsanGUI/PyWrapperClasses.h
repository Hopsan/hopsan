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

//class PyLogVariableDataWrapper : public QObject
//{
//    Q_OBJECT
//public slots:

//};

class PyLogDataHandlerClassWrapper : public QObject
{
    Q_OBJECT
public slots:
    QString addVariables(LogDataHandler* o, const QString &a, const QString &b);
    QString subVariables(LogDataHandler* o, const QString &a, const QString &b);
    QString multVariables(LogDataHandler* o, const QString &a, const QString &b);
    QString divVariables(LogDataHandler* o, const QString &a, const QString &b);
    QString assignVariables(LogDataHandler* o, const QString &a, const QString &b);
    bool pokeVariables(LogDataHandler* o, const QString &a, const int index, const double value);
    double peekVariables(LogDataHandler* o, const QString &varName, const int index);
    QString delVariables(LogDataHandler* o, const QString &a);
    QString saveVariables(LogDataHandler* o, const QString &currName, const QString &newName);
    QString addVariablesWithScalar(LogDataHandler* o, const QString &VarName, const int &ScaName);
    QString subVariablesWithScalar(LogDataHandler* o, const QString &VarName, const int &ScaName);
    QString multVariablesWithScalar(LogDataHandler* o, const QString &VarName, const int &ScaName);
    QString divVariablesWithScalar(LogDataHandler* o, const QString &VarName, const int &ScaName);
    QVector<double> data(LogDataHandler* o, const QString fullName);
    //LogVariableData* getcurrentVariable(LogDataHandler* o, QString name);
    //LogVariableContainer getcurrentContainer(LogDataHandler* o, QString name);
};


class PyPortClassWrapper : public QObject
{
    Q_OBJECT
public slots:
    QString plot(Port* o, const QString& dataName);
    double lastData(Port* o, const QString& dataName);
    QVector<double> data(Port* o, const QString& dataName);
    QVector<double> time(Port* o);
    QStringList variableNames(Port* o);
};


class PyModelObjectClassWrapper : public QObject
{
    Q_OBJECT

public slots:
    double parameter(ModelObject* o, const QString& parName);
    void setParameter(ModelObject* o, const QString& parName, const double& value);
    void setParameter(ModelObject* o, const QString& parName, const QString& value);
    Port* port(ModelObject* o, const QString& portName);
    QStringList portNames(ModelObject* o);
};


class PyMainWindowClassWrapper : public QObject
{
    Q_OBJECT

public slots:
    void newModel(MainWindow* o);
    void loadModel(MainWindow* o, const QString& modelFileName);
    void closeAllModels(MainWindow* o);
    void gotoTab(MainWindow* o, int tab);
    void printMessage(MainWindow* o, const QString& message);
    void printInfo(MainWindow* o, const QString& message);
    void printWarning(MainWindow* o, const QString& message);
    void printError(MainWindow* o, const QString& message);
    ModelObject* component(MainWindow* o, const QString& compName);
    void setStartTime(MainWindow* o, const double& start);
    void setTimeStep(MainWindow* o, const double& timestep);
    void setFinishTime(MainWindow* o, const double& stop);
    double getStartTime(MainWindow* o);
    double getTimeStep(MainWindow* o);
    double getFinishTime(MainWindow* o);
    bool simulate(MainWindow* o);
    bool simulateAllOpenModels(MainWindow* o, bool modelsHaveNotChanged);
    double getParameter(MainWindow* o, const QString& compName, const QString& parName);
    void setParameter(MainWindow* o, const QString& compName, const QString& parName, const double& value);
    void setParameter(MainWindow* o, const QString& compName, const QString& parName, const QString& value);
    void setSystemParameter(MainWindow* o, const QString& parName, const double& value);
    QString addComponent(MainWindow* o, const QString& name, const QString& typeName, const int& x, const int& y, const int& rot);
    QString addComponent(MainWindow* o, const QString& name, const QString& typeName, const QString& subTypeName, const int& x, const int& y, const int& rot);
    bool connect(MainWindow* o, const QString& comp1, const QString& port1, const QString& comp2, const QString& port2);
    void enterSystem(MainWindow* o, const QString& sysName);
    void exitSystem(MainWindow* o);
    void clear(MainWindow* o);
    void plot(MainWindow* o, const QString& compName, const QString& portName, const QString& dataName);
    void plot(MainWindow* o, const QString &portAlias);
    //! @todo maybe need a version for alias too,
    void plotToWindow(MainWindow* o, const int& generation, const QString& compName, const QString& portName, const QString& dataName, const QString& windowName);
    void offset(MainWindow* o, const QString aliasName, const double value, const int gen=-1);
    void savePlotData(MainWindow* o, const QString& fileName, const QString &windowName);
    int getSimulationTime(MainWindow* o);
    void useMultiCore(MainWindow* o);
    void useSingleCore(MainWindow* o);
    void setNumberOfThreads(MainWindow* o, const int& value);
    void turnOnProgressBar(MainWindow* o);
    void turnOffProgressBar(MainWindow* o);
    QStringList componentNames(MainWindow* o);
    LogDataHandler* getLogDataHandler(MainWindow* o);
};

#endif // PYWRAPPERCLASSES_H
