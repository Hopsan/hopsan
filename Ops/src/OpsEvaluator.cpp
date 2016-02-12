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
//! @brief Contains the optimization evaluator base class
//!
//$Id$

#include <cassert>

#include "OpsEvaluator.h"
#include "OpsWorker.h"

using namespace Ops;

Evaluator::Evaluator()
{
}


void Evaluator::setWorker(Worker *pWorker)
{
    mpWorker = pWorker;
}



void Evaluator::evaluateAllPoints()
{
    if(mpWorker->mNumCandidates == mpWorker->mNumPoints)
    {
        mpWorker->mCandidatePoints = mpWorker->mPoints;
        evaluateAllCandidates();
        mpWorker->mObjectives = mpWorker->mCandidateObjectives;
    }
    else
    {
        for(size_t i=0; i<mpWorker->mNumPoints; ++i)
        {
            mpWorker->mCandidatePoints[0] = mpWorker->mPoints[i];
            evaluateCandidate(0);
            mpWorker->mObjectives[i] = mpWorker->mCandidateObjectives[0];
        }
    }
}


void Evaluator::evaluateCandidate(size_t idx)
{
    (void)idx;
    assert("You should implement your own evaluate() function." == 0);
}


void Evaluator::evaluateAllCandidates()
{
    for(size_t i=0; i<mpWorker->mNumCandidates; ++i)
    {
        evaluateCandidate(i);
    }
}
