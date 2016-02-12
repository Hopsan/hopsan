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
                if(mCandidateObjectives[i] < mObjectives[ids[mNumCandidates]])
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


