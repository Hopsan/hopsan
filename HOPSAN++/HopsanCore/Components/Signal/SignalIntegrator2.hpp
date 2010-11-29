//!
//! @file   SignalIntegrator2.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-22
//!
//! @brief Contains a Signal Integrator Component using CoreUtilities
//!
//$Id$

#ifndef SIGNALINTEGRATOR2_HPP_INCLUDED
#define SIGNALINTEGRATOR2_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalIntegrator2 : public ComponentSignal
    {

    private:
        Integrator mIntegrator;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalIntegrator2("Integrator");
        }

        SignalIntegrator2(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalIntegrator2";

            //mIntegrator.initializeValues(0.0, mStartY, mTimestep, mTime);

            mpIn = addReadPort("in", "NodeSignal");
            mpOut = addWritePort("out", "NodeSignal");
        }


        void initialize()
        {
            double startY = mpOut->getStartValue(NodeSignal::VALUE);
            mIntegrator.initialize(mTimestep, startY, startY);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double u = mpIn->readNode(NodeSignal::VALUE);

            //Filter equation

            //Write new values to nodes
            mpOut->writeNode(NodeSignal::VALUE, mIntegrator.update(u));

        }
    };
}

#endif // SIGNALINTEGRATOR_HPP_INCLUDED


