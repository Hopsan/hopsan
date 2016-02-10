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

