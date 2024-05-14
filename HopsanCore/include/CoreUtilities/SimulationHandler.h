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
//! @file   SimulationHandler.h
//! @author FluMeS
//! @date   2009-12-20
//!
//! @brief Contains the simulation handler help class
//!
//$Id$

#ifndef SIMULATIONHANDLER_H
#define SIMULATIONHANDLER_H

#include <cstddef>
#include <vector>
#include "win32dll.h"

namespace hopsan {

enum ParallelAlgorithmT {APrioriScheduling,
                         TaskPoolAlgorithm,
                         TaskStealingAlgorithm,
                         ForkJoinAlgorithm,
                         ClusteredForkJoinAlgorithm};

// Forward declaration
class ComponentSystem;

class HOPSANCORE_DLLAPI SimulationHandler
{
public:
    enum SimulationErrorTypesT {NotRedy, InitFailed, SimuFailed, FiniFailed};

    //! @todo a doitall function
    //! @todo use the error enums
    bool initializeSystem(const double startT, const double stopT, ComponentSystem* pSystem);
    bool initializeSystem(const double startT, const double stopT, std::vector<ComponentSystem*> &rSystemVector);

    bool simulateSystem(const double startT, const double stopT, const int nDesiredThreads, ComponentSystem* pSystem, bool noChanges=false, ParallelAlgorithmT algorithm=APrioriScheduling);
    bool simulateSystem(const double startT, const double stopT, const int nDesiredThreads, std::vector<ComponentSystem*> &rSystemVector, bool noChanges=false, ParallelAlgorithmT algorithm=APrioriScheduling);

    bool startRealtimeSimulation(ComponentSystem *pSystem, double realtimeFactor=1);
    void stopRealtimeSimulation(ComponentSystem *pSystem);

    void finalizeSystem(ComponentSystem* pSystem);
    void finalizeSystem(std::vector<ComponentSystem*> &rSystemVector);

private:
    bool simulateMultipleSystemsMultiThreaded(const double startT, const double stopT, const size_t nDesiredThreads, const std::vector<ComponentSystem*> &rSystemVector, bool noChanges=false);
    bool simulateMultipleSystems(const double stopT, const std::vector<ComponentSystem *> &rSystemVector);

    std::vector< std::vector<ComponentSystem*> > distributeSystems(const std::vector<ComponentSystem*> &rSystemVector, size_t nThreads);
    void sortSystemsByTotalMeasuredTime(std::vector<ComponentSystem*> &rSystemVector);

    std::vector< std::vector<ComponentSystem*> > mSplitSystemVector;
};

}

#endif // SIMULATIONHANDLER_H
