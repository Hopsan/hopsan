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
        double *mpND_in, *mpND_out;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalIntegratorLimited2("Integrator");
        }

        SignalIntegratorLimited2(const std::string name) : ComponentSignal(name)
        {
            mStartY = 0.0;

            mMin = -1.5E+300;   //! @todo Shouldn't these be registered parameters?
            mMax = 1.5E+300;

            mpIn = addReadPort("in", "NodeSignal", Port::NOTREQUIRED);
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);
        }


        void initialize()
        {
            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, 0);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            mIntegrator.initialize(mTimestep, (*mpND_in), mStartY, mMin, mMax);

            (*mpND_out) = (*mpND_in);
            limitValue((*mpND_out), mMin, mMax);
        }


        void simulateOneTimestep()
        {

            //Filter equation
            (*mpND_out) = mIntegrator.update((*mpND_in));
        }
    };
}

#endif // SIGNALINTEGRATORLIMITED2_HPP_INCLUDED


