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
//! @file   HydraulicTLMRLineR.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2009-12-21
//!
//! @brief Contains a Hydraulic Transmission Line Component with Resistors in the ends
//!
//$Id$

#ifndef HYDRAULICTLMRLINER_HPP_INCLUDED
#define HYDRAULICTLMRLINER_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicTLMRlineR : public ComponentC
    {

    private:
        double mTimeDelay;
        double mAlpha;
        double mZc;
        double mR1;
        double mR2;

        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2;

        Delay mDelayedC1;
        Delay mDelayedC2;
        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new HydraulicTLMRlineR();
        }

        void configure()
        {
            //Set member attributes
            mTimeDelay     = 0.1;
            mZc            = 1.0e9;
            mAlpha         = 0.0;
            mR1            = 0.5;
            mR2            = 0.5;

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("deltat", "Time delay", "[s]",   mTimeDelay);
            registerParameter("alpha", "Low pass coefficient", "[-]", mAlpha);
            registerParameter("Z_c", "Characteristic Impedance", "[Ns/m^5]",  mZc);
            registerParameter("R_1", "Resistance 1", "[Ns/m^5]",  mR1);
            registerParameter("R_2", "Resistance 2", "[Ns/m^5]",  mR2);

            setStartValue(mpP1, NodeHydraulic::Flow, 0.0);
            setStartValue(mpP1, NodeHydraulic::Pressure, 1.0e5);
            setStartValue(mpP2, NodeHydraulic::Flow, 0.0);
            setStartValue(mpP2, NodeHydraulic::Pressure, 1.0e5);
        }


        void initialize()
        {
            mpND_p1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::Pressure);
            mpND_q1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::Flow);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::WaveVariable);
            mpND_Zc1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::CharImpedance);

            mpND_p2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::Pressure);
            mpND_q2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::Flow);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::WaveVariable);
            mpND_Zc2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::CharImpedance);

            //Write to nodes
            (*mpND_c1) = getStartValue(mpP1,NodeHydraulic::Pressure)+(mZc+mR1)*getStartValue(mpP1,NodeHydraulic::Flow);
            (*mpND_Zc1) = mZc+mR1;
            (*mpND_c2) = getStartValue(mpP2,NodeHydraulic::Pressure)+(mZc+mR2)*getStartValue(mpP2,NodeHydraulic::Flow);
            (*mpND_Zc2) = mZc+mR2;

            if (mTimeDelay-mTimestep < 0)
            {
                addWarningMessage("TimeDelay must be >= Ts");
                //stopSimulation();
            }

            //Init delay
            // We use -Ts to make the delay one step shorter as the TLM already have one built in timstep delay
            //! @todo for Td=Ts the delay will actually be 2Ts (the delay and inherited) need if check to avoid using delays if Td=Ts
            mDelayedC1.initialize(mTimeDelay-mTimestep, mTimestep, (*mpND_c1));
            mDelayedC2.initialize(mTimeDelay-mTimestep, mTimestep, (*mpND_c2));
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
            c10 = p2 + (mZc+mR2) * q2;
            c20 = p1 + (mZc+mR1) * q1;
            c1  = mAlpha*c1 + (1.0-mAlpha)*c10;
            c2  = mAlpha*c2 + (1.0-mAlpha)*c20;

            //Write new values to nodes
            (*mpND_c1) = mDelayedC1.update(c1);
            (*mpND_Zc1) = mZc + mR1;
            (*mpND_c2) = mDelayedC2.update(c2);
            (*mpND_Zc2) = mZc + mR2;
        }
    };
}

#endif // HYDRAULICTLMRLINER_HPP_INCLUDED
