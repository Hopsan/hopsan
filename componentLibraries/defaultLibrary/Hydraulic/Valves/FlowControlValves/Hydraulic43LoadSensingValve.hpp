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
//! @file   Hydraulic43LoadSensingValve.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-12
//!
//! @brief Contains a hydraulic 4/3-valve with load sensing port of Q-type
//$Id$

#ifndef HYDRAULIC43LOADSENSINGVALVE_HPP_INCLUDED
#define HYDRAULIC43LOADSENSINGVALVE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief Hydraulic 4/3-valve (closed centre) with load sensing port of Q-type.
    //! @ingroup HydraulicComponents
    //!
    class Hydraulic43LoadSensingValve : public ComponentQ
    {
    private:
        // Member variables
        SecondOrderTransferFunction mSpoolPosTF;
        TurbulentFlowFunction mQTurb_pa;
        TurbulentFlowFunction mQTurb_pb;
        TurbulentFlowFunction mQTurb_at;
        TurbulentFlowFunction mQTurb_bt;

        // Port and node data pointers
        Port *mpPP, *mpPT, *mpPA, *mpPB, *mpPL;
        double *mpPP_p, *mpPP_q, *mpPP_c, *mpPP_Zc,
               *mpPT_p, *mpPT_q, *mpPT_c, *mpPT_Zc,
               *mpPA_p, *mpPA_q, *mpPA_c, *mpPA_Zc,
               *mpPB_p, *mpPB_q, *mpPB_c, *mpPB_Zc,
               *mpPL_p, *mpPL_q, *mpPL_c, *mpPL_Zc;
        double *mpXvIn, *mpXv;
        double *mpCq, *mpD, *mpF_pa, *mpF_pb, *mpF_at, *mpF_bt, *mpXvmax, *mpRho, *mpX_pa, *mpX_pb, *mpX_at, *mpX_bt;

        // Constants
        double mOmegah, mDeltah;

    public:
        static Component *Creator()
        {
            return new Hydraulic43LoadSensingValve();
        }

        void configure()
        {
            mpPP = addPowerPort("PP", "NodeHydraulic");
            mpPT = addPowerPort("PT", "NodeHydraulic");
            mpPA = addPowerPort("PA", "NodeHydraulic");
            mpPB = addPowerPort("PB", "NodeHydraulic");
            mpPL = addPowerPort("PL", "NodeHydraulic");

            addOutputVariable("xv", "Spool position", "m", 0.0, &mpXv);

            addInputVariable("in", "Desired spool position", "m", 0.0, &mpXvIn);

            addInputVariable("C_q", "Flow Coefficient", "-", 0.67, &mpCq);
            addInputVariable("rho", "Oil density", "kg/m^3", 870, &mpRho);
            addInputVariable("d", "Spool Diameter", "m", 0.01, &mpD);
            addInputVariable("f_pa", "Fraction of spool circumference that is opening P-A", "-", 1.0, &mpF_pa);
            addInputVariable("f_pb", "Fraction of spool circumference that is opening P-B", "-", 1.0, &mpF_pb);
            addInputVariable("f_at", "Fraction of spool circumference that is opening A-T", "-", 1.0, &mpF_at);
            addInputVariable("f_bt", "Fraction of spool circumference that is opening B-T", "-", 1.0, &mpF_bt);
            addInputVariable("x_pa", "Spool Overlap From Port P To A", "m", -1e-6, &mpX_pa);
            addInputVariable("x_pb", "Spool Overlap From Port P To B", "m", -1e-6, &mpX_pb);
            addInputVariable("x_at", "Spool Overlap From Port A To T", "m", -1e-6, &mpX_at);
            addInputVariable("x_bt", "Spool Overlap From Port A To T", "m", -1e-6, &mpX_bt);
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

            mpPL_p = getSafeNodeDataPtr(mpPL, NodeHydraulic::Pressure);
            mpPL_q = getSafeNodeDataPtr(mpPL, NodeHydraulic::Flow);
            mpPL_c = getSafeNodeDataPtr(mpPL, NodeHydraulic::WaveVariable);
            mpPL_Zc = getSafeNodeDataPtr(mpPL, NodeHydraulic::CharImpedance);

            double num[3] = {1.0, 0.0, 0.0};
            double den[3] = {1.0, 2.0*mDeltah/mOmegah, 1.0/(mOmegah*mOmegah)};
            double initXv = limit(*mpXv,-(*mpXvmax),(*mpXvmax));
            mSpoolPosTF.initialize(mTimestep, num, den, initXv, initXv, -(*mpXvmax), (*mpXvmax));
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double xv, xpanom, xpbnom, xatnom, xbtnom, Kcpa, Kcpb, Kcat, Kcbt, qpa, qpb, qat, qbt;
            double Cq, rho, xvmax, d, f_pa, f_pb, f_at, f_bt, x_pa, x_pb, x_at, x_bt;
            double pp, qp, cp, Zcp, pt, qt, ct, Zct, xvin, pa, qa, ca, Zca, pb, qb, cb, Zcb, pload, cload, Zcload;
            bool cav = false;

            //Get variable values from nodes
            cp = (*mpPP_c);
            Zcp = (*mpPP_Zc);
            ct = (*mpPT_c);
            Zct = (*mpPT_Zc);
            ca = (*mpPA_c);
            Zca = (*mpPA_Zc);
            cb = (*mpPB_c);
            Zcb = (*mpPB_Zc);
            xvin = (*mpXvIn);
            cload = (*mpPL_c);
            Zcload = (*mpPL_Zc);

            Cq = (*mpCq);
            rho = (*mpRho);
            xvmax = (*mpXvmax);
            d = (*mpD);
            f_pa = (*mpF_pa);
            f_pb = (*mpF_pb);
            f_at = (*mpF_at);
            f_bt = (*mpF_bt);
            x_pa = (*mpX_pa);
            x_pb = (*mpX_pb);
            x_at = (*mpX_at);
            x_bt = (*mpX_bt);

            limitValue(xvin, -xvmax, xvmax);
            mSpoolPosTF.update(xvin);
            xv = mSpoolPosTF.value();

            xpanom = std::max(xv-x_pa,0.0);
            xpbnom = std::max(-xv-x_pb,0.0);
            xatnom = std::max(-xv-x_at,0.0);
            xbtnom = std::max(xv-x_bt,0.0);

            Kcpa = Cq*f_pa*pi*d*xpanom*sqrt(2.0/rho);
            Kcpb = Cq*f_pb*pi*d*xpbnom*sqrt(2.0/rho);
            Kcat = Cq*f_at*pi*d*xatnom*sqrt(2.0/rho);
            Kcbt = Cq*f_bt*pi*d*xbtnom*sqrt(2.0/rho);

            //With TurbulentFlowFunction:
            mQTurb_pa.setFlowCoefficient(Kcpa);
            mQTurb_pb.setFlowCoefficient(Kcpb);
            mQTurb_at.setFlowCoefficient(Kcat);
            mQTurb_bt.setFlowCoefficient(Kcbt);

            qpa = mQTurb_pa.getFlow(cp, ca, Zcp, Zca);
            qpb = mQTurb_pb.getFlow(cp, cb, Zcp, Zcb);
            qat = mQTurb_at.getFlow(ca, ct, Zca, Zct);
            qbt = mQTurb_bt.getFlow(cb, ct, Zcb, Zct);

            qp = -qpa-qpb;
            qa = qpa-qat;
            qb = -qbt+qpb;
            qt = qat+qbt;

            pp = cp + qp*Zcp;
            pt = ct + qt*Zct;
            pa = ca + qa*Zca;
            pb = cb + qb*Zcb;

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

            if(cav)
            {
                qpa = mQTurb_pa.getFlow(cp, ca, Zcp, Zca);
                qpb = mQTurb_pb.getFlow(cp, cb, Zcp, Zcb);
                qat = mQTurb_at.getFlow(ca, ct, Zca, Zct);
                qbt = mQTurb_bt.getFlow(cb, ct, Zcb, Zct);

                qp = -qpa-qpb;
                qa = qpa-qat;
                qb = -qbt+qpb;
                qt = qat+qbt;

                pp = cp + qp*Zcp;
                pt = ct + qt*Zct;
                pa = ca + qa*Zca;
                pb = cb + qb*Zcb;
            }


            if(xv >= 0.0)
            {
                pload = pa;
            }
            else
            {
                pload = pb;
            }

            //Write new values to nodes

            (*mpPP_p) = pp;
            (*mpPP_q) = qp;
            (*mpPT_p) = pt;
            (*mpPT_q) = qt;
            (*mpPA_p) = pa;
            (*mpPA_q) = qa;
            (*mpPB_p) = pb;
            (*mpPB_q) = qb;
            (*mpPL_p) = pload;
            if(Zcload != 0)
            {
                (*mpPL_q) = (pload - cload)/Zcload;
            }
            else
            {
                (*mpPL_q) = 0;  //Fixes singular point in flow due to Zcload=0
            }
            (*mpXv) = xv;
        }
    };
}

#endif // HYDRAULIC43LOADSENSINGVALVE_HPP_INCLUDED

