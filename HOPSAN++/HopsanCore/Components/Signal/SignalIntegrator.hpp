//!
//! @file   SignalIntegrator.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-17
//!
//! @brief Contains a Signal Integrator Component
//!
//$Id$

#ifndef SIGNALINTEGRATOR_HPP_INCLUDED
#define SIGNALINTEGRATOR_HPP_INCLUDED

#include "HopsanCore.h"

class SignalIntegrator : public ComponentSignal
{

private:
    double mStartY;
    Delay mDelayU;
    Delay mDelayY;
    enum {in, out};

public:
    static Component *Creator()
    {
        std::cout << "running Integrator creator" << std::endl;
        return new SignalIntegrator("DefaultIntegratorName");
    }

    SignalIntegrator(const string name,
                     const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        mTypeName = "SignalIntegrator";
        mStartY = 0.0;

        mDelayU.setStepDelay(1);
        mDelayY.setStepDelay(1);

        addReadPort("in", "NodeSignal", in);
        addWritePort("out", "NodeSignal", out);
    }


	void initialize()
	{
	    double u0 = mPortPtrs[in]->readNode(NodeSignal::VALUE);
	    mDelayU.initialize(mTime, u0);
	    mDelayY.initialize(mTime, mStartY);
	    ///TODO: Write out values into node as well? (I think so) This is true for all components
	}


    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double u = mPortPtrs[in]->readNode(NodeSignal::VALUE);

        //Filter equation
        //Bilinear transform is used
		double y = mDelayY.value() + mTimestep/2.0*(u + mDelayU.value());

        //Write new values to nodes
        mPortPtrs[out]->writeNode(NodeSignal::VALUE, y);

        //Update filter:
        mDelayU.update(u);
        mDelayY.update(y);
    }
};

#endif // SIGNALINTEGRATOR_HPP_INCLUDED


