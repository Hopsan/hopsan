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
//! @brief Contains the optimization worker class for the Nelder-Mead algorithm
//!
//$Id$

#include "OpsWorkerNelderMead.h"
#include "OpsEvaluator.h"
#include <math.h>

using namespace Ops;

WorkerNelderMead::WorkerNelderMead(Evaluator *pEvaluator)
    : WorkerSimplex(pEvaluator)
{
}


AlgorithmT WorkerNelderMead::getAlgorithm()
{
    return NelderMead;
}


void WorkerNelderMead::run()
{
    emit message("Running optimization with Nelder-Mead algorithm.");

    distributePoints();

    calculateBestAndWorstId();

    //Evaluate initial objective values
    mpEvaluator->evaluateAllPoints();
    emit objectivesChanged();


    //Run optimization loop
    mIterationCounter=0;
    for(; mIterationCounter<mnMaxIterations && !mIsAborted; ++mIterationCounter)
    {
        //Check convergence
        if(checkForConvergence()) break;

        calculateBestAndWorstId();

        findCentroidPoint();

        mCandidatePoints[0] = reflect(mPoints[mWorstId], mCentroidPoint, mAlpha);   //Reflect
        emit candidateChanged(0);
        mpEvaluator->evaluateCandidate(0);

        QVector<double> reflectedPoint = mCandidatePoints[0];
        double reflectedObj = mCandidateObjectives[0];
        QVector<int> ids = getIdsSortedFromWorstToBest();

        int bestId = ids.last();
        int worstId = ids.first();
        int secondWorstId = ids.at(ids.size()-2);

        if(mCandidateObjectives[0] < mObjectives.at(secondWorstId) && mCandidateObjectives[0] > mObjectives.at(bestId))
        {
            mPoints[worstId] = mCandidatePoints[0];
            mObjectives[worstId] = mCandidateObjectives[0];
            emit pointChanged(worstId);
            emit objectiveChanged(worstId);
        }
        else if(mCandidateObjectives[0] < mObjectives.at(bestId) && !mIsAborted)
        {
            mCandidatePoints[0] = reflect(mPoints[worstId], mCentroidPoint, mGamma);   //Expand
            emit candidateChanged(0);
            mpEvaluator->evaluateCandidate(0);

            if(mCandidateObjectives[0] < reflectedObj)
            {
                mPoints[worstId] = mCandidatePoints[0];
                mObjectives[worstId] = mCandidateObjectives[0];
                emit pointChanged(worstId);
                emit objectiveChanged(worstId);
            }
            else
            {
                mPoints[worstId] = reflectedPoint;
                mObjectives[worstId] = reflectedObj;
                emit pointChanged(worstId);
                emit objectiveChanged(worstId);
            }
        }
        else if(!mIsAborted)
        {
            mCandidatePoints[0] = reflect(mPoints[mWorstId], mCentroidPoint, mRho);   //Contract
            emit candidateChanged(0);
            mpEvaluator->evaluateCandidate(0);

            if(mCandidateObjectives[0] < mObjectives[mWorstId])
            {
                mPoints[mWorstId] = mCandidatePoints[0];
                mObjectives[mWorstId] = mCandidateObjectives[0];
                emit pointChanged(mWorstId);
                emit objectiveChanged(mWorstId);
            }
            else if(!mIsAborted)
            {
                reduce();   //Reduce
                emit pointsChanged();
                mpEvaluator->evaluateAllPoints();
                emit objectivesChanged();
            }
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

void WorkerNelderMead::setReflectionFactor(double value)
{
    mAlpha = value;
}

void WorkerNelderMead::setExpansionFactor(double value)
{
    mGamma = value;
}

void WorkerNelderMead::setContractionFactor(double value)
{
    mRho = value;
}

void WorkerNelderMead::setReductionFactor(double value)
{
    mSigma = value;
}


void WorkerNelderMead::reduce()
{
    for(int i=0; i<mNumPoints; ++i)
    {
        if(i==mBestId) continue;

        for(int j=0; j<mNumParameters; ++j)
        {
            //Reflect
            double best = mPoints[mBestId][j];
            mPoints[i][j] = best + mSigma*(mPoints[i][j] - best);
        }
    }
}
