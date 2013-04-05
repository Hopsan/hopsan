/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   SignalSum.hpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2011-04-04
//!
//! @brief Contains a mathematical multiport summator component
//!
//$Id$

#ifndef SIGNALSUM_HPP_INCLUDED
#define SIGNALSUM_HPP_INCLUDED

#include "ComponentEssentials.h"
#include <vector>
#include <sstream>

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSum : public ComponentSignal
    {

    private:
        size_t nInputs;
        std::vector<double *> mNDp_in_vec;
        double *mpND_out;
        Port *mpMultiInPort, *mpOutPort;

    public:
        static Component *Creator()
        {
            return new SignalSum();
        }

        void configure()
        {
            mpMultiInPort = addReadMultiPort("in", "NodeSignal", Port::NotRequired);
            mpOutPort = addWritePort("out", "NodeSignal", Port::NotRequired);
            disableStartValue(mpOutPort, NodeSignal::Value);
        }


        void initialize()
        {
            // We need at least one dummy port even if no port is connected
            nInputs = std::max(mpMultiInPort->getNumPorts(),size_t(1));

            mNDp_in_vec.resize(nInputs);
            for (size_t i=0; i<nInputs; ++i)
            {
                mNDp_in_vec[i] = getSafeMultiPortNodeDataPtr(mpMultiInPort, i, NodeSignal::Value);
            }
            mpND_out = getSafeNodeDataPtr(mpOutPort, NodeSignal::Value);
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            double sum = 0;
            for (size_t i=0; i<nInputs; ++i)
            {
                sum += *mNDp_in_vec[i];
            }
            (*mpND_out) = sum; //Write value to output node
        }
    };
}
#endif
