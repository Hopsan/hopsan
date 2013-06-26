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
//! @file   HydraulicVolume.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2009-12-19
//!
//! @brief Contains a Hydraulic Volume Component
//!
//$Id$

#ifndef HYDRAULICVOLUME_HPP_INCLUDED
#define HYDRAULICVOLUME_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief A hydraulic volume component
    //! @ingroup HydraulicComponents
    //!
    class HydraulicVolume : public ComponentC
    {

    private:
        double *mpAlpha;
        double Zc;
        double V;
        double betae;

        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2;

        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new HydraulicVolume();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");

            addInputVariable("alpha", "Low pass coeficient to dampen standing delayline waves", "-", 0.1 ,&mpAlpha);

            addConstant("V", "Volume", "m^3", 1.0e-3, V);
            addConstant("Beta_e", "Bulkmodulus", "Pa", 1.0e9, betae);

            setDefaultStartValue(mpP1, NodeHydraulic::Flow, 0.0);
            setDefaultStartValue(mpP1, NodeHydraulic::Pressure, 1.0e5);
            setDefaultStartValue(mpP2, NodeHydraulic::Flow, 0.0);
            setDefaultStartValue(mpP2, NodeHydraulic::Pressure, 1.0e5);
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

            double alpha = (*mpAlpha);
            Zc = betae/V*mTimestep/(1.0-alpha); //Need to be updated at simulation start since it is volume and bulk that are set.

            //Write to nodes
            (*mpND_c1) = getStartValue(mpP2,NodeHydraulic::Pressure)+Zc*getStartValue(mpP2,NodeHydraulic::Flow);
            (*mpND_Zc1) = Zc;
            (*mpND_c2) = getStartValue(mpP1,NodeHydraulic::Pressure)+Zc*getStartValue(mpP1,NodeHydraulic::Flow);
            (*mpND_Zc2) = Zc;
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double q1, c1, q2, c2, c10, c20, alpha;

            //Get variable values from nodes
            q1 = (*mpND_q1);
            q2 = (*mpND_q2);
            c1 = (*mpND_c1);
            c2 = (*mpND_c2);
            alpha = (*mpAlpha);

            //Volume equations
            c10 = c2 + 2.0*Zc * q2;     //These two equations are from old Hopsan
            c20 = c1 + 2.0*Zc * q1;

            c1 = alpha*c1 + (1.0-alpha)*c10;
            c2 = alpha*c2 + (1.0-alpha)*c20;

            //Write new values to nodes
            (*mpND_c1) = c1;
            (*mpND_Zc1) = Zc;
            (*mpND_c2) = c2;
            (*mpND_Zc2) = Zc;
        }

        void finalize()
        {

        }
    };
}

#endif // HYDRAULICVOLUME_HPP_INCLUDED
