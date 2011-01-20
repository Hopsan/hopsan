//!
//! @file   HydraulicVolume.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-01-04
//!
//! @brief Contains a Hydraulic Volume Component With Four Ports (temporary solution)
//!
//$Id$

#ifndef HYDRAULICVOLUME4_HPP_INCLUDED
#define HYDRAULICVOLUME4_HPP_INCLUDED

#include "../../ComponentEssentials.h"

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
        double *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_q2, *mpND_c2, *mpND_Zc2, *mpND_q3, *mpND_c3, *mpND_Zc3, *mpND_q4, *mpND_c4, *mpND_Zc4;

        Port *mpP1, *mpP2, *mpP3, *mpP4;

    public:
        static Component *Creator()
        {
            return new HydraulicVolume4("Volume4");
        }

        HydraulicVolume4(const std::string name) : ComponentC(name)
        {
            //Set member attributes
            mTypeName = "HydraulicVolume4";
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
            registerParameter("Be", "Bulkmodulus", "[Pa]", mBulkmodulus);
            registerParameter("a", "Low pass coeficient to dampen standing delayline waves", "[-]",  mAlpha);

            setStartValue(mpP1, NodeHydraulic::FLOW, 0.0);
            setStartValue(mpP1, NodeHydraulic::PRESSURE, 1.0e5);
            setStartValue(mpP2, NodeHydraulic::FLOW, 0.0);
            setStartValue(mpP2, NodeHydraulic::PRESSURE, 1.0e5);
            setStartValue(mpP3, NodeHydraulic::FLOW, 0.0);
            setStartValue(mpP3, NodeHydraulic::PRESSURE, 1.0e5);
        }


        void initialize()
        {
            mpND_q1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::FLOW);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::CHARIMP);
            mpND_q2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::FLOW);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::CHARIMP);
            mpND_q3 = getSafeNodeDataPtr(mpP3, NodeHydraulic::FLOW);
            mpND_c3 = getSafeNodeDataPtr(mpP3, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc3 = getSafeNodeDataPtr(mpP3, NodeHydraulic::CHARIMP);
            mpND_q4 = getSafeNodeDataPtr(mpP4, NodeHydraulic::FLOW);
            mpND_c4 = getSafeNodeDataPtr(mpP4, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc4 = getSafeNodeDataPtr(mpP4, NodeHydraulic::CHARIMP);

            mZc = 3 / 2 * mBulkmodulus/mVolume*mTimestep/(1-mAlpha); //Need to be updated at simulation start since it is volume and bulk that are set.

            //Write to nodes
            mpP1->writeNode(NodeHydraulic::FLOW,         getStartValue(mpP1,NodeHydraulic::FLOW));
            mpP1->writeNode(NodeHydraulic::PRESSURE,     getStartValue(mpP1,NodeHydraulic::PRESSURE));
            mpP1->writeNode(NodeHydraulic::WAVEVARIABLE, getStartValue(mpP1,NodeHydraulic::PRESSURE)+mZc*getStartValue(mpP1,NodeHydraulic::FLOW));
            mpP1->writeNode(NodeHydraulic::CHARIMP,      mZc);
            mpP2->writeNode(NodeHydraulic::FLOW,         getStartValue(mpP2,NodeHydraulic::FLOW));
            mpP2->writeNode(NodeHydraulic::PRESSURE,     getStartValue(mpP2,NodeHydraulic::PRESSURE));
            mpP2->writeNode(NodeHydraulic::WAVEVARIABLE, getStartValue(mpP2,NodeHydraulic::PRESSURE)+mZc*getStartValue(mpP2,NodeHydraulic::FLOW));
            mpP2->writeNode(NodeHydraulic::CHARIMP,      mZc);
            mpP3->writeNode(NodeHydraulic::FLOW,         getStartValue(mpP3,NodeHydraulic::FLOW));
            mpP3->writeNode(NodeHydraulic::PRESSURE,     getStartValue(mpP3,NodeHydraulic::PRESSURE));
            mpP3->writeNode(NodeHydraulic::WAVEVARIABLE, getStartValue(mpP3,NodeHydraulic::PRESSURE)+mZc*getStartValue(mpP3,NodeHydraulic::FLOW));
            mpP3->writeNode(NodeHydraulic::CHARIMP,      mZc);
            mpP4->writeNode(NodeHydraulic::FLOW,         getStartValue(mpP4,NodeHydraulic::FLOW));
            mpP4->writeNode(NodeHydraulic::PRESSURE,     getStartValue(mpP4,NodeHydraulic::PRESSURE));
            mpP4->writeNode(NodeHydraulic::WAVEVARIABLE, getStartValue(mpP4,NodeHydraulic::PRESSURE)+mZc*getStartValue(mpP4,NodeHydraulic::FLOW));
            mpP4->writeNode(NodeHydraulic::CHARIMP,      mZc);
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
