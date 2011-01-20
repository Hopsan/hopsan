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
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalIntegrator : public ComponentSignal
    {

    private:
        double mPrevU;
        double mPrevY;
        double *mpND_in, *mpND_out;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalIntegrator("Integrator");
        }

        SignalIntegrator(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalIntegrator";

            mpIn = addReadPort("in", "NodeSignal", Port::NOTREQUIRED);
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);
        }


        void initialize()
        {
            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, 0);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            double startY = mpOut->getStartValue(NodeSignal::VALUE);
            mPrevU = startY;
            mPrevY = startY;

            (*mpND_out) = (*mpND_in);
        }


        void simulateOneTimestep()
        {
            //Filter equation
            //Bilinear transform is used
            (*mpND_out) = mPrevY + mTimestep/2.0*((*mpND_in) + mPrevU);

            //Update filter:
            mPrevU = (*mpND_in);
            mPrevY = (*mpND_out);
        }
    };
}

#endif // SIGNALINTEGRATOR_HPP_INCLUDED


