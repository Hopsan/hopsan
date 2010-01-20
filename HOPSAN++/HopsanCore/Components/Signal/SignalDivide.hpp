/*
 *  SignalDivide.hpp
 *  HOPSAN++
 *
 *  Created by Robert Braun on 2010-01-11.
 *  Copyright 2010 LiU. All rights reserved.
 *
 */

#ifndef SIGNALDIVIDE_HPP_INCLUDED
#define SIGNALDIVIDE_HPP_INCLUDED

#include "HopsanCore.h"

class SignalDivide : public ComponentSignal
{

private:
    enum {in1, in2, out};

public:
    static Component *Creator()
    {
        std::cout << "running Division creator" << std::endl;
        return new SignalDivide("DefaultDivideName");
    }

    SignalDivide(const string name,
                          const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        addPortRead("in1", "NodeSignal", in1);
        addPortRead("in2", "NodeSignal", in2);
        addPortWrite("out", "NodeSignal", out);
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
            signal1 = mPortPtrs[in2]->readNode(NodeSignal::VALUE);
        }
        else if (mPortPtrs[in1]->isConnected() && !mPortPtrs[in2]->isConnected())       //Port 1 connected, port 2 disconnected (no division since no denominator)
        {
            signal1 = mPortPtrs[in1]->readNode(NodeSignal::VALUE);
            signal2 = 1;
        }
        else if (!mPortPtrs[in1]->isConnected() && mPortPtrs[in2]->isConnected())       //Port 2 connected, port 1 disconnected (nothing to divide, return zero)
        {
            signal1 = 0;
            signal1 = mPortPtrs[in2]->readNode(NodeSignal::VALUE);
        }
        else
        {
            signal1 = 0;                                                     //Nothing connected, return zero
            signal2 = 1;
        }


        //Gain equations
		double output = signal1 / signal2;                         //TODO: Add division-by-zero check -> exception

        //Write new values to nodes
        mPortPtrs[out]->writeNode(NodeSignal::VALUE, output);
    }
};

#endif // SIGNALDIVIDE_HPP_INCLUDED
