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
//! @brief Contains a signal split component for 2d nodes
//!
//$Id$


#ifndef SIGNAL2DWRITEREAD
#define SIGNAL2DWRITEREAD

#include "ComponentEssentials.h"

namespace hopsan {

//!
//! @brief
//! @ingroup SignalComponents
//!
class Signal2DWriteRead : public ComponentSignal
{

private:
    Port *mpP2d;
    double *mpV1, *mpV2;


public:
    static Component *Creator()
    {
        return new Signal2DWriteRead();
    }

    void configure()
    {
        // Here we create the BiDirectional 2D port and set the sort hint as "Source",
        // it should be paired with a Signal2DReadWrite component that is a "Destination"
        mpP2d = addPort("P2d", BiDirectionalSignalPortType, "NodeSignal2D", "The two dimensional signal port", Port::NotRequired);
        mpP2d->setSortHint(Source);

        // Note! I have inverted the names here, the "write" port is of type ReadPort (input variable) and
        //       "read" is of type WritePort (output variable)
        addInputVariable("write","","",0,&mpV1);
        addOutputVariable("read","","",0,&mpV2);
    }


    void initialize()
    {
        simulateOneTimestep();
    }


    void simulateOneTimestep()
    {
        mpP2d->writeNode(0, *mpV1); // Write output
        writeOutputVariable(mpV2, mpP2d->readNode(1)); // Read input
    }
};
}

#endif // SIGNAL2DWRITEREAD

