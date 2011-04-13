//!
//! @file   SignalSource.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-05
//!
//! @brief Contains a Signal Source Component
//!
//$Id$

#ifndef SIGNALSOURCE_HPP_INCLUDED
#define SIGNALSOURCE_HPP_INCLUDED

#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSource : public ComponentSignal
    {

    private:
        double mValue;
        double *mpND_out;
        Port *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalSource("Source");
        }

        SignalSource(const std::string name) : ComponentSignal(name)
        {
            mValue = 1.0;

            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

            registerParameter("Value", "Source Value", "[-]", mValue);

            disableStartValue(mpOut, NodeSignal::VALUE);
        }


        void initialize()
        {
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE, mValue);

            //Initialize value to the node
           (*mpND_out) = mValue;
        }


        void simulateOneTimestep()
        {
           (*mpND_out) = mValue;          //Temporary RT solution  

			//Nothing to do (only one write port can exist in the node, so no one else shall write to the value)
        }
    };
}

#endif // SIGNALSOURCE_HPP_INCLUDED
