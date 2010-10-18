//!
//! @file   SignalGreaterThan.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-10-18
//!
//! @brief Contains a Greater Than Component
//!
//$Id$

#ifndef SIGNALGREATERTHAN_HPP_INCLUDED
#define SIGNALGREATERTHAN_HPP_INCLUDED

#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalGreaterThan : public ComponentSignal
    {

    private:
        double mLimit;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalGreaterThan("GreaterThan");
        }

        SignalGreaterThan(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalGreaterThan";
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

            //Greater than equations
            double y;
            if(u > mLimit)
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

#endif // SIGNALGREATERTHAN_HPP_INCLUDED
