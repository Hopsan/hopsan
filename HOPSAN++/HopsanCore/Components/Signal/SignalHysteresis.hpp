//!
//! @file   SignalHysteresis.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-02-04
//!
//! @brief Contains a Signal Hysteresis Component
//!
//$Id$

#ifndef SIGNALHYSTERESIS_HPP_INCLUDED
#define SIGNALHYSTERESIS_HPP_INCLUDED

#include "HopsanCore.h"

class SignalHysteresis : public ComponentSignal
{

private:
    double mHysteresisWidth;
    Delay mDelayedInput;
    ValveHysteresis mHyst;
    enum {in, out};

public:
    static Component *Creator()
    {
        std::cout << "running Hysteresis creator" << std::endl;
        return new SignalHysteresis("DefaultHysteresisName");
    }

    SignalHysteresis(const string name,
               const double hysteresiswidth = 1.0,
               const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        mTypeName = "SignalHysteresis";
        mHysteresisWidth = hysteresiswidth;

        addReadPort("in", "NodeSignal", in);
        addWritePort("out", "NodeSignal", out);

        registerParameter("HysteresisWidth", "Width of the Hysteresis", "-", mHysteresisWidth);
    }

	void initialize()
	{
        mDelayedInput.initialize(mTime, 0.0);
        mDelayedInput.setStepDelay(1);
	}


    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double input = mPortPtrs[in]->readNode(NodeSignal::VALUE);

        //Hysteresis equations
        double output = mHyst.getValue(input, mHysteresisWidth, mDelayedInput.value());
        mDelayedInput.update(output);

        //Write new values to nodes
        mPortPtrs[out]->writeNode(NodeSignal::VALUE, output);
    }
};

#endif // SIGNALHYSTERESIS_HPP_INCLUDED
