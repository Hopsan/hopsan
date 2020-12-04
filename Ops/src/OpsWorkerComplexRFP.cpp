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
//! @brief Contains the optimization worker class for the parallel Complex-RF algorithm
//!
//$Id$

#include "OpsWorkerComplexRFP.h"
#include "OpsEvaluator.h"
#include "OpsMessageHandler.h"
#include <math.h>
#include <algorithm>

using namespace Ops;

WorkerComplexRFP::WorkerComplexRFP(Evaluator *pEvaluator, MessageHandler *pMessageHandler)
    : WorkerComplexRF(pEvaluator, pMessageHandler)
{
    mRetractionCounter = 0;
    mMethod = TaskPrediction;
    mpFailedCandidate = 0;
}

AlgorithmT WorkerComplexRFP::getAlgorithm()
{
    return ComplexRFP;
}

void WorkerComplexRFP::initialize()
{
    WorkerComplexRF::initialize();
}

void WorkerComplexRFP::run()
{

    if(mMethod == MultiDistance)
    {
        mvAlpha.clear();
        if(mNumCandidates == 1)
        {
            mvAlpha.push_back(mAlpha);
        }
        else
        {
            for(size_t i=0; i<mNumCandidates; ++i)
            {
                mvAlpha.push_back(mAlphaMin + double(i+1.0)/(double(mNumCandidates)+1.0)*(mAlphaMax-mAlphaMin));
            }
        }
    }

    mpMessageHandler->printMessage("Running optimization with Complex-RFP algorithm.");

    distributePoints();
    mpMessageHandler->pointsChanged();

    mpEvaluator->evaluateAllPointsWithSurrogateModel();
    mpMessageHandler->objectivesChanged();

    calculateBestAndWorstId();

    //Run optimization loop
    mIterationCounter=0;
    for(; mIterationCounter<mnMaxIterations && !mpMessageHandler->aborted(); ++mIterationCounter)
    {
        //Check convergence
        if(checkForConvergence()) break;

        //Increase all objective values (forgetting principle)
        //applyForgettingFactor();

        //Identify best and worst point
        calculateBestAndWorstId();

        //Find geometrical center
        findCentroidPoint();

        //Pick candidates (depending on algorithm)
        pickCandidateParticles();

        //Evaluate candidates
        mpEvaluator->evaluateAllCandidatesWithSurrogateModel();

        //Examine outcome of evaluation (depending on algorithm)
        examineCandidateParticles();

        //Identify best and worst point
        calculateBestAndWorstId();

        mpMessageHandler->stepCompleted(mIterationCounter);

        //Retract towards centroid if last reflection failed
        mRetractionCounter = 0;
        bool doBreak = false;
        while(mWorstId == mpFailedCandidate->idx && mIterationCounter<mnMaxIterations && !mpMessageHandler->aborted())
        {
            mpMessageHandler->stepCompleted(mIterationCounter);

            if(multiRetract())
            {
                doBreak = true;
                break;
            }
        }

        if(doBreak)
        {
            break;
        }
        mpMessageHandler->stepCompleted(mIterationCounter);
    }


    if(mpMessageHandler->aborted())
    {
        mpMessageHandler->printMessage("Optimization was aborted after "+std::to_string(mIterationCounter)+" iterations.");
    }
    else if(mIterationCounter == mnMaxIterations)
    {

        mpMessageHandler->printMessage("Optimization failed to converge after "+std::to_string(mIterationCounter)+" iterations.");
    }
    else
    {
        mpMessageHandler->printMessage("Optimization converged in parameter values after "+std::to_string(mIterationCounter)+" iterations.");
    }

    // Clean up
    finalize();

    return;
}

void WorkerComplexRFP::setParallelMethod(ParallelMethodT method)
{
    mMethod = method;
}

ParallelMethodT WorkerComplexRFP::getParallelMethod() const
{
    return mMethod;
}

void WorkerComplexRFP::setNumberOfPredictions(size_t value)
{
    mnPredictions = value;
}

void WorkerComplexRFP::setNumberOfRetractions(size_t value)
{
    mnRetractions = value;
}

void WorkerComplexRFP::setMinimumReflectionFactor(double value)
{
    mAlphaMin = value;
}

void WorkerComplexRFP::setMaximumReflectionFactor(double value)
{
    mAlphaMax = value;
}


void WorkerComplexRFP::pickCandidateParticles()
{

    for(size_t i=0; i<mTopLevelCandidates.size(); ++i)
    {
        delete(mTopLevelCandidates[i]);
    }
    mTopLevelCandidates.clear();


   if(mMethod == MultiDirection)
    {
        std::vector<size_t> ids = getIdsSortedFromWorstToBest();

        for(size_t i=0; i<std::min(mNumPoints, mNumCandidates); ++i)
        {
            mWorstId = ids[i];
            findCentroidPoint();

            //Reflect first point
            mCandidatePoints[i] = reflect(mPoints[ids[i]], mCentroidPoint, mAlpha);

            Candidate *pCandidate = new Candidate();
            pCandidate->mpPoint = &mCandidatePoints[i];
            pCandidate->mpObjective = &mCandidateObjectives[i];
            pCandidate->idx = ids[0];
            mTopLevelCandidates.push_back(pCandidate);
        }
        mpMessageHandler->candidatesChanged();
    }
    else if(mMethod == MultiDistance)     //Multi-distance
    {
        calculateBestAndWorstId();
        findCentroidPoint();

        for(size_t i=0; i<mNumCandidates; ++i)
        {
            mCandidatePoints[i] = reflect(mPoints[mWorstId], mCentroidPoint, mvAlpha[i]);

            Candidate *pCandidate = new Candidate();
            pCandidate->mpPoint = &mCandidatePoints[i];
            pCandidate->mpObjective = &mCandidateObjectives[i];
            pCandidate->idx = mWorstId;
            mTopLevelCandidates.push_back(pCandidate);
        }
        mpMessageHandler->candidatesChanged();
    }
    else if(mMethod == TaskPrediction)     //TaskPrediction
    {
        //Sort ids by objective value (worst to best)
        std::vector<size_t> ids = getIdsSortedFromWorstToBest();

        Candidate *pCandidate = new Candidate();
        mTopLevelCandidates.push_back(pCandidate);


        std::vector< std::vector<double> > otherPoints = mPoints;
        std::vector< std::vector<double> > centerPoints;
        for(size_t i=0; i<std::min(mnPredictions, mNumCandidates); ++i)
        {
            if(i!=0)
            {
                pCandidate->subCandidates.push_back(new Candidate());
                pCandidate = pCandidate->subCandidates[0];
            }

            if(i<mNumPoints)
            {
                removeFromVector(otherPoints,ids[i]);
                findCentroidPoint(otherPoints);
                centerPoints.push_back(mCentroidPoint);

                mCandidatePoints[i] = reflect(mPoints[ids[i]], mCentroidPoint, mAlpha);

                pCandidate->mpPoint = &mCandidatePoints[i];
                pCandidate->mpObjective = &mCandidateObjectives[i];
                pCandidate->idx = ids[i];

                otherPoints.insert(otherPoints.begin()+ids[i], mCandidatePoints[i]);
            }
            else
            {
                std::vector<double> worstPoint = otherPoints.at(ids[i%mNumPoints]);
                removeFromVector(otherPoints,ids[i%mNumPoints]);
                findCentroidPoint(otherPoints);
                centerPoints.push_back(mCentroidPoint);

                mCandidatePoints[i] = reflect(worstPoint, mCentroidPoint, mAlpha);

                pCandidate->mpPoint = &mCandidatePoints[i];
                pCandidate->mpObjective = &mCandidateObjectives[i];
                pCandidate->idx = ids[i%mNumPoints];

                otherPoints.insert(otherPoints.begin()+ids[i%mNumPoints], mCandidatePoints[i]);
            }
        }

        //Use additional threads to compute a few retraction steps from first candidate
        size_t worstCounter=0;
        size_t extraSteps = mNumCandidates-mnPredictions-mnRetractions;

        std::vector<double> newPoint = (*mTopLevelCandidates[0]->mpPoint);
        for(size_t t=mnPredictions; t<mNumCandidates-extraSteps; ++t)
        {
            double a1 = 1.0-exp(-double(worstCounter)/5.0);
            findCentroidPoint();
            for(size_t j=0; j<mNumParameters; ++j)
            {
                double best = mPoints[mBestId][j];
                std::vector<std::vector<double> > points = mPoints;
                points[mWorstId] = newPoint;
                double maxDiff = getMaxPercentalParameterDiff(points)*10/(9.0+worstCounter);
                double r = opsRand();
                mCandidatePoints[t][j] = (mCentroidPoint[j]*(1.0-a1) + best*a1 + newPoint[j])/2.0 + mRandomFactor*(mParameterMax[j]-mParameterMin[j])*maxDiff*(r-0.5);
                mCandidatePoints[t][j] = std::min(mCandidatePoints[t][j], mParameterMax[j]);
                mCandidatePoints[t][j] = std::max(mCandidatePoints[t][j], mParameterMin[j]);
            }

            Candidate *pRetractionCandidate = new Candidate();
            pRetractionCandidate->mpPoint = &mCandidatePoints[t];
            pRetractionCandidate->mpObjective = &mCandidateObjectives[t];
            mTopLevelCandidates[0]->retractions.push_back(pRetractionCandidate);

            newPoint = mCandidatePoints[t];
            ++worstCounter;
        }


        //Also calculate first retraction for next extraSteps candidates
        size_t centerCounter = 0;
        if(!mTopLevelCandidates[0]->subCandidates.size() == 0)
        {
            Candidate *pCandidate = mTopLevelCandidates[0]->subCandidates[0];
            for(size_t i=0; i<extraSteps; ++i)
            {
                newPoint = (*pCandidate->mpPoint);
                int t=mNumCandidates-extraSteps+i;
                ++centerCounter;
                mCentroidPoint = centerPoints[centerCounter];
                for(size_t j=0; j<mNumParameters; ++j)
                {
                    std::vector<std::vector<double> > points = mPoints;
                    points[mWorstId] = newPoint;
                    double maxDiff = getMaxPercentalParameterDiff(points)*10/(9.0+0);
                    double r = opsRand();
                    mCandidatePoints[t][j] = (mCentroidPoint[j] + newPoint[j])/2.0 + mRandomFactor*(mParameterMax[j]-mParameterMin[j])*maxDiff*(r-0.5);
                    mCandidatePoints[t][j] = std::min(mCandidatePoints[t][j], mParameterMax[j]);
                    mCandidatePoints[t][j] = std::max(mCandidatePoints[t][j], mParameterMin[j]);
                }
                Candidate *pRetractionCandidate = new Candidate();
                pRetractionCandidate->mpPoint = &mCandidatePoints[t];
                pRetractionCandidate->mpObjective = &mCandidateObjectives[t];
                pCandidate->retractions.push_back(pRetractionCandidate);

                if(!pCandidate->subCandidates.size() == 0)
                {
                    pCandidate = pCandidate->subCandidates[0];
                }
                else
                {
                    break;
                }
            }
        }

        mpMessageHandler->candidatesChanged();
    }
}

void WorkerComplexRFP::examineCandidateParticles()
{
    Candidate *pCandidate = mTopLevelCandidates[0];
    for(size_t i=1; i<mTopLevelCandidates.size(); ++i)
    {
        if((*mTopLevelCandidates[i]->mpObjective) < (*pCandidate->mpObjective))
        {
            pCandidate = mTopLevelCandidates[i];
        }
    }

    while(true)
    {
        applyForgettingFactor();

        int nWorsePoints=0;
        for(size_t ptId=0; ptId<mNumPoints; ++ptId)
        {
            if(mObjectives[ptId] > (*pCandidate->mpObjective))
            {
                ++nWorsePoints;
            }
        }

        mPoints[pCandidate->idx] = (*pCandidate->mpPoint);
        mObjectives[pCandidate->idx] = (*pCandidate->mpObjective);
        mpMessageHandler->pointChanged(pCandidate->idx);
        mpMessageHandler->objectiveChanged(pCandidate->idx);

        calculateBestAndWorstId();
        if(mWorstId == pCandidate->idx)
        {
            mpFailedCandidate = pCandidate;
            break;
        }

        if(!pCandidate->subCandidates.size() == 0)
        {
            pCandidate = pCandidate->subCandidates[0];
            for(size_t i=1; i<pCandidate->subCandidates.size(); ++i)
            {
                if((*pCandidate->subCandidates[i]->mpObjective) < (*pCandidate->mpObjective))
                {
                    pCandidate = pCandidate->subCandidates[i];
                    mWorstId = pCandidate->idx;
                }
            }
        }
        else
        {
            mpFailedCandidate = pCandidate;
            break;
        }
    }
}

bool WorkerComplexRFP::multiRetract()
{
    //Check the already evaluated iteration points (if any)
    if(!mpFailedCandidate->retractions.size() == 0)
    {
        Candidate *pCandidate = mpFailedCandidate->retractions[0];

        mPoints[mWorstId] = (*pCandidate->mpPoint);
        mObjectives[mWorstId] = (*pCandidate->mpObjective);
        mpMessageHandler->objectiveChanged(mWorstId);
        mpMessageHandler->pointChanged(mWorstId);

        removeFromVector(mpFailedCandidate->retractions,0);

        ++mRetractionCounter;

        calculateBestAndWorstId();

        return checkForConvergence();
    }

    ++mIterationCounter;    //Only increase iteration counter for non-pre-evaluated retractions

    std::vector<double> newPoint = mPoints[mWorstId];

    //Move first reflected point
    for(size_t t=0; t<mNumCandidates; ++t)
    {
        double a1 = 1.0-exp(-double(mRetractionCounter)/5.0);
        for(size_t j=0; j<mNumParameters; ++j)
        {
            double best = mPoints[mBestId][j];
            double maxDiff = getMaxPercentalParameterDiff()*10/(9.0+mRetractionCounter);;
            double r = opsRand();
            mCandidatePoints[t][j] = (mCentroidPoint[j]*(1.0-a1) + best*a1 + newPoint[j])/2.0 + mRandomFactor*(mParameterMax[j]-mParameterMin[j])*maxDiff*(r-0.5);
            mCandidatePoints[t][j] = std::min(mCandidatePoints[t][j], mParameterMax[j]);
            mCandidatePoints[t][j] = std::max(mCandidatePoints[t][j], mParameterMin[j]);
        }

        newPoint = mCandidatePoints[t];
        ++mRetractionCounter;
    }
    mpMessageHandler->candidatesChanged();

    mpEvaluator->evaluateAllCandidatesWithSurrogateModel();
    mpMessageHandler->objectivesChanged();

    //Replace worst point with first candidate point that is better, if any
    for(size_t o=0; o<mNumCandidates; ++o)
    {
        mPoints[mWorstId] = mCandidatePoints[o];
        mObjectives[mWorstId] = mCandidateObjectives[o];
        mpMessageHandler->pointChanged(mWorstId);
        mpMessageHandler->objectiveChanged(mWorstId);

        size_t prevWorst = mWorstId;
        calculateBestAndWorstId();
        if(prevWorst != mWorstId)
        {
            mIterCount = mRetractionCounter-mNumCandidates+o+1;

            return checkForConvergence();
        }
    }

    //Calculate best and worst points
    calculateBestAndWorstId();


    return checkForConvergence();
}



Candidate::Candidate()
{
    this->idx = 0;
    this->mpObjective = 0;
    this->mpPoint = 0;
}

Candidate::~Candidate()
{
    for(size_t i=0; i<subCandidates.size(); ++i)
    {
        delete(subCandidates[i]);
    }
    subCandidates.clear();

    for(size_t i=0; i<retractions.size(); ++i)
    {
        delete(retractions[i]);
    }
    retractions.clear();
}
