//!
//! @file   SignalSubtract.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-11
//!
//! @brief Contains a mathematical subtraction function
//!
//$Id$

#ifndef SIGNALSUBTRACT_HPP_INCLUDED
#define SIGNALSUBTRACT_HPP_INCLUDED

#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSubtract : public ComponentSignal
    {

    private:
        Port *mpIn1, *mpIn2, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalSubtract("Subtract");
        }

        SignalSubtract(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalSubtract";

            mpIn1 = addReadPort("in1", "NodeSignal", Port::NOTREQUIRED);
            mpIn2 = addReadPort("in2", "NodeSignal", Port::NOTREQUIRED);
            mpOut = addWritePort("out", "NodeSignal");
        }


        void initialize()
        {
            //simulateOneTimestep();
            mpOut->writeNode(NodeSignal::VALUE, 0.0);

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
                signal2 = mpIn2->readNode(NodeSignal::VALUE);
            }
            else
            {
                signal1 = 0;                                                     //Nothing connected
                signal2 = 0;
            }


            //Gain equations
            double output = signal1 - signal2;

            //Write new values to nodes
            mpOut->writeNode(NodeSignal::VALUE, output);
        }
    };




    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalOptimizedSubtract : public ComponentSignal
    {

    private:
        Port *mpIn1, *mpIn2, *mpOut;

        double *in1, *in2, *output;

    public:
        static Component *Creator()
        {
            return new SignalOptimizedSubtract("Subtract");
        }

        SignalOptimizedSubtract(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalOptimizedSubtract";

            mpIn1 = addReadPort("in1", "NodeSignal", Port::NOTREQUIRED);
            mpIn2 = addReadPort("in2", "NodeSignal", Port::NOTREQUIRED);
            mpOut = addWritePort("out", "NodeSignal");
        }


        void initialize()
        {
                //Make in1 or in2 be zero if they are not connected
            if(mpIn1->isConnected())
            {
                in1 = mpIn1->getNodeDataPtr(NodeSignal::VALUE);
            }
            else
            {
                in1 = new double(0);
            }

            if(mpIn2->isConnected())
            {
                in2 = mpIn2->getNodeDataPtr(NodeSignal::VALUE);
            }
            else
            {
                in2 = new double(0);
            }

                //output must be connected
            output = mpOut->getNodeDataPtr(NodeSignal::VALUE);

            (*output) = 0.0;
        }


        void simulateOneTimestep()
        {
            (*output) = (*in1) - (*in2);
        }
    };
}

#endif // SIGNALSUBTRACT_HPP_INCLUDED

