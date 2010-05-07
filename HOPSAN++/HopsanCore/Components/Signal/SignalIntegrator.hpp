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

#include "../../ComponentEssentials.h"

//!
//! @brief
//! @ingroup SignalComponents
//!
class SignalIntegrator : public ComponentSignal
{

private:
    double mStartY;
    Delay mDelayU;
    Delay mDelayY;
    Port *mpIn, *mpOut;

public:
    static Component *Creator()
    {
        return new SignalIntegrator("Integrator");
    }

    SignalIntegrator(const string name) : ComponentSignal(name)
    {
        mTypeName = "SignalIntegrator";
        mStartY = 0.0;

        mDelayU.setStepDelay(1);
        mDelayY.setStepDelay(1);

        mpIn = addReadPort("in", "NodeSignal");
        mpOut = addWritePort("out", "NodeSignal");
    }


    void initialize()
    {
        double u0 = mpIn->readNode(NodeSignal::VALUE);
        mDelayU.initialize(mTime, u0);
        mDelayY.initialize(mTime, mStartY);
        //! @todo Write out values into node as well? (I think so) This is true for all components
    }


    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double u = mpIn->readNode(NodeSignal::VALUE);

        //Filter equation
        //Bilinear transform is used
        double y = mDelayY.value() + mTimestep/2.0*(u + mDelayU.value());

        //Write new values to nodes
        mpOut->writeNode(NodeSignal::VALUE, y);

        //Update filter:
        mDelayU.update(u);
        mDelayY.update(y);
    }
};

#endif // SIGNALINTEGRATOR_HPP_INCLUDED


