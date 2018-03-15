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
//! @brief Contains the optimization worker class for the PSO algorithm
//!
//$Id$

#ifndef OPSWORKERDIFFERENTIALEVOLUTION_H
#define OPSWORKERDIFFERENTIALEVOLUTION_H
#include "OpsWorker.h"

namespace Ops {

class OPS_DLLAPI WorkerDifferentialEvolution : public Worker
{
public:
    WorkerDifferentialEvolution(Evaluator *pEvaluator, MessageHandler *pMessageHandler);

    AlgorithmT getAlgorithm();

    void run();

    void setCrossoverProbability(double value);
    void setDifferentialWeight(double value);

private:
    void moveParticle(int p);
protected:
    double mCR, mF;
    void getRandomIds(size_t notId, size_t &id1, size_t &id2, size_t &id3, size_t &id4);
    bool isCandidateFeasible(int id);
};

}

#endif // OPSWORKERDIFFERENTIALEVOLUTION_H
