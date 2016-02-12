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



//! @brief Executes a particle swarm algorithm. optParticleInit() must be called before this one.
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



