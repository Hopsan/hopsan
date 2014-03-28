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
#include <QPointer>
#include <QStringList>
#include <QVector>

// Qt Forward declaritions
class QProgressDialog;
class GUIMessageHandler;

// Hopsan Forward declarations
class VectorVariable;
class ModelObject;
class Port;
class PlotWindow;

class PythonHopsanInterface : public QObject
{
    Q_OBJECT
public:
    PythonHopsanInterface(GUIMessageHandler *pPythonMessageHandler);

public slots:
    // Messages
    void printMessage(const QString& message);
    void printInfo(const QString& message);
    void printWarning(const QString& message);
    void printError(const QString& message);

    // Simulation
    void setStartTime(const QString start);
    void setTimeStep(const QString timestep);
    void setFinishTime(const QString stop);
    double startTime();
    double timeStep();
    double finishTime();

    bool simulate();
    int getSimulationTime();

    void openAbortDialog(const QString &text);
    bool isAborted();
    void abort();

    // Simulation options
    void useMultiCore(const bool tf);
    void setNumberOfThreads(const int numThreads);
    void enableProgressBar(const bool tf);

    // Model loading and closing
    void newModel();
    void loadModel(const QString& rModelFileName); //!< @todo should return a model wraper
    void closeAllModels();

    // Model navigation
    void gotoTab(int tab); //!< @todo should take modelwrapper as input
    void enterSystem(const QString& rSysName);
    void exitSystem();

    // Model access
    ModelObject* component(const QString &rCompName);
    QStringList componentNames();

    // Parameter access
    double parameter(const QString &rCompName, const QString &rParName);
    void setParameter(const QString &rCompName, const QString &rParName, const double value);
    void setParameter(const QString &rCompName, const QString &rParName, const QString &value);
    void setSystemParameter(const QString &rSysParName, const double value);

    // Modell manipulation
    ModelObject* addComponent(const QString& rName, const QString& rTypeName, const int x, const int y, const int rot=0);
    ModelObject* addComponent(const QString& rName, const QString& rTypeName, const QString& rSubTypeName, const int x, const int y, const int rot=0);
    bool connect(const QString& rComp1, const QString& rPort1, const QString& rComp2, const QString& rPort2);
    void clearComponents();

    // Plot functions
    void plot(const QString& rCompName, const QString& rPortName, const QString& rDataName, const int gen=-1);
    void plot(const QString& rName, const int gen=-1);
    void plot(const VectorVariable* pVariable);
    void figure(const QString& rName);
    void savePlotDataCSV(const QString& rFileName);
    void savePlotDataPLO(const QString& rFileName);

    // Log data access
    VectorVariable* getVariable(const QString& rCompName, const QString& rPortName, const QString& rDataName, const int gen=-1);
    VectorVariable* getVariable(const QString& rName, const int gen=-1);

    VectorVariable* addVectorVariable(const QString& rName, QVector<double> &rData);
    VectorVariable* addTimeVariable(const QString& rName, QVector<double> &rTime, QVector<double> &rData);
    VectorVariable* addFrequencyVariable(const QString& rName, QVector<double> &rFrequency, QVector<double> &rData);

private:

    QPointer<PlotWindow> mpPlotWindow;
    bool mAbort;
    QProgressDialog *mpAbortDialog;
    GUIMessageHandler *mpPythonMessageHandler;
};


class PyVectorVariableClassWrapper : public QObject
{
    Q_OBJECT
public slots:
    QString variableType(VectorVariable* o) const;

    QVector<double> time(VectorVariable* o);
    QVector<double> frequency(VectorVariable* o);
    QVector<double> data(VectorVariable* o);
    double peek(VectorVariable* o, const int index);

    bool poke(VectorVariable* o, const int index, const double value);
    void assign(VectorVariable* o, const QVector<double> &rSrc);
    void assign(VectorVariable* o, const QVector<double> &rSrcX, const QVector<double> &rSrcY);
};

class PyPortClassWrapper : public QObject
{
    Q_OBJECT
public slots:
    void plot(Port* o, const QString& rDataName);
    double lastData(Port* o, const QString& rDataName);
    QVector<double> data(Port* o, const QString& rDataName);
    QVector<double> time(Port* o);
    VectorVariable *variable(Port* o, const QString& rDataName);
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

#endif // PYWRAPPERCLASSES_H
