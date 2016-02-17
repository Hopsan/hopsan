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

        // Note! I have inverted the names here, the "write" port is of type ReadPort (input variable) and
        //       "read" is of type WritePort (output variable)
        addOutputVariable("read","","",0,&mpV1);
        Port * pIVPort = addInputVariable("write","","",0,&mpV2);

        // Here the sort hint is set to independent destination, since the input will not directly affect the output in this time step.
        // This port will thereby be allowed to break an algebraic loop
        pIVPort->setSortHint(IndependentDestination);
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

