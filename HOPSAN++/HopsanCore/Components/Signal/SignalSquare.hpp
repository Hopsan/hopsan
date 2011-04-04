//!
//! @file   SignalSquare.hpp
//! @author Björn Eriksson <robert.braun@liu.se>
//! @date   2010-09-28
//!
//! @brief Contains a mathematical square component
//!
//$Id$

#ifndef SIGNALSQUARE_HPP_INCLUDED
#define SIGNALSQUARE_HPP_INCLUDED

#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSquare : public ComponentSignal
    {

    private:
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalSquare("Square");
        }

        SignalSquare(const std::string name) : ComponentSignal(name)
        {

            mpIn = addReadPort("in", "NodeSignal");
            mpOut = addWritePort("out", "NodeSignal");
        }


        void initialize()
        {
//            simulateOneTimestep();
            mpOut->writeNode(NodeSignal::VALUE, 0.0);
        }


        void simulateOneTimestep()
        {

            //Get variable values from nodes
            double in;

            if (mpIn->isConnected())       //In-port connected
            {
                in = mpIn->readNode(NodeSignal::VALUE);

            }
            else
            {
                in = 0;                                           //Nothing connected
            }

            //Write new values to nodes
            mpOut->writeNode(NodeSignal::VALUE, in*in);
        }
    };
}
#endif // SIGNALSQUARE_HPP_INCLUDED
