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
//! @file   Hydraulic32DirectionalValve.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-12-06
//!
//! @brief Contains a hydraulic directional3/2-valve of Q-type
// $Id$

#ifndef HYDRAULIC32DIRECTIONALVALVE_HPP_INCLUDED
#define HYDRAULIC32DIRECTIONALVALVE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief Hydraulic directional 3/2-valve of Q-type.
    //! @ingroup HydraulicComponents
    //!
    class Hydraulic32DirectionalValve : public ComponentQ
    {
    private:
        // Member variables
        SecondOrderTransferFunction mSpoolPosTF;
        TurbulentFlowFunction qTurb_pa;
        TurbulentFlowFunction qTurb_at;

        // Port and node data pointers
        Port *mpPP, *mpPT, *mpPA, *mpPB;
        double *mpPA_p, *mpPA_q, *mpPA_c, *mpPA_Zc, *mpPP_p, *mpPP_q, *mpPP_c, *mpPP_Zc, *mpPT_p, *mpPT_q, *mpPT_c, *mpPT_Zc;
        double *mpCq, *mpD, *mpF, *mpXvmax, *mpRho;
        double *mpXvIn, *mpXv;

        // Constants
        double mOmegah, mDeltah;

    public:
        static Component *Creator()
        {
            return new Hydraulic32DirectionalValve();
        }

        void configure()
        {
            mpPP = addPowerPort("PP", "NodeHydraulic");
            mpPT = addPowerPort("PT", "NodeHydraulic");
            mpPA = addPowerPort("PA", "NodeHydraulic");

            addOutputVariable("xv", "Spool position", "m", 0.0, &mpXv);
            addInputVariable("in", "<0.5 (closed), >0.5 (open)", "", 0.0, &mpXvIn);

            addInputVariable("C_q", "Flow Coefficient", "-", 0.67, &mpCq);
            addInputVariable("rho", "Oil density", "kg/m^3", 870, &mpRho);
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

            mpPT_p = getSafeNodeDataPtr(mpPT, NodeHydraulic::Pressure);
            mpPT_q = getSafeNodeDataPtr(mpPT, NodeHydraulic::Flow);
            mpPT_c = getSafeNodeDataPtr(mpPT, NodeHydraulic::WaveVariable);
            mpPT_Zc = getSafeNodeDataPtr(mpPT, NodeHydraulic::CharImpedance);

            mpPA_p = getSafeNodeDataPtr(mpPA, NodeHydraulic::Pressure);
            mpPA_q = getSafeNodeDataPtr(mpPA, NodeHydraulic::Flow);
            mpPA_c = getSafeNodeDataPtr(mpPA, NodeHydraulic::WaveVariable);
            mpPA_Zc = getSafeNodeDataPtr(mpPA, NodeHydraulic::CharImpedance);

            double num[3];// = {1.0, 0.0, 0.0};
            double den[3];// = {1.0, 2.0*deltah/omegah, 1.0/(omegah*omegah)};
            num[0] = 1.0;
            num[1] = 0.0;
            num[2] = 0.0;
            den[0] = 1.0;
            den[1] = 2.0*mDeltah/mOmegah;
            den[2] = 1.0/(mOmegah*mOmegah);

            double initialXv = limit(*mpXv, 0, (*mpXvmax));
            mSpoolPosTF.initialize(mTimestep, num, den, initialXv, initialXv, 0, (*mpXvmax));
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double xv, xpanom, xatnom, Kcpa, Kcat, qpa, qat;
            double pa, qa, ca, Zca, pp, qp, cp, Zcp, pt, qt, ct, Zct, xvin;
            double rho, xvmax, Cq, d, f;
            bool cav = false;

            //Get variable values from nodes
            cp = (*mpPP_c);
            Zcp = (*mpPP_Zc);
            ct = (*mpPT_c);
            Zct = (*mpPT_Zc);
            ca = (*mpPA_c);
            Zca = (*mpPA_Zc);
            xvin = (*mpXvIn);

            rho = (*mpRho);
            xvmax = (*mpXvmax);
            Cq = (*mpCq);
            d = (*mpD);
            f = (*mpF);

            if(doubleToBool(xvin))
            {
                mSpoolPosTF.update(xvmax);
            }
            else
            {
                mSpoolPosTF.update(0);
            }

            xv = mSpoolPosTF.value();

            xpanom = std::max(xv,0.0);
            xatnom = std::max(xvmax-xv,0.0);

            Kcpa = Cq*f*pi*d*xpanom*sqrt(2.0/rho);
            Kcat = Cq*f*pi*d*xatnom*sqrt(2.0/rho);

            //With TurbulentFlowFunction:
            qTurb_pa.setFlowCoefficient(Kcpa);
            qTurb_at.setFlowCoefficient(Kcat);

            qpa = qTurb_pa.getFlow(cp, ca, Zcp, Zca);
            qat = qTurb_at.getFlow(ca, ct, Zca, Zct);

            qp = -qpa;
            qa = qpa-qat;
            qt = qat;

            pp = cp + qp*Zcp;
            pa = ca + qa*Zca;
            pt = ct + qt*Zct;

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
                qa = qpa-qat;
                qt = qat;

                pp = cp + qp*Zcp;
                pa = ca + qa*Zca;
                pt = ct + qt*Zct;
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

#endif // HYDRAULIC32DIRECTIONALVALVE_HPP_INCLUDED

