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
    Port *mpIn, *mpOut;

public:
    static Component *Creator()
    {
        return new SignalIntegratorLimited("IntegratorLimited");
    }

    SignalIntegratorLimited(const std::string name) : ComponentSignal(name)
    {
        mTypeName = "SignalIntegratorLimited";
        mStartY = 0.0;

        mMin = -1.5E+300;
        mMax = 1.5E+300;

        mDelayU.setStepDelay(1);
        mDelayY.setStepDelay(1);

        mpIn = addReadPort("in", "NodeSignal");
        mpOut = addWritePort("out", "NodeSignal");
    }


    void initialize()
    {
        double u0 = mpIn->readNode(NodeSignal::VALUE);
        mDelayU.initialize(mTime, u0);
        mDelayY.initialize(mTime, std::max(std::min(mStartY, mMax), mMin));
    }


    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double u = mpIn->readNode(NodeSignal::VALUE);

        //Filter equations
        //Bilinear transform is used
        double y = mDelayY.value() + mTimestep/2.0*(u + mDelayU.value());

        if (y >= mMax)
        {
            mpOut->writeNode(NodeSignal::VALUE, mMax);
        }
        else if (y <= mMin)
        {
            mpOut->writeNode(NodeSignal::VALUE, mMin);
        }
        else
        {
            //Write new values to nodes
            mpOut->writeNode(NodeSignal::VALUE, y);

            //Update filter:
            mDelayU.update(u);
            mDelayY.update(y);
        }
    }
};

#endif // SIGNALINTEGRATORLIMITED_HPP_INCLUDED


