//!
//! @file   SignalAdd.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-11
//!
//! @brief Contains a mathematical summator component
//!
//$Id$

#ifndef SIGNALADD_HPP_INCLUDED
#define SIGNALADD_HPP_INCLUDED

#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalAdd : public ComponentSignal
    {

    private:
        Port *mpIn1, *mpIn2, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalAdd("Add");
        }

        SignalAdd(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalAdd";

            mpIn1 = addReadPort("in1", "NodeSignal");
            mpIn2 = addReadPort("in2", "NodeSignal");
            mpOut = addWritePort("out", "NodeSignal");
        }


        void initialize()
        {
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {

            //Get variable values from nodes
            double signal1, signal2;

            if (mpIn1->isConnected() && mpIn2->isConnected())       //Both ports connected
            {
                signal1 = mpIn1->readNode(NodeSignal::VALUE);
                signal2 = mpIn2->readNode(NodeSignal::VALUE);

            }
            else if (mpIn1->isConnected() && !mpIn2->isConnected())       //Port 1 connected, port 2 disconnected
            {
                signal1 = mpIn1->readNode(NodeSignal::VALUE);
                signal2 = 0;
            }
            else if (!mpIn1->isConnected() && mpIn2->isConnected())       //Port 2 connected, port 1 disconnected
            {
                signal1 = 0;
                signal1 = mpIn2->readNode(NodeSignal::VALUE);
            }
            else
            {
                signal1 = 0;                                                     //Nothing connected
                signal2 = 0;
            }


            //Gain equations
            double output = signal1 + signal2;

            //Write new values to nodes
            mpOut->writeNode(NodeSignal::VALUE, output);
        }
    };
}
#endif // SIGNALADD_HPP_INCLUDED
