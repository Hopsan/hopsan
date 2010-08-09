//!
//! @file   SignalMultiply.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-11
//!
//! @brief Contains a mathematical multiplication function
//!
//$Id$

#ifndef SIGNALMULTIPLY_HPP_INCLUDED
#define SIGNALMULTIPLY_HPP_INCLUDED

#include "../../ComponentEssentials.h"

//!
//! @brief
//! @ingroup SignalComponents
//!
class SignalMultiply : public ComponentSignal
{

private:
    Port *mpIn1, *mpIn2, *mpOut;

public:
    static Component *Creator()
    {
        return new SignalMultiply("Multiply");
    }

    SignalMultiply(const std::string name) : ComponentSignal(name)
    {
        mTypeName = "SignalMultiply";

        mpIn1 = addReadPort("in1", "NodeSignal");
        mpIn2 = addReadPort("in2", "NodeSignal");
        mpOut = addWritePort("out", "NodeSignal");
    }


    void initialize()
    {
        //Nothing to initilize
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
            signal2 = 1;
        }
        else if (!mpIn1->isConnected() && mpIn2->isConnected())       //Port 2 connected, port 1 disconnected
        {
            signal1 = 1;
            signal2 = mpIn2->readNode(NodeSignal::VALUE);
        }
        else
        {
            signal1 = 0;                                                     //Nothing connected
            signal2 = 0;
        }


        //Gain equations
		double output = signal1 * signal2;

        //Write new values to nodes
        mpOut->writeNode(NodeSignal::VALUE, output);

    }
};

#endif // SIGNALMULTIPLY_HPP_INCLUDED
