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
//! @version $Id$
//!
//! @brief Contains a base class for optimization worker objects
//!

#ifndef OPTIMIZATIONWORKER_H
#define OPTIMIZATIONWORKER_H

#include <QObject>
#include <QVector>
#include <QFile>
#include <QStringList>
#include "LogVariable.h"

class OptimizationHandler;
class ModelWidget;

class OptimizationWorker : public QObject
{
    Q_OBJECT

    friend class OptimizationHandler;
public:
    OptimizationWorker(OptimizationHandler *pHandler);
    ~OptimizationWorker();

    virtual void init();
    virtual void run();
    virtual void finalize();

    virtual void printLogFile();

    virtual bool checkForConvergence();
    void calculateBestAndWorstId();

    void plotPoints();
    void plotObjectiveFunctionValues();
    void plotParameters();
    void plotEntropy();

    virtual void setOptVar(const QString &var, const QString &value);
    virtual double getOptVar(const QString &var, bool &ok);

    void setParMin(int idx, double value);
    void setParMax(int idx, double value);

    void setOptimizationObjectiveValue(int idx, double value);
    double getOptimizationObjectiveValue(int idx);
    virtual double getParameter(const int pointIdx, const int parIdx) const;
    double getMaxParDiff();
    double getMaxParDiff(QVector<QVector<double> > &points);

    QStringList *getParNamesPtr();

    void clearModels();

    //Public members
    //! @todo These should not be required
    QVector<ModelWidget *> mModelPtrs;

protected:
    void print(const QString &msg);
    void print(const QString &msg, const QString &tag, bool timeStamp=true);
    void printError(const QString &msg);
    void printError(const QString &msg, const QString &tag, bool timeStamp=true);
    void execute(const QString &cmd);
    void updateProgressBar(int i);

    OptimizationHandler *mpHandler;

    //Used to remember if connections with model widget was disconnected before optimization and should be re-connected
    bool mDisconnectedFromModelHandler;

    int mNumPoints;
    QVector<int> mvIdx;
    bool mDoLog;
    bool mFinalEval;
    QFile mLogFile;
    int mEvalId;
    int mNumParameters;
    QStringList mParNames;
    QVector<double> mParMin, mParMax;
    QVector< QVector<double> > mParameters;
    QVector<double> mObjectives;
    double mMaxEvals;
    int mWorstId, mBestId, mLastWorstId, mSecondBestId;
    int mConvergenceReason;
    double mParTol, mFuncTol;
    bool mPlotPoints;
    bool mPlotObjectiveFunctionValues;
    bool mPlotParameters;
    bool mPlotVariables;
    bool mPlotEntropy;
    int mPercent;

    int mIterations;
    int mEvaluations;
    int mMetaModelEvaluations;
    void logPoint(int idx);
    void logWorstPoint();
    void logAllPoints();

    bool mOrgProgressBarSetting;
    bool mOrgLimitDataGenerationsSetting;

    //Plotting points
    QList<SharedVectorVariableT> mPointVars_x;
    QList<SharedVectorVariableT> mPointVars_y;

    //Plotting best/worst/newest objective
    SharedVectorVariableT mBestVar;
    SharedVectorVariableT mWorstVar;
    SharedVectorVariableT mNewestVar;

    //Plotting parameters
    QList<SharedVectorVariableT> mParVars;

    //Plotting entropy
    SharedVectorVariableT mEntropyVar;
};

#endif // OPTIMIZATIONWORKER_H
