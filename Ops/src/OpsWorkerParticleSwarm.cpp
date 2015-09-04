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
//$Id: SymHop.cc 8138 2015-06-17 12:27:23Z petno25 $

#include "OpsWorkerParticleSwarm.h"
#include "OpsEvaluator.h"

#include <math.h>

using namespace Ops;

//! @brief Initializes a particle swarm optimization
WorkerParticleSwarm::WorkerParticleSwarm(Evaluator *pEvaluator)
    : Worker(pEvaluator)
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

    for(int p=0; p<mNumPoints; ++p)
    {
        for(int i=0; i<mNumParameters; ++i)
        {
            //Initialize velocities
            double minVel = -fabs(mParameterMax[i]-mParameterMin[i]);
            double maxVel = fabs(mParameterMax[i]-mParameterMin[i]);
            double r = double(rand()) / double(RAND_MAX);
            mVelocities[p][i] = minVel + r*(maxVel-minVel);
        }
    }
}


//! @brief Executes a particle swarm algorithm. optParticleInit() must be called before this one.
void WorkerParticleSwarm::run()
{
    emit message("Running optimization with particle swarm algorithm.");

    distributePoints();

    //Evaluate initial objective values
    mpEvaluator->evaluateAllPoints();
    emit objectivesChanged();

    //Initialize best known point for each point
    for(int i=0; i<mNumPoints; ++i)
    {
        mLocalBestPoints[i] = mPoints[i];
        mLocalBestObjectives[i] = mObjectives[i];
    }

    //Calculate best known global position
    calculateBestAndWorstId();
    mBestObjective = mObjectives[mBestId];
    mBestPoint = mPoints[mBestId];


    mIterationCounter=0;
    for(; mIterationCounter<mnMaxIterations && !mIsAborted; ++mIterationCounter)
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
            emit message("Unknown inertia strategy, aborting.");
            return;
        }

        //Move particles
        moveParticles();
        emit pointsChanged();

        //Evaluate objective values
        mpEvaluator->evaluateAllPoints();
        emit objectivesChanged();

        //Calculate best known positions
        for(int p=0; p<mNumPoints; ++p)
        {
            if(mObjectives[p] < mLocalBestObjectives[p])
            {
                mLocalBestPoints[p] = mPoints[p];
                mLocalBestObjectives[p] = mObjectives[p];
            }
        }

        //Calculate best known global position
        calculateBestAndWorstId();
        if(mObjectives[mBestId] < mBestObjective)
        {
            mBestObjective = mObjectives[mBestId];
            mBestPoint = mPoints[mBestId];
        }

        //Check convergence
        if(checkForConvergence()) break;      //Use complex method, it's the same principle

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


void WorkerParticleSwarm::setNumberOfPoints(int value)
{
    Worker::setNumberOfPoints(value);

    mVelocities.resize(value);
    mLocalBestPoints.resize(value);
    mLocalBestObjectives.resize(value);
    for(int i=0; i<value; ++i)
    {
        mVelocities[i].resize(mNumPoints);
        mLocalBestPoints[i].resize(mNumPoints);
    }
}


void WorkerParticleSwarm::setNumberOfParameters(int value)
{
    Worker::setNumberOfParameters(value);

    mBestPoint.resize(value);
    for(int i=0; i<mNumPoints; ++i)
    {
        mVelocities[i].resize(mNumPoints);
        mLocalBestPoints[i].resize(mNumPoints);
    }
}



//! @brief Moves the particles (for particle swarm optimization)
void WorkerParticleSwarm::moveParticles()
{
    for (int p=0; p<mNumPoints; ++p)
    {
        moveParticle(p);
    }
}


//! @brief Moves specified particles (for particle swarm optimization)
void WorkerParticleSwarm::moveParticle(int p)
{
    double r1 = double(rand())/double(RAND_MAX);
    double r2 = double(rand())/double(RAND_MAX);
    for(int j=0; j<mNumParameters; ++j)
    {
        mVelocities[p][j] = mOmega*mVelocities[p][j] + mC1*r1*(mLocalBestPoints[p][j]-mPoints[p][j]) + mC2*r2*(mBestPoint[j]-mPoints[p][j]);
        mVelocities[p][j] = qMin(mVelocities[p][j], mVmax);
        mPoints[p][j] = mPoints[p][j]+mVelocities[p][j];
        if(mPoints[p][j] <= mParameterMin[j])
        {
            mPoints[p][j] = mParameterMin[j];
            mVelocities[p][j] = 0.0;
        }
        if(mPoints[p][j] >= mParameterMax[j])
        {
            mPoints[p][j] = mParameterMax[j];
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



