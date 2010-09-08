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

namespace hopsan {

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
            return new SignalIntegratorLimited2("Integrator");
        }

        SignalIntegratorLimited2(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalIntegratorLimited2";
            mStartY = 0.0;

            mMin = -1.5E+300;
            mMax = 1.5E+300;

            mpIn = addReadPort("in", "NodeSignal");
            mpOut = addWritePort("out", "NodeSignal");
        }


        void initialize()
        {
            double u0 = mpIn->readNode(NodeSignal::VALUE);

            mIntegrator.initialize(mTime, mTimestep, u0, mStartY, mMin, mMax);
            mpOut->writeNode(NodeSignal::VALUE, mStartY);
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
}

#endif // SIGNALINTEGRATORLIMITED2_HPP_INCLUDED


