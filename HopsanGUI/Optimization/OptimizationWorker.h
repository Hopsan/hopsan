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
//! @file   OptimizationWorker.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-02-13
//! @version $Id: OptimizationHandler.cpp 6525 2014-01-30 15:58:59Z petno25 $
//!
//! @brief Contains a base class for optimization worker objects
//!

#ifndef OPTIMIZATIONWORKER_H
#define OPTIMIZATIONWORKER_H

#include <QObject>
#include <QVector>

class OptimizationHandler;
class ModelWidget;

class OptimizationWorker : public QObject
{
    Q_OBJECT

    friend class OptimizationHandler;
public:
    OptimizationWorker(OptimizationHandler *pHandler);

    virtual void init();
    virtual void run();
    virtual void finalize();

    bool checkForConvergence();
    void calculateBestAndWorstId();

    void plotPoints();
    void plotObjectiveFunctionValues();
    void plotParameters();

    virtual void setOptVar(const QString &var, const QString &value);
    virtual double getOptVar(const QString &var, bool &ok);

    void setParMin(int idx, double value);
    void setParMax(int idx, double value);

    void setOptimizationObjectiveValue(int idx, double value);
    double getOptimizationObjectiveValue(int idx);
    virtual double getParameter(const int pointIdx, const int parIdx) const;
    double getMaxParDiff();

    //Public members
    //! @todo These should not be required
    QVector<ModelWidget *> mModelPtrs;

protected:
    void print(const QString &msg, const QString &tag="", bool timeStamp=true);
    void printError(const QString &msg, const QString &tag="", bool timeStamp=true);
    void execute(const QString &cmd);

    OptimizationHandler *mpHandler;

    //Used to remember if connections with model widget was disconnected before optimization and should be re-connected
    bool mDisconnectedFromModelHandler;

    int mNumPoints;
    int mEvalId;
    int mNumParameters;
    QVector<double> mParMin, mParMax;
    QVector< QVector<double> > mParameters;
    QVector<double> mObjectives;
    double mMaxEvals;
    int mWorstId, mBestId, mLastWorstId;
    int mConvergenceReason;
    double mParTol, mFuncTol;
    bool mPlotPoints;
    bool mPlotObjectiveFunctionValues;
    bool mPlotParameters;
    bool mPlotVariables;

    int mTotalIterations;
};

#endif // OPTIMIZATIONWORKER_H
