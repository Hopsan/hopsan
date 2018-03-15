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

#ifndef OPSWORKERCOMPLEXRFP_H
#define OPSWORKERCOMPLEXRFP_H

#include "OpsWorkerComplexRF.h"

namespace Ops {

enum ParallelMethodT {MultiDistance, TaskPrediction, MultiDirection, MultiDirectionBurmen};

class OPS_DLLAPI Candidate
{
public:
    Candidate();
    ~Candidate();
    std::vector<double> *mpPoint;
    double *mpObjective;
    std::vector<Candidate*> subCandidates;
    std::vector<Candidate*> retractions;
    size_t idx;
};

class OPS_DLLAPI WorkerComplexRFP : public WorkerComplexRF
{
public:
    WorkerComplexRFP(Evaluator *pEvaluator, MessageHandler *pMessageHandler);

    AlgorithmT getAlgorithm();

    void initialize();
    void run();

    void setParallelMethod(ParallelMethodT method);
    ParallelMethodT getParallelMethod() const;
    void setNumberOfPredictions(size_t value);
    void setNumberOfRetractions(size_t value);
    void setMinimumReflectionFactor(double value);
    void setMaximumReflectionFactor(double value);


private:
    virtual void pickCandidateParticles();
    virtual void examineCandidateParticles();

    bool multiRetract();

    Candidate *mpFailedCandidate;

    double mAlphaMin, mAlphaMax;
    std::vector<double> mvAlpha;
    ParallelMethodT mMethod;
    std::vector<Candidate*> mTopLevelCandidates;

    //Method 3 members
    size_t mnPredictions, mnRetractions;
    size_t mDistCount, mDirCount, mIterCount;
};

}

#endif // OPSWORKERCOMPLEXRFP_H
