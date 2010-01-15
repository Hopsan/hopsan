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
        addPort("in1", "NodeSignal", in1);
        addPort("in2", "NodeSignal", in2);
        addPort("out", "NodeSignal", out);
    }


	void initialize()
	{
        //Nothing to initilize
	}


    void simulateOneTimestep()
    {
        //read fron nodes
   		Node* p1_ptr = mPortPtrs[in1]->getNodePtr();
   		Node* p2_ptr = mPortPtrs[in2]->getNodePtr();
   		Node* p3_ptr = mPortPtrs[out]->getNodePtr();

        //Get variable values from nodes
        double signal1, signal2;

        if (mPortPtrs[in1]->isConnected() && mPortPtrs[in2]->isConnected())       //Both ports connected
        {
            signal1 = p1_ptr->getData(NodeSignal::VALUE);
            signal2 = p2_ptr->getData(NodeSignal::VALUE);
        }
        else if (mPortPtrs[in1]->isConnected() && !mPortPtrs[in2]->isConnected())       //Port 1 connected, port 2 disconnected (no division since no denominator)
        {
            signal1 = p1_ptr->getData(NodeSignal::VALUE);
            signal2 = 1;
        }
        else if (!mPortPtrs[in1]->isConnected() && mPortPtrs[in2]->isConnected())       //Port 2 connected, port 1 disconnected (nothing to divide, return zero)
        {
            signal1 = 0;
            signal2 = p2_ptr->getData(NodeSignal::VALUE);
        }
        else
        {
            signal1 = 0;                                                     //Nothing connected, return zero
            signal2 = 1;
        }


        //Gain equations
		double out = signal1 / signal2;                         //TODO: Add division-by-zero check -> exception

        //Write new values to nodes
        p3_ptr->setData(NodeSignal::VALUE, out);
    }
};

#endif // SIGNALDIVIDE_HPP_INCLUDED
