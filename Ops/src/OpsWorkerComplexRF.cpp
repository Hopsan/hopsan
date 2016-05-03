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
//! @brief Contains the optimization worker class for the Complex-RF algorithm
//!
//$Id$

#include "OpsWorkerComplexRF.h"
#include "OpsEvaluator.h"
#include "OpsMessageHandler.h"
#include <math.h>
#include <sstream>

using namespace Ops;

WorkerComplexRF::WorkerComplexRF(Evaluator *pEvaluator, MessageHandler *pMessageHandler)
    : WorkerSimplex(pEvaluator, pMessageHandler)
{
    mRetractionCounter = 0;
}

AlgorithmT WorkerComplexRF::getAlgorithm()
{
    return ComplexRF;
}

void WorkerComplexRF::initialize()
{
    WorkerSimplex::initialize();

    mKf = 1.0-pow(mAlpha/2.0, mGamma/mNumPoints);
}

void WorkerComplexRF::run()
{
    mpMessageHandler->printMessage("Running optimization with Complex-RF algorithm.");

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
        findCentroidPoint();

        //Reflect worst point
        mCandidatePoints[0] = reflect(mPoints[mWorstId], mCentroidPoint, mAlpha);
        mpMessageHandler->candidateChanged(0);
        //std::vector<double> newPoint = mCandidatePoints[0]; //Remember the new point, in case we need to iterate below

        //Evaluate new point
        mpEvaluator->evaluateCandidate(0);
        mPoints[mWorstId] = mCandidatePoints[0];
        mObjectives[mWorstId] = mCandidateObjectives[0];

        mpMessageHandler->pointChanged(mWorstId);
        mpMessageHandler->objectiveChanged(mWorstId);

        calculateBestAndWorstId();

        mpMessageHandler->stepCompleted(mIterationCounter);

        //Retract until worst point is no longer the same
        mRetractionCounter = 0;
        bool doBreak = false;
        while(mLastWorstId == mWorstId && !mpMessageHandler->aborted())
        {
           // mpMessageHandler->stepCompleted(mIterationCounter);

            ++mIterationCounter;

            mpMessageHandler->stepCompleted(mIterationCounter);

            if(mIterationCounter>=mnMaxIterations)
            {
                --mIterationCounter;    //Needed because for-loop will increase it by one anyway
                break;
            }

            if(retract())
            {
                doBreak = true;
                break;
            }
        }

        if(doBreak)
        {
            break;
        }
    }


    if(mpMessageHandler->aborted())
    {
        mpMessageHandler->printMessage("Optimization was aborted after "+std::to_string(mIterationCounter)+" iterations.");
    }
    else if(mIterationCounter == mnMaxIterations)
    {
        mpMessageHandler->printMessage(std::string("Optimization failed to converge after "+std::to_string(mIterationCounter)+" iterations"));
    }
    else
    {
        mpMessageHandler->printMessage("Optimization converged in parameter values after "+std::to_string(mIterationCounter)+" iterations.");
    }

    // Clean up
    finalize();

    return;
}

void WorkerComplexRF::setReflectionFactor(double value)
{
    mAlpha = value;
}

void WorkerComplexRF::setForgettingFactor(double value)
{
    mGamma = value;
}

bool WorkerComplexRF::retract()
{
    std::vector<double> newPoint = mPoints[mWorstId];

        //Move first reflected point
    double a1 = 1.0-exp(-double(mRetractionCounter)/5.0);
    for(size_t j=0; j<mNumParameters; ++j)
    {
        double best = mPoints[mBestId][j];
        double maxDiff = getMaxPercentalParameterDiff()*10/(9.0+mRetractionCounter);
        double r = opsRand();
        mCandidatePoints[0][j] = (mCentroidPoint[j]*(1.0-a1) + best*a1 + newPoint[j])/2.0;
        mCandidatePoints[0][j] += mRandomFactor*(mParameterMax[j]-mParameterMin[j])*maxDiff*(r-0.5);
        mCandidatePoints[0][j] = std::min(mCandidatePoints[0][j], mParameterMax[j]);
        mCandidatePoints[0][j] = std::max(mCandidatePoints[0][j], mParameterMin[j]);
    }
    mpMessageHandler->candidateChanged(0);

    ++mRetractionCounter;

    //Evaluate new point
    mpEvaluator->evaluateCandidate(0);
    mPoints[mWorstId] = mCandidatePoints[0];
    mObjectives[mWorstId] = mCandidateObjectives[0];

    mpMessageHandler->pointChanged(mWorstId);
    mpMessageHandler->objectiveChanged(mWorstId);

    calculateBestAndWorstId();

    mpMessageHandler->pointChanged(mWorstId);
    mpMessageHandler->objectiveChanged(mWorstId);

    return checkForConvergence();
}


void WorkerComplexRF::applyForgettingFactor()
{
    double maxObj = mObjectives[0];
    double minObj = mObjectives[0];
    for(size_t i=0; i<mNumPoints; ++i)
    {
        double obj = mObjectives[i];
        if(obj > maxObj) maxObj = obj;
        if(obj < minObj) minObj = obj;
    }
    for(size_t i=0; i<mNumPoints; ++i)
    {
        mObjectives[i] = mObjectives[i]+(maxObj-minObj)*mKf;
    }
    mpMessageHandler->objectivesChanged();
}

