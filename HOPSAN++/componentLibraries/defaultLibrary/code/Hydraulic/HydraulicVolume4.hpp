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
//! @file   HydraulicVolume4.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-01-04
//!
//! @brief Contains a Hydraulic Volume Component With Four Ports (temporary solution)
//!
//$Id$

#ifndef HYDRAULICVOLUME4_HPP_INCLUDED
#define HYDRAULICVOLUME4_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief A hydraulic volume component
    //! @ingroup HydraulicComponents
    //!
    class HydraulicVolume4 : public ComponentC
    {

    private:
        double mZc;
        double mAlpha;
        double mVolume;
        double mBulkmodulus;
        double pMean, c10, c20, c30, c40;

        double q1, c1, Zc1, q2, c2, Zc2, q3, c3, Zc3, q4, c4, Zc4;
        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2, *mpND_p3, *mpND_q3, *mpND_c3, *mpND_Zc3, *mpND_p4, *mpND_q4, *mpND_c4, *mpND_Zc4;

        Port *mpP1, *mpP2, *mpP3, *mpP4;

    public:
        static Component *Creator()
        {
            return new HydraulicVolume4();
        }

        HydraulicVolume4() : ComponentC()
        {
            //Set member attributes
            mBulkmodulus   = 1.0e9;
            mVolume        = 1.0e-3;
            mAlpha         = 0.1;

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
            mpP3 = addPowerPort("P3", "NodeHydraulic");
            mpP4 = addPowerPort("P4", "NodeHydraulic");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("V", "Volume", "[m^3]",            mVolume);
            registerParameter("Beta_e", "Bulkmodulus", "[Pa]", mBulkmodulus);
            registerParameter("alpha", "Low pass coeficient to dampen standing delayline waves", "[-]",  mAlpha);

            setStartValue(mpP1, NodeHydraulic::FLOW, 0.0);
            setStartValue(mpP1, NodeHydraulic::PRESSURE, 1.0e5);
            setStartValue(mpP2, NodeHydraulic::FLOW, 0.0);
            setStartValue(mpP2, NodeHydraulic::PRESSURE, 1.0e5);
            setStartValue(mpP3, NodeHydraulic::FLOW, 0.0);
            setStartValue(mpP3, NodeHydraulic::PRESSURE, 1.0e5);
            setStartValue(mpP4, NodeHydraulic::FLOW, 0.0);
            setStartValue(mpP4, NodeHydraulic::PRESSURE, 1.0e5);
        }


        void initialize()
        {
            mpND_p1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE);
            mpND_q1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::FLOW);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::CHARIMP);
            mpND_p2 = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE);
            mpND_q2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::FLOW);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::CHARIMP);
            mpND_p3 = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE);
            mpND_q3 = getSafeNodeDataPtr(mpP3, NodeHydraulic::FLOW);
            mpND_c3 = getSafeNodeDataPtr(mpP3, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc3 = getSafeNodeDataPtr(mpP3, NodeHydraulic::CHARIMP);
            mpND_p4 = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE);
            mpND_q4 = getSafeNodeDataPtr(mpP4, NodeHydraulic::FLOW);
            mpND_c4 = getSafeNodeDataPtr(mpP4, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc4 = getSafeNodeDataPtr(mpP4, NodeHydraulic::CHARIMP);

            mZc = 3 / 2 * mBulkmodulus/mVolume*mTimestep/(1-mAlpha); //Need to be updated at simulation start since it is volume and bulk that are set.

            //Write to nodes
            (*mpND_q1) = getStartValue(mpP1,NodeHydraulic::FLOW);
            (*mpND_p1) = getStartValue(mpP1,NodeHydraulic::PRESSURE);
            (*mpND_c1) = getStartValue(mpP1,NodeHydraulic::PRESSURE)+mZc*getStartValue(mpP1,NodeHydraulic::FLOW);
            (*mpND_Zc1) = mZc;
            (*mpND_q2) = getStartValue(mpP2,NodeHydraulic::FLOW);
            (*mpND_p2) = getStartValue(mpP2,NodeHydraulic::PRESSURE);
            (*mpND_c2) = getStartValue(mpP2,NodeHydraulic::PRESSURE)+mZc*getStartValue(mpP2,NodeHydraulic::FLOW);
            (*mpND_Zc2) = mZc;
            (*mpND_q3) = getStartValue(mpP3,NodeHydraulic::FLOW);
            (*mpND_p3) = getStartValue(mpP3,NodeHydraulic::PRESSURE);
            (*mpND_c3) = getStartValue(mpP3,NodeHydraulic::PRESSURE)+mZc*getStartValue(mpP3,NodeHydraulic::FLOW);
            (*mpND_Zc3) = mZc;
            (*mpND_q4) = getStartValue(mpP4,NodeHydraulic::FLOW);
            (*mpND_p4) = getStartValue(mpP4,NodeHydraulic::PRESSURE);
            (*mpND_c4) = getStartValue(mpP4,NodeHydraulic::PRESSURE)+mZc*getStartValue(mpP4,NodeHydraulic::FLOW);
            (*mpND_Zc4) = mZc;
        }

        void simulateOneTimestep()
        {
            //Read values from nodes
            q1 = (*mpND_q1);
            c1 = (*mpND_c1);
            Zc1 = (*mpND_Zc1);
            q2 = (*mpND_q2);
            c2 = (*mpND_c2);
            Zc2 = (*mpND_Zc2);
            q3 = (*mpND_q3);
            c3 = (*mpND_c3);
            Zc3 = (*mpND_Zc3);
            q4 = (*mpND_q4);
            c4 = (*mpND_c4);
            Zc4 = (*mpND_Zc4);

            //Volume equations
            pMean = ((c1 + Zc1*2*q1) + (c2 + Zc2*2*q2) + (c3 + Zc3*2*q3) + (c4 + Zc4*2*q4)) / 4;

            c10 = 2*pMean - c1 - 2*Zc1*q1;
            c1 = mAlpha*c1 + (1.0 - mAlpha)*c10 + (Zc1 - mZc)*q1;

            c20 = 2*pMean - c2 - 2*Zc2*q2;
            c2 = mAlpha*c2 + (1.0 - mAlpha)*c20 + (Zc2 - mZc)*q2;

            c30 = 2*pMean - c3 - 2*Zc3*q3;
            c3 = mAlpha*c3 + (1.0 - mAlpha)*c30 + (Zc3 - mZc)*q3;

            c40 = 2*pMean - c4 - 2*Zc4*q4;
            c4 = mAlpha*c4 + (1.0 - mAlpha)*c40 + (Zc4 - mZc)*q4;

            //Write new values to nodes
            (*mpND_c1) = c1;
            (*mpND_c2) = c2;
            (*mpND_c3) = c3;
            (*mpND_c4) = c4;
            (*mpND_Zc1) = Zc1;
            (*mpND_Zc2) = Zc2;
            (*mpND_Zc3) = Zc3;
            (*mpND_Zc4) = Zc4;
        }

        void finalize()
        {

        }
    };
}

#endif // HYDRAULICVOLUME3_HPP_INCLUDED
