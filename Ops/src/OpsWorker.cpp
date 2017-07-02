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
//! @brief Contains the optimization worker base class
//!
//$Id$

#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <time.h>
#include <algorithm>

//#include <QDebug>
#include "OpsWorker.h"
#include "OpsEvaluator.h"
#include "OpsMessageHandler.h"


using namespace Ops;


//! @brief Checks for convergence (in either of the algorithms)
Worker::Worker(Evaluator *pEvaluator, MessageHandler *pMessageHandler)
{
    mpEvaluator = pEvaluator;
    mpEvaluator->setWorker(this);

    mpMessageHandler = pMessageHandler;

    mNumCandidates = 1;
    mNumPoints = 1;
    mNumParameters = 1;

    mCandidateObjectives.resize(1);
    mCandidatePoints.resize(1);
    mObjectives.resize(1);
    mPoints.resize(1);
    mDistribution = SamplingRandom;
}

Worker::~Worker()
{
    //Nothing to do
}


AlgorithmT Worker::getAlgorithm()
{
    return Undefined;
}


//! @brief Initialization function for optimization worker base class (should never be called directly)
void Worker::initialize()
{
    mBestId = 0;
    mWorstId = 1;

    mpMessageHandler->setAborted(false);
}


//! @brief Run function for optimization worker base class (should never be called directly)
void Worker::run()
{
    assert("You should implement your own run() function." == 0);
}


//! @brief Finalize function for optimization worker base class (should never be called directly)
void Worker::finalize()
{
}

void Worker::distrubteCandidatePoints()
{
    distributePoints(&mCandidatePoints);
}

void Worker::distributePoints()
{
    distributePoints(&mPoints);
}


void Worker::distributePoints(std::vector<std::vector<double> > *pVector)
{
    srand ( time(NULL) );

    size_t nPoints = pVector->size();

    if(mDistribution == SamplingRandom)
    {
        for(size_t p=0; p<nPoints; ++p)
        {
            for(size_t i=0; i<mNumParameters; ++i)
            {
                auto temp = std::make_pair(p, i);
                if (std::find(mIgnoredWhenSampling.begin(),
                              mIgnoredWhenSampling.end(),
                              temp) != mIgnoredWhenSampling.end())
                {
                    continue;
                }

                double r = opsRand();
                (*pVector)[p][i] = mParameterMin[i] + r*(mParameterMax[i]-mParameterMin[i]);
            }
        }
    }
    else if(mDistribution == SamplingLatinHypercube)
    {
        size_t m=nPoints;
        size_t n=mNumParameters;

        std::vector<std::vector<int> > usedIntervals;
        std::vector<std::vector<double> > points;
        for(size_t i=0; i<m; ++i)
        {
            std::vector<double> newPoint;
            std::vector<int> interval;
            for(size_t j=0; j<n; ++j)
            {
                double min = mParameterMin[j];
                double max = mParameterMax[j];
                double x;
                auto temp = std::make_pair(i, j);
                if (std::find(mIgnoredWhenSampling.begin(),
                              mIgnoredWhenSampling.end(),
                              temp) != mIgnoredWhenSampling.end())
                {
                    x = (*pVector)[i][j];
                }
                else
                {
                    x = min+opsRand()*(max-min);
                }
                newPoint.push_back(x);
                //interval.push_back((max-min)/m),fmod(newPoint.last());
                interval.push_back(int(floor(x/(max-min)*m)));
            }

            if(inVector(usedIntervals,interval))
            {
                --i;
                continue;
            }
            else
            {
                usedIntervals.push_back(interval);
                (*pVector)[i] = newPoint;
            }
        }
    }

    mpMessageHandler->pointsChanged();
}


//! @brief Checks whether or not any of the convergence criteria has been fulfilled
bool Worker::checkForConvergence()
{
    return (getMaxPercentalParameterDiff() < mTolerance);
}



//! @brief Calculates indexes of best and worst point
void Worker::calculateBestAndWorstId()
{
    mLastWorstId = mWorstId;

    double maxObj = mObjectives[0];
    double minObj = mObjectives[0];
    mWorstId=0;
    mBestId=0;
    for(size_t i=1; i<mNumPoints; ++i)
    {
        double obj = mObjectives[i];
        if(obj > maxObj)
        {
            maxObj = obj;
            mWorstId = i;
        }
        if(obj < minObj)
        {
            minObj = obj;
            mBestId = i;
        }
    }
    if(mWorstId == mBestId)
    {
        mWorstId = 0;
        mBestId = 1;
    }
}

std::vector<size_t> Worker::getIdsSortedFromWorstToBest()
{
    //Sort ids by objective value (worst to best)
    std::vector<size_t> ids;
    while(ids.size() != mNumPoints)
    {
        int worstId = 0;
        double worstObjective = -1000000000;
        for(size_t i=0; i<mNumPoints; ++i)
        {
            if(inVector(ids, i)) continue;  //Ignore already added indexes

            if(mObjectives[i] > worstObjective)
            {
                worstObjective = mObjectives[i];
                worstId = i;
            }
        }
        ids.push_back(worstId);
    }

    return ids;
}


size_t Worker::getBestId()
{
    return mBestId;
}

size_t Worker::getWorstId()
{
    return mWorstId;
}

size_t Worker::getLastWorstId()
{
    return mLastWorstId;
}



void Worker::setNumberOfPoints(size_t value)
{
    mNumPoints = value;

    mPoints.resize(mNumPoints);
    mObjectives.resize(mNumPoints);
    for(size_t p=0; p<mNumPoints; ++p)
    {
        mPoints[p].resize(mNumParameters);
    }
}


void Worker::setNumberOfParameters(size_t value)
{
    mNumParameters = value;
    mParameterMin.resize(mNumParameters);
    mParameterMax.resize(mNumParameters);
    for(size_t p=0; p<mNumPoints; ++p)
    {
        mPoints[p].resize(mNumParameters);
    }
    for(size_t p=0; p<mNumCandidates; ++p)
    {
        mCandidatePoints[p].resize(mNumParameters);
    }
}


void Worker::setNumberOfCandidates(size_t value)
{
    mNumCandidates = value;

    mCandidatePoints.resize(mNumCandidates);
    mCandidateObjectives.resize(mNumCandidates);
    for(size_t p=0; p<mNumCandidates; ++p)
    {
        mCandidatePoints[p].resize(mNumParameters);
    }
}


void Worker::setParameterNames(std::vector<const char*> names)
{
    for (const char* &name : names)
    {
        mParameterNames.push_back(name);
    }
}


void Worker::setConvergenceTolerance(double value)
{
    mTolerance = value;
}


void Worker::setMaxNumberOfIterations(size_t value)
{
    mnMaxIterations = value;
}

void Worker::setTolerance(double value)
{
    mTolerance = value;
}

void Worker::setSamplingMethod(SamplingT dist)
{
    mDistribution = dist;
}

size_t Worker::getNumberOfCandidates()
{
    return mNumCandidates;
}

size_t Worker::getNumberOfPoints()
{
    return mNumPoints;
}

size_t Worker::getNumberOfParameters()
{
    return mNumParameters;
}

size_t Worker::getMaxNumberOfIterations()
{
    return mnMaxIterations;
}

size_t Worker::getCurrentNumberOfIterations()
{
    return mIterationCounter;
}

double Worker::opsRand()
{
    return double(rand())/double(RAND_MAX);
}

void Worker::setParameterLimits(size_t idx, double min, double max)
{
    mParameterMin[idx] = min;
    mParameterMax[idx] = max;
}

void Worker::getParameterLimits(size_t idx, double &min, double &max) const
{
    min = mParameterMin[idx];
    max = mParameterMax[idx];
}


void Worker::setCandidateObjectiveValue(size_t idx, double value)
{
    if(idx > mCandidateObjectives.size()-1)
    {
        return;
    }
    mCandidateObjectives[idx] = value;
}


double Worker::getObjectiveValue(size_t idx) const
{
    if(idx > mObjectives.size()-1)
    {
        return 0;
    }
    return mObjectives[idx];
}

std::vector<double> &Worker::getObjectiveValues()
{
    return mObjectives;
}

std::vector<std::vector<double> > &Worker::getPoints()
{
    return mPoints;
}

std::vector<std::vector<double> > &Worker::getCandidatePoints()
{
    return mCandidatePoints;
}


double Worker::getCandidateParameter(const size_t pointIdx, const size_t parIdx) const
{
    if(mCandidatePoints.size() < pointIdx+1)
    {
        return 0;
    }
    else if(mCandidatePoints[pointIdx].size() < parIdx)
    {
        return 0;
    }
    return mCandidatePoints[pointIdx][parIdx];
}

double Worker::getParameter(const size_t pointIdx, const size_t parIdx) const
{
    if(mPoints.size() < pointIdx+1)
    {
        return 0;
    }
    else if(mPoints[pointIdx].size() < parIdx)
    {
        return 0;
    }
    return mPoints[pointIdx][parIdx];
}


void Worker::setIgnoreParameterWhenSampling(const size_t pointIdx, const size_t parIdx)
{
    mIgnoredWhenSampling.push_back(std::pair<size_t,size_t>(pointIdx,parIdx));
}


void Worker::setParameter(const size_t pointIdx, const size_t parIdx, const double value)
{
    if(mPoints.size() < pointIdx+1)
    {
        return;
    }
    else if(mPoints[pointIdx].size() < parIdx)
    {
        return;
    }
    mPoints[pointIdx][parIdx] = value;
}



double Worker::getMaxPercentalParameterDiff()
{
    return getMaxPercentalParameterDiff(mPoints);
}

double Worker::getMaxPercentalParameterDiff(std::vector<std::vector<double> > &points)
{
    double maxDiff = -1e100;
    for(size_t i=0; i<mNumParameters; ++i)
    {
        double maxPar = -1e100;
        double minPar = 1e100;
        for(size_t p=0; p<points.size(); ++p)
        {
            if(points[p][i] > maxPar) maxPar = points[p][i];
            if(points[p][i] < minPar) minPar = points[p][i];
        }
        if(mParameterMax[i] != mParameterMin[i] && (maxPar-minPar)/(mParameterMax[i]-mParameterMin[i]) > maxDiff)
        {
            maxDiff = (maxPar-minPar)/(mParameterMax[i]-mParameterMin[i]);
        }
    }
    return maxDiff;
}

std::vector<const char*> Worker::getParameterNamesPtr()
{
    std::vector<const char*> ret;
    for(std::string name : mParameterNames)
    {
        ret.push_back(name.c_str());
    }
    return ret;
}



