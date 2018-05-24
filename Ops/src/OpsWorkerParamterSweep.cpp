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
//! @brief Contains the optimization worker class for the PSO algorithm
//!
//$Id$

#include "OpsWorkerParameterSweep.h"
#include "OpsEvaluator.h"
#include "OpsMessageHandler.h"
#include <math.h>

using namespace Ops;

//! @brief Initializes a particle swarm optimization
WorkerParameterSweep::WorkerParameterSweep(Evaluator *pEvaluator, MessageHandler *pMessageHandler)
    : Worker(pEvaluator, pMessageHandler) {}

AlgorithmT WorkerParameterSweep::getAlgorithm()
{
    return ParameterSweep;
}



//! @brief Executes a parameter sweep algorithm
void WorkerParameterSweep::run()
{
    mpMessageHandler->printMessage("Running optimization with parameter sweep algorithm.");

    distributePoints();

    //Evaluate initial objective values
    mpEvaluator->evaluateAllPoints();
    mpMessageHandler->objectivesChanged();

    //Calculate best known global position
    calculateBestAndWorstId();
    mpMessageHandler->pointsChanged();

    mIterationCounter=0;
    for(; mIterationCounter<mnMaxIterations && !mpMessageHandler->aborted(); ++mIterationCounter)
    {
        //Move particles
        distrubteCandidatePoints();

        //Evaluate objective values
        mpEvaluator->evaluateAllCandidates();
        mpMessageHandler->objectivesChanged();

        std::vector<size_t> ids = getIdsSortedFromWorstToBest();

        for(size_t i=0; i<mCandidateObjectives.size(); ++i)
        {
            ids = getIdsSortedFromWorstToBest();
            for(size_t j=0; j<mObjectives.size(); ++j)
            {
                if(mCandidateObjectives[i] < mObjectives[ids[j]])
                {
                    mObjectives[ids[j]] = mCandidateObjectives[i];
                    mPoints[ids[j]] = mCandidatePoints[i];
                    break;
                }
            }
        }

        mpMessageHandler->pointsChanged();

        mpMessageHandler->stepCompleted(mIterationCounter);
    }

    if(mpMessageHandler->aborted())
    {
        mpMessageHandler->printMessage("Optimization was aborted after "+std::to_string(mIterationCounter)+" iterations.");
    }
    else
    {
        mpMessageHandler->printMessage("Optimization finished after "+std::to_string(mIterationCounter)+" iterations");
    }

    // Clean up
    finalize();

    return;
}



