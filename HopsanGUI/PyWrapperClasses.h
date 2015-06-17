/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
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

// Qt Forward declarations
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
    void loadModel(const QString& rModelFileName); //!< @todo should return a model wrapper
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
