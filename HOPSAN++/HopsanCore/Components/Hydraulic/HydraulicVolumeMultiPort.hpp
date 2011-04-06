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
#include <sstream>
#include <iostream>

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

        std::vector<double*> mvpN_p, mvpN_q, mvpN_c, mvpN_Zc;
        std::vector<double> mvp_C0;
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
            mNumPorts = mpP1->getNumPorts();

            mvpN_p.resize(mNumPorts);
            mvpN_q.resize(mNumPorts);
            mvpN_c.resize(mNumPorts);
            mvpN_Zc.resize(mNumPorts);
            mvp_C0.resize(mNumPorts);

            mZc = mNumPorts*mBulkmodulus/(2.0*mVolume)*mTimestep/(1.0-mAlpha);

            for (size_t i=0; i<mNumPorts; ++i)
            {
                mvpN_p[i]  = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE, 0.0, i);
                mvpN_q[i]  = getSafeNodeDataPtr(mpP1, NodeHydraulic::FLOW, 0.0, i);
                mvpN_c[i]  = getSafeNodeDataPtr(mpP1, NodeHydraulic::WAVEVARIABLE, 0.0, i);
                mvpN_Zc[i] = getSafeNodeDataPtr(mpP1, NodeHydraulic::CHARIMP, 0.0, i);

                *mvpN_p[i] = getStartValue(mpP1, NodeHydraulic::PRESSURE);
                *mvpN_q[i] = getStartValue(mpP1, NodeHydraulic::FLOW)/mNumPorts;
                *mvpN_c[i] = getStartValue(mpP1, NodeHydraulic::PRESSURE);
                *mvpN_Zc[i] = mZc;
//                std::stringstream ss;
//                ss << i << "::StartValues: Flow: " << *mvpN_q[i] << "  Pressure: " << *mvpN_p[i];
//                addInfoMessage(ss.str());
            }
        }


        void simulateOneTimestep()
        {
//            if(mTime<.002)
//            {
//                std::stringstream ss;
//                ss << "mTime: " << mTime << "  q:" << *mvpN_q[0];
//                addInfoMessage(ss.str());
//            }
            double cTot = 0.0;
            double pAvg;

            for (size_t i=0; i<mNumPorts; ++i)
            {
                cTot += (*mvpN_c[i]) + 2.0*mZc*(*mvpN_q[i]);
            }
            pAvg = cTot/mNumPorts;

            for (size_t i=0; i<mNumPorts; ++i)
            {
                mvp_C0[i] = pAvg*2.0-(*mvpN_c[i]) - 2.0*mZc*(*mvpN_q[i]);
                (*mvpN_c[i]) = mAlpha*(*mvpN_c[i]) + (1.0-mAlpha)*mvp_C0[i];
                (*mvpN_Zc[i]) = mZc;
            }
        }


        void finalize()
        {
        }
    };
}

#endif // HYDRAULICVOLUMEMULTIPORT_HPP_INCLUDED
