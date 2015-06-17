/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

//!
//! @file   Hydraulic22DirectionalValve.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-02-24
//!
//! @brief Contains a hydraulic on/off valve of Q-type
//$Id$

#ifndef HYDRAULIC22DIRECTIONALVALVE_HPP_INCLUDED
#define HYDRAULIC22DIRECTIONALVALVE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief Hydraulic on/off 2/2-valve of Q-type.
    //! @todo the typename and class name is incorrect, but we cant just rename it, need an auto update function for that
    //! @ingroup HydraulicComponents
    //!
    class Hydraulic22DirectionalValve : public ComponentQ
    {
    private:
        // Member variables
        SecondOrderTransferFunction mValveSpoolPosFilter;
        TurbulentFlowFunction qTurb;

        // Port and node data pointers
        Port *mpP1, *mpP2, *mpIn, *mpOut;
        double *mpP1_p, *mpP1_q, *mpP1_c, *mpP1_Zc, *mpP2_p, *mpP2_q, *mpP2_c, *mpP2_Zc;
        double *mpIn_xv, *mpOut_xv;
        double *mpCq, *mpD, *mpF, *mpXvmax, *mpRho;

        // Constants
        double mOmegah;
        double mDeltah;

    public:
        static Component *Creator()
        {
            return new Hydraulic22DirectionalValve();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");

            mpIn = addInputVariable("in", "<0.5 (closed), >0.5 (open)", "", 0.0);
            mpOut = addOutputVariable("xv", "Spool position", "m", 0.0);

            addInputVariable("C_q", "Flow Coefficient", "-", 0.67, &mpCq);
            addInputVariable("rho", "Oil Density", "kg/m^3", 890, &mpRho);
            addInputVariable("d", "Spool Diameter", "m", 0.01, &mpD);
            addInputVariable("f", "Spool Fraction of the Diameter", "-", 1.0, &mpF);
            addInputVariable("x_vmax", "Maximum Spool Displacement", "m", 0.01, &mpXvmax);

            addConstant("omega_h", "Resonance Frequency", "Frequency", 100.0, mOmegah);
            addConstant("delta_h", "Damping Factor", "-", 1.0, mDeltah);
        }


        void initialize()
        {
            mpP1_p = getSafeNodeDataPtr(mpP1, NodeHydraulic::Pressure);
            mpP1_q = getSafeNodeDataPtr(mpP1, NodeHydraulic::Flow);
            mpP1_c = getSafeNodeDataPtr(mpP1, NodeHydraulic::WaveVariable);
            mpP1_Zc = getSafeNodeDataPtr(mpP1, NodeHydraulic::CharImpedance);

            mpP2_p = getSafeNodeDataPtr(mpP2, NodeHydraulic::Pressure);
            mpP2_q = getSafeNodeDataPtr(mpP2, NodeHydraulic::Flow);
            mpP2_c = getSafeNodeDataPtr(mpP2, NodeHydraulic::WaveVariable);
            mpP2_Zc = getSafeNodeDataPtr(mpP2, NodeHydraulic::CharImpedance);

            mpIn_xv = getSafeNodeDataPtr(mpIn, NodeSignal::Value);
            mpOut_xv = getSafeNodeDataPtr(mpOut, NodeSignal::Value);

            double num[3] = {1.0, 0.0, 0.0};
            double den[3] = {1.0, 2.0*mDeltah/mOmegah, 1.0/(mOmegah*mOmegah)};

            double initialXv = limit(*mpOut_xv, 0, (*mpXvmax));
            mValveSpoolPosFilter.initialize(mTimestep, num, den, initialXv, initialXv, 0, (*mpXvmax));
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            // Declare local variables
            double xv, xnom, Kc, q, Cq, rho, d, f, xvmax;
            double p1, q1, c1, Zc1, p2, q2, c2, Zc2, xvin;
            bool cav = false;

            // Get variable values from nodes
            c1 = (*mpP1_c);
            Zc1 = (*mpP1_Zc);
            c2 = (*mpP2_c);
            Zc2 = (*mpP2_Zc);
            xvin = (*mpIn_xv);

            Cq = (*mpCq);
            rho = (*mpRho);
            d = (*mpD);
            f = (*mpF);
            xvmax = (*mpXvmax);

            if(doubleToBool(xvin))
            {
                mValveSpoolPosFilter.update(xvmax);
            }
            else
            {
                mValveSpoolPosFilter.update(0);
            }

            xv = mValveSpoolPosFilter.value();

            xnom = std::max(xv,0.0);

            Kc = Cq*f*pi*d*xnom*sqrt(2.0/rho);

            // With TurbulentFlowFunction:
            qTurb.setFlowCoefficient(Kc);

            q = qTurb.getFlow(c1, c2, Zc1, Zc2);

            q1 = -q;
            q2 = q;

            p1 = c1 + q1*Zc1;
            p2 = c2 + q2*Zc2;

            // Cavitation check
            if(p1 < 0.0)
            {
                c1 = 0.0;
                Zc1 = 0;
                cav = true;
            }
            if(p2 < 0.0)
            {
                c2 = 0.0;
                Zc2 = 0;
                cav = true;
            }

            if(cav)
            {
                q = qTurb.getFlow(c1, c2, Zc1, Zc2);

                q1 = -q;
                q2 = q;

                p1 = c1 + q1*Zc1;
                p2 = c2 + q2*Zc2;
            }

            // Write new values to nodes

            (*mpP1_p) = p1;
            (*mpP1_q) = q1;
            (*mpP2_p) = p2;
            (*mpP2_q) = q2;
            (*mpOut_xv) = xnom;
        }
    };
}

#endif // HYDRAULIC22DIRECTIONALVALVE_HPP_INCLUDED

