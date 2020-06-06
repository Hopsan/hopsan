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
//! @file   OpsWorkerComplexBurmen.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2016-01-08
//!
//! @brief Contains the optimization worker class for the Complex-RF algorithm by Bürmen
//!
//$Id$

#include "OpsWorkerComplexBurmen.h"
#include "OpsEvaluator.h"
#include "OpsMessageHandler.h"
#include <math.h>
#include <algorithm>

using namespace Ops;

WorkerComplexBurmen::WorkerComplexBurmen(Evaluator *pEvaluator, MessageHandler *pMessageHandler)
    : WorkerComplexRF(pEvaluator, pMessageHandler)
{
    mRetractionCounter = 0;
}

AlgorithmT WorkerComplexBurmen::getAlgorithm()
{
    return ComplexBurmen;
}

void WorkerComplexBurmen::initialize()
{
    WorkerComplexRF::initialize();
}

void WorkerComplexBurmen::run()
{
    mpMessageHandler->printMessage("Running optimization with Complex-RF algorithm by Bürmen.");

    distributePoints();

    mpEvaluator->evaluateAllPoints();
    mpMessageHandler->objectivesChanged();

    calculateBestAndWorstId();

    //Run optimization loop
    mIterationCounter=0;
    for(; mIterationCounter<mnMaxIterations && !mpMessageHandler->aborted(); ++mIterationCounter)
    {
        //Check convergence
        if(checkForConvergence()) break;

        //Increase all objective values (forgetting principle)
        applyForgettingFactor();

        //Calculate best and worst point
        calculateBestAndWorstId();

        //Find geometrical center
        std::vector<size_t> ids = getIdsSortedFromWorstToBest();

        std::vector< std::vector<double> > nonReflectedPoints;
        for(size_t i=mNumPoints-1; i>mNumCandidates-1; --i)
        {
            nonReflectedPoints.push_back(mPoints[ids[i]]);
        }
        findCentroidPoint(nonReflectedPoints);

        //Reflect n worst point
        std::vector< std::vector<double> > newPoints;
        for(size_t i=0; i<mNumCandidates; ++i)
        {
            //Reflect first point
            mCandidatePoints[i] = reflect(mPoints[ids[i]], mCentroidPoint, mAlpha);
            newPoints.push_back(mCandidatePoints[i]);
        }
        mpMessageHandler->candidatesChanged();

        //Evaluate new points
        mpEvaluator->evaluateAllCandidates();

        std::vector<size_t> finishedCandidates;
        for(size_t i=0; i<mNumCandidates; ++i)
        {
            if(inVector(finishedCandidates,i)) continue;
            if(mCandidateObjectives[i] < mObjectives[ids[mNumCandidates]]/*mObjectives[ids[i]]*/)
            {
                mPoints[ids[i]] = mCandidatePoints[i];
                mObjectives[ids[i]] = mCandidateObjectives[i];
                finishedCandidates.push_back(i);
            }
        }

        mRetractionCounter = 0;
        bool doBreak = false;
        while(finishedCandidates.size() != mNumCandidates && !mpMessageHandler->aborted())
        {
            calculateBestAndWorstId();
            mpMessageHandler->stepCompleted(mIterationCounter);

            ++mIterationCounter;

            for(size_t i=0; i<mNumCandidates; ++i)
            {
                if(inVector(finishedCandidates,i)) continue;
                double a1 = 1.0-exp(-double(mRetractionCounter)/5.0);
                for(size_t j=0; j<mNumParameters; ++j)
                {
                    double best = mPoints[mBestId][j];
                    double maxDiff = getMaxPercentalParameterDiff()*10/(9.0+mRetractionCounter);
                    double r = opsRand();
                    mCandidatePoints[i][j] = (mCentroidPoint[j]*(1.0-a1) + best*a1 + newPoints[i][j])/2.0;
                    mCandidatePoints[i][j] += mRandomFactor*(mParameterMax[j]-mParameterMin[j])*maxDiff*(r-0.5);
                    mCandidatePoints[i][j] = std::min(mCandidatePoints[i][j], mParameterMax[j]);
                    mCandidatePoints[i][j] = std::max(mCandidatePoints[i][j], mParameterMin[j]);
                }
            }
            mpMessageHandler->candidatesChanged();
            mpEvaluator->evaluateAllCandidates();

            for(size_t i=0; i<mNumCandidates; ++i)
            {
                if(inVector(finishedCandidates,i)) continue;
                if(mCandidateObjectives[i] < mObjectives[ids[mNumCandidates]]/*mObjectives[ids[i]]*/)
                {
                    mPoints[ids[i]] = mCandidatePoints[i];
                    mObjectives[ids[i]] = mCandidateObjectives[i];
                    finishedCandidates.push_back(i);
                }
                else
                {
                    newPoints[i] = mCandidatePoints[i];
                }
            }

            if(checkForConvergence())
            {
                doBreak = true;
                break;
            }

            ++mRetractionCounter;
        }

        if(doBreak)
        {
            break;
        }

        mpMessageHandler->stepCompleted(mIterationCounter);
    }


    if(mpMessageHandler->aborted())
    {
        mpMessageHandler->printMessage("Optimization was aborted after %1"+std::to_string(mIterationCounter)+" iterations.");
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


