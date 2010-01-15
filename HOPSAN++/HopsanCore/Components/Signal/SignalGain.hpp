/*
 *  Gain.hpp
 *  HOPSAN++
 *
 *  Created by Björn Eriksson on 2009-01-05.
 *  Copyright 2010 LiU. All rights reserved.
 *
 */


#ifndef SIGNALGAIN_HPP_INCLUDED
#define SIGNALGAIN_HPP_INCLUDED

#include "HopsanCore.h"

class SignalGain : public ComponentSignal
{

private:
    double mGain;
    enum {in, out};

public:
    static Component *Creator()
    {
        std::cout << "running Gain creator" << std::endl;
        return new SignalGain("DefaultGainName");
    }

    SignalGain(const string name,
                          const double gain     = 1.0,
                          const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        mGain = gain;

        addPort("in", "NodeSignal", in);
        addPort("out", "NodeSignal", out);

        registerParameter("Gain", "Förstärkning", "-", mGain);
    }


	void initialize()
	{
        //Nothing to initilize
	}


    void simulateOneTimestep()
    {
        //read fron nodes
   		Node* p1_ptr = mPortPtrs[in]->getNodePtr();
   		Node* p2_ptr = mPortPtrs[out]->getNodePtr();

        //Get variable values from nodes
        double u = p1_ptr->getData(NodeSignal::VALUE);

        //Gain equations
		double y = mGain*u;

        //Write new values to nodes
        p2_ptr->setData(NodeSignal::VALUE, y);
    }
};

#endif // SIGNALGAIN_HPP_INCLUDED
