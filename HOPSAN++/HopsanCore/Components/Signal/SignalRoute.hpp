//!
//! @file   SignalRoute.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2011-04-21
//!
//! @brief Contains a signal routering component
//!
//$Id$

#ifndef SIGNALROUTE_HPP_INCLUDED
#define SIGNALROUTE_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include <vector>

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalRoute : public ComponentSignal
    {

    private:
        size_t nInputs;
        std::vector<double *> mNDp_in_vec;
        double *mpND_in;
        double *mpND_out;
        Port *mpMultiInPort, *mpInPort, *mpOutPort;

    public:
        static Component *Creator()
        {
            return new SignalRoute();
        }

        SignalRoute() : ComponentSignal()
        {
            mpInPort = addReadPort("in1", "NodeSignal", Port::REQUIRED);
            mpMultiInPort = addReadMultiPort("in2", "NodeSignal", Port::NOTREQUIRED);
            mpOutPort = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);
        }


        void initialize()
        {
            nInputs = mpMultiInPort->getNumPorts();
            mNDp_in_vec.resize(nInputs);
            for (size_t i=0; i<nInputs; ++i)
            {
                mNDp_in_vec[i] = getSafeMultiPortNodeDataPtr(mpMultiInPort, i, NodeSignal::VALUE, 0);
            }
            mpND_in  = getSafeNodeDataPtr(mpInPort,  NodeSignal::VALUE, 1);
            mpND_out = getSafeNodeDataPtr(mpOutPort, NodeSignal::VALUE, 0);
        }


        void simulateOneTimestep()
        {
            int choice = floor((*mpND_in) + 0.5 );
            if(choice < 0.5)
            {
                choice = 1;
            }
            else if(choice > nInputs + 0.5)
            {
                choice = nInputs;
            }
            (*mpND_out) = (*mNDp_in_vec[choice - 1]); //Write value to output node
        }
    };
}
#endif
