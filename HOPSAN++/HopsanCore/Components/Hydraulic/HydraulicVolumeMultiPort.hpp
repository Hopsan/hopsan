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
#include <vector>

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

        std::vector<double*> vpN_p, vpN_q, vpN_c, vpN_Zc, vp_C0;
        size_t mNumPorts;
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
            mZc = mBulkmodulus/mVolume*mTimestep/(1.0-mAlpha); //Need to be updated at simulation start since it is volume and bulk that are set.

            mNumPorts = mpP1->getNumPorts();
            vpN_p.resize(mNumPorts);
            vpN_q.resize(mNumPorts);
            vpN_c.resize(mNumPorts);
            vpN_Zc.resize(mNumPorts);
            vp_C0.resize(mNumPorts);
            for (size_t i=0; i<mNumPorts; ++i)
            {
                vpN_p[i]  = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE, 0.0, i);
                vpN_q[i]  = getSafeNodeDataPtr(mpP1, NodeHydraulic::FLOW, 0.0, i);
                vpN_c[i]  = getSafeNodeDataPtr(mpP1, NodeHydraulic::WAVEVARIABLE, 0.0, i);
                vpN_Zc[i] = getSafeNodeDataPtr(mpP1, NodeHydraulic::CHARIMP, 0.0, i);
                (*vpN_Zc[i]) = mZc;
            }
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            for (size_t i=0; i<mNumPorts; ++i)
            {
                (*vp_C0[i]) = (*vpN_p[i]) + mZc*(*vpN_q[i]);
                (*vpN_c[i]) = mAlpha*(*vpN_c[i]) + (1.0-mAlpha)*(*vp_C0[i]);
            }
        }


        void finalize()
        {

        }
    };
}

#endif // HYDRAULICVOLUMEMULTIPORT_HPP_INCLUDED
