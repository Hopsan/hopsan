/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   Hydraulic33Valve.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-12-06
//!
//! @brief Contains a hydraulic 3/3-valve of Q-type
//$Id$

#ifndef HYDRAULIC33VALVE_HPP_INCLUDED
#define HYDRAULIC33VALVE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief Hydraulic 3/3-valve (closed centre) of Q-type.
    //! @ingroup HydraulicComponents
    //!
    class Hydraulic33Valve : public ComponentQ
    {
    private:
        // Member variables
        SecondOrderTransferFunction mSpoolPosTF;
        TurbulentFlowFunction qTurb_pa;
        TurbulentFlowFunction qTurb_at;

        // Port and node data pointers
        Port *mpPP, *mpPT, *mpPA, *mpPB;
        double *mpPA_p, *mpPA_q, *mpPA_c, *mpPA_Zc, *mpPP_p, *mpPP_q, *mpPP_c, *mpPP_Zc, *mpPT_p, *mpPT_q, *mpPT_c, *mpPT_Zc;
        double *mpXvIn, *mpXv, *mpCq, *mpD, *mpF_pa, *mpF_at, *mpXvmax, *mpRho, *mpX_pa, *mpX_at;

        // Constants
        double mOmegah, mDeltah;

    public:
        static Component *Creator()
        {
            return new Hydraulic33Valve();
        }

        void configure()
        {
            mpPP = addPowerPort("PP", "NodeHydraulic");
            mpPT = addPowerPort("PT", "NodeHydraulic");
            mpPA = addPowerPort("PA", "NodeHydraulic");

            addOutputVariable("xv", "Spool position", "m", 0.0, &mpXv);
            addInputVariable("in", "Desired spool position", "m", 0.0, &mpXvIn);

            addInputVariable("C_q", "Flow Coefficient", "-", 0.67, &mpCq);
            addInputVariable("rho", "Oil density", "kg/m^3", 870, &mpRho);
            addInputVariable("d", "Spool Diameter", "m", 0.01, &mpD);
            addInputVariable("f_pa", "Fraction of spool circumference that is opening P-A", "-", 1.0, &mpF_pa);
            addInputVariable("f_at", "Fraction of spool circumference that is opening A-T", "-", 1.0, &mpF_at);
            addInputVariable("x_vmax", "Maximum Spool Displacement", "m", 0.01, &mpXvmax);
            addInputVariable("x_pa", "Spool Overlap From Port P To A", "m", -1e-6, &mpX_pa);
            addInputVariable("x_at", "Spool Overlap From Port A To T", "m", -1e-6, &mpX_at);

            addConstant("omega_h", "Resonance Frequency", "Frequency", 100.0, mOmegah);
            addConstant("delta_h", "Damping Factor", "-", 1.0, mDeltah);
        }


        void initialize()
        {
            mpPP_p = getSafeNodeDataPtr(mpPP, NodeHydraulic::Pressure);
            mpPP_q = getSafeNodeDataPtr(mpPP, NodeHydraulic::Flow);
            mpPP_c = getSafeNodeDataPtr(mpPP, NodeHydraulic::WaveVariable);
            mpPP_Zc = getSafeNodeDataPtr(mpPP, NodeHydraulic::CharImpedance);

            mpPT_p = getSafeNodeDataPtr(mpPT, NodeHydraulic::Pressure);
            mpPT_q = getSafeNodeDataPtr(mpPT, NodeHydraulic::Flow);
            mpPT_c = getSafeNodeDataPtr(mpPT, NodeHydraulic::WaveVariable);
            mpPT_Zc = getSafeNodeDataPtr(mpPT, NodeHydraulic::CharImpedance);

            mpPA_p = getSafeNodeDataPtr(mpPA, NodeHydraulic::Pressure);
            mpPA_q = getSafeNodeDataPtr(mpPA, NodeHydraulic::Flow);
            mpPA_c = getSafeNodeDataPtr(mpPA, NodeHydraulic::WaveVariable);
            mpPA_Zc = getSafeNodeDataPtr(mpPA, NodeHydraulic::CharImpedance);

            double num[3] = {1.0, 0.0, 0.0};
            double den[3] = {1.0, 2.0*mDeltah/mOmegah, 1.0/(mOmegah*mOmegah)};
            double initXv = limit(*mpXv,-(*mpXvmax),(*mpXvmax));
            mSpoolPosTF.initialize(mTimestep, num, den, initXv, initXv, -(*mpXvmax), (*mpXvmax));
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double xv, xpanom, xatnom, Kcpa, Kcat, qpa, qat;
            double pp, pt, pa, qa, ca, Zca, qp, cp, Zcp, qt, ct, Zct, xvin, Cq, rho, xvmax, d, f_pa, f_at, x_pa, x_at;
            bool cav = false;

            //Get variable values from nodes
            cp = (*mpPP_c);
            Zcp = (*mpPP_Zc);
            ct = (*mpPT_c);
            Zct = (*mpPT_Zc);
            ca = (*mpPA_c);
            Zca = (*mpPA_Zc);
            xvin = (*mpXvIn);

            Cq = (*mpCq);
            rho = (*mpRho);
            xvmax = (*mpXvmax);
            d = (*mpD);
            f_pa = (*mpF_pa);
            f_at = (*mpF_at);
            x_pa = (*mpX_pa);
            x_at = (*mpX_at);


            limitValue(xvin, -xvmax, xvmax);
            mSpoolPosTF.update(xvin);
            xv = mSpoolPosTF.value();

            xpanom = std::max(xv-x_pa,0.0);
            xatnom = std::max(-xv-x_at,0.0);

            Kcpa = Cq*f_pa*pi*d*xpanom*sqrt(2.0/rho);
            Kcat = Cq*f_at*pi*d*xatnom*sqrt(2.0/rho);

            //With TurbulentFlowFunction:
            qTurb_pa.setFlowCoefficient(Kcpa);
            qTurb_at.setFlowCoefficient(Kcat);

            qpa = qTurb_pa.getFlow(cp, ca, Zcp, Zca);
            qat = qTurb_at.getFlow(ca, ct, Zca, Zct);

            qp = -qpa;
            qt = qat;
            qa = qpa-qat;

            pp = cp + qp*Zcp;
            pt = ct + qt*Zct;
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
            if(pt < 0.0)
            {
                ct = 0.0;
                Zct = 0;
                cav = true;
            }

            if(cav)
            {
                qpa = qTurb_pa.getFlow(cp, ca, Zcp, Zca);
                qat = qTurb_at.getFlow(ca, ct, Zca, Zct);

                if (xv >= 0.0)
                {
                    qp = -qpa;
                    qa = qpa;
                    qt = 0;
                }
                else
                {
                    qp = 0;
                    qa = -qat;
                    qt = qat;
                }

                pp = cp + qp*Zcp;
                pt = ct + qt*Zct;
                pa = ca + qa*Zca;
            }

            //Write new values to nodes
            (*mpPP_p) = pp;
            (*mpPP_q) = qp;
            (*mpPA_p) = pa;
            (*mpPA_q) = qa;
            (*mpPT_p) = pt;
            (*mpPT_q) = qt;
            (*mpXv) = xv;
        }
    };
}

#endif // HYDRAULIC33VALVE_HPP_INCLUDED
