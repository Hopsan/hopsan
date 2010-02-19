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
    enum {in1, in2, out};

public:
    static Component *Creator()
    {
        std::cout << "running Subtraction creator" << std::endl;
        return new SignalSubtract("DefaultSubtractName");
    }

    SignalSubtract(const string name,
                   const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        mTypeName = "SignalSubtract";

        addReadPort("in1", "NodeSignal", in1);
        addReadPort("in2", "NodeSignal", in2);
        addWritePort("out", "NodeSignal", out);
    }


	void initialize()
	{
        //Nothing to initilize
	}


    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double signal1, signal2;

        if (mPortPtrs[in1]->isConnected() && mPortPtrs[in2]->isConnected())       //Both ports connected
        {
            signal1 = mPortPtrs[in1]->readNode(NodeSignal::VALUE);
            signal2 = mPortPtrs[in2]->readNode(NodeSignal::VALUE);
        }
        else if (mPortPtrs[in1]->isConnected() && !mPortPtrs[in2]->isConnected())       //Port 1 connected, port 2 disconnected
        {
            signal1 = mPortPtrs[in1]->readNode(NodeSignal::VALUE);
            signal2 = 0;
        }
        else if (!mPortPtrs[in1]->isConnected() && mPortPtrs[in2]->isConnected())       //Port 2 connected, port 1 disconnected
        {
            signal1 = 0;
            signal2 = mPortPtrs[in2]->readNode(NodeSignal::VALUE);
        }
        else
        {
            signal1 = 0;                                                     //Nothing connected
            signal2 = 0;
        }


        //Gain equations
		double output = signal1 - signal2;

        //Write new values to nodes
        mPortPtrs[out]->writeNode(NodeSignal::VALUE, output);
    }
};

#endif // SIGNALSUBTRACT_HPP_INCLUDED

