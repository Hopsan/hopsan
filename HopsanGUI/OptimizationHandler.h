/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

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

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   OptimizationHandler.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2013-08-02
//! @version $Id$
//!
//! @brief Contains a handler for optimizations
//!
//$Id$

#ifndef OPTIMIZATIONHANDLER_H
#define OPTIMIZATIONHANDLER_H

//Qt includes
#include <QVector>
#include <QStringList>
#include "LogVariable.h"
#include "OpsWorker.h"
#include "OpsEvaluator.h"
#include "OpsMessageHandler.h"
#include "qdatetime.h"

//Forward declarations
class ModelWidget;
class TerminalWidget;
class HcomHandler;
class Configuration;
class OptimizationWorker;
class GUIMessageHandler;
class OptimizationHandler;


class OptimizationEvaluator : public Ops::Evaluator
{
public:
    OptimizationEvaluator(OptimizationHandler *pHandler);
    //void evaluateAllPoints();
    void evaluateCandidate(size_t idx);
    void evaluateAllCandidates();
private:
    OptimizationHandler *mpHandler;
};


class OptimizationMessageHandler : public QObject, public Ops::MessageHandler
{
    Q_OBJECT
public:
    OptimizationMessageHandler(OptimizationHandler *pHandler);
    void printMessage(char *str);
    void pointsChanged();
    void pointChanged(size_t idx);
    void objectivesChanged();
    void objectiveChanged(size_t idx);
    void candidatesChanged();
    void candidateChanged(size_t idx);
    void stepCompleted(size_t steps) override;

public slots:
    void abort();

private:
    OptimizationHandler *mpHandler;
};

// Forward declaration
class RemoteSimulationQueueHandler;

class OptimizationHandler : public QObject
{
    Q_OBJECT

    friend class HcomHandler;           //! @todo Should probably not be friends
    friend class OptimizationDialog;    //! @todo Should probably not be friends
    friend class OptimizationMessageHandler;    //! @note Ok to be friends due to inherited class from Ops
public:
    //Enums
    enum DataT{Integer, Double};
    enum AlgorithmT{NelderMead, ComplexRF, ComplexRFM, ComplexRFP, PSO, ParameterSweep, Uninitialized};
    enum PointPlotContentT{AllPoints, Candidates};

    //Constructor
    OptimizationHandler(HcomHandler *pHandler);

    //Public access functions
    void startOptimization(ModelWidget *pModel, QString &modelPath);
    void initModels(ModelWidget *pModel, int nModels, QString &modelPath);
    void setCandidateObjectiveValue(int idx, double value);
    void setParameterLimits(int idx, double min, double max);
    double getObjectiveValue(int idx);
    double getOptVar(const QString &var);
    double getOptVar(const QString &var, bool &ok) const;
    void setOptVar(const QString &var, const QString &value, bool &ok);
    double getCandidateParameter(const int pointIdx, const int parIdx) const;
    double getParameter(const int pointIdx, const int parIdx) const;
    void setIsRunning(bool value);
    bool isRunning();
    QStringList getOptParNamesPtr();
    void setStartValue(const int pointIdx, const int parIdx, const double value);

    bool evaluateCandidate(int idx);
    bool evaluateAllCandidates();

    const QVector<ModelWidget *> *getModelPtrs() const;
    void clearModels();
    void addModel(ModelWidget *pModel);

    void clearPlotVariables();

    Ops::AlgorithmT getAlgorithm() const;
    GUIMessageHandler *getMessageHandler();

    Configuration *mpConfig;
    HcomHandler *mpHcomHandler;
    bool mDisconnectedFromModelHandler;

    DataT mParameterType; //! @todo Should be public
    int mEvalId;
    int mEvaluations;

protected slots:

    void updateProgressBar(int i);
    void printResultFile();
    void printLogFile();
    void printDebugFile();
    void checkIfRescheduleIsNeeded();

public slots:
    //! @note These are public because they are used in optimization message handler
    void plotPoints(PointPlotContentT content=AllPoints);
    void updateOutputs();
    void plotParameters();
    void logAllPoints();
    void plotEntropy();
    void logPoint(int idx);
    void plotObjectiveValues();

private:
    void reInitialize(int nModels);
    void rescheduleForBestSpeedup(int &pm, int &pa, double &su, bool doBenchmark=false);

    QString mModelPath;
    GUIMessageHandler *mpMessageHandler;
    OptimizationEvaluator *mpEvaluator;
    OptimizationMessageHandler *mpOpsMessageHandler;
    Ops::Worker *mpWorker;
    AlgorithmT mAlgorithm;
    bool mIsRunning;
    QDateTime mStartTime;

    QVector<ModelWidget *> mModelPtrs;

    //Plotting points
    bool mPlotPoints;
    QList<SharedVectorVariableT> mPointVars_x;
    QList<SharedVectorVariableT> mPointVars_y;

    //Plotting best/worst/newest objective
    bool mPlotObjectiveValues;
    SharedVectorVariableT mBestVar;
    SharedVectorVariableT mWorstVar;
    SharedVectorVariableT mNewestVar;

    //Plotting parameters
    bool mPlotParameters;
    QList<SharedVectorVariableT> mParVars;

    //Plotting entropy
    bool mPlotEntropy;
    QVector<double> mEntropy;
    SharedVectorVariableT mEntropyVar;

    bool mPrintLogFile, mPrintResultFile, mPrintDebugFile;

    int mCurrentProgressBarPercent;

    QVector< QVector<double> > mLoggedParameters;

    bool mNeedsRescheduling;
    RemoteSimulationQueueHandler *mpRemoteSimulationQueueHandler=0;

    bool mOrgSetPwdToMwdSetting;
    bool mOrgProgressBarSetting;
    bool mOrgLimitDataGenerationsSetting;
};

#endif // OPTIMIZATIONHANDLER_H
