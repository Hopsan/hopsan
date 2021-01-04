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
//! @file   HydraulicOpenCenterValve.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-01-03
//!
//! @brief Contains a hydraulic Open Center valve of Q-type
//$Id$

#ifndef HYDRAULICOPENCENTERVALVE_HPP_INCLUDED
#define HYDRAULICOPENCENTERVALVE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief Hydraulic Open Center valve of Q-type.
    //! @ingroup HydraulicComponents
    //!
    class HydraulicOpenCenterValve : public ComponentQ
    {
    private:
        // Member variables
        SecondOrderTransferFunction mSpoolPosTF;
        TurbulentFlowFunction qTurb_pa;
        TurbulentFlowFunction qTurb_pb;
        TurbulentFlowFunction qTurb_at;
        TurbulentFlowFunction qTurb_bt;
        TurbulentFlowFunction qTurb_cc;

        // Port and node data pointers
        Port *mpPP, *mpPC1, *mpPT, *mpPA, *mpPC2, *mpPB;
        double *mpPP_p, *mpPP_q, *mpPP_c, *mpPP_Zc, *mpPT_p, *mpPT_q, *mpPT_c, *mpPT_Zc, *mpPA_p, *mpPA_q, *mpPA_c, *mpPA_Zc, *mpPB_p, *mpPB_q, *mpPB_c, *mpPB_Zc, *mpPC1_p, *mpPC1_q, *mpPC1_c, *mpPC1_Zc, *mpPC2_p, *mpPC2_q, *mpPC2_c, *mpPC2_Zc;
        double *mpXvIn, *mpXv, *mpCq, *mpD, *mpF_pa, *mpF_pb, *mpF_at, *mpF_bt, *mpF_cc, *mpXvmax, *mpRho, *mpX_pa, *mpX_pb, *mpX_at, *mpX_bt, *mpX_cc;

        // Constants
        double mOmegah, mDeltah;

    public:
        static Component *Creator()
        {
            return new HydraulicOpenCenterValve();
        }

        void configure()
        {
            mpPP = addPowerPort("PP", "NodeHydraulic");
            mpPT = addPowerPort("PT", "NodeHydraulic");
            mpPA = addPowerPort("PA", "NodeHydraulic");
            mpPB = addPowerPort("PB", "NodeHydraulic");
            mpPC1 = addPowerPort("PC1", "NodeHydraulic");
            mpPC2 = addPowerPort("PC2", "NodeHydraulic");

            addOutputVariable("xv", "Spool position", "m", 0.0, &mpXv);
            addInputVariable("in", "Desired spool position", "m", 0.0, &mpXvIn);

            addInputVariable("C_q", "Flow Coefficient", "-", 0.67, &mpCq);
            addInputVariable("rho", "Oil density", "kg/m^3", 870, &mpRho);
            addInputVariable("d", "Spool Diameter", "m", 0.01, &mpD);
            addInputVariable("f_pa", "Fraction of spool circumference that is opening P-A", "-", 1.0, &mpF_pa);
            addInputVariable("f_pb", "Fraction of spool circumference that is opening B-T", "-", 1.0, &mpF_pb);
            addInputVariable("f_at", "Fraction of spool circumference that is opening P-A", "-", 1.0, &mpF_at);
            addInputVariable("f_bt", "Fraction of spool circumference that is opening B-T", "-", 1.0, &mpF_bt);
            addInputVariable("f_cc", "Fraction of spool circumference that is opening C-C", "-", 1.0, &mpF_cc);
            addInputVariable("x_pa", "Spool Overlap From Port P To A", "m", -1e-6, &mpX_pa);
            addInputVariable("x_pb", "Spool Overlap From Port A To T", "m", -1e-6, &mpX_pb);
            addInputVariable("x_at", "Spool Overlap From Port P To A", "m", -1e-6, &mpX_at);
            addInputVariable("x_bt", "Spool Overlap From Port A To T", "m", -1e-6, &mpX_bt);
            addInputVariable("x_cc", "Spool Overlap From Port C1 To C2", "m", -1e-6, &mpX_cc);
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

            mpPB_p = getSafeNodeDataPtr(mpPB, NodeHydraulic::Pressure);
            mpPB_q = getSafeNodeDataPtr(mpPB, NodeHydraulic::Flow);
            mpPB_c = getSafeNodeDataPtr(mpPB, NodeHydraulic::WaveVariable);
            mpPB_Zc = getSafeNodeDataPtr(mpPB, NodeHydraulic::CharImpedance);

            mpPC1_p = getSafeNodeDataPtr(mpPC1, NodeHydraulic::Pressure);
            mpPC1_q = getSafeNodeDataPtr(mpPC1, NodeHydraulic::Flow);
            mpPC1_c = getSafeNodeDataPtr(mpPC1, NodeHydraulic::WaveVariable);
            mpPC1_Zc = getSafeNodeDataPtr(mpPC1, NodeHydraulic::CharImpedance);

            mpPC2_p = getSafeNodeDataPtr(mpPC2, NodeHydraulic::Pressure);
            mpPC2_q = getSafeNodeDataPtr(mpPC2, NodeHydraulic::Flow);
            mpPC2_c = getSafeNodeDataPtr(mpPC2, NodeHydraulic::WaveVariable);
            mpPC2_Zc = getSafeNodeDataPtr(mpPC2, NodeHydraulic::CharImpedance);

            double num[3] = {1.0, 0.0, 0.0};
            double den[3] = {1.0, 2.0*mDeltah/mOmegah, 1.0/(mOmegah*mOmegah)};
            double initXv = limit((*mpXv), -(*mpXvmax), (*mpXvmax));
            mSpoolPosTF.initialize(mTimestep, num, den, initXv, initXv, -(*mpXvmax), (*mpXvmax));
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double xv, xpanom, xpbnom, xatnom, xbtnom, xccnom, Kcpa, Kcpb, Kcat, Kcbt, Kccc, qpa, qpb, qat, qbt, qcc;
            double pp, qp, cp, Zcp, pt, qt, ct, Zct, xvin, pa, qa, ca, Zca, pb, qb, cb, Zcb, pc1, qc1, cc1, Zcc1, pc2, qc2, cc2, Zcc2;
            double Cq, rho, xvmax, d, f_pa, f_pb, f_at, f_bt, f_cc, x_pa, x_pb, x_at, x_bt, x_cc;
            bool cav = false;

            //Get variable values from nodes
            cp = (*mpPP_c);
            Zcp = (*mpPP_Zc);
            ct  = (*mpPT_c);
            Zct = (*mpPT_Zc);
            ca  = (*mpPA_c);
            Zca = (*mpPA_Zc);
            cb  = (*mpPB_c);
            Zcb = (*mpPB_Zc);
            cc1  = (*mpPC1_c);
            Zcc1 = (*mpPC1_Zc);
            cc2  = (*mpPC2_c);
            Zcc2 = (*mpPC2_Zc);

            xvin  = (*mpXvIn);
            Cq = (*mpCq);
            rho = (*mpRho);
            xvmax = (*mpXvmax);
            d = (*mpD);
            f_pa = (*mpF_pa);
            f_pb = (*mpF_pb);
            f_at = (*mpF_at);
            f_bt = (*mpF_bt);
            f_cc = (*mpF_cc);
            x_pa = (*mpX_pa);
            x_pb = (*mpX_pb);
            x_at = (*mpX_at);
            x_bt = (*mpX_bt);
            x_cc = (*mpX_cc);

            limitValue(xvin, -xvmax, xvmax);
            mSpoolPosTF.update(xvin);
            xv = mSpoolPosTF.value();

            //Valve equations
            xpanom = std::max(xv-x_pa,0.0);                       //These orifices are closed in central position, and fully opened at -xvmax and xvmax
            xpbnom = std::max(-xv-x_pb,0.0);
            xatnom = std::max(-xv-x_at,0.0);
            xbtnom = std::max(xv-x_bt,0.0);
            xccnom = xvmax - std::max(std::min(xvmax,fabs(xv)-x_cc), 0.0);       //Center orifice is open in central position, and closed at -xvmax and xvmax

            Kcpa = Cq*f_pa*pi*d*xpanom*sqrt(2.0/rho);
            Kcpb = Cq*f_pb*pi*d*xpbnom*sqrt(2.0/rho);
            Kcat = Cq*f_at*pi*d*xatnom*sqrt(2.0/rho);
            Kcbt = Cq*f_bt*pi*d*xbtnom*sqrt(2.0/rho);
            Kccc = Cq*f_cc*pi*d*xccnom*sqrt(2.0/rho);

            //With TurbulentFlowFunction:
            qTurb_pa.setFlowCoefficient(Kcpa);
            qTurb_pb.setFlowCoefficient(Kcpb);
            qTurb_at.setFlowCoefficient(Kcat);
            qTurb_bt.setFlowCoefficient(Kcbt);
            qTurb_cc.setFlowCoefficient(Kccc);

            qpa = qTurb_pa.getFlow(cp, ca, Zcp, Zca);
            qpb = qTurb_pb.getFlow(cp, cb, Zcp, Zcb);
            qat = qTurb_at.getFlow(ca, ct, Zca, Zct);
            qbt = qTurb_bt.getFlow(cb, ct, Zcb, Zct);
            qcc = qTurb_cc.getFlow(cc1, cc2, Zcc1, Zcc2);

            qc1 = -qcc;
            qc2 = qcc;

            qp = -qpa-qpb;
            qa = qpa-qat;
            qb = -qbt+qpb;
            qt = qat+qbt;

            pp = cp + qp*Zcp;
            pt = ct + qt*Zct;
            pa = ca + qa*Zca;
            pb = cb + qb*Zcb;
            pc1 = cc1 + qc1*Zcc1;
            pc2 = cc2 + qc2*Zcc2;

            //Cavitation check
            if(pa < 0.0)
            {
                ca = 0.0;
                Zca = 0;
                cav = true;
            }
            if(pb < 0.0)
            {
                cb = 0.0;
                Zcb = 0;
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
            if(pc1 < 0.0)
            {
                cc1 = 0.0;
                Zcc1 = 0;
                cav = true;
            }
            if(pc2 < 0.0)
            {
                cc2 = 0.0;
                Zcc2 = 0;
                cav = true;
            }

            if(cav)
            {
                qpa = qTurb_pa.getFlow(cp, ca, Zcp, Zca);
                qpb = qTurb_pb.getFlow(cp, cb, Zcp, Zcb);
                qat = qTurb_at.getFlow(ca, ct, Zca, Zct);
                qbt = qTurb_bt.getFlow(cb, ct, Zcb, Zct);
                qcc = qTurb_cc.getFlow(cc1, cc2, Zcc1, Zcc2);

                qc1 = -qcc;
                qc2 = qcc;

                qp = -qpa-qpb;
                qa = qpa-qat;
                qb = -qbt+qpb;
                qt = qat+qbt;

                pp = cp + qp*Zcp;
                pt = ct + qt*Zct;
                pa = ca + qa*Zca;
                pb = cb + qb*Zcb;
                pc1 = cc1 + qc1*Zcc1;
                pc2 = cc2 + qc2*Zcc2;
            }

            //Write new values to nodes

            (*mpPP_p) = cp + qp*Zcp;
            (*mpPP_q) = qp;
            (*mpPT_p) = ct + qt*Zct;
            (*mpPT_q) = qt;
            (*mpPA_p) = ca + qa*Zca;
            (*mpPA_q) = qa;
            (*mpPB_p) = cb + qb*Zcb;
            (*mpPB_q) = qb;
            (*mpPC1_p) = cc1 + qc1*Zcc1;
            (*mpPC1_q) = qc1;
            (*mpPC2_p) = cc2 + qc2*Zcc2;
            (*mpPC2_q) = qc2;
            (*mpXv) = xv;
        }
    };
}

#endif // HYDRAULICOPENCENTERVALVE_HPP_INCLUDED

