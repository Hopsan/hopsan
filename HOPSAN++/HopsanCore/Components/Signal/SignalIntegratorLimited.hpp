//!
//! @file   SignalIntegratorLimited.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-17
//!
//! @brief Contains a Signal Integrator Component with Limited Output
//!
//$Id$

#ifndef SIGNALINTEGRATORLIMITED_HPP_INCLUDED
#define SIGNALINTEGRATORLIMITED_HPP_INCLUDED

#include "../../ComponentEssentials.h"

//!
//! @brief
//! @ingroup SignalComponents
//!
class SignalIntegratorLimited : public ComponentSignal
{

private:
    double mStartY;
    double mMin, mMax;
    Delay mDelayU;
    Delay mDelayY;
    enum {in, out};

public:
    static Component *Creator()
    {
        std::cout << "running IntegratorLimited creator" << std::endl;
        return new SignalIntegratorLimited("DefaultIntegratorLimitedName");
    }

    SignalIntegratorLimited(const string name,
                            const double min = -1.5E+300,
                            const double max =  1.5E+300,
                            const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        mTypeName = "SignalIntegratorLimited";
        mStartY = 0.0;

        mMin = min;
        mMax = max;

        mDelayU.setStepDelay(1);
        mDelayY.setStepDelay(1);

        addReadPort("in", "NodeSignal", in);
        addWritePort("out", "NodeSignal", out);
    }


	void initialize()
	{
	    double u0 = mPortPtrs[in]->readNode(NodeSignal::VALUE);
	    mDelayU.initialize(mTime, u0);
	    mDelayY.initialize(mTime, max(min(mStartY, mMax), mMin));
	}


    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double u = mPortPtrs[in]->readNode(NodeSignal::VALUE);

        //Filter equations
        //Bilinear transform is used
		double y = mDelayY.value() + mTimestep/2.0*(u + mDelayU.value());

        if (y >= mMax)
        {
            mPortPtrs[out]->writeNode(NodeSignal::VALUE, mMax);
        }
        else if (y <= mMin)
        {
            mPortPtrs[out]->writeNode(NodeSignal::VALUE, mMin);
        }
        else
        {
            //Write new values to nodes
            mPortPtrs[out]->writeNode(NodeSignal::VALUE, y);

            //Update filter:
            mDelayU.update(u);
            mDelayY.update(y);
        }
    }
};

#endif // SIGNALINTEGRATORLIMITED_HPP_INCLUDED


