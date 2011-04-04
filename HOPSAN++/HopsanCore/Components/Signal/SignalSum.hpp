//!
//! @file   SignalSum.hpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2011-04-04
//!
//! @brief Contains a mathematical multiport summator component
//!
//$Id: SignalAdd.hpp 2688 2011-04-04 09:41:46Z petno25 $

#ifndef SIGNALSUM_HPP_INCLUDED
#define SIGNALSUM_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include <vector>

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

        SignalSum() : ComponentSignal()
        {
            mpMultiInPort = addReadMultiPort("in", "NodeSignal", Port::NOTREQUIRED);
            mpOutPort = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);
        }


        void initialize()
        {
            nInputs = mpMultiInPort->getNumPorts();
            for (size_t i=0; i<nInputs; ++i)
            {
                mNDp_in_vec.push_back( getSafeMultiPortNodeDataPtr(mpMultiInPort, i, NodeSignal::VALUE, 0) );
            }
            mpND_out = getSafeNodeDataPtr(mpOutPort, NodeSignal::VALUE, 0);
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
