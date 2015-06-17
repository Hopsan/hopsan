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
//! @file   Hydraulic22Valve.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-12-20
//!
//! @brief Contains a hydraulic 2/2-valve of Q-type
//$Id$

#ifndef HYDRAULIC22VALVE_HPP_INCLUDED
#define HYDRAULIC22VALVE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief Hydraulic 2/2-valve of Q-type.
    //! @ingroup HydraulicComponents
    //!
    class Hydraulic22Valve : public ComponentQ
    {
    private:
        // Member variables
        SecondOrderTransferFunction filter;
        TurbulentFlowFunction qTurb_pa;

        // Port and node data pointers
        Port *mpPP, *mpPA;
        double *mpPP_p, *mpPP_q, *mpPP_c, *mpPP_Zc;
        double *mpPA_p, *mpPA_q, *mpPA_c, *mpPA_Zc;
        double *mpIn_xv, *mpOut_xv;
        double *mpCq, *mpD, *mpF, *mpXvmax, *mpRho;

        // Constants
        double mOmegah;
        double mDeltah;

    public:
        static Component *Creator()
        {
            return new Hydraulic22Valve();
        }

        void configure()
        {
            mpPP = addPowerPort("PP", "NodeHydraulic");
            mpPA = addPowerPort("PA", "NodeHydraulic");

            addInputVariable("in", "Desired spool position", "", 0.0, &mpIn_xv);
            addOutputVariable("xv", "Spool position", "", 0.0, &mpOut_xv);

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
            mpPP_p = getSafeNodeDataPtr(mpPP, NodeHydraulic::Pressure);
            mpPP_q = getSafeNodeDataPtr(mpPP, NodeHydraulic::Flow);
            mpPP_c = getSafeNodeDataPtr(mpPP, NodeHydraulic::WaveVariable);
            mpPP_Zc = getSafeNodeDataPtr(mpPP, NodeHydraulic::CharImpedance);

            mpPA_p = getSafeNodeDataPtr(mpPA, NodeHydraulic::Pressure);
            mpPA_q = getSafeNodeDataPtr(mpPA, NodeHydraulic::Flow);
            mpPA_c = getSafeNodeDataPtr(mpPA, NodeHydraulic::WaveVariable);
            mpPA_Zc = getSafeNodeDataPtr(mpPA, NodeHydraulic::CharImpedance);

            double xvmax = (*mpXvmax);

            //Initiate second order low pass filter
            double num[3] = {1.0, 0.0, 0.0};
            double den[3] = {1.0, 2.0*mDeltah/mOmegah, 1.0/(mOmegah*mOmegah)};
            const double initxv = limit(*mpOut_xv,0,xvmax);
            filter.initialize(mTimestep, num, den, initxv, initxv, 0, xvmax);
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double cp, Zcp, ca, Zca, xvin, pp, qp, pa, qa, Cq, rho, d, f, xvmax, xv;
            bool cav = false;

            //Read variables from nodes
            cp = (*mpPP_c);
            Zcp = (*mpPP_Zc);
            ca = (*mpPA_c);
            Zca = (*mpPA_Zc);
            xvin = (*mpIn_xv);

            Cq = (*mpCq);
            rho = (*mpRho);
            d = (*mpD);
            f = (*mpF);
            xvmax = (*mpXvmax);

            //Dynamics of spool position (second order low pass filter)
            limitValue(xvin, 0, xvmax);
            filter.update(xvin);
            xv = filter.value();

            //Determine flow coefficient
            double xpanom = xv;
            double Kcpa = Cq*f*pi*d*xpanom*sqrt(2.0/rho);

            //Calculate flow
            qTurb_pa.setFlowCoefficient(Kcpa);
            double qpa = qTurb_pa.getFlow(cp, ca, Zcp, Zca);

            qp = -qpa;
            qa = qpa;

            pp = cp + qp*Zcp;
            pa = ca + qa*Zca;

            //Cavitation check
            if(pa < 0.0)
            {
                ca = 0.0;
                Zca = 0;
                cav = true;
            }
            if(pp < 0.0)
            {
                cp = 0.0;
                Zcp = 0;
                cav = true;
            }

            if(cav)
            {
                qpa = qTurb_pa.getFlow(cp, ca, Zcp, Zca);

                if (xv >= 0.0)
                {
                    qp = -qpa;
                    qa = qpa;
                }
                else
                {
                    qp = 0;
                    qa = 0;
                }

                pp = cp + qp*Zcp;
                pa = ca + qa*Zca;
            }

            //Calculate pressures from flow and impedance
            (*mpPP_p) = pp;
            (*mpPP_q) = qp;
            (*mpPA_p) = pa;
            (*mpPA_q) = qa;
            (*mpOut_xv) = xv;
        }
    };
}

#endif // HYDRAULIC22VALVE_HPP_INCLUDED
