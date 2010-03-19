//!
//! @file   SignalDivide.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-11
//!
//! @brief Contains a mathematical division function
//!
//$Id$

#ifndef SIGNALDIVIDE_HPP_INCLUDED
#define SIGNALDIVIDE_HPP_INCLUDED

#include "../../ComponentEssentials.h"

//!
//! @brief
//! @ingroup SignalComponents
//!
class SignalDivide : public ComponentSignal
{

private:
    Port *mpIn1, *mpIn2, *mpOut;

public:
    static Component *Creator()
    {
        std::cout << "running Division creator" << std::endl;
        return new SignalDivide("Divide");
    }

    SignalDivide(const string name,
                          const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        mTypeName = "SignalDivide";

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
        else if (mpIn1->isConnected() && !mpIn2->isConnected())       //Port 1 connected, port 2 disconnected (no division since no denominator)
        {
            signal1 = mpIn1->readNode(NodeSignal::VALUE);
            signal2 = 1;
        }
        else if (!mpIn1->isConnected() && mpIn2->isConnected())       //Port 2 connected, port 1 disconnected (nothing to divide, return zero)
        {
            signal1 = 0;
            signal2 = mpIn2->readNode(NodeSignal::VALUE);
        }
        else
        {
            signal1 = 0;                                                     //Nothing connected, return zero
            signal2 = 1;
        }


        //Gain equations
                double output = signal1 / signal2;                         //! @todo Add division-by-zero check -> exception

        //Write new values to nodes
        mpOut->writeNode(NodeSignal::VALUE, output);
    }
};

#endif // SIGNALDIVIDE_HPP_INCLUDED
