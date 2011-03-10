//!
//! @file   SignalSaturation.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-12
//!
//! @brief Contains a signal saturation component
//!
//$Id$

#ifndef SIGNALSATURATION_HPP_INCLUDED
#define SIGNALSATURATION_HPP_INCLUDED

#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSaturation : public ComponentSignal
    {

    private:
        double mUpperLimit;
        double mLowerLimit;
        double *mpND_in, *mpND_out;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalSaturation("Saturation");
        }

        SignalSaturation(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalSaturation";
            mUpperLimit = 1.0;
            mLowerLimit = -1.0;

            mpIn = addReadPort("in", "NodeSignal");
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

            registerParameter("UpperLimit", "Upper Limit", "[-]", mUpperLimit);
            registerParameter("LowerLimit", "Lower Limit", "[-]", mLowerLimit);
        }


        void initialize()
        {
            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
               //Gain equations
            if ( (*mpND_in) > mUpperLimit )
            {
                (*mpND_out) = mUpperLimit;
            }
            else if ( (*mpND_in) < mLowerLimit )
            {
                (*mpND_out) = mLowerLimit;
            }
            else
            {
                (*mpND_out) = (*mpND_in);
            }
        }
    };
}

#endif // SIGNALSATURATION_HPP_INCLUDED
