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
            mpOut = addWritePort("out", "NodeSignal");

            registerParameter("UpperLimit", "Upper Limit", "-", mUpperLimit);
            registerParameter("LowerLimit", "Lower Limit", "-", mLowerLimit);
        }


        void initialize()
        {
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double input = mpIn->readNode(NodeSignal::VALUE);

            //Gain equations
            double output;
            if (input > mUpperLimit)
            {
                output = mUpperLimit;
            }
            else if (input < mLowerLimit)
            {
                output = mLowerLimit;
            }
            else
            {
                output = input;
            }

            //Write new values to nodes
            mpOut->writeNode(NodeSignal::VALUE, output);
        }
    };
}

#endif // SIGNALSATURATION_HPP_INCLUDED
