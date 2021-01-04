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
//! @file   Hydraulic42Valve.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-12-20
//!
//! @brief Contains a hydraulic 4/2-valve of Q-type
//$Id$

#ifndef HYDRAULIC42DIRECTIONALVALVE_HPP_INCLUDED
#define HYDRAULIC42DIRECTIONALVALVE_HPP_INCLUDED


#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief Hydraulic 4/2-valve of Q-type.
    //! @ingroup HydraulicComponents
    //!
    class Hydraulic42DirectionalValve : public ComponentQ
    {
    private:
        // Member variables
        SecondOrderTransferFunction mSpoolPosTF;
        TurbulentFlowFunction mQTurb_pa;
        TurbulentFlowFunction mQTurb_bt;
        TurbulentFlowFunction mQTurb_pb;
        TurbulentFlowFunction mQTurb_at;

        // Port and node data pointers
        Port *mpPP, *mpPT, *mpPA, *mpPB;
        double *mpPP_p, *mpPP_q, *mpPP_c, *mpPP_Zc, *mpPT_p, *mpPT_q, *mpPT_c, *mpPT_Zc, *mpPA_p, *mpPA_q, *mpPA_c, *mpPA_Zc, *mpPB_p, *mpPB_q, *mpPB_c, *mpPB_Zc;
        double *mpXvIn, *mpXv, *mpCq, *mpD, *mpF_pa, *mpF_bt, *mpF_pb, *mpF_at, *mpXvmax, *mpRho;

        // Constants
        double mOmegah, mDeltah;

    public:
        static Component *Creator()
        {
            return new Hydraulic42DirectionalValve();
        }

        void configure()
        {
            mpPP = addPowerPort("PP", "NodeHydraulic");
            mpPT = addPowerPort("PT", "NodeHydraulic");
            mpPA = addPowerPort("PA", "NodeHydraulic");
            mpPB = addPowerPort("PB", "NodeHydraulic");

            addOutputVariable("xv", "Spool position", "m", &mpXv);
            addInputVariable("in", "Desired spool position", "m", 0.0, &mpXvIn);

            addInputVariable("C_q", "Flow Coefficient", "-", 0.67, &mpCq);
            addInputVariable("rho", "Oil density", "kg/m^3", 870, &mpRho);
            addInputVariable("d", "Spool Diameter", "m", 0.01, &mpD);
            addInputVariable("f_pa", "Fraction of spool circumference that is opening P-A", "-", 1.0, &mpF_pa);
            addInputVariable("f_bt", "Fraction of spool circumference that is opening B-T", "-", 1.0, &mpF_bt);
            addInputVariable("f_pb", "Fraction of spool circumference that is opening P-B", "-", 1.0, &mpF_pb);
            addInputVariable("f_at", "Fraction of spool circumference that is opening A-T", "-", 1.0, &mpF_at);
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

            //Initiate second order low pass filter
            double num[3] = {1.0, 0.0, 0.0};
            double den[3] = {1.0, 2.0*mDeltah/mOmegah, 1.0/(mOmegah*mOmegah)};
            double initXv = limit(*mpXvIn,0.0,(*mpXvmax));
            mSpoolPosTF.initialize(mTimestep, num, den, initXv, initXv, 0, (*mpXvmax));
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double cp, Zcp, ct, Zct, ca, Zca, cb, Zcb, xvin, xv, xpanom, xpbnom, xatnom, xbtnom, Kcpa, Kcpb, Kcat, Kcbt, qpa, qpb, qat, qbt, qp, qa, qb, qt, pa, pb, pt, pp;
            double Cq, rho, xvmax, d, f_pa, f_pb, f_at, f_bt;
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

            xvin  = (*mpXvIn);
            Cq = (*mpCq);
            rho = (*mpRho);
            xvmax = (*mpXvmax);
            d = (*mpD);
            f_pa = (*mpF_pa);
            f_pb = (*mpF_pb);
            f_at = (*mpF_at);
            f_bt = (*mpF_bt);

            if(doubleToBool(xvin))
            {
                mSpoolPosTF.update(xvmax);
            }
            else
            {
                mSpoolPosTF.update(0);
            }
            xv = mSpoolPosTF.value();

            //Valve equations
            xpanom = xv;
            xpbnom = xvmax-xv;
            xatnom = xvmax-xv;
            xbtnom = xv;

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

            //Write new values to nodes

            (*mpPP_p) = cp + qp*Zcp;
            (*mpPP_q) = qp;
            (*mpPT_p) = ct + qt*Zct;
            (*mpPT_q) = qt;
            (*mpPA_p) = ca + qa*Zca;
            (*mpPA_q) = qa;
            (*mpPB_p) = cb + qb*Zcb;
            (*mpPB_q) = qb;
            (*mpXv) = xv;
        }
    };
}

#endif // HYDRAULIC42DIRECTIONALVALVE_HPP_INCLUDED
