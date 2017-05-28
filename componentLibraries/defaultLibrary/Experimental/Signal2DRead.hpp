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
//! @file   Signal2DSplit.hpp
//! @date   2016-02-09
//!
//! @brief Contains a signal component for reading from 2d nodes
//!
//$Id$


#ifndef SIGNAL2DREAD
#define SIGNAL2DREAD

#include "ComponentEssentials.h"

namespace hopsan {

//!
//! @brief
//! @ingroup SignalComponents
//!
class Signal2DRead : public ComponentSignal
{

private:
    Port *mpP2d;
    double *mpV1, *mpV2;


public:
    static Component *Creator()
    {
        return new Signal2DRead();
    }

    void configure()
    {
        // Here we create the BiDirectional 2D port and set the sort hint as "Source",
        // it should be paired with a Signal2DReadWrite component that is a "Destination"
        mpP2d = addPort("P2d", ReadPortType, "NodeSignal2D", "The two dimensional signal port", Port::NotRequired);
        mpP2d->setSortHint(Source);

        // Note! I have inverted the names here, the "write" port is of type ReadPort (input variable) and
        //       "read" is of type WritePort (output variable)
        addOutputVariable("out1","","",0,&mpV1);
        addOutputVariable("out2","","",0,&mpV2);
    }


    void initialize()
    {
        simulateOneTimestep();
    }


    void simulateOneTimestep()
    {
        writeOutputVariable(mpV1, mpP2d->readNode(0)); // Read input
        writeOutputVariable(mpV2, mpP2d->readNode(1)); // Read input
    }
};
}

#endif // SIGNAL2DREAD

