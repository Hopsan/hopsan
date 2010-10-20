//!
//! @file   SignalGain.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-05
//!
//! @brief Contains a Signal Gain Component
//!
//$Id$

#ifndef SIGNALGAIN_HPP_INCLUDED
#define SIGNALGAIN_HPP_INCLUDED

#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalGain : public ComponentSignal
    {

    private:
        double mGain;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalGain("Gain");
        }

        SignalGain(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalGain";
            mGain = 1.0;

            mpIn = addReadPort("in", "NodeSignal");
            mpOut = addWritePort("out", "NodeSignal");

            registerParameter("Gain", "Gain value", "-", mGain);
        }


        void initialize()
        {
            //simulateOneTimestep();
            mpOut->writeNode(NodeSignal::VALUE, 0.0);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double u = mpIn->readNode(NodeSignal::VALUE);

            //Gain equations
            double y = mGain*u;

            //Write new values to nodes
            mpOut->writeNode(NodeSignal::VALUE, y);
        }
    };
}

#endif // SIGNALGAIN_HPP_INCLUDED
