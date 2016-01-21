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
//! @file   Ops.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2015-08-31
//!
//! @brief Contains the optimization worker base class
//!
//$Id: SymHop.cc 8138 2015-06-17 12:27:23Z petno25 $

#ifndef OPSWORKER_H
#define OPSWORKER_H

#include <QVector>
#include <QString>
#include <QObject>
#include <QStringList>

namespace Ops {

enum AlgorithmT {Undefined, NelderMead, ComplexRF, ComplexRFP,  ParticleSwarm, DifferentialEvolution, ParameterSweep, ControlledRandomSearch, ComplexBurmen};
enum SamplingT {SamplingRandom, SamplingLatinHypercube};

class Evaluator;

class Worker : public QObject
{
    Q_OBJECT

    friend class Evaluator;
public:
    Worker(Evaluator *pEvaluator);
    ~Worker();

    virtual AlgorithmT getAlgorithm();

    virtual void initialize();
    virtual void run();
    virtual void finalize();

    void distrubteCandidatePoints();
    void distributePoints();
    virtual void distributePoints(QVector<QVector<double> > *pVector);

    virtual bool checkForConvergence();
    void calculateBestAndWorstId();
    QVector<int> getIdsSortedFromWorstToBest();
    int getBestId();
    int getWorstId();
    int getLastWorstId();



    void setParameterLimits(int idx, double min, double max);
    void getParameterLimits(int idx, double &min, double &max);

    void setCandidateObjectiveValue(int idx, double value);
    double getObjectiveValue(int idx);
    QVector<double> &getObjectiveValues();
    QVector<QVector<double> > &getPoints();
    QVector<QVector<double> > &getCandidatePoints();
    double getCandidateParameter(const int pointIdx, const int parIdx) const;
    double getParameter(const int pointIdx, const int parIdx) const;
    double getMaxPercentalParameterDiff();
    double getMaxPercentalParameterDiff(QVector<QVector<double> > &points);

    QStringList *getParameterNamesPtr();

    virtual void setNumberOfPoints(int value);
    void setNumberOfCandidates(int value);
    virtual void setNumberOfParameters(int value);
    void setParameterNames(QStringList names);
    void setConvergenceTolerance(double value);
    void setMaxNumberOfIterations(int value);
    void setTolerance(double value);
    void setSamplingMethod(SamplingT dist);

    int getNumberOfCandidates();
    int getNumberOfPoints();
    int getNumberOfParameters();
    int getMaxNumberOfIterations();
    int getCurrentNumberOfIterations();

    double opsRand();

public slots:
    void abort();

protected:
    int mIterationCounter;
    int mNumCandidates;
    int mNumPoints;
    int mNumParameters;
    QStringList mParameterNames;
    QVector<double> mParameterMin, mParameterMax;
    QVector< QVector<double> > mPoints;
    QVector< QVector<double> > mCandidatePoints;
    QVector<double> mCandidateObjectives;
    QVector<double> mObjectives;
    double mnMaxIterations;
    int mWorstId, mBestId, mLastWorstId, mSecondBestId;
    double mTolerance;
    SamplingT mDistribution;
    bool mIsAborted;
    Evaluator *mpEvaluator;

signals:
    void pointsChanged();
    void pointChanged(int idx);
    void candidatesChanged();
    void candidateChanged(int idx);
    void objectivesChanged();
    void objectiveChanged(int idx);
    void message(QString msg);
    void errorReceived(QString msg);
    void stepCompleted(int);

};

}

#endif // OPSWORKER_H
