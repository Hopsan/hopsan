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
//! @file   SignalMax.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2011-04-21
//!
//! @brief Contains a maximum component
//!
//$Id$

#ifndef SIGNALMAX_HPP_INCLUDED
#define SIGNALMAX_HPP_INCLUDED

#include "ComponentEssentials.h"
#include <vector>

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalMax : public ComponentSignal
    {

    private:
        size_t nInputs;
        std::vector<double *> mNDp_in_vec;
        double *mpND_out;
        Port *mpMultiInPort, *mpOutPort;

    public:
        static Component *Creator()
        {
            return new SignalMax();
        }

        void configure()
        {
            mpMultiInPort = addReadMultiPort("in", "NodeSignal", Port::NotRequired);
            mpOutPort = addWritePort("out", "NodeSignal", Port::NotRequired);
        }


        void initialize()
        {
            nInputs = mpMultiInPort->getNumPorts();
            //We need at least one dummy port even if no port is connected
            if (nInputs < 1)
            {
                nInputs = 1;
            }

            mNDp_in_vec.resize(nInputs);
            for (size_t i=0; i<nInputs; ++i)
            {
                mNDp_in_vec[i] = getSafeMultiPortNodeDataPtr(mpMultiInPort, i, NodeSignal::VALUE, 0);
            }
            mpND_out = getSafeNodeDataPtr(mpOutPort, NodeSignal::VALUE, 0);
        }


        void simulateOneTimestep()
        {
            double max = (*mNDp_in_vec[0]);
            for (size_t i=1; i<nInputs; ++i)
            {
                if(*mNDp_in_vec[i]>max)
                {
                    max = *mNDp_in_vec[i];
                }
            }
            (*mpND_out) = max; //Write value to output node
        }
    };
}
#endif
