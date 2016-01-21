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
//$Id: SymHop.cc 8138 2015-06-17 12:27:23Z petno25 $

#include "OpsWorkerComplexBurmen.h"
#include "OpsEvaluator.h"
#include <math.h>

using namespace Ops;

WorkerComplexBurmen::WorkerComplexBurmen(Evaluator *pEvaluator)
    : WorkerComplexRF(pEvaluator)
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
    emit message("Running optimization with Complex-RF algorithm by Bürmen.");

    distributePoints();

    mpEvaluator->evaluateAllPoints();
    emit objectivesChanged();

    calculateBestAndWorstId();

    //Run optimization loop
    mIterationCounter=0;
    for(; mIterationCounter<mnMaxIterations && !mIsAborted; ++mIterationCounter)
    {
        //Check convergence
        if(checkForConvergence()) break;

        //Increase all objective values (forgetting principle)
        applyForgettingFactor();

        //Calculate best and worst point
        calculateBestAndWorstId();

        //Find geometrical center
        QVector<int> ids = getIdsSortedFromWorstToBest();

        QVector< QVector<double> > nonReflectedPoints;
        for(int i=mNumPoints-1; i>mNumCandidates-1; --i)
        {
            nonReflectedPoints.append(mPoints[ids[i]]);
        }
        findCentroidPoint(nonReflectedPoints);

        //Reflect n worst point
        QVector< QVector<double> > newPoints;
        for(int i=0; i<mNumCandidates; ++i)
        {
            //Reflect first point
            mCandidatePoints[i] = reflect(mPoints[ids[i]], mCentroidPoint, mAlpha);
            newPoints.append(mCandidatePoints[i]);
        }
        emit candidatesChanged();

        //Evaluate new points
        mpEvaluator->evaluateAllCandidates();

        QVector<int> finishedCandidates;
        for(int i=0; i<mNumCandidates; ++i)
        {
            if(finishedCandidates.contains(i)) continue;
            if(mCandidateObjectives[i] < mObjectives[ids[mNumCandidates]]/*mObjectives[ids[i]]*/)
            {
                mPoints[ids[i]] = mCandidatePoints[i];
                mObjectives[ids[i]] = mCandidateObjectives[i];
                finishedCandidates.append(i);
            }
        }

        mRetractionCounter = 0;
        bool doBreak = false;
        while(finishedCandidates.size() != mNumCandidates && !mIsAborted)
        {
            ++mIterationCounter;

            for(int i=0; i<mNumCandidates; ++i)
            {
                if(finishedCandidates.contains(i)) continue;
                double a1 = 1.0-exp(-double(mRetractionCounter)/5.0);
                for(int j=0; j<mNumParameters; ++j)
                {
                    double best = mPoints[mBestId][j];
                    double maxDiff = getMaxPercentalParameterDiff()*10/(9.0+mRetractionCounter);
                    double r = opsRand();
                    mCandidatePoints[i][j] = (mCentroidPoint[j]*(1.0-a1) + best*a1 + newPoints[i][j])/2.0;
                    mCandidatePoints[i][j] += mRandomFactor*(mParameterMax[j]-mParameterMin[j])*maxDiff*(r-0.5);
                    mCandidatePoints[i][j] = qMin(mCandidatePoints[i][j], mParameterMax[j]);
                    mCandidatePoints[i][j] = qMax(mCandidatePoints[i][j], mParameterMin[j]);
                }
            }
            emit candidatesChanged();

            mpEvaluator->evaluateAllCandidates();

            for(int i=0; i<mNumCandidates; ++i)
            {
                if(finishedCandidates.contains(i)) continue;
                if(mCandidateObjectives[i] < mObjectives[ids[mNumCandidates]])
                {
                    mPoints[ids[i]] = mCandidatePoints[i];
                    mObjectives[ids[i]] = mCandidateObjectives[i];
                    finishedCandidates.append(i);
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


