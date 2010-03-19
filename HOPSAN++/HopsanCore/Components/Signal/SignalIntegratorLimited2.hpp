//!
//! @file   SignalIntegrator2.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-22
//!
//! @brief Contains a Signal Integrator Component using CoreUtilities
//!
//$Id$

#ifndef SIGNALINTEGRATORLIMITED2_HPP_INCLUDED
#define SIGNALINTEGRATORLIMITED2_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

//!
//! @brief
//! @ingroup SignalComponents
//!
class SignalIntegratorLimited2 : public ComponentSignal
{

private:
    IntegratorLimited mIntegrator;
    double mStartY;
    double mMin, mMax;
    Port *mpIn, *mpOut;

public:
    static Component *Creator()
    {
        std::cout << "running Integrator creator" << std::endl;
        return new SignalIntegrator2("Integrator");
    }

    SignalIntegratorLimited2(const string name,
                            const double min = -1.5E+300,
                            const double max =  1.5E+300,
                            const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        mTypeName = "SignalIntegratorLimited2";
        mStartY = 0.0;

        mMin = min;
        mMax = max;

        mpIn = addReadPort("in", "NodeSignal");
        mpOut = addWritePort("out", "NodeSignal");
    }


    void initialize()
    {
        double u0 = mpIn->readNode(NodeSignal::VALUE);

        mIntegrator.initialize(mTime, mTimestep, u0, mStartY, mMin, mMax);
        //! @todo Write out values into node as well? (I think so) This is true for all components
    }


    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double u = mpIn->readNode(NodeSignal::VALUE);

        //Filter equation
        //Get variable values from nodes

        //Write new values to nodes
        mpOut->writeNode(NodeSignal::VALUE, mIntegrator.value(u));
    }
};

#endif // SIGNALINTEGRATORLIMITED2_HPP_INCLUDED


