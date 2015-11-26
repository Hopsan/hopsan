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

enum ParallelAlgorithmT {OfflineSchedulingAlgorithm, TaskPoolAlgorithm, TaskStealingAlgorithm, ParallelForAlgorithm,
                         ParallelForTbbAlgorithm, GroupedParallelForAlgorithm, RandomTaskPoolAlgorithm, OfflineReschedulingAlgorithm};

// Forward declaration
class ComponentSystem;

class DLLIMPORTEXPORT SimulationHandler
{
public:
    enum SimulationErrorTypesT {NotRedy, InitFailed, SimuFailed, FiniFailed};

    //! @todo a doitall function
    //! @todo use the error enums
    bool initializeSystem(const double startT, const double stopT, ComponentSystem* pSystem);
    bool initializeSystem(const double startT, const double stopT, std::vector<ComponentSystem*> &rSystemVector);

    bool simulateSystem(const double startT, const double stopT, const int nDesiredThreads, ComponentSystem* pSystem, bool noChanges=false, ParallelAlgorithmT algorithm=OfflineSchedulingAlgorithm);
    bool simulateSystem(const double startT, const double stopT, const int nDesiredThreads, std::vector<ComponentSystem*> &rSystemVector, bool noChanges=false, ParallelAlgorithmT algorithm=OfflineSchedulingAlgorithm);

    void finalizeSystem(ComponentSystem* pSystem);
    void finalizeSystem(std::vector<ComponentSystem*> &rSystemVector);

    void runCoSimulation(ComponentSystem *pSystem);

private:
    bool simulateMultipleSystemsMultiThreaded(const double startT, const double stopT, const size_t nDesiredThreads, std::vector<ComponentSystem*> &rSystemVector, bool noChanges=false);
    bool simulateMultipleSystems(const double stopT, std::vector<ComponentSystem*> &rSystemVector);

    std::vector< std::vector<ComponentSystem*> > distributeSystems(const std::vector<ComponentSystem*> &rSystemVector, size_t nThreads);
    void sortSystemsByTotalMeasuredTime(std::vector<ComponentSystem*> &rSystemVector);

    std::vector< std::vector<ComponentSystem*> > mSplitSystemVector;
};

}

#endif // SIMULATIONHANDLER_H
