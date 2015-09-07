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
//! @brief Contains the optimization worker class for the Complex-RF algorithm
//!
//$Id: SymHop.cc 8138 2015-06-17 12:27:23Z petno25 $

#include "OpsWorkerComplexRF.h"
#include "OpsEvaluator.h"
#include <math.h>

using namespace Ops;

WorkerComplexRF::WorkerComplexRF(Evaluator *pEvaluator)
    : WorkerSimplex(pEvaluator)
{
    mRetractionCounter = 0;
}

AlgorithmT WorkerComplexRF::getAlgorithm()
{
    return ComplexRF;
}

void WorkerComplexRF::initialize()
{
    WorkerSimplex::initialize();

    mKf = 1.0-pow(mAlpha/2.0, mGamma/mNumPoints);
}

void WorkerComplexRF::run()
{
    emit message("Running optimization with Complex-RF algorithm.");

    distributePoints();

    mpEvaluator->evaluateAllPoints();
    emit objectivesChanged();

    calculateBestAndWorstId();

    //Run optimization loop
    mIterationCounter=0;
    for(; mIterationCounter<mnMaxIterations && !mIsAborted; ++mIterationCounter)
    {
        //Check convergence
        if(checkForConvergence()) break;

        //Increase all objective values (forgetting principle)
        applyForgettingFactor();

        //Calculate best and worst point
        calculateBestAndWorstId();

        //Find geometrical center
        findCentroidPoint();

        //Reflect worst point
        mCandidatePoints[0] = reflect(mPoints[mWorstId], mCentroidPoint, mAlpha);
        emit candidateChanged(0);
        QVector<double> newPoint = mCandidatePoints[0]; //Remember the new point, in case we need to iterate below

        //Evaluate new point
        mpEvaluator->evaluateCandidate(0);
        mPoints[mWorstId] = mCandidatePoints[0];
        mObjectives[mWorstId] = mCandidateObjectives[0];
        emit pointChanged(mWorstId);
        emit objectiveChanged(mWorstId);

        calculateBestAndWorstId();

        //Retract until worst point is no longer the same
        mRetractionCounter = 0;
        bool doBreak = false;
        while(mLastWorstId == mWorstId && !mIsAborted)
        {
            ++mIterationCounter;
            emit stepCompleted(mIterationCounter);
            if(mIterationCounter>=mnMaxIterations)
            {
                --mIterationCounter;    //Needed because for-loop will increase it by one anyway
                break;
            }

            if(retract())
            {
                doBreak = true;
                break;
            }
        }

        if(doBreak)
        {
            break;
        }

        emit stepCompleted(mIterationCounter);
    }


    if(mIsAborted)
    {
        emit message("Optimization was aborted after "+QString::number(mIterationCounter)+" iterations.");
    }
    else if(mIterationCounter == mnMaxIterations)
    {
        emit message("Optimization failed to converge after "+QString::number(mIterationCounter)+" iterations");
    }
    else
    {
        emit message("Optimization converged in parameter values after "+QString::number(mIterationCounter)+" iterations.");
    }

    // Clean up
    finalize();

    return;
}

void WorkerComplexRF::setReflectionFactor(double value)
{
    mAlpha = value;
}

void WorkerComplexRF::setForgettingFactor(double value)
{
    mGamma = value;
}

bool WorkerComplexRF::retract()
{
    QVector<double> newPoint = mPoints[mWorstId];

        //Move first reflected point
    double a1 = 1.0-exp(-double(mRetractionCounter)/5.0);
    for(int j=0; j<mNumParameters; ++j)
    {
        double best = mPoints[mBestId][j];
        double maxDiff = getMaxPercentalParameterDiff()*10/(9.0+mRetractionCounter);
        double r = opsRand();
        mCandidatePoints[0][j] = (mCentroidPoint[j]*(1.0-a1) + best*a1 + newPoint[j])/2.0;
        mCandidatePoints[0][j] += mRandomFactor*(mParameterMax[j]-mParameterMin[j])*maxDiff*(r-0.5);
        mCandidatePoints[0][j] = qMin(mCandidatePoints[0][j], mParameterMax[j]);
        mCandidatePoints[0][j] = qMax(mCandidatePoints[0][j], mParameterMin[j]);
    }
    emit candidateChanged(0);

    ++mRetractionCounter;

    //Evaluate new point
    mpEvaluator->evaluateCandidate(0);
    mPoints[mWorstId] = mCandidatePoints[0];
    mObjectives[mWorstId] = mCandidateObjectives[0];
    emit pointChanged(mWorstId);
    emit objectiveChanged(mWorstId);

    calculateBestAndWorstId();

    emit objectiveChanged(mWorstId);
    emit pointChanged(mWorstId);

    return checkForConvergence();
}


void WorkerComplexRF::applyForgettingFactor()
{
    double maxObj = mObjectives[0];
    double minObj = mObjectives[0];
    for(int i=0; i<mNumPoints; ++i)
    {
        double obj = mObjectives[i];
        if(obj > maxObj) maxObj = obj;
        if(obj < minObj) minObj = obj;
    }
    for(int i=0; i<mNumPoints; ++i)
    {
        mObjectives[i] = mObjectives[i]+(maxObj-minObj)*mKf;
    }

    emit objectivesChanged();
}

