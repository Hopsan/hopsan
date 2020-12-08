/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

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
    mpEvaluator->evaluateAllPointsWithSurrogateModel();
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

        mpEvaluator->evaluateCandidateWithSurrogateModel(0);

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
            mpEvaluator->evaluateCandidateWithSurrogateModel(0);
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
            mpEvaluator->evaluateCandidateWithSurrogateModel(0);
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
