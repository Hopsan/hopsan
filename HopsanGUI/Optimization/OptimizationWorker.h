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

    virtual void init(const ModelWidget *pModel, const QString &modelPath);
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

    virtual void setOptimizationObjectiveValue(int idx, double value);
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

    int mNumModels;
    int mNumPoints;
    QVector<int> mvIdx;
    bool mDoLog;
    bool mFinalEval;
    QFile mLogFile;
    int mEvalId;
    int mNumParameters;
    QVector< QVector<double> > mLoggedParameters;
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
    bool mNoOutput;

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
