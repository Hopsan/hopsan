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
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalIntegratorLimited("IntegratorLimited");
        }

        SignalIntegratorLimited(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalIntegratorLimited";

            mMin = -1.5E+300;
            mMax = 1.5E+300;

            mpIn = addReadPort("in", "NodeSignal");
            mpOut = addWritePort("out", "NodeSignal");
        }


        void initialize()
        {
            double startY = mpOut->getStartValue(NodeSignal::VALUE);
            mPrevU = startY;
            mPrevY = std::max(std::min(startY, mMax), mMin);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double u = mpIn->readNode(NodeSignal::VALUE);

            //Filter equations
            //Bilinear transform is used
            double y = mPrevY + mTimestep/2.0*(u + mPrevU);

            if (y >= mMax)
            {
                mpOut->writeNode(NodeSignal::VALUE, mMax);
            }
            else if (y <= mMin)
            {
                mpOut->writeNode(NodeSignal::VALUE, mMin);
            }
            else
            {
                //Write new values to nodes
                mpOut->writeNode(NodeSignal::VALUE, y);

                //Update filter:
                mPrevU = u;
                mPrevY = y;
            }
        }
    };
}

#endif // SIGNALINTEGRATORLIMITED_HPP_INCLUDED


