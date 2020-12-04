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
#include <string>
#include <sstream>
#include <algorithm>

#include "OpsEvaluator.h"
#include "OpsWorker.h"
#include "OpsMessageHandler.h"
#include "ludcmp.h"
#include "matrix.h"

#include <cmath>

using namespace Ops;

Evaluator::Evaluator()
{
    mSurrogateModelInitialized = false;
}

Evaluator::~Evaluator()
{
    if(mpSurrogateModelCoefficients != nullptr) {
        delete mpSurrogateModelCoefficients;
    }
    if(mpStoredObjectives != nullptr) {
        delete mpStoredObjectives;
    }
    if(mpMatrix != nullptr) {
        delete mpMatrix;
    }
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


void Evaluator::evaluateAllPointsWithSurrogateModel()
{
    if(mpWorker->mNumCandidates == mpWorker->mNumPoints)
    {
        mpWorker->mCandidatePoints = mpWorker->mPoints;
        evaluateAllCandidatesWithSurrogateModel();
        mpWorker->mObjectives = mpWorker->mCandidateObjectives;
    }
    else
    {
        for(size_t i=0; i<mpWorker->mNumPoints && !mpWorker->aborted(); ++i)
        {
            mpWorker->mCandidatePoints[0] = mpWorker->mPoints[i];
            evaluateCandidateWithSurrogateModel(0);
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


bool Evaluator::evaluateAllCandidatesWithSurrogateModel()
{
    if(!mpWorker->mUseSurrogateModel) {
        evaluateAllCandidates();
        return false;
    }
    if(!mSurrogateModelInitialized) {
        //Calculate how many iterations to store for meta model
        size_t npars = mpWorker->getNumberOfParameters();
        mStorageSize = size_t(npars*npars/2.0)+size_t(1.5*npars)+1;
        mpSurrogateModelCoefficients = new Vec(int(mStorageSize));
        mStorageSize = std::max(size_t(mStorageSize*1.5),mpWorker->getNumberOfCandidates());
        mpStoredObjectives = new Vec(int(mStorageSize));
        mStoredParameters.clear();
        mSurrogateModelEvaluations=0;
        mSurrogateModelInitialized = true;
    }

    if(mSurrogateModelEvaluations > mpWorker->mNumSurrogateModelUpdateInterval || mStoredParameters.size() < mStorageSize ) {
        evaluateAllCandidates();
        for(size_t i=0; i<mpWorker->getNumberOfCandidates(); ++i) {
            storeValuesForMetaModel(i);
        }
        if(mStoredParameters.size() >= mStorageSize) {
            updateSurrogateModel();
        }
        mSurrogateModelEvaluations = 0;
    }
    else {
        ++mSurrogateModelEvaluations;

        for(size_t idx=0; idx<mpWorker->getNumberOfCandidates(); ++idx) {
            double obj=1*(*mpSurrogateModelCoefficients)[0];

            size_t npars = mpWorker->getNumberOfParameters();
            for(size_t i=0; i<npars; ++i)
            {
                obj += (*mpSurrogateModelCoefficients)[int(i)+1]*mpWorker->getParameter(idx, i);
            }

            for(size_t i=0; i<npars; ++i)
            {
                for(size_t j=i; j<npars; ++j)
                {
                    obj += (*mpSurrogateModelCoefficients)[int(1+i*npars+j+npars)]*mpWorker->getParameter(idx, i)*mpWorker->getParameter(idx, j);
                }
            }

            //Use regular evaluate if objective is NaN
            if(std::isnan(obj))
            {
                evaluateCandidate(idx);
                storeValuesForMetaModel(idx);
            }
            else
            {
                mpWorker->mCandidateObjectives[idx] = obj;
            }
        }
        return true;
    }
    return false;
}


void Evaluator::evaluateCandidateWithSurrogateModel(size_t idx)
{
    if(!mpWorker->mUseSurrogateModel) {
        evaluateCandidate(idx);
        return;
    }

    if(!mSurrogateModelInitialized) {
        //Calculate how many iterations to store for meta model
        size_t npars = mpWorker->getNumberOfParameters();
        mStorageSize = size_t(npars*npars/2.0)+size_t(1.5*npars)+1;
        mpSurrogateModelCoefficients = new Vec(int(mStorageSize));
        mStorageSize = size_t(mStorageSize*2);
        mpStoredObjectives = new Vec(int(mStorageSize));
        mStoredParameters.clear();
        mSurrogateModelEvaluations=0;
        mSurrogateModelInitialized = true;
    }

    if(mSurrogateModelEvaluations > mpWorker->mNumSurrogateModelUpdateInterval || mStoredParameters.size() < mStorageSize ) {
        evaluateCandidate(idx);
        storeValuesForMetaModel(idx);
        if(mStoredParameters.size() >= mStorageSize) {
            updateSurrogateModel();
        }
        mSurrogateModelEvaluations = 0;
        if(mSurrogateModelExist) {
            evaluateAllPointsWithSurrogateModel();
        }
    }
    else {
        ++mSurrogateModelEvaluations;

        double obj=1*(*mpSurrogateModelCoefficients)[0];

        size_t npars = mpWorker->getNumberOfParameters();
        for(size_t i=0; i<npars; ++i)
        {
            obj += (*mpSurrogateModelCoefficients)[int(i)+1]*mpWorker->getCandidateParameter(idx, i);
        }

        for(size_t i=0; i<npars; ++i)
        {
            for(size_t j=i; j<npars; ++j)
            {
                obj += (*mpSurrogateModelCoefficients)[int(1+i*npars+j+npars)]*mpWorker->getCandidateParameter(idx, i)*mpWorker->getCandidateParameter(idx, j);
            }
        }

        //Use regular evaluate if objective is NaN
        if(std::isnan(obj))
        {
            evaluateCandidate(idx);
            storeValuesForMetaModel(idx);
        }
        else
        {
            mpWorker->mCandidateObjectives[idx] = obj;
        }
    }
}


void Evaluator::storeValuesForMetaModel(size_t idx)
{
    mStoredParameters.push_back(std::vector<double>());
    for(size_t p=0; p<mpWorker->getNumberOfParameters(); ++p)
    {
        mStoredParameters.back().push_back(mpWorker->getCandidateParameter(idx, p));
    }

    //Shift vector to the left
    for(int i=0; i<mpStoredObjectives->length()-1; ++i)
    {
        mpStoredObjectives[i] = mpStoredObjectives[i+1];
    }
    mpStoredObjectives[mpStoredObjectives->length()-1] = mpWorker->mCandidateObjectives[idx];

    //Remove first element if stored vectors are too long
    if(mStoredParameters.size() > mStorageSize)
    {
        mStoredParameters.pop_front();
    }
}



void Evaluator::updateSurrogateModel()
{
    mpStoredObjectives->print();
    Vec previousSurrogateModelCoefficients = (*mpSurrogateModelCoefficients);

    //Create surrogate model
    int n=mStorageSize;
    int m=mpSurrogateModelCoefficients->length();

    mpMatrix = new Matrix(n, m);

    for(int i=0; i<n; ++i)
    {
        (*mpMatrix)[i][0] = 1;
    }

    size_t npars = mpWorker->getNumberOfParameters();

    for(size_t i=0; i<size_t(n); ++i)
    {
        for(size_t j=0; j<npars; ++j)
        {
            (*mpMatrix)[i][j+1] = mStoredParameters[i][j];
        }
    }

    for(int i=0; i<n; ++i)
    {
        size_t col=npars+1;
        for(size_t j=0; j<npars; ++j)
        {
            for(size_t k=j; k<npars; ++k)
            {
                (*mpMatrix)[i][col] = mStoredParameters[i][j]*mStoredParameters[i][k];
                ++col;
            }
        }
    }

    //Solve system using L and U matrices
    Matrix matrixT = (*mpMatrix).transpose();       //Multiply matrix and vector with transpose of matrix, because we need a square matrix
    Matrix tempMatrix = matrixT*(*mpMatrix);
    Vec tempVec = matrixT*(*mpStoredObjectives);
    int* order = new int[size_t(m)];
    ludcmp(tempMatrix, order);
    solvlu(tempMatrix,tempVec,(*mpSurrogateModelCoefficients),order);

    mSurrogateModelExist = true;
}

