//!
//! @file   HydraulicTLMlossless.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2009-12-19
//!
//! @brief Contains a Hydraulic Lossless Transmission Line Component
//!
//$Id$

#ifndef HYDRAULICTLMLOSSLESS_HPP_INCLUDED
#define HYDRAULICTLMLOSSLESS_HPP_INCLUDED

#include <iostream>

#include "Component.h"
#include "Nodes/Nodes.h"
#include "CoreUtilities/Delay.h"

class HydraulicTLMlossless : public ComponentC
{

private:
    double mStartPressure;
    double mStartFlow;
    double mTimeDelay;
    double mAlpha;
    double mZc;
	Delay mDelayedC1;
	Delay mDelayedC2;
    enum {P1, P2};

public:
    HydraulicTLMlossless(const string name,
                         const double zc        = 1.0e9,
                         const double timeDelay = 0.1,
                         const double alpha     = 0.0,
                         const double timestep  = 0.001)
	: ComponentC(name, timestep)
    {
        //Set member attributes
        mStartPressure = 1.0;
        mStartFlow     = 0.0;
        mTimeDelay     = timeDelay;
        mZc            = zc;
		mAlpha         = alpha;

		//Add ports to the component
        addPowerPort("P1", "NodeHydraulic", P1);
        addPowerPort("P2", "NodeHydraulic", P2);

        //Register changable parameters to the HOPSAN++ core
        registerParameter("TD", "Tidsfördröjning", "s",   mTimeDelay);
        registerParameter("a", "Lågpasskoeficient", "-", mAlpha);
        registerParameter("Zc", "Impedans", "Ns/m^5",  mZc);
    }


	void initialize()
    {
        //Write to nodes
        mPortPtrs[P1]->writeNode(NodeHydraulic::MASSFLOW,     mStartFlow);
        mPortPtrs[P1]->writeNode(NodeHydraulic::PRESSURE,     mStartPressure);
        mPortPtrs[P1]->writeNode(NodeHydraulic::WAVEVARIABLE, mStartPressure+mZc*mStartFlow);
        mPortPtrs[P1]->writeNode(NodeHydraulic::CHARIMP,      mZc);
        mPortPtrs[P2]->writeNode(NodeHydraulic::MASSFLOW,     mStartFlow);
        mPortPtrs[P2]->writeNode(NodeHydraulic::PRESSURE,     mStartPressure);
        mPortPtrs[P2]->writeNode(NodeHydraulic::WAVEVARIABLE, mStartPressure+mZc*mStartFlow);
        mPortPtrs[P2]->writeNode(NodeHydraulic::CHARIMP,      mZc);

		//Init delay
        mDelayedC1.initialize(mTime, mStartPressure+mZc*mStartFlow);
		mDelayedC2.initialize(mTime, mStartPressure+mZc*mStartFlow);

		//Set external parameters
		mDelayedC1.setTimeDelay(mTimeDelay-mTimestep, mTimestep); //-mTimestep sue to calc time
		mDelayedC2.setTimeDelay(mTimeDelay-mTimestep, mTimestep);
	}


	void simulateOneTimestep()
    {
        //Get variable values from nodes
        double q1 = mPortPtrs[P1]->readNode(NodeHydraulic::MASSFLOW);
        double p1 = mPortPtrs[P1]->readNode(NodeHydraulic::PRESSURE);
        double q2 = mPortPtrs[P2]->readNode(NodeHydraulic::MASSFLOW);
        double p2 = mPortPtrs[P2]->readNode(NodeHydraulic::PRESSURE);
        double c1 = mPortPtrs[P1]->readNode(NodeHydraulic::WAVEVARIABLE);
        double c2 = mPortPtrs[P2]->readNode(NodeHydraulic::WAVEVARIABLE);

        //Delay Line equations
        double c10 = p2 + mZc * q2;
        double c20 = p1 + mZc * q1;
        c1  = mAlpha*c1 + (1.0-mAlpha)*c10;
        c2  = mAlpha*c2 + (1.0-mAlpha)*c20;

        //Write new values to nodes
        mPortPtrs[P1]->writeNode(NodeHydraulic::WAVEVARIABLE, mDelayedC1.value(c1));
        mPortPtrs[P1]->writeNode(NodeHydraulic::CHARIMP,      mZc);
        mPortPtrs[P2]->writeNode(NodeHydraulic::WAVEVARIABLE, mDelayedC2.value(c1));
        mPortPtrs[P2]->writeNode(NodeHydraulic::CHARIMP,      mZc);

        //Update the delayed variabels
		mDelayedC1.update(c1);
		mDelayedC2.update(c2);
    }
};

#endif // HYDRAULICTLMLOSSLESS_HPP_INCLUDED
