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
//! @file   HydraulicVolumeMultiPort.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2011-04-02
//!
//! @brief Contains a Hydraulic Volume Component
//!
//$Id$

#ifndef HYDRAULICVOLUMEMULTIPORT_HPP_INCLUDED
#define HYDRAULICVOLUMEMULTIPORT_HPP_INCLUDED

#include "ComponentEssentials.h"
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
            return new HydraulicVolumeMultiPort();
        }

        HydraulicVolumeMultiPort() : ComponentC()
        {
            //Set member attributes
            mBulkmodulus   = 1.0e9;
            mVolume        = 1.0e-3;
            mAlpha         = 0.1;

            //Add ports to the component
            mpP1 = addPowerMultiPort("P1", "NodeHydraulic");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("V", "Volume", "[m^3]",            mVolume);
            registerParameter("Beta_e", "Bulkmodulus", "[Pa]", mBulkmodulus);
            registerParameter("alpha", "Low pass coeficient to dampen standing delayline waves", "[-]",  mAlpha);

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

            double pTot=0.0;
            for (size_t i=0; i<mNumPorts; ++i)
            {
                mvpN_p[i]  = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE, 0.0, i);
                mvpN_q[i]  = getSafeNodeDataPtr(mpP1, NodeHydraulic::FLOW, 0.0, i);
                mvpN_c[i]  = getSafeNodeDataPtr(mpP1, NodeHydraulic::WAVEVARIABLE, 0.0, i);
                mvpN_Zc[i] = getSafeNodeDataPtr(mpP1, NodeHydraulic::CHARIMP, 0.0, i);

                *mvpN_p[i]  = getStartValue(mpP1, NodeHydraulic::PRESSURE, i);
                *mvpN_q[i]  = getStartValue(mpP1, NodeHydraulic::FLOW, i);
                pTot       += getStartValue(mpP1,NodeHydraulic::PRESSURE, i)+mZc*getStartValue(mpP1,NodeHydraulic::FLOW, i);
                *mvpN_Zc[i] = mZc;
            }
            pTot = pTot/mNumPorts;
            for (size_t i=0; i<mNumPorts; ++i)
            {
                *mvpN_c[i] = pTot*2.0-(*mvpN_p[i]) - mZc*(*mvpN_q[i]);
            }
        }


        void simulateOneTimestep()
        {
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
