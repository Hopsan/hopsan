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
//! @brief Contains the optimization worker base class for simplex-based algorithms
//!
//$Id$

#include "OpsWorkerSimplex.h"
#include "OpsMessageHandler.h"
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
