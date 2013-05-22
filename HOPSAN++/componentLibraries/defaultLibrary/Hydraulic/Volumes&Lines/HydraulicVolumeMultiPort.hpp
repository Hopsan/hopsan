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
#include "ComponentSystem.h"
#include <vector>
#include <sstream>
#include <iostream>
#include <cmath>

namespace hopsan {

    //!
    //! @brief A hydraulic volume component
    //! @ingroup HydraulicComponents
    //!
    class HydraulicVolumeMultiPort : public ComponentC
    {

    private:
        double *mpAlpha, *mpBetae, *mpV;

        std::vector<double*> mvpN_p, mvpN_q, mvpN_c, mvpN_Zc;
        std::vector<double> mv_c_new;
        std::vector<double> mvp_C0;
        size_t mNumPorts;
        Port *mpP1;

    public:
        static Component *Creator()
        {
            return new HydraulicVolumeMultiPort();
        }

        void configure()
        {
            mpP1 = addPowerMultiPort("P1", "NodeHydraulic");

            addInputVariable("V", "Volume", "[m^3]", 1.0e-3, &mpV);
            addInputVariable("Beta_e", "Bulkmodulus", "[Pa]", 1.0e9, &mpBetae);
            addInputVariable("alpha", "Low pass coeficient to dampen standing delayline waves", "[-]", 0.1, &mpAlpha);

            setDefaultStartValue(mpP1, NodeHydraulic::Flow, 0.0);
            setDefaultStartValue(mpP1, NodeHydraulic::Pressure, 1.0e5);
        }


        void initialize()
        {
            double Zc, betae, alpha, V;
            betae = (*mpBetae);
            alpha = (*mpAlpha);
            V = (*mpV);

            mNumPorts = mpP1->getNumPorts();

            mvpN_p.resize(mNumPorts);
            mvpN_q.resize(mNumPorts);
            mvpN_c.resize(mNumPorts);
            mvpN_Zc.resize(mNumPorts);
            mvp_C0.resize(mNumPorts);

            mv_c_new.resize(mNumPorts);

            Zc = mNumPorts*betae/(2.0*V)*mTimestep/(1.0-alpha);

            double pTot=0.0;
            for (size_t i=0; i<mNumPorts; ++i)
            {
                mvpN_p[i]  = getSafeMultiPortNodeDataPtr(mpP1, i, NodeHydraulic::Pressure, 0.0);
                mvpN_q[i]  = getSafeMultiPortNodeDataPtr(mpP1, i, NodeHydraulic::Flow, 0.0);
                mvpN_c[i]  = getSafeMultiPortNodeDataPtr(mpP1, i, NodeHydraulic::WaveVariable, 0.0);
                mvpN_Zc[i] = getSafeMultiPortNodeDataPtr(mpP1, i, NodeHydraulic::CharImpedance, 0.0);

                *mvpN_p[i]  = getStartValue(mpP1, NodeHydraulic::Pressure, i);
                *mvpN_q[i]  = getStartValue(mpP1, NodeHydraulic::Flow, i);
                pTot       += getStartValue(mpP1,NodeHydraulic::Pressure, i)+Zc*getStartValue(mpP1,NodeHydraulic::Flow, i);
                *mvpN_Zc[i] = Zc;
            }
            pTot = pTot/double(mNumPorts);
            for (size_t i=0; i<mNumPorts; ++i)
            {
                *mvpN_c[i] = pTot*2.0-(*mvpN_p[i]) - Zc*(*mvpN_q[i]);
            }
        }


        void simulateOneTimestep()
        {
            double cTot = 0.0;
            double pAvg, Zc, V, betae, alpha;
            betae = (*mpBetae);
            alpha = (*mpAlpha);
            V = (*mpV);

            Zc = mNumPorts*betae/(2.0*V)*mTimestep/(1.0-alpha);

            //Equations
            for (size_t i=0; i<mNumPorts; ++i)
            {
                cTot += (*mvpN_c[i]) + 2.0*Zc*(*mvpN_q[i]);
            }
            pAvg = cTot/double(mNumPorts);

            for (size_t i=0; i<mNumPorts; ++i)
            {
                mvp_C0[i] = pAvg*2.0-(*mvpN_c[i]) - 2.0*Zc*(*mvpN_q[i]);
                mv_c_new[i] = alpha*(*mvpN_c[i]) + (1.0-alpha)*mvp_C0[i];
            }

            //Write new values
            for(size_t i=0; i<mNumPorts; ++i)
            {
                (*mvpN_Zc[i]) = Zc;
                (*mvpN_c[i]) = mv_c_new[i];
            }
        }


        void finalize()
        {
        }
    };
}

#endif // HYDRAULICVOLUMEMULTIPORT_HPP_INCLUDED
