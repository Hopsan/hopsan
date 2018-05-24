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
//! @file   OpsWorkerGenetic.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2018-02-05
//!
//! @brief Contains the optimization worker class for the genetic algorithm
//!
//$Id$

#include "OpsWorkerGenetic.h"
#include "OpsEvaluator.h"
#include "OpsMessageHandler.h"
#include <math.h>
#include <random>

using namespace Ops;

//! @brief Initializes a particle swarm optimization
WorkerGenetic::WorkerGenetic(Evaluator *pEvaluator, MessageHandler *pMessageHandler)
    : Worker(pEvaluator, pMessageHandler) { }

AlgorithmT WorkerGenetic::getAlgorithm()
{
    return Genetic;
}


//! @brief Initialization function
void WorkerGenetic::initialize()
{
    Worker::initialize();
}


//! @brief Executes a genetic algorithm
void WorkerGenetic::run()
{
    mpMessageHandler->printMessage("Running optimization with genetic algorithm.");

    distributePoints();

    //Evaluate initial objective values
    mpEvaluator->evaluateAllPoints();
    mpMessageHandler->objectivesChanged();


    mIterationCounter=0;
    for(; mIterationCounter<mnMaxIterations && !mpMessageHandler->aborted(); ++mIterationCounter)
    {
        for(size_t i=0; i<mNumCandidates; i+=2) {
            selectParents();
            crossOver();
        }
        mutate();

        mpMessageHandler->candidatesChanged();

        //Evaluate objective values
        mpEvaluator->evaluateAllCandidates();


        for(size_t i=0; i<mNumPoints; ++i)
        {
            calculateBestAndWorstId();
            int worst = getWorstId();
            if(mCandidateObjectives[i] < mObjectives[worst])
            {
                mObjectives[worst] = mCandidateObjectives[i];
                mPoints[worst] = mCandidatePoints[i];
            }
        }



        mpMessageHandler->objectivesChanged();

        mpMessageHandler->stepCompleted(mIterationCounter);

        //Check convergence
        if(checkForConvergence()) break;      //Use complex method, it's the same principle
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


//! @brief Set number of optimization parameters
void WorkerGenetic::setNumberOfParameters(size_t value)
{
    Worker::setNumberOfParameters(value);

    mChild1.resize(mNumParameters);
    mChild2.resize(mNumParameters);
}


//! @brief Set crossover probability parameter
void WorkerGenetic::setCrossoverProbability(double value)
{
    mCrossoverProbability = value;
}


//! @brief Set mutation probability parameter
void WorkerGenetic::setMutationProbability(double value)
{
    mMutationProbability = value;
}


//! @brief Auxiliary function for selecting parents for crossover
void WorkerGenetic::selectParents() {
    double bestValue = std::numeric_limits<double>::max();
    for(size_t i=0; i<mNumPoints; ++i) {
        double value = mObjectives[i]*opsRand();
        if(value < bestValue) {
            mParent1 = i;
            bestValue = value;
        }
    }

    bestValue = std::numeric_limits<double>::max();
    for(size_t i=0; i<mNumPoints; ++i) {
        double value = mObjectives[i]*opsRand();
        if(value < bestValue && i != mParent1) {
            mParent2 = i;
            bestValue = value;
        }
    }
}


//! @brief Auxiliary function for perform crossover
void WorkerGenetic::crossOver()
{
    for(size_t i=0; i<mNumParameters; ++i)
    {
        double x1 = mPoints[mParent1][i];
        double x2 = mPoints[mParent2][i];

        double doCrossOver = opsRand();
        if(doCrossOver > mCrossoverProbability)
        {
            mChild1[i] = x1;
            mChild2[i] = x2;
        }
        else
        {
            double mean = (x1+x2)/2.0;
            double dev = fabs(x2-x1)/2.0;

            mChild1[i] = gaussian(mean,dev,mParameterMin[i],mParameterMax[i]);
            mChild2[i] = gaussian(mean,dev,mParameterMin[i],mParameterMax[i]);
        }
    }
    mCandidatePoints[mParent1] = mChild1;
    mCandidatePoints[mParent2] = mChild2;
}


//! @brief Auxiliary function for mutating children
void WorkerGenetic::mutate()
{
    for(size_t i=0; i<mNumPoints; ++i) {
        for(size_t j=0; j<mNumParameters; ++j) {
            double doMutation = opsRand();
            if(doMutation < mMutationProbability) {
                double mean = mCandidatePoints[i][j];
                double dev = (mParameterMax[j]-mParameterMin[j])/2.0;
                mCandidatePoints[i][j] = gaussian(mean,dev,mParameterMin[j],mParameterMax[j]);
            }
        }
    }
}


//! @brief Auxiliary function for gaussian distribution
//! @param mean Mean value
//! @param dev Standard deviation
//! @param min Minimum output limit
//! @param max Maximum output limit
//! @returns Random value from normal distribution
double WorkerGenetic::gaussian(double mean, double dev, double min, double max)
{
  std::normal_distribution<double> distribution{mean,dev};
  double temp = distribution(mRandomGenerator);
  return std::max(min,std::min(max,temp));
}



