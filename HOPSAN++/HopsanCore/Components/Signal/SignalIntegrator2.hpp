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
        double *input, *output;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalIntegrator2("Integrator");
        }

        SignalIntegrator2(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalIntegrator2";

            mpIn = addReadPort("in", "NodeSignal", Port::NOTREQUIRED);
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);
        }


        void initialize()
        {
            if(mpIn->isConnected())
            {
                input = mpIn->getNodeDataPtr(NodeSignal::VALUE);
            }
            else
            {
                input = new double(0);
            }

            if(mpOut->isConnected())
            {
                output = mpOut->getNodeDataPtr(NodeSignal::VALUE);
            }
            else
            {
                output = new double();
            }

            double startY = mpOut->getStartValue(NodeSignal::VALUE);
            mIntegrator.initialize(mTimestep, startY, startY);
        }


        void simulateOneTimestep()
        {
            //Filter equation
           (*output) = mIntegrator.update((*input));
        }
    };
}

#endif // SIGNALINTEGRATOR_HPP_INCLUDED


