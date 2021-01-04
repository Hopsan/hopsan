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
//! @file   Hydraulic33ShuttleValve.hpp
//! @author Bjorn Eriksson <bjorn.eriksson@liu.se>
//! @date   2011-08-18
//!
//! @brief Contains a hydraulic 3/3 shuttle valve of Q-type

#ifndef HYDRAULIC33SHUTTLEVALVE_HPP_INCLUDED
#define HYDRAULIC33SHUTTLEVALVE_HPP_INCLUDED



#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief Hydraulic 3/3 shuttle valve (closed centre) of Q-type.
    //! @ingroup HydraulicComponents
    //!
    class Hydraulic33ShuttleValve : public ComponentQ
    {
    private:
        // Member variables
        IntegratorLimited xIntegrator;
        TurbulentFlowFunction qTurb_pa;
        TurbulentFlowFunction qTurb_at;
        double *mpCq, *mpD, *mpRho, *mpF_pa, *mpF_at, *mpX_pa, *mpX_at;

        // Port and node data pointers
        Port *mpPP, *mpPT, *mpPA, *mpPB, *mpX;
        double *mpPA_p, *mpPA_q, *mpPA_c, *mpPA_Zc, *mpPP_p, *mpPP_q, *mpPP_c, *mpPP_Zc, *mpPT_p, *mpPT_q, *mpPT_c, *mpPT_Zc;
        double *mpXvout;

        // Constants
        double xvmax, d1, d2, K1, K2, A;

    public:
        static Component *Creator()
        {
            return new Hydraulic33ShuttleValve();
        }

        void configure()
        {
            mpPP = addPowerPort("PP", "NodeHydraulic");
            mpPT = addPowerPort("PT", "NodeHydraulic");
            mpPA = addPowerPort("PA", "NodeHydraulic");

            addOutputVariable("xv_out", "Spool position", "", 0.0, &mpXvout);

            addInputVariable("C_q", "Flow Coefficient", "-", 0.67, &mpCq);
            addInputVariable("rho", "Oil density", "kg/m^3", 870, &mpRho);
            addInputVariable("d", "Spool Diameter", "m", 0.01, &mpD);
            addInputVariable("f_pa", "Fraction of spool circumference that is opening P-A", "-", 1.0, &mpF_pa);
            addInputVariable("f_at", "Fraction of spool circumference that is opening A-T", "-", 1.0, &mpF_at);
            addInputVariable("x_pa", "Spool Overlap From Port P To A", "m", -1e-6, &mpX_pa);
            addInputVariable("x_at", "Spool Overlap From Port A To T", "m", -1e-6, &mpX_at);

            addConstant("x_vmax", "Maximum Spool Displacement", "m", 0.01, xvmax);
            addConstant("d_1", "Damp orifice 1 diam.", "mm", 1.0e-3, d1);
            addConstant("d_2", "Damp orifice 2 diam.", "mm", 1.0e-3, d2);
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

            double Cq = (*mpCq);
            double rho = (*mpRho);
            double d = (*mpD);

            double initXv = limit(*mpXvout, -xvmax, xvmax);
            xIntegrator.initialize(mTimestep, initXv, initXv, -xvmax, xvmax);

            K1 = Cq*pi*d1*d1/4.0*sqrt(2.0/rho);
            K2 = Cq*pi*d2*d2/4.0*sqrt(2.0/rho);

            A = pi*d*d/4.0;
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double Cq, rho, d, f_pa, f_at, x_pa, x_at;
            double xv, xpanom, xatnom, Kcpa, Kcat, qpa, qat;
            double pp, pt, pa, qa, ca, Zca, qp, cp, Zcp, qt, ct, Zct;
            double p, v;
            bool cav = false;

            //Get variable values from nodes
            cp = (*mpPP_c);
            Zcp = (*mpPP_Zc);
            ct = (*mpPT_c);
            Zct = (*mpPT_Zc);
            ca = (*mpPA_c);
            Zca = (*mpPA_Zc);

            Cq = (*mpCq);
            rho = (*mpRho);
            d = (*mpD);
            f_pa = (*mpF_pa);
            f_at = (*mpF_at);
            x_pa = (*mpX_pa);
            x_at = (*mpX_at);

            p = (K1*K1*(*mpPP_p) + K2*K2*(*mpPT_p))/(K1*K1+K2*K2);
            v = sign((*mpPP_p)-p)*K1*sqrt(fabs((*mpPP_p)-p))/A;
            xv = xIntegrator.update(v);

            //xv=-.01+mTime/10.0*0.02; //Test to see q(xv)

            xpanom = std::max(-xv-x_pa,0.0);
            xatnom = std::max(xv-x_at,0.0);

            Kcpa = Cq*f_pa*pi*d*xpanom*sqrt(2.0/rho);
            Kcat = Cq*f_at*pi*d*xatnom*sqrt(2.0/rho);

            //With TurbulentFlowFunction:
            qTurb_pa.setFlowCoefficient(Kcpa);
            qTurb_at.setFlowCoefficient(Kcat);

            qpa = qTurb_pa.getFlow(cp, ca, Zcp, Zca);
            qat = qTurb_at.getFlow(ca, ct, Zca, Zct);

            qp = -qpa;
            qa = qpa - qat;
            qt = qat;

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

                qp = -qpa;
                qa = qpa - qat;
                qt = qat;

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
            (*mpXvout) = xv;
        }
    };
}

#endif // HYDRAULIC33SHUTTLEVALVE_HPP_INCLUDED
