//!
//! @file   HydraulicTLMlossless.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2009-12-19
//!
//! @brief Contains a Hydraulic Lossless Transmission Line Component
//!
//$Id$

#ifndef HYDRAULICTLMLOSSLESS_HPP_INCLUDED
#define HYDRAULICTLMLOSSLESS_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicTLMlossless : public ComponentC
    {

    private:
        double mTimeDelay;
        double mAlpha;
        double mZc;
        Delay mDelayedC1;
        Delay mDelayedC2;
        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new HydraulicTLMlossless("TLMlossless");
        }

        HydraulicTLMlossless(const std::string name) : ComponentC(name)
        {
            //Set member attributes
            mTypeName = "HydraulicTLMlossless";
            mTimeDelay     = 0.1;
            mZc            = 1.0e9;
            mAlpha         = 0.0;

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("TD", "Time delay", "s",   mTimeDelay);
            registerParameter("a", "Low pass coeficient", "-", mAlpha);
            registerParameter("Zc", "Impedance", "Ns/m^5",  mZc);

            setStartValue(mpP1, NodeHydraulic::FLOW, 0.0);
            setStartValue(mpP1, NodeHydraulic::PRESSURE, 1.0e5);
            setStartValue(mpP2, NodeHydraulic::FLOW, 0.0);
            setStartValue(mpP2, NodeHydraulic::PRESSURE, 1.0e5);
        }


        void initialize()
        {
            //Write to nodes
            mpP1->writeNode(NodeHydraulic::FLOW,         getStartValue(mpP1,NodeHydraulic::FLOW));
            mpP1->writeNode(NodeHydraulic::PRESSURE,     getStartValue(mpP1,NodeHydraulic::PRESSURE));
            mpP1->writeNode(NodeHydraulic::WAVEVARIABLE, getStartValue(mpP1,NodeHydraulic::PRESSURE)+mZc*getStartValue(mpP1,NodeHydraulic::FLOW));
            mpP1->writeNode(NodeHydraulic::CHARIMP,      mZc);
            mpP2->writeNode(NodeHydraulic::FLOW,         getStartValue(mpP2,NodeHydraulic::FLOW));
            mpP2->writeNode(NodeHydraulic::PRESSURE,     getStartValue(mpP2,NodeHydraulic::PRESSURE));
            mpP2->writeNode(NodeHydraulic::WAVEVARIABLE, getStartValue(mpP2,NodeHydraulic::PRESSURE)+mZc*getStartValue(mpP2,NodeHydraulic::FLOW));
            mpP2->writeNode(NodeHydraulic::CHARIMP,      mZc);

            //Init delay
            mDelayedC1.initialize(mTimeDelay-mTimestep, mTimestep, getStartValue(mpP1,NodeHydraulic::PRESSURE)+mZc*getStartValue(mpP1,NodeHydraulic::FLOW)); //-mTimestep due to calc time
            mDelayedC2.initialize(mTimeDelay-mTimestep, mTimestep, getStartValue(mpP2,NodeHydraulic::PRESSURE)+mZc*getStartValue(mpP2,NodeHydraulic::FLOW));
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double q1 = mpP1->readNode(NodeHydraulic::FLOW);
            double p1 = mpP1->readNode(NodeHydraulic::PRESSURE);
            double q2 = mpP2->readNode(NodeHydraulic::FLOW);
            double p2 = mpP2->readNode(NodeHydraulic::PRESSURE);
            double c1 = mpP1->readNode(NodeHydraulic::WAVEVARIABLE);
            double c2 = mpP2->readNode(NodeHydraulic::WAVEVARIABLE);

            //Delay Line equations
            double c10 = p2 + mZc * q2;
            double c20 = p1 + mZc * q1;
            c1  = mAlpha*c1 + (1.0-mAlpha)*c10;
            c2  = mAlpha*c2 + (1.0-mAlpha)*c20;

            //Write new values to nodes
            //! @todo now when we update, in the next step we will read a value that is delayed two times, or??
            mpP1->writeNode(NodeHydraulic::WAVEVARIABLE, mDelayedC1.update(c1));
            mpP1->writeNode(NodeHydraulic::CHARIMP,      mZc);
            mpP2->writeNode(NodeHydraulic::WAVEVARIABLE, mDelayedC2.update(c2));
            mpP2->writeNode(NodeHydraulic::CHARIMP,      mZc);

        }
    };
}

#endif // HYDRAULICTLMLOSSLESS_HPP_INCLUDED
