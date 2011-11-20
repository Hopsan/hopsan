/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

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
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

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

        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2;

        Delay mDelayedC1;
        Delay mDelayedC2;
        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new HydraulicTLMlossless();
        }

        HydraulicTLMlossless() : ComponentC()
        {
            //Set member attributes
            mTimeDelay     = 0.1;
            mZc            = 1.0e9;
            mAlpha         = 0.0;

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("deltat", "Time delay", "[s]",   mTimeDelay);
            registerParameter("alpha", "Low pass coeficient", "[-]", mAlpha);
            registerParameter("Z_c", "Impedance", "[Ns/m^5]",  mZc);

            setStartValue(mpP1, NodeHydraulic::FLOW, 0.0);
            setStartValue(mpP1, NodeHydraulic::PRESSURE, 1.0e5);
            setStartValue(mpP2, NodeHydraulic::FLOW, 0.0);
            setStartValue(mpP2, NodeHydraulic::PRESSURE, 1.0e5);
        }


        void initialize()
        {
            mpND_p1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE);
            mpND_q1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::FLOW);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::CHARIMP);

            mpND_p2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::PRESSURE);
            mpND_q2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::FLOW);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::CHARIMP);

            //Write to nodes
            (*mpND_q1) = getStartValue(mpP1,NodeHydraulic::FLOW);
            (*mpND_p1) = getStartValue(mpP1,NodeHydraulic::PRESSURE);
            (*mpND_c1) = getStartValue(mpP1,NodeHydraulic::PRESSURE)+mZc*getStartValue(mpP1,NodeHydraulic::FLOW);
            (*mpND_Zc1) = mZc;
            (*mpND_q2) = getStartValue(mpP2,NodeHydraulic::FLOW);
            (*mpND_p2) = getStartValue(mpP2,NodeHydraulic::PRESSURE);
            (*mpND_c1) = getStartValue(mpP2,NodeHydraulic::PRESSURE)+mZc*getStartValue(mpP2,NodeHydraulic::FLOW);
            (*mpND_Zc2) = mZc;

            //Init delay
            mDelayedC1.initialize(mTimeDelay-mTimestep, mTimestep, getStartValue(mpP1,NodeHydraulic::PRESSURE)+mZc*getStartValue(mpP1,NodeHydraulic::FLOW)); //-mTimestep due to calc time
            mDelayedC2.initialize(mTimeDelay-mTimestep, mTimestep, getStartValue(mpP2,NodeHydraulic::PRESSURE)+mZc*getStartValue(mpP2,NodeHydraulic::FLOW));
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double p1, q1, c1, p2, q2, c2, c10, c20;

            //Read variables from nodes
            p1 = (*mpND_p1);
            q1 = (*mpND_q1);
            p2 = (*mpND_p2);
            q2 = (*mpND_q2);
            c1 = (*mpND_c1);
            c2 = (*mpND_c2);

            //Delay Line equations
            c10 = p2 + mZc * q2;
            c20 = p1 + mZc * q1;
            c1  = mAlpha*c1 + (1.0-mAlpha)*c10;
            c2  = mAlpha*c2 + (1.0-mAlpha)*c20;

            //Write new values to nodes
            //! @todo now when we update, in the next step we will read a value that is delayed two times, or??
            (*mpND_c1) = mDelayedC1.update(c1);
            (*mpND_Zc1) = mZc;
            (*mpND_c2) = mDelayedC2.update(c2);
            (*mpND_Zc2) = mZc;

        }
    };
}

#endif // HYDRAULICTLMLOSSLESS_HPP_INCLUDED
