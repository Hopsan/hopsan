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
        double *input, *output;
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

            mMin = -1.5E+300;   //! @todo Shouldn't these be registered parameters?
            mMax = 1.5E+300;

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

            mIntegrator.initialize(mTimestep, (*input), mStartY, mMin, mMax);
            mpOut->writeNode(NodeSignal::VALUE, mStartY);
        }


        void simulateOneTimestep()
        {

            //Filter equation
            (*output) = mIntegrator.update((*input));
        }
    };
}

#endif // SIGNALINTEGRATORLIMITED2_HPP_INCLUDED


