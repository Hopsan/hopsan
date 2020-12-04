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

#include "OpsWorkerParticleSwarm.h"
#include "OpsEvaluator.h"
#include "OpsMessageHandler.h"
#include <math.h>
#include <algorithm>

using namespace Ops;

//! @brief Initializes a particle swarm optimization
WorkerParticleSwarm::WorkerParticleSwarm(Evaluator *pEvaluator, MessageHandler *pMessageHandler)
    : Worker(pEvaluator, pMessageHandler)
{
    mInertiaStrategy = InertiaLinearDecreasing;
}

AlgorithmT WorkerParticleSwarm::getAlgorithm()
{
    return ParticleSwarm;
}



void WorkerParticleSwarm::initialize()
{
    Worker::initialize();

    for(size_t p=0; p<mNumPoints; ++p)
    {
        for(size_t i=0; i<mNumParameters; ++i)
        {
            //Initialize velocities
            double minVel = -fabs(mParameterMax[i]-mParameterMin[i]);
            double maxVel = fabs(mParameterMax[i]-mParameterMin[i]);
            double r = opsRand();
            mVelocities[p][i] = minVel + r*(maxVel-minVel);
        }
    }
}


//! @brief Executes a particle swarm algorithm. optParticleInit() must be called before this one.
void WorkerParticleSwarm::run()
{
    if(mNumCandidates != mNumPoints)
    {
        mpMessageHandler->printMessage("Error: Differential evolution algorithm requires same number of candidates and points.");
        return;
    }

    mpMessageHandler->printMessage("Running optimization with particle swarm algorithm.");

    distributePoints();

    //Evaluate initial objective values
    mpEvaluator->evaluateAllPointsWithSurrogateModel();
    mpMessageHandler->objectivesChanged();

    //Initialize best known point for each point
    for(size_t i=0; i<mNumPoints; ++i)
    {
        mLocalBestPoints[i] = mPoints[i];
        mLocalBestObjectives[i] = mObjectives[i];
    }

    //Calculate best known global position
    calculateBestAndWorstId();
    mBestObjective = mObjectives[mBestId];
    mBestPoint = mPoints[mBestId];


    mIterationCounter=0;
    for(; mIterationCounter<mnMaxIterations && !mpMessageHandler->aborted(); ++mIterationCounter)
    {
        //Update weight (linearly decreasing)
        if(mInertiaStrategy == InertiaConstant)
        {
            mOmega = mOmega1;
        }
        else if(mInertiaStrategy == InertiaLinearDecreasing)
        {
            mOmega = mOmega1 + (mOmega2-mOmega1)*mIterationCounter/mnMaxIterations;
        }
        else
        {
            mpMessageHandler->printMessage("Unknown inertia strategy, aborting.");
            return;
        }

        //Move particles
        moveParticles();
        mpMessageHandler->pointsChanged();

        //Evaluate objective values
        bool usedSurrogateModel = mpEvaluator->evaluateAllCandidatesWithSurrogateModel();
        mpMessageHandler->objectivesChanged();

        if(!usedSurrogateModel) {
            //Calculate best known positions
            for(size_t p=0; p<mNumPoints; ++p)
            {
                if(mCandidateObjectives[p] < mObjectives[p])
                {
                    mPoints[p] = mCandidatePoints[p];
                    mObjectives[p] = mCandidateObjectives[p];
                }
            }

            //Calculate best known global position
            calculateBestAndWorstId();
            if(mObjectives[mBestId] < mBestObjective)
            {
                mBestObjective = mObjectives[mBestId];
                mBestPoint = mPoints[mBestId];
            }
        }

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


void WorkerParticleSwarm::setNumberOfPoints(size_t value)
{
    Worker::setNumberOfPoints(value);

    mVelocities.resize(value);
    mLocalBestPoints.resize(value);
    mLocalBestObjectives.resize(value);
    for(size_t i=0; i<value; ++i)
    {
        mVelocities[i].resize(mNumPoints);
        mLocalBestPoints[i].resize(mNumPoints);
    }
}


void WorkerParticleSwarm::setNumberOfParameters(size_t value)
{
    Worker::setNumberOfParameters(value);

    mBestPoint.resize(value);
    for(size_t i=0; i<mNumPoints; ++i)
    {
        mVelocities[i].resize(mNumPoints);
        mLocalBestPoints[i].resize(mNumPoints);
    }
}



//! @brief Moves the particles (for particle swarm optimization)
void WorkerParticleSwarm::moveParticles()
{
    for (size_t p=0; p<mNumPoints; ++p)
    {
        moveParticle(p);
    }
}


//! @brief Moves specified particles (for particle swarm optimization)
void WorkerParticleSwarm::moveParticle(int p)
{
    double r1 = opsRand();
    double r2 = opsRand();
    for(size_t j=0; j<mNumParameters; ++j)
    {
        mVelocities[p][j] = mOmega*mVelocities[p][j] + mC1*r1*(mPoints[p][j]-mCandidatePoints[p][j]) + mC2*r2*(mBestPoint[j]-mCandidatePoints[p][j]);
        mVelocities[p][j] = std::min(mVelocities[p][j], mVmax);
        mCandidatePoints[p][j] = mCandidatePoints[p][j]+mVelocities[p][j];
        if(mCandidatePoints[p][j] <= mParameterMin[j])
        {
            mCandidatePoints[p][j] = mParameterMin[j];
            mVelocities[p][j] = 0.0;
        }
        if(mCandidatePoints[p][j] >= mParameterMax[j])
        {
            mCandidatePoints[p][j] = mParameterMax[j];
            mVelocities[p][j] = 0.0;
        }
    }
}



void WorkerParticleSwarm::setOmega1(double value)
{
    mOmega1 = value;
}


void WorkerParticleSwarm::setOmega2(double value)
{
    mOmega2 = value;
}


void WorkerParticleSwarm::setC1(double value)
{
    mC1 = value;
}


void WorkerParticleSwarm::setC2(double value)
{
    mC2 = value;
}


void WorkerParticleSwarm::setVmax(double value)
{
    mVmax = value;
}

void WorkerParticleSwarm::setInertiaStrategy(OpsInertiaStrategy strategy)
{
    mInertiaStrategy = strategy;
}



