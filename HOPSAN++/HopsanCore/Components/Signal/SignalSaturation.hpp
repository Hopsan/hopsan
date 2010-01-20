/*
 *  SignalSaturation.hpp
 *  HOPSAN++
 *
 *  Created by Robert Braun on 2010-01-12.
 *  Copyright 2010 LiU. All rights reserved.
 *
 */

#ifndef SIGNALSATURATION_HPP_INCLUDED
#define SIGNALSATURATION_HPP_INCLUDED

#include "HopsanCore.h"

class SignalSaturation : public ComponentSignal
{

private:
    double mUpperLimit;
    double mLowerLimit;
    enum {in, out};

public:
    static Component *Creator()
    {
        std::cout << "running Saturation creator" << std::endl;
        return new SignalSaturation("DefaultSaturationName");
    }

    SignalSaturation(const string name,
                     const double upperlimit = 1.0,
                     const double lowerlimit = -1.0,
                     const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        mUpperLimit = upperlimit;
        mLowerLimit = lowerlimit;

        addReadPort("in", "NodeSignal", in);
        addWritePort("out", "NodeSignal", out);

        registerParameter("UpperLimit", "Upper Limit", "-", mUpperLimit);
        registerParameter("LowerLimit", "Lower Limit", "-", mLowerLimit);
    }


	void initialize()
	{
        //Nothing to initilize
	}


    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double input = mPortPtrs[in]->readNode(NodeSignal::VALUE);

        //Gain equations
		double output;
		if (input > mUpperLimit)
		{
		    output = mUpperLimit;
		}
		else if (input < mLowerLimit)
		{
		    output = mLowerLimit;
		}
		else
		{
		    output = input;
		}

        //Write new values to nodes
        mPortPtrs[out]->writeNode(NodeSignal::VALUE, output);

    }
};

#endif // SIGNALSATURATION_HPP_INCLUDED

