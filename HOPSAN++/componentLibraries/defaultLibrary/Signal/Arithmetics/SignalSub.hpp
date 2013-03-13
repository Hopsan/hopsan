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
//! @file   SignalSub.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2012-02-29
//!
//! @brief Contains a mathematical multiport subtractor component
//!
//$Id$

#ifndef SIGNALSUB_HPP_INCLUDED
#define SIGNALSUB_HPP_INCLUDED

#include "ComponentEssentials.h"
#include <vector>
#include <sstream>

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSub : public ComponentSignal
    {

    private:
        size_t nSumInputs, nSubInputs;
        std::vector<double *> mNDp_in_sum_vec, mNDp_in_sub_vec;
        double *mpND_out;
        Port *mpMultiInSumPort, *mpMultiInSubPort, *mpOutPort;

    public:
        static Component *Creator()
        {
            return new SignalSub();
        }

        void configure()
        {
            mpMultiInSumPort = addReadMultiPort("insum", "NodeSignal", Port::NotRequired);
            mpMultiInSubPort = addReadMultiPort("insub", "NodeSignal", Port::NotRequired);
            mpOutPort = addWritePort("out", "NodeSignal", Port::NotRequired);
        }


        void initialize()
        {
            nSumInputs = mpMultiInSumPort->getNumPorts();
            nSubInputs = mpMultiInSubPort->getNumPorts();
            //We need at least one dummy port even if no port is connected
            if (nSumInputs < 1)
            {
                nSumInputs = 1;
            }
            if (nSubInputs < 1)
            {
                nSubInputs = 1;
            }

            mNDp_in_sum_vec.resize(nSumInputs);
            for (size_t i=0; i<nSumInputs; ++i)
            {
                mNDp_in_sum_vec[i] = getSafeMultiPortNodeDataPtr(mpMultiInSumPort, i, NodeSignal::VALUE, 0);
            }
            mNDp_in_sub_vec.resize(nSubInputs);
            for (size_t i=0; i<nSubInputs; ++i)
            {
                mNDp_in_sub_vec[i] = getSafeMultiPortNodeDataPtr(mpMultiInSubPort, i, NodeSignal::VALUE, 0);
            }
            mpND_out = getSafeNodeDataPtr(mpOutPort, NodeSignal::VALUE, 0);
//            std::stringstream ss;
//            ss << "nInputs: " << nSumInputs;
//            addDebugMessage(ss.str());
        }


        void simulateOneTimestep()
        {
            double sum = 0;
            for (size_t i=0; i<nSumInputs; ++i)
            {
                sum += *mNDp_in_sum_vec[i];
            }
            for (size_t i=0; i<nSubInputs; ++i)
            {
                sum -= *mNDp_in_sub_vec[i];
            }
            (*mpND_out) = sum; //Write value to output node
        }
    };
}
#endif
