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


#ifndef SIGNAL2DSPLIT
#define SIGNAL2DSPLIT

#include "ComponentEssentials.h"

namespace hopsan {

//!
//! @brief
//! @ingroup SignalComponents
//!
class Signal2DSplit : public ComponentSignal
{

private:
    Port *mpP2d, *mpPv1, *mpPv2;
    bool mWriteV1, mWriteV2;

public:
    static Component *Creator()
    {
        return new Signal2DSplit();
    }

    void configure()
    {
        // Default to read ports for v1 and v2
        mWriteV1 = false;
        mWriteV2 = false;

        mpP2d = addPort("P2d", BiDirectionalSignalPortType, "NodeSignal2D", "The two dimensional signal port", Port::NotRequired);
        mpPv1 = addPort("v1", BiDirectionalSignalPortType, "NodeSignal", "The first dimension", Port::NotRequired);
        mpPv2 = addPort("v2", BiDirectionalSignalPortType, "NodeSignal", "The second dimension", Port::NotRequired);
    }


    void initialize()
    {
        // Figure out if v1 or v2 are read or write ports
        std::vector<Port*> ports1 = mpPv1->getConnectedPorts();
        for (size_t p=0; p<ports1.size(); ++p)
        {
            if (ports1[p]->getPortType() == WritePortType)
            {
                mWriteV1 = true;
                break;
            }
        }
        std::vector<Port*> ports2 = mpPv2->getConnectedPorts();
        for (size_t p=0; p<ports2.size(); ++p)
        {
            if (ports2[p]->getPortType() == WritePortType)
            {
                mWriteV2 = true;
                break;
            }
        }

        simulateOneTimestep();
    }


    void simulateOneTimestep()
    {
        if (mWriteV1)
        {
            mpP2d->writeNode(0, mpPv1->readNode(0));
        }
        else
        {
            mpPv1->writeNode(0, mpP2d->readNode(0));
        }

        if (mWriteV2)
        {
            mpP2d->writeNode(1, mpPv2->readNode(0));
        }
        else
        {
            mpPv2->writeNode(0, mpP2d->readNode(1));
        }
    }
};
}

#endif // SIGNAL2DSPLIT

