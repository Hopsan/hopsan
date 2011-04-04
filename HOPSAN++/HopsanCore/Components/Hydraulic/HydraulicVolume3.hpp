//!
//! @file   HydraulicVolume.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-07-01
//!
//! @brief Contains a Hydraulic Volume Component With Three Ports (temporary solution)
//!
//$Id$

#ifndef HYDRAULICVOLUME3_HPP_INCLUDED
#define HYDRAULICVOLUME3_HPP_INCLUDED

#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief A hydraulic volume component
    //! @ingroup HydraulicComponents
    //!
    class HydraulicVolume3 : public ComponentC
    {

    private:
        double mZc;
        double mAlpha;
        double mVolume;
        double mBulkmodulus;

        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2, *mpND_p3, *mpND_q3, *mpND_c3, *mpND_Zc3;

        Port *mpP1, *mpP2, *mpP3;

    public:
        static Component *Creator()
        {
            return new HydraulicVolume3("Volume3");
        }

        HydraulicVolume3(const std::string name) : ComponentC(name)
        {
            //Set member attributes
            mBulkmodulus   = 1.0e9;
            mVolume        = 1.0e-3;
            mAlpha         = 0.1;

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
            mpP3 = addPowerPort("P3", "NodeHydraulic");

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
            mpND_p1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE);
            mpND_q1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::FLOW);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::CHARIMP);

            mpND_p2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::PRESSURE);
            mpND_q2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::FLOW);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::CHARIMP);

            mpND_p3 = getSafeNodeDataPtr(mpP3, NodeHydraulic::PRESSURE);
            mpND_q3 = getSafeNodeDataPtr(mpP3, NodeHydraulic::FLOW);
            mpND_c3 = getSafeNodeDataPtr(mpP3, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc3 = getSafeNodeDataPtr(mpP3, NodeHydraulic::CHARIMP);

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
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double q1, c1, Zc1, q2, c2, Zc2,q3, c3, Zc3, pMean, c10, c20, c30;

            //Read variables from nodes
            q1 = (*mpND_q1);
            c1 = (*mpND_c1);
            Zc1 = (*mpND_Zc2);
            q2 = (*mpND_q2);
            c2 = (*mpND_c2);
            Zc2 = (*mpND_Zc2);
            q3 = (*mpND_q3);
            c3 = (*mpND_c3);
            Zc3 = (*mpND_Zc3);

            //Volume equations

            pMean = ((c1 + Zc1 * 2 * q1) + (c2 + Zc2 * 2 * q2) + (c3 + Zc3 * 2 * q3)) / 3;

            c10 = pMean * 2 - c1 - 2 * Zc1 * q1;
            c1 = mAlpha * c1 + (1.0 - mAlpha)*c10 + (Zc1 - mZc)*q1;

            c20 = pMean * 2 - c2 - 2 * Zc2 * q2;
            c2 = mAlpha * c2 + (1.0 - mAlpha)*c20 + (Zc2 - mZc)*q2;

            c30 = pMean * 2 - c3 - 2 * Zc3 * q3;
            c3 = mAlpha * c3 + (1.0 - mAlpha)*c30 + (Zc3 - mZc)*q3;

            //Write new values to nodes
            (*mpND_c1) = c1;
            (*mpND_c2) = c2;
            (*mpND_c3) = c3;
            (*mpND_Zc1) = mZc;
            (*mpND_Zc2) = mZc;
            (*mpND_Zc3) = mZc;
        }

        void finalize()
        {

        }
    };
}

#endif // HYDRAULICVOLUME3_HPP_INCLUDED
