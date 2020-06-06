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
//! @brief Contains the optimization worker base class for simplex-based algorithms
//!
//$Id$

#include "OpsWorkerSimplex.h"
#include "OpsMessageHandler.h"
#include <algorithm>
using namespace Ops;

WorkerSimplex::WorkerSimplex(Evaluator *pEvaluator, MessageHandler *pMessageHandler)
    : Worker(pEvaluator, pMessageHandler)
{
}

void WorkerSimplex::initialize()
{
    Worker::initialize();

    mCentroidPoint.resize(mNumParameters);
}



//! @brief Reflects specified point through specified centroid and returns the reflected point
//! @param point Point to reflect
//! @param center Point to reflect through
//! @param alpha Reflection factor
std::vector<double> WorkerSimplex::reflect(std::vector<double> point, std::vector<double> center, double alpha)
{
    std::vector<double> newPoint;
    newPoint.resize(mNumParameters);

    for(size_t j=0; j<mNumParameters; ++j)
    {
        //Reflect
        newPoint[j] = center[j] + (center[j]-point[j])*alpha;

        //Add some random noise
        double maxDiff = getMaxPercentalParameterDiff();
        double r = (double)rand() / (double)RAND_MAX;
        newPoint[j] = newPoint[j] + mRandomFactor*(mParameterMax[j]-mParameterMin[j])*maxDiff*(r-0.5);
        newPoint[j] = std::min(newPoint[j], mParameterMax[j]);
        newPoint[j] = std::max(newPoint[j], mParameterMin[j]);
    }

    return newPoint;
}

void WorkerSimplex::setRandomFactor(double value)
{
    mRandomFactor = value;
}




void WorkerSimplex::findCentroidPoint()
{
    std::vector< std::vector<double> > points = mPoints;
    removeFromVector(points,mWorstId);
    WorkerSimplex::findCentroidPoint(points);
}

void WorkerSimplex::findCentroidPoint(std::vector<std::vector<double> > &points)
{
    for(size_t i=0; i<mNumParameters; ++i)
    {
        mCentroidPoint[i] = 0;
    }
    for(size_t p=0; p<points.size(); ++p)
    {
        for(size_t i=0; i<mNumParameters; ++i)
        {
            mCentroidPoint[i] = mCentroidPoint[i]+points[p][i];
        }
    }
    for(size_t i=0; i<mNumParameters; ++i)
    {
        mCentroidPoint[i] = mCentroidPoint[i]/double(points.size());
    }
}
