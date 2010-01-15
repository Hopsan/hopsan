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

        addReadPort("in", "NodeSignal", in);
        addWritePort("out", "NodeSignal", out);

        registerParameter("Gain", "Förstärkning", "-", mGain);
    }


	void initialize()
	{
        //Nothing to initilize
	}


    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double u = mPortPtrs[in]->ReadNode(NodeSignal::VALUE);

        //Gain equations
		double y = mGain*u;

        //Write new values to nodes
        mPortPtrs[out]->WriteNode(NodeSignal::VALUE, y);
    }
};

#endif // SIGNALGAIN_HPP_INCLUDED
