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

#include "HopsanCore.h"

class SignalAdd : public ComponentSignal
{

private:
    enum {in1, in2, out};

public:
    static Component *Creator()
    {
        std::cout << "running Summation creator" << std::endl;
        return new SignalAdd("DefaultAddName");
    }

    SignalAdd(const string name,
                          const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        mTypeName = "SignalAdd";

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
            signal1 = mPortPtrs[in2]->readNode(NodeSignal::VALUE);
        }
        else
        {
            signal1 = 0;                                                     //Nothing connected
            signal2 = 0;
        }


        //Gain equations
		double output = signal1 + signal2;

        //Write new values to nodes
        mPortPtrs[out]->writeNode(NodeSignal::VALUE, output);
    }
};

#endif // SIGNALADD_HPP_INCLUDED

