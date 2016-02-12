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
#include "OpsMessageHandler.h"
#include <math.h>

using namespace Ops;

WorkerNelderMead::WorkerNelderMead(Evaluator *pEvaluator, MessageHandler *pMessageHandler)
    : WorkerSimplex(pEvaluator, pMessageHandler)
{
}


AlgorithmT WorkerNelderMead::getAlgorithm()
{
    return NelderMead;
}


void WorkerNelderMead::run()
{
    mpMessageHandler->printMessage("Running optimization with Nelder-Mead algorithm.");

    distributePoints();

    calculateBestAndWorstId();

    //Evaluate initial objective values
    mpEvaluator->evaluateAllPoints();
    mpMessageHandler->objectivesChanged();


    //Run optimization loop
    mIterationCounter=0;
    for(; mIterationCounter<mnMaxIterations && !mpMessageHandler->aborted(); ++mIterationCounter)
    {
        //Check convergence
        if(checkForConvergence()) break;

        calculateBestAndWorstId();

        findCentroidPoint();

        mCandidatePoints[0] = reflect(mPoints[mWorstId], mCentroidPoint, mAlpha);   //Reflect
        mpMessageHandler->candidateChanged(0);

        mpEvaluator->evaluateCandidate(0);

        std::vector<double> reflectedPoint = mCandidatePoints[0];
        double reflectedObj = mCandidateObjectives[0];
        std::vector<size_t> ids = getIdsSortedFromWorstToBest();

        size_t bestId = ids[ids.size()-1];
        size_t worstId = ids[0];
        size_t secondWorstId = ids.at(ids.size()-2);

        if(mCandidateObjectives[0] < mObjectives.at(secondWorstId) && mCandidateObjectives[0] > mObjectives.at(bestId))
        {
            mPoints[worstId] = mCandidatePoints[0];
            mObjectives[worstId] = mCandidateObjectives[0];

            mpMessageHandler->pointChanged(worstId);
            mpMessageHandler->objectiveChanged(worstId);
        }
        else if(mCandidateObjectives[0] < mObjectives.at(bestId) && !mpMessageHandler->aborted())
        {
            mCandidatePoints[0] = reflect(mPoints[worstId], mCentroidPoint, mGamma);   //Expand
            mpMessageHandler->candidateChanged(0);
            mpEvaluator->evaluateCandidate(0);
            ++mIterationCounter;

            if(mCandidateObjectives[0] < reflectedObj)
            {
                mPoints[worstId] = mCandidatePoints[0];
                mObjectives[worstId] = mCandidateObjectives[0];
                mpMessageHandler->pointChanged(worstId);
                mpMessageHandler->objectiveChanged(worstId);
            }
            else
            {
                mPoints[worstId] = reflectedPoint;
                mObjectives[worstId] = reflectedObj;
                mpMessageHandler->pointChanged(worstId);
                mpMessageHandler->objectiveChanged(worstId);
            }
        }
        else if(!mpMessageHandler->aborted())
        {
            mCandidatePoints[0] = reflect(mPoints[mWorstId], mCentroidPoint, mRho);   //Contract
            mpMessageHandler->candidateChanged(0);
            mpEvaluator->evaluateCandidate(0);
            ++mIterationCounter;

            if(mCandidateObjectives[0] < mObjectives[mWorstId])
            {
                mPoints[mWorstId] = mCandidatePoints[0];
                mObjectives[mWorstId] = mCandidateObjectives[0];
                mpMessageHandler->pointChanged(mWorstId);
                mpMessageHandler->objectiveChanged(mWorstId);
            }
            else if(!mpMessageHandler->aborted())
            {
                reduce();   //Reduce
                mpMessageHandler->pointsChanged();
                mpEvaluator->evaluateAllPoints();
                mpMessageHandler->objectivesChanged();
            }
        }

        mpMessageHandler->stepCompleted(mIterationCounter);
    }


    if(mpMessageHandler->aborted())
    {
        mpMessageHandler->printMessage("Optimization was aborted after "+std::to_string(mIterationCounter)+" iterations.");
    }
    else if(mIterationCounter == mnMaxIterations)
    {
        mpMessageHandler->printMessage("Optimization failed to converge after "+std::to_string(mIterationCounter)+" iterations");
    }
    else
    {
        mpMessageHandler->printMessage("Optimization converged in parameter values after "+std::to_string(mIterationCounter)+" iterations.");
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
    for(size_t i=0; i<mNumPoints; ++i)
    {
        if(i==mBestId) continue;

        for(size_t j=0; j<mNumParameters; ++j)
        {
            //Reflect
            double best = mPoints[mBestId][j];
            mPoints[i][j] = best + mSigma*(mPoints[i][j] - best);
        }
    }
}
