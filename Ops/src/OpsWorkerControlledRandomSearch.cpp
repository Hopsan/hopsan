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

#include "OpsWorkerControlledRandomSearch.h"
#include "OpsWorkerSimplex.h"
#include "OpsEvaluator.h"
#include "OpsMessageHandler.h"
#include <math.h>

using namespace Ops;

WorkerControlledRandomSearch::WorkerControlledRandomSearch(Evaluator *pEvaluator, MessageHandler *pMessageHandler)
    : WorkerSimplex(pEvaluator, pMessageHandler)
{
}

AlgorithmT WorkerControlledRandomSearch::getAlgorithm()
{
    return ControlledRandomSearch;
}

void WorkerControlledRandomSearch::initialize()
{
    WorkerSimplex::initialize();
}

void WorkerControlledRandomSearch::run()
{
    mpMessageHandler->printMessage("Running optimization with controlled random search (CRS2) algorithm.");

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

        //Calculate best and worst point
        calculateBestAndWorstId();

        for(size_t i=0; i<mNumCandidates; ++i)
        {

            std::vector<size_t> chosenIdx;
            std::vector< std::vector<double> > chosenPoints;
            chosenIdx.push_back(mBestId);
            chosenIdx.push_back(mWorstId);
            chosenPoints.push_back(mPoints[mBestId]);
            for(size_t j=0; j<mNumParameters-1; ++j)
            {
                size_t idx = opsRand();
                while(inVector(chosenIdx,idx))
                {
                    idx = round(opsRand()*(mNumPoints-1));
                }
                chosenIdx.push_back(idx);
                chosenPoints.push_back(mPoints[idx]);
            }

            //Find geometrical center
            findCentroidPoint(chosenPoints);

            //Reflect worst point
            mCandidatePoints[i] = reflect(mPoints[mWorstId], mCentroidPoint, 1.0);
            mpMessageHandler->candidateChanged(0);

            //Check if constraints are violated, if so, do new reflection
            //bool constraintsViolated=false;
            for(size_t p=0; p<mNumParameters; ++p)
            {
                if(mCandidatePoints[i][p] < mParameterMin[p] ||
                        mCandidatePoints[i][p] > mParameterMax[p])
                {
                    --i;
                }
            }
        }

        mpEvaluator->evaluateAllCandidates();
        double bestObj = 1e100;
        int bestId = -1;
        for(size_t i=0; i<mNumCandidates; ++i)
        {
            if(mCandidateObjectives[i] < bestObj)
            {
                bestObj = mCandidateObjectives[i];
                bestId = i;
            }
        }

        //Check if new point is better; if so, keep it
        if(mCandidateObjectives[bestId] < mObjectives[mWorstId])
        {
            mPoints[mWorstId] = mCandidatePoints[bestId];
            mObjectives[mWorstId] = mCandidateObjectives[bestId];
            mpMessageHandler->pointChanged(mWorstId);
            mpMessageHandler->objectiveChanged(mWorstId);
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
