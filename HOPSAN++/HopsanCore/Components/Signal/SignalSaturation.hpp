//!
//! @file   SignalSaturation.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-12
//!
//! @brief Contains a signal saturation component
//!
//$Id$

#ifndef SIGNALSATURATION_HPP_INCLUDED
#define SIGNALSATURATION_HPP_INCLUDED

#include "../../ComponentEssentials.h"

//!
//! @brief
//! @ingroup SignalComponents
//!
class SignalSaturation : public ComponentSignal
{

private:
    double mUpperLimit;
    double mLowerLimit;
    Port *mpIn, *mpOut;

public:
    static Component *Creator()
    {
        std::cout << "running Saturation creator" << std::endl;
        return new SignalSaturation("Saturation");
    }

    SignalSaturation(const string name,
                     const double upperlimit = 1.0,
                     const double lowerlimit = -1.0,
                     const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        mTypeName = "SignalSaturation";
        mUpperLimit = upperlimit;
        mLowerLimit = lowerlimit;

        mpIn = addReadPort("in", "NodeSignal");
        mpOut = addWritePort("out", "NodeSignal");

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
        double input = mpIn->readNode(NodeSignal::VALUE);

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
        mpOut->writeNode(NodeSignal::VALUE, output);

    }
};

#endif // SIGNALSATURATION_HPP_INCLUDED

