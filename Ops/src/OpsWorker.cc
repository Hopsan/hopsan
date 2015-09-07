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

#include <assert.h>
#include <math.h>
#include <QDebug>
#include "OpsWorker.h"
#include "OpsEvaluator.h"

using namespace Ops;

//! @brief Checks for convergence (in either of the algorithms)
Worker::Worker(Evaluator *pEvaluator)
{
    mpEvaluator = pEvaluator;
    mpEvaluator->setWorker(this);

    mNumCandidates = 1;
    mNumPoints = 1;
    mNumParameters = 1;

    mCandidateObjectives.resize(1);
    mCandidatePoints.resize(1);
    mObjectives.resize(1);
    mPoints.resize(1);
    mDistribution = SamplingRandom;
}

Worker::~Worker()
{
}

AlgorithmT Worker::getAlgorithm()
{
    return Undefined;
}


//! @brief Initialization function for optimization worker base class (should never be called directly)
void Worker::initialize()
{
    mBestId = 0;
    mWorstId = 1;
    mIsAborted = false;
}


//! @brief Run function for optimization worker base class (should never be called directly)
void Worker::run()
{
    assert("You should implement your own run() function." == 0);
}


//! @brief Finalize function for optimization worker base class (should never be called directly)
void Worker::finalize()
{
}


void Worker::distributePoints()
{
    if(mDistribution == SamplingRandom)
    {
        for(int p=0; p<mNumPoints; ++p)
        {
            for(int i=0; i<mNumParameters; ++i)
            {
                double r = (double)rand() / (double)RAND_MAX;
                mPoints[p][i] = mParameterMin[i] + r*(mParameterMax[i]-mParameterMin[i]);
            }
        }
    }
    else if(mDistribution == SamplingLatinHypercube)
    {
        //DEBUG
        int m=mNumPoints;
        int n=mNumParameters;

        QList<QVector<int> > usedIntervals;
        QList<QVector<double> > points;
        for(int i=0; i<m; ++i)
        {
            QVector<double> newPoint;
            QVector<int> interval;
            for(int j=0; j<n; ++j)
            {
                double min = mParameterMin[j];
                double max = mParameterMax[j];
                double x = min+double(qrand())/double(RAND_MAX)*(max-min);
                newPoint.append(x);
                //interval.append((max-min)/m),fmod(newPoint.last());
                interval.append(int(floor(x/(max-min)*m)));
            }
            if(usedIntervals.contains(interval))
            {
                --i;
                continue;
            }
            else
            {
                usedIntervals.append(interval);
                mPoints[i] = newPoint;
            }
        }
        qDebug() << "INTERVALS: " << usedIntervals;
        qDebug() << "POINTS: " << mPoints;
        //END DEBUG
    }

    emit pointsChanged();
}


//! @brief Checks whether or not any of the convergence criteria has been fulfilled
bool Worker::checkForConvergence()
{
    return (getMaxPercentalParameterDiff() < mTolerance);
}



//! @brief Calculates indexes of best and worst point
void Worker::calculateBestAndWorstId()
{
    mLastWorstId = mWorstId;

    double maxObj = mObjectives[0];
    double minObj = mObjectives[0];
    mWorstId=0;
    mBestId=0;
    for(int i=1; i<mNumPoints; ++i)
    {
        double obj = mObjectives[i];
        if(obj > maxObj)
        {
            maxObj = obj;
            mWorstId = i;
        }
        if(obj < minObj)
        {
            minObj = obj;
            mBestId = i;
        }
    }
    if(mWorstId == mBestId)
    {
        mWorstId = 0;
        mBestId = 1;
    }
}

QVector<int> Worker::getIdsSortedFromWorstToBest()
{
    //Sort ids by objective value (worst to best)
    QVector<int> ids;
    while(ids.size() != mNumPoints)
    {
        int worstId = 0;
        double worstObjective = -1000000000;
        for(int i=0; i<mNumPoints; ++i)
        {
            if(ids.contains(i)) continue;  //Ignore already added indexes

            if(mObjectives[i] > worstObjective)
            {
                worstObjective = mObjectives[i];
                worstId = i;
            }
        }
        ids.append(worstId);
    }

    return ids;
}


int Worker::getBestId()
{
    return mBestId;
}

int Worker::getWorstId()
{
    return mWorstId;
}

int Worker::getLastWorstId()
{
    return mLastWorstId;
}



void Worker::setNumberOfPoints(int value)
{
    mNumPoints = value;

    mPoints.resize(mNumPoints);
    mObjectives.resize(mNumPoints);
    for(int p=0; p<mNumPoints; ++p)
    {
        mPoints[p].resize(mNumParameters);
    }
}


void Worker::setNumberOfParameters(int value)
{
    mNumParameters = value;
    mParameterMin.resize(mNumParameters);
    mParameterMax.resize(mNumParameters);
    for(int p=0; p<mNumPoints; ++p)
    {
        mPoints[p].resize(mNumParameters);
    }
    for(int p=0; p<mNumCandidates; ++p)
    {
        mCandidatePoints[p].resize(mNumParameters);
    }
}


void Worker::setNumberOfCandidates(int value)
{
    mNumCandidates = value;

    mCandidatePoints.resize(mNumCandidates);
    mCandidateObjectives.resize(mNumCandidates);
    for(int p=0; p<mNumCandidates; ++p)
    {
        mCandidatePoints[p].resize(mNumParameters);
    }
}


void Worker::setParameterNames(QStringList names)
{
    mParameterNames = names;
}


void Worker::setConvergenceTolerance(double value)
{
    mTolerance = value;
}


void Worker::setMaxNumberOfIterations(int value)
{
    mnMaxIterations = value;
}

void Worker::setTolerance(double value)
{
    mTolerance = value;
}

void Worker::setSamplingMethod(SamplingT dist)
{
    mDistribution = dist;
}

int Worker::getNumberOfCandidates()
{
    return mNumCandidates;
}

int Worker::getNumberOfPoints()
{
    return mNumPoints;
}

int Worker::getNumberOfParameters()
{
    return mNumParameters;
}

int Worker::getMaxNumberOfIterations()
{
    return mnMaxIterations;
}

int Worker::getCurrentNumberOfIterations()
{
    return mIterationCounter;
}

void Worker::abort()
{
    mIsAborted = true;
}


void Worker::setParameterLimits(int idx, double min, double max)
{
    mParameterMin[idx] = min;
    mParameterMax[idx] = max;
}

void Worker::getParameterLimits(int idx, double &min, double &max)
{
    min = mParameterMin[idx];
    max = mParameterMax[idx];
}


void Worker::setCandidateObjectiveValue(int idx, double value)
{
    if(idx<0 || idx > mCandidateObjectives.size()-1)
    {
        return;
    }
    mCandidateObjectives[idx] = value;
}


double Worker::getObjectiveValue(int idx)
{
    if(idx<0 || idx > mObjectives.size()-1)
    {
        return 0;
    }
    return mObjectives[idx];
}

QVector<double> &Worker::getObjectiveValues()
{
    return mObjectives;
}

QVector<QVector<double> > &Worker::getPoints()
{
    return mPoints;
}


double Worker::getCandidateParameter(const int pointIdx, const int parIdx) const
{
    if(mCandidatePoints.size() < pointIdx+1)
    {
        return 0;
    }
    else if(mCandidatePoints[pointIdx].size() < parIdx)
    {
        return 0;
    }
    return mCandidatePoints[pointIdx][parIdx];
}

double Worker::getParameter(const int pointIdx, const int parIdx) const
{
    if(mPoints.size() < pointIdx+1)
    {
        return 0;
    }
    else if(mPoints[pointIdx].size() < parIdx)
    {
        return 0;
    }
    return mPoints[pointIdx][parIdx];
}



double Worker::getMaxPercentalParameterDiff()
{
    return getMaxPercentalParameterDiff(mPoints);
}

double Worker::getMaxPercentalParameterDiff(QVector<QVector<double> > &points)
{
    double maxDiff = -1e100;
    for(int i=0; i<mNumParameters; ++i)
    {
        double maxPar = -1e100;
        double minPar = 1e100;
        for(int p=0; p<points.size(); ++p)
        {
            if(points[p][i] > maxPar) maxPar = points[p][i];
            if(points[p][i] < minPar) minPar = points[p][i];
        }
        if(mParameterMax[i] != mParameterMin[i] && (maxPar-minPar)/(mParameterMax[i]-mParameterMin[i]) > maxDiff)
        {
            maxDiff = (maxPar-minPar)/(mParameterMax[i]-mParameterMin[i]);
        }
    }
    return maxDiff;
}

QStringList *Worker::getParameterNamesPtr()
{
    return &mParameterNames;
}
