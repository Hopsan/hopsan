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
//! @file   SimulationHandler.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2012-05-25
//!
//! @brief Contains a class for running the simulation and signaling a progress bar
//!
//$Id$

#ifndef SIMULATIONHANDLER_H
#define SIMULATIONHANDLER_H

#include <QThread>
#include <QTimer>
#include <QProgressDialog>

// Forward Declaration
class SystemContainer;
class SimulationObject;

class SimulationObject : public QObject
{
    Q_OBJECT

private:
    QVector<SystemContainer*> mvpSystems;
    double mStartTime;
    double mStopTime;
    unsigned int mnLogSamples;
    bool mNoChanges;

public:
    SimulationObject(QVector<SystemContainer*> vpSystems, const double startTime, const double stopTime, const unsigned int nLogSamples, const bool noChanges)
        : mvpSystems(vpSystems), mStartTime(startTime), mStopTime(stopTime), mnLogSamples(nLogSamples), mNoChanges(noChanges){}
    void connectProgressDialog(QProgressDialog *pProgressDialog);

public slots:
    void doIt();

signals:
    void setProgressBarRange(int, int);
    void setProgressBarText(QString text);
    void initDone(bool, int);
    void simulateDone(bool, int);
    void finalizeDone(bool, int);
};

class ProgressBarObject : public QObject
{
    Q_OBJECT

private:
    QTimer mProgressDialogRefreshTimer;
    QProgressDialog *mpProgressDialog;
    SystemContainer *mpSystem; //!< @todo what about multiple systems
    int mLastProgressRefreshStep;
    double mStartT, mStopT;

protected slots:
    void refreshProgressBar();

public:
    ProgressBarObject(const double startTime, const double stopTime, const int refreshTime, SystemContainer *pSystem);
    ~ProgressBarObject();
    QProgressDialog *getProgressDialog();


signals:
    void setProgressBarValue(int);
};

class SimulationThreadHandler  : public QObject
{
    Q_OBJECT

public:
    SimulationThreadHandler() : mpSimulationObject(0), mpProgressBarObject(0){}

    void setSimulationTimevariables(const double startTime, const double stopTime, const unsigned int nLogSamples);
    void initSimulateFinalize(SystemContainer* pSystem, const bool noChanges=false);
    void initSimulateFinalize(QVector<SystemContainer*> vpSystems, const bool noChanges=false);
    bool wasSuccessful();

signals:
    void startSimulation();
    void done(bool);

protected slots:
    void initDone(bool success, int ms);
    void simulateDone(bool success, int ms);
    void finalizeDone(bool success, int ms);
    void abort();


protected:
    QVector<SystemContainer*> mvpSystems;

    SimulationObject *mpSimulationObject;
    ProgressBarObject *mpProgressBarObject;

    QThread mSimulationThread;
    QThread mProgressBarThread;

    double mStartT, mStopT;
    int mnTthreads, mnLogSamples;

    bool mInitSuccess, mSimuSucess, mFiniSucess, mAborted;
    int mInitTime, mSimuTime, mFiniTime;
};


#endif // SIMULATIONHANDLER_H
