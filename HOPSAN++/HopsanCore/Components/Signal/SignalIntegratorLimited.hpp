//!
//! @file   SignalIntegratorLimited.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-17
//!
//! @brief Contains a Signal Integrator Component with Limited Output
//!
//$Id$

#ifndef SIGNALINTEGRATORLIMITED_HPP_INCLUDED
#define SIGNALINTEGRATORLIMITED_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalIntegratorLimited : public ComponentSignal
    {

    private:
        double mMin, mMax;
        double mPrevU, mPrevY;
        double *input, *output;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalIntegratorLimited("IntegratorLimited");
        }

        SignalIntegratorLimited(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalIntegratorLimited";

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

            double startY = mpOut->getStartValue(NodeSignal::VALUE);
            mPrevU = startY;
            limit(startY, mMin, mMax);

            limit((*input), mMin, mMax);
        }


        void simulateOneTimestep()
        {
            //Filter equations
            //Bilinear transform is used
            (*output) = mPrevY + mTimestep/2.0*((*input) + mPrevU);

            if ((*output) >= mMax)
            {
                (*output) = mMax;
            }
            else if ((*output) <= mMin)
            {
                (*output) = mMin;
            }

            //Update filter:
            mPrevU = (*input);
            mPrevY = (*output);
        }
    };
}

#endif // SIGNALINTEGRATORLIMITED_HPP_INCLUDED


