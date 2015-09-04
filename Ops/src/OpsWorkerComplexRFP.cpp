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
//! @brief Contains the optimization worker class for the parallel Complex-RF algorithm
//!
//$Id: SymHop.cc 8138 2015-06-17 12:27:23Z petno25 $

#include "OpsWorkerComplexRFP.h"
#include "OpsEvaluator.h"
#include <math.h>

using namespace Ops;

WorkerComplexRFP::WorkerComplexRFP(Evaluator *pEvaluator)
    : WorkerComplexRF(pEvaluator)
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
            mvAlpha.append(mAlpha);
        }
        else if(mNumCandidates == 2)
        {
            mvAlpha.append(mAlphaMin);
            mvAlpha.append(mAlphaMax);
        }
        else
        {
            for(int i=0; i<mNumCandidates; ++i)
            {
                mvAlpha.append(mAlphaMin + double(i+1.0)/(double(mNumCandidates)+1.0)*(mAlphaMax-mAlphaMin));
            }
        }
    }

    emit message("Running optimization with Complex-RFP algorithm.");

    distributePoints();
    emit pointsChanged();

    mpEvaluator->evaluateAllPoints();
    emit objectivesChanged();

    calculateBestAndWorstId();

    //Run optimization loop
    mIterationCounter=0;
    for(; mIterationCounter<mnMaxIterations && !mIsAborted; ++mIterationCounter)
    {
        //Check convergence
        if(checkForConvergence()) break;

        applyForgettingFactor();

        calculateBestAndWorstId();

        findCentroidPoint();

        pickCandidateParticles();

        mpEvaluator->evaluateAllCandidates();

        examineCandidateParticles();

        mLastWorstId=mWorstId;
        calculateBestAndWorstId();

        bool doBreak = false;
        while(mLastWorstId == mWorstId && mIterationCounter<mnMaxIterations && !mIsAborted)
        {
            emit stepCompleted(mIterationCounter);
            ++mIterationCounter;
        //! @note Always iterate multiple steps (iterateSingle() is used only for statistics)
//            if(mMethod == TaskPrediction)
//            {
            if(multiRetract())
            {
                doBreak = true;
                break;
            }
//            }
//            else
//            {
//                abort = retract();
//            }
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

void WorkerComplexRFP::setParallelMethod(ParallelMethodT method)
{
    mMethod = method;
}

ParallelMethodT WorkerComplexRFP::getParallelMethod() const
{
    return mMethod;
}

void WorkerComplexRFP::setNumberOfPredictions(int value)
{
    mnPredictions = value;
}

void WorkerComplexRFP::setNumberOfRetractions(int value)
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

    for(int i=0; i<mTopLevelCandidates.size(); ++i)
    {
        delete(mTopLevelCandidates[i]);
    }
    mTopLevelCandidates.clear();


   if(mMethod == MultiDirection)
    {
        QVector<int> ids = getIdsSortedFromWorstToBest();

        for(int i=0; i<qMin(mNumPoints, mNumCandidates); ++i)
        {
            mWorstId = ids[i];
            findCentroidPoint();

            //Reflect first point
            mCandidatePoints[i] = reflect(mPoints[ids[i]], mCentroidPoint, mAlpha);

            Candidate *pCandidate = new Candidate();
            pCandidate->mpPoint = &mCandidatePoints[i];
            pCandidate->mpObjective = &mCandidateObjectives[i];
            pCandidate->idx = ids[0];
            mTopLevelCandidates.append(pCandidate);
        }
        emit candidatesChanged();
    }
    else if(mMethod == MultiDistance)     //Multi-distance
    {
        calculateBestAndWorstId();
        findCentroidPoint();

        for(int i=0; i<mNumCandidates; ++i)
        {
            mCandidatePoints[i] = reflect(mPoints[mWorstId], mCentroidPoint, mvAlpha[i]);

            Candidate *pCandidate = new Candidate();
            pCandidate->mpPoint = &mCandidatePoints[i];
            pCandidate->mpObjective = &mCandidateObjectives[i];
            pCandidate->idx = mWorstId;
            mTopLevelCandidates.append(pCandidate);
        }
        emit candidatesChanged();
    }
    else if(mMethod == TaskPrediction)     //TaskPrediction
    {
        //Sort ids by objective value (worst to best)
        QVector<int> ids = getIdsSortedFromWorstToBest();

        Candidate *pCandidate = new Candidate();
        mTopLevelCandidates.append(pCandidate);

        QVector< QVector<double> > otherPoints = mPoints;
        QVector< QVector<double> > centerPoints;
        for(int i=0; i<qMin(mnPredictions, mNumCandidates); ++i)
        {
            if(i!=0)
            {
                pCandidate->subCandidates.append(new Candidate());
                pCandidate = pCandidate->subCandidates.first();
            }

            if(i<mNumPoints)
            {
                otherPoints.remove(ids[i]);
                findCentroidPoint(otherPoints);
                centerPoints.append(mCentroidPoint);

                mCandidatePoints[i] = reflect(mPoints[ids[i]], mCentroidPoint, mAlpha);

                pCandidate->mpPoint = &mCandidatePoints[i];
                pCandidate->mpObjective = &mCandidateObjectives[i];
                pCandidate->idx = ids[i];

                otherPoints.insert(ids[i], mCandidatePoints[i]);
            }
            else
            {
                QVector<double> worstPoint = otherPoints.at(ids[i%mNumPoints]);
                otherPoints.remove(ids[i%mNumPoints]);
                findCentroidPoint(otherPoints);
                centerPoints.append(mCentroidPoint);

                mCandidatePoints[i] = reflect(worstPoint, mCentroidPoint, mAlpha);

                pCandidate->mpPoint = &mCandidatePoints[i];
                pCandidate->mpObjective = &mCandidateObjectives[i];
                pCandidate->idx = ids[i%mNumPoints];

                otherPoints.insert(ids[i%mNumPoints], mCandidatePoints[i]);
            }
        }

        //Use additional threads to compute a few retraction steps from first candidate
        int worstCounter=0;
        int extraSteps = mNumCandidates-mnPredictions-mnRetractions;

        QVector<double> newPoint = (*mTopLevelCandidates[0]->mpPoint);
        for(int t=mnPredictions; t<qMin(mNumCandidates-extraSteps, mNumCandidates-mnPredictions); ++t)
        {
            double a1 = 1.0-exp(-double(worstCounter)/5.0);
            findCentroidPoint();
            for(int j=0; j<mNumParameters; ++j)
            {
                double best = mPoints[mBestId][j];
                QVector<QVector<double> > points = mPoints;
                points[mWorstId] = newPoint;
                double maxDiff = getMaxPercentalParameterDiff(points);
                double r = (double)rand() / (double)RAND_MAX;
                mCandidatePoints[t][j] = (mCentroidPoint[j]*(1.0-a1) + best*a1 + newPoint[j])/2.0 + mRandomFactor*(mParameterMax[j]-mParameterMin[j])*maxDiff*(r-0.5);
                mCandidatePoints[t][j] = qMin(mCandidatePoints[t][j], mParameterMax[j]);
                mCandidatePoints[t][j] = qMax(mCandidatePoints[t][j], mParameterMin[j]);
            }

            Candidate *pRetractionCandidate = new Candidate();
            pRetractionCandidate->mpPoint = &mCandidatePoints[t];
            pRetractionCandidate->mpObjective = &mCandidateObjectives[t];
            mTopLevelCandidates[0]->retractions.append(pRetractionCandidate);

            newPoint = mCandidatePoints[t];
            ++worstCounter;
        }


        //Also calculate first retraction for next extraSteps candidates
        int centerCounter = 0;
        if(!mTopLevelCandidates[0]->subCandidates.isEmpty())
        {
            Candidate *pCandidate = mTopLevelCandidates[0]->subCandidates[0];
            for(int i=0; i<extraSteps; ++i)
            {
                newPoint = (*pCandidate->mpPoint);
                int t=mNumCandidates-extraSteps+i;
                ++centerCounter;
                mCentroidPoint = centerPoints[centerCounter];
                for(int j=0; j<mNumParameters; ++j)
                {
                    QVector<QVector<double> > points = mPoints;
                    points[mWorstId] = newPoint;
                    double maxDiff = getMaxPercentalParameterDiff(points);
                    double r = (double)rand() / (double)RAND_MAX;
                    mCandidatePoints[t][j] = (mCentroidPoint[j] + newPoint[j])/2.0 + mRandomFactor*(mParameterMax[j]-mParameterMin[j])*maxDiff*(r-0.5);
                    mCandidatePoints[t][j] = qMin(mCandidatePoints[t][j], mParameterMax[j]);
                    mCandidatePoints[t][j] = qMax(mCandidatePoints[t][j], mParameterMin[j]);
                }
                Candidate *pRetractionCandidate = new Candidate();
                pRetractionCandidate->mpPoint = &mCandidatePoints[t];
                pRetractionCandidate->mpObjective = &mCandidateObjectives[t];
                pCandidate->retractions.append(pRetractionCandidate);

                if(!pCandidate->subCandidates.isEmpty())
                {
                    pCandidate = pCandidate->subCandidates[0];
                }
                else
                {
                    break;
                }
            }
        }

        emit candidatesChanged();
    }
}

void WorkerComplexRFP::examineCandidateParticles()
{
    Candidate *pCandidate = mTopLevelCandidates[0];
    for(int i=1; i<mTopLevelCandidates.size(); ++i)
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
        for(int ptId=0; ptId<mNumPoints; ++ptId)
        {
            if(mObjectives[ptId] > (*pCandidate->mpObjective))
            {
                ++nWorsePoints;
            }
        }

        mPoints[pCandidate->idx] = (*pCandidate->mpPoint);
        mObjectives[pCandidate->idx] = (*pCandidate->mpObjective);
        emit pointChanged(pCandidate->idx);
        emit objectiveChanged(pCandidate->idx);

        mLastWorstId = mWorstId;
        calculateBestAndWorstId();
        if(mWorstId == mLastWorstId)
        {
            mpFailedCandidate = pCandidate;
            break;
        }

        if(!pCandidate->subCandidates.isEmpty())
        {
            pCandidate = pCandidate->subCandidates[0];
            for(int i=1; i<pCandidate->subCandidates.size(); ++i)
            {
                if((*pCandidate->subCandidates[i]->mpObjective) < (*pCandidate->mpObjective))
                {
                    pCandidate = pCandidate->subCandidates[i];
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
    if(!mpFailedCandidate->retractions.isEmpty())
    {
        Candidate *pCandidate = mpFailedCandidate->retractions.first();

        mPoints[mWorstId] = (*pCandidate->mpPoint);
        mObjectives[mWorstId] = (*pCandidate->mpObjective);
        emit objectiveChanged(mWorstId);
        emit pointChanged(mWorstId);

        mpFailedCandidate->retractions.remove(0);

        ++mRetractionCounter;

        return checkForConvergence();
    }

    QVector<double> newPoint = mPoints[mWorstId];

    //Move first reflected point
    for(int t=0; t<mNumCandidates; ++t)
    {
        double a1 = 1.0-exp(-double(mRetractionCounter)/5.0);
        for(int j=0; j<mNumParameters; ++j)
        {
            double best = mPoints[mBestId][j];
            double maxDiff = getMaxPercentalParameterDiff();
            double r = (double)rand() / (double)RAND_MAX;
            mCandidatePoints[t][j] = (mCentroidPoint[j]*(1.0-a1) + best*a1 + newPoint[j])/2.0 + mRandomFactor*(mParameterMax[j]-mParameterMin[j])*maxDiff*(r-0.5);
            mCandidatePoints[t][j] = qMin(mCandidatePoints[t][j], mParameterMax[j]);
            mCandidatePoints[t][j] = qMax(mCandidatePoints[t][j], mParameterMin[j]);
        }

        newPoint = mCandidatePoints[t];
        ++mRetractionCounter;
    }
    emit candidatesChanged();

    mpEvaluator->evaluateAllCandidates();
    emit objectivesChanged();

    //Replace worst point with first candidate point that is better, if any
    for(int o=0; o<mNumCandidates; ++o)
    {
        mPoints[mWorstId] = mCandidatePoints[o];
        mObjectives[mWorstId] = mCandidateObjectives[o];
        emit pointChanged(mWorstId);
        emit objectiveChanged(mWorstId);

        int prevWorst = mWorstId;
        calculateBestAndWorstId();
        if(prevWorst != mWorstId)
        {
            mIterCount = mRetractionCounter-mNumCandidates+o+1;

            return checkForConvergence();
        }
    }

    //Calculate best and worst points
    mLastWorstId=mWorstId;
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
    for(int i=0; i<subCandidates.size(); ++i)
    {
        delete(subCandidates[i]);
    }
    subCandidates.clear();

    for(int i=0; i<retractions.size(); ++i)
    {
        delete(retractions[i]);
    }
    retractions.clear();
}
