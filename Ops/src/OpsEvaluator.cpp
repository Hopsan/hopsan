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
        for(size_t i=0; i<mpWorker->mNumPoints && !mpWorker->aborted(); ++i)
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
    for(size_t i=0; i<mpWorker->mNumCandidates && !mpWorker->aborted(); ++i)
    {
        evaluateCandidate(i);
    }
}
