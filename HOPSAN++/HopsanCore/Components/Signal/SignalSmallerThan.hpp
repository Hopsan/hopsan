//!
//! @file   SignalSmallerThan.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-10-18
//!
//! @brief Contains a Smaller Than Component
//!
//$Id$

#ifndef SIGNALSMALLERTHAN_HPP_INCLUDED
#define SIGNALSMALLERTHAN_HPP_INCLUDED

#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSmallerThan : public ComponentSignal
    {

    private:
        double mLimit;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalSmallerThan("SmallerThan");
        }

        SignalSmallerThan(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalSmallerThan";
            mLimit = 0.0;

            mpIn = addReadPort("in", "NodeSignal");
            mpOut = addWritePort("out", "NodeSignal");

            registerParameter("x", "Limit Value", "-", mLimit);
        }


        void initialize()
        {
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double u = mpIn->readNode(NodeSignal::VALUE);

            //Smaller than equations
            double y;
            if(u < mLimit)
            {
                y = 1.0;
            }
            else
            {
                y = 0.0;
            }

            //Write new values to nodes
            mpOut->writeNode(NodeSignal::VALUE, y);
        }
    };
}

#endif // SIGNALSMALLERTHAN_HPP_INCLUDED
