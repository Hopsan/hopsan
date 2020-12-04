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

#include "OpsWorkerDifferentialEvolution.h"
#include "OpsEvaluator.h"
#include "OpsMessageHandler.h"
#include <math.h>

using namespace Ops;

//! @brief Initializes a particle swarm optimization
WorkerDifferentialEvolution::WorkerDifferentialEvolution(Evaluator *pEvaluator, MessageHandler *pMessageHandler)
    : Worker(pEvaluator, pMessageHandler)
{
}

AlgorithmT WorkerDifferentialEvolution::getAlgorithm()
{
    return DifferentialEvolution;
}



//! @brief Executes a differential evolution algorithm
void WorkerDifferentialEvolution::run()
{
    if(mNumCandidates != mNumPoints)
    {
        mpMessageHandler->printMessage("Error: Differential evolution algorithm requires same number of candidates and points.");
        return;
    }
    if(mNumCandidates < 5) {
        mpMessageHandler->printMessage("Error: Differential evolution algorithm requires more than 4 candidates.");
        return;
    }
    mpMessageHandler->printMessage("Running optimization with differential evolution algorithm.");

    distributePoints();

    //Evaluate initial objective values
    mpEvaluator->evaluateAllPointsWithSurrogateModel();
    mpMessageHandler->objectivesChanged();

    mIterationCounter=0;
    for(; mIterationCounter<mnMaxIterations && !mpMessageHandler->aborted(); ++mIterationCounter)
    {
        for(size_t p=0; p<mNumPoints; ++p)
        {
            bool feasible=false;
            while(!feasible)
            {
                size_t a,b,c,R;
                getRandomIds(p,a,b,c,R);

                mCandidatePoints[p] = mPoints[p];
                for(size_t i=0; i<mNumParameters; ++i)
                {
                    double r = opsRand();
                    if(r < mCR || i == R)
                    {
                        double A = mPoints[a][i];
                        double B = mPoints[b][i];
                        double C = mPoints[c][i];
                        mCandidatePoints[p][i] = A + mF * (B - C);
                    }
                }
                feasible = isCandidateFeasible(p);
            }
        }

        mpEvaluator->evaluateAllCandidatesWithSurrogateModel();
        mpMessageHandler->candidatesChanged();

        for(size_t p=0; p<mNumPoints; ++p)
        {
            if(mCandidateObjectives[p] < mObjectives[p])
            {
                mPoints[p] = mCandidatePoints[p];
                mObjectives[p] = mCandidateObjectives[p];
            }
        }
        mpMessageHandler->pointsChanged();
        mpMessageHandler->objectivesChanged();

        //Check convergence
        if(checkForConvergence()) break;      //Use complex method, it's the same principle

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


void WorkerDifferentialEvolution::setCrossoverProbability(double value)
{
    mCR = value;
}


void WorkerDifferentialEvolution::setDifferentialWeight(double value)
{
    mF = value;
}


void WorkerDifferentialEvolution::getRandomIds(size_t notId, size_t &id1, size_t &id2, size_t &id3, size_t &id4)
{
    id1 = mNumPoints * opsRand();
    while(id1 == notId || id1 > mNumPoints-1)
    {
        id1 = mNumPoints * opsRand();
    }
    id2 = mNumPoints * opsRand();
    while(id2 == notId || id2 > mNumPoints-1 || id2 == id1)
    {
        id2 = mNumPoints * opsRand();
    }
    id3 = mNumPoints * opsRand();
    while(id3 == notId || id3 > mNumPoints-1 || id3 == id1 || id3 == id2)
    {
        id3 = mNumPoints * opsRand();
    }
    id4 = mNumParameters * opsRand();
    while(id4 > mNumPoints-1)
    {
        id4 = mNumPoints * opsRand();
    }
}

bool WorkerDifferentialEvolution::isCandidateFeasible(int id)
{
    for(size_t p=0; p<mNumParameters; ++p)
    {
        if(mCandidatePoints[id][p] > mParameterMax[p] || mCandidatePoints[id][p] < mParameterMin[p])
            return false;
    }
    return true;
}
