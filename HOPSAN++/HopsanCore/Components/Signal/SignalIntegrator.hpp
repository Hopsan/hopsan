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
        double *input, *output;
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
            mPrevU = startY;
            mPrevY = startY;
        }


        void simulateOneTimestep()
        {
            //Filter equation
            //Bilinear transform is used
            (*output) = mPrevY + mTimestep/2.0*((*input) + mPrevU);

            //Update filter:
            mPrevU = (*input);
            mPrevY = (*output);
        }
    };
}

#endif // SIGNALINTEGRATOR_HPP_INCLUDED


