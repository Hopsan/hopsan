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

#ifndef OPSEVALUATOR_H
#define OPSEVALUATOR_H

#include <stdlib.h>
#include <vector>
#include <deque>

#include "OpsWin32DLL.h"
#include "matrix.h"

namespace Ops {

class Worker;

class OPS_DLLAPI Evaluator
{
public:
    Evaluator();
    ~Evaluator();

    void setWorker(Worker *pWorker);

    virtual void evaluateAllPoints();               //Can be re-implemented
    virtual void evaluateCandidate(size_t idx);        //Must be re-implemented
    virtual void evaluateAllCandidates();           //Can be re-implemented
    void evaluateAllPointsWithSurrogateModel();
    bool evaluateAllCandidatesWithSurrogateModel();
    void evaluateCandidateWithSurrogateModel(size_t idx);

protected:
    Worker *mpWorker;

private:
    void updateSurrogateModel();
    void storeValuesForMetaModel(size_t idx);

    size_t mStorageSize;
    Vec* mpSurrogateModelCoefficients = nullptr;
    std::deque< std::vector<double> > mStoredParameters;

    Vec *mpStoredObjectives = nullptr;
    Matrix *mpMatrix = nullptr;

    double mPercDiff;
    int mCountMax;

    bool mUseMetaModel;
    bool mSurrogateModelExist;
    size_t mSurrogateModelEvaluations;
    bool mSurrogateModelInitialized;
};

}

#endif // OPSEVALUATOR_H
