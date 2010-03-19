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
        std::cout << "running Subtraction creator" << std::endl;
        return new SignalSubtract("Subtract");
    }

    SignalSubtract(const string name,
                   const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        mTypeName = "SignalSubtract";

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

#endif // SIGNALSUBTRACT_HPP_INCLUDED

