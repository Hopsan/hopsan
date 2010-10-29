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
        double mStartY;
        double mPrevU;
        double mPrevY;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalIntegrator("Integrator");
        }

        SignalIntegrator(const std::string name) : ComponentSignal(name)
        {
            mTypeName = "SignalIntegrator";
            mStartY = 0.0;

            mpIn = addReadPort("in", "NodeSignal");
            mpOut = addWritePort("out", "NodeSignal");
        }


        void initialize()
        {
            double u0 = mpIn->readNode(NodeSignal::VALUE);
            mPrevU = mStartY;
            mPrevY = mStartY;
            mpOut->writeNode(NodeSignal::VALUE, mStartY);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double u = mpIn->readNode(NodeSignal::VALUE);

            //Filter equation
            //Bilinear transform is used
            double y = mPrevY + mTimestep/2.0*(u + mPrevU);

            //Write new values to nodes
            mpOut->writeNode(NodeSignal::VALUE, y);

            //Update filter:
            mPrevU = u;
            mPrevY = y;
        }
    };
}

#endif // SIGNALINTEGRATOR_HPP_INCLUDED


