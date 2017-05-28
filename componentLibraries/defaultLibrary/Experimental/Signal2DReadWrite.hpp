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


#ifndef SIGNAL2DREADWRITE_HPP
#define SIGNAL2DREADWRITE_HPP

#include "ComponentEssentials.h"

namespace hopsan {

//!
//! @brief
//! @ingroup SignalComponents
//!
class Signal2DReadWrite : public ComponentSignal
{

private:
    Port *mpP2d;
    double *mpV1, *mpV2;
    int m2DPortSortHint;



public:
    static Component *Creator()
    {
        return new Signal2DReadWrite();
    }

    void configure()
    {
        // Here we create the BiDirectional 2D port and set the sort hint as "Destination",
        // it should be paired with a Signal2DWriteRead component that is a "Source"
        mpP2d = addPort("P2d", BiDirectionalSignalPortType, "NodeSignal2D", "The two dimensional signal port", Port::NotRequired);
        mpP2d->setSortHint(Destination);

        std::vector<HString> hints;
        hints.push_back("Destination");
        hints.push_back("IndependentDestination");
        addConditionalConstant("sh2d", "2D Port sort hint", hints, 0, m2DPortSortHint);



        // Note! I have inverted the names here, the "write" port is of type ReadPort (input variable) and
        //       "read" is of type WritePort (output variable)
        addOutputVariable("read","","",0,&mpV1);
        Port * pIVPort = addInputVariable("write","","",0,&mpV2);

        // Here the sort hint is set to independent destination, since the input will not directly affect the output in this time step.
        // This port will thereby be allowed to break an algebraic loop
        pIVPort->setSortHint(IndependentDestination);
    }

    bool preInitialize()
    {
        // Note! This may not work, since the parameter may not have been evaluated
        // if set from numhop or systemparameter, but it needs to be done here before sorting
        mpP2d->setSortHint(SortHintEnumT(Destination+m2DPortSortHint));
        return true;
    }


    void initialize()
    {
        simulateOneTimestep();
    }


    void simulateOneTimestep()
    {
        writeOutputVariable(mpV1, mpP2d->readNode(0)); // Read input
        mpP2d->writeNode(1, *mpV2); // Write output
    }
};
}

#endif // SIGNAL2DREADWRITE_HPP

