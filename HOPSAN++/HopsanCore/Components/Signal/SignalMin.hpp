//!
//! @file   SignalMin.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2011-04-21
//!
//! @brief Contains a maximum component
//!
//$Id$

#ifndef SIGNALMIN_HPP_INCLUDED
#define SIGNALMIN_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include <vector>

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalMin : public ComponentSignal
    {

    private:
        size_t nInputs;
        std::vector<double *> mNDp_in_vec;
        double *mpND_out;
        Port *mpMultiInPort, *mpOutPort;

    public:
        static Component *Creator()
        {
            return new SignalMin();
        }

        SignalMin() : ComponentSignal()
        {
            mpMultiInPort = addReadMultiPort("in", "NodeSignal", Port::NOTREQUIRED);
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
            mpND_out = getSafeNodeDataPtr(mpOutPort, NodeSignal::VALUE, 0);
        }


        void simulateOneTimestep()
        {
            double min = (*mNDp_in_vec[0]);
            for (size_t i=1; i<nInputs; ++i)
            {
                if(*mNDp_in_vec[i]<min)
                {
                    min = *mNDp_in_vec[i];
                }
            }
            (*mpND_out) = min; //Write value to output node
        }
    };
}
#endif
