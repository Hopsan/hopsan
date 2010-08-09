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
        double mStartY;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalIntegrator2("Integrator");
        }

        SignalIntegrator2(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalIntegrator2";
            mStartY = 0.0;

            //mIntegrator.initializeValues(0.0, mStartY, mTimestep, mTime);

            mpIn = addReadPort("in", "NodeSignal");
            mpOut = addWritePort("out", "NodeSignal");
        }


        void initialize()
        {
            double u0 = mpIn->readNode(NodeSignal::VALUE);
            mIntegrator.initialize(mTime, mTimestep, u0, mStartY);
            //! @todo Write out values into node as well? (I think so) This is true for all components
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double u = mpIn->readNode(NodeSignal::VALUE);

            //Filter equation

            //Write new values to nodes
            mpOut->writeNode(NodeSignal::VALUE, mIntegrator.value(u));

        }
    };
}

#endif // SIGNALINTEGRATOR_HPP_INCLUDED


