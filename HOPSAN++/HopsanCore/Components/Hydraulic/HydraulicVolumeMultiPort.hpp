//!
//! @file   HydraulicVolumeMultiPort.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2011-04-02
//!
//! @brief Contains a Hydraulic Volume Component
//!
//$Id$

#ifndef HYDRAULICVOLUMEMULTIPORT_HPP_INCLUDED
#define HYDRAULICVOLUMEMULTIPORT_HPP_INCLUDED

#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief A hydraulic volume component
    //! @ingroup HydraulicComponents
    //!
    class HydraulicVolumeMultiPort : public ComponentC
    {

    private:
        double mZc;
        double mAlpha;
        double mVolume;
        double mBulkmodulus;

        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2;

        Port *mpP1;

    public:
        static Component *Creator()
        {
            return new HydraulicVolumeMultiPort("VolumeMultiPort");
        }

        HydraulicVolumeMultiPort(const std::string name) : ComponentC(name)
        {
            //Set member attributes
            mBulkmodulus   = 1.0e9;
            mVolume        = 1.0e-3;
            mAlpha         = 0.1;

            //Add ports to the component
            mpP1 = addPowerMultiPort("P1", "NodeHydraulic");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("V", "Volume", "[m^3]",            mVolume);
            registerParameter("Be", "Bulkmodulus", "[Pa]", mBulkmodulus);
            registerParameter("a", "Low pass coeficient to dampen standing delayline waves", "[-]",  mAlpha);

            setStartValue(mpP1, NodeHydraulic::FLOW, 0.0);
            setStartValue(mpP1, NodeHydraulic::PRESSURE, 1.0e5);
        }


        void initialize()
        {
            mpND_p1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE);
            mpND_q1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::FLOW);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::CHARIMP);

            mZc = mBulkmodulus/mVolume*mTimestep/(1.0-mAlpha); //Need to be updated at simulation start since it is volume and bulk that are set.

            //Write to nodes
            (*mpND_c1) = getStartValue(mpP1,NodeHydraulic::PRESSURE)+mZc*getStartValue(mpP1,NodeHydraulic::FLOW);
            (*mpND_Zc1) = mZc;
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double q1, c1, c10;

            //Get variable values from nodes
            q1 = (*mpND_q1);
            c1 = (*mpND_c1);

            //Write new values to nodes
            (*mpND_c1) = c1;
            (*mpND_Zc1) = mZc;
        }

        void finalize()
        {

        }
    };
}

#endif // HYDRAULICVOLUMEMULTIPORT_HPP_INCLUDED
