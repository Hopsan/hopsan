//!
//! @file   Hydraulic43LoadSensingValve.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-12
//!
//! @brief Contains a hydraulic 4/3-valve with load sensing port of Q-type
#ifndef HYDRAULIC43LOADSENSINGVALVE_HPP_INCLUDED
#define HYDRAULIC43LOADSENSINGVALVE_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief Hydraulic 4/3-valve (closed centre) with load sensing port of Q-type.
    //! @ingroup HydraulicComponents
    //!
    class Hydraulic43LoadSensingValve : public ComponentQ
    {
    private:
        double mCq;
        double md;
        double mf;
        double mxvmax;
        double moverlap_pa;
        double moverlap_pb;
        double moverlap_at;
        double moverlap_bt;
        double momegah;
        double mdeltah;
        SecondOrderFilter myFilter;
        TurbulentFlowFunction mQturbpa;
        TurbulentFlowFunction mQturbpb;
        TurbulentFlowFunction mQturbat;
        TurbulentFlowFunction mQturbbt;
#define pi 3.14159
        Port *mpPP, *mpPT, *mpPA, *mpPB, *mpIn, *mpPL;
        double sign(double x)
        {
            if (x>=0.0)
            {
                return 1.0;
            }
            else
            {
                return -1.0;
            }
        }
    public:
        static Component *Creator()
        {
            return new Hydraulic43LoadSensingValve("Hydraulic 4/3 Valve with Load Sensing Port");
        }

        Hydraulic43LoadSensingValve(const std::string name) : ComponentQ(name)
        {
            mTypeName = "Hydraulic43LoadSensingValve";
            mCq = 0.67;
            md = 0.01;
            mf = 1.0;
            mxvmax = 0.01;
            moverlap_pa = 0.0;
            moverlap_pb = 0.0;
            moverlap_at = 0.0;
            moverlap_bt = 0.0;
            momegah = 100.0;
            mdeltah = 0.0;

            mpPP = addPowerPort("PP", "NodeHydraulic");
            mpPT = addPowerPort("PT", "NodeHydraulic");
            mpPA = addPowerPort("PA", "NodeHydraulic");
            mpPB = addPowerPort("PB", "NodeHydraulic");
            mpIn = addReadPort("in", "NodeSignal");
            mpPL = addWritePort("PL", "NodeSignal");

            registerParameter("Cq", "Flow Coefficient", "[-]", mCq);
            registerParameter("d", "Diameter", "[m]", md);
            registerParameter("f", "Spool Fraction of the Diameter", "[-]", mf);
            registerParameter("xvmax", "Maximum Spool Displacement", "[m]", mxvmax);
            registerParameter("overlap_pa", "Spool Overlap From Port P To A", "[m]", moverlap_pa);
            registerParameter("overlap_pb", "Spool Overlap From Port P To B", "[m]", moverlap_pb);
            registerParameter("overlap_at", "Spool Overlap From Port A To T", "[m]", moverlap_at);
            registerParameter("overlap_pa", "Spool Overlap From Port B To T", "[m]", moverlap_bt);
            registerParameter("omegah", "Resonance Frequency", "[rad/s]", momegah);
            registerParameter("deltah", "Damping Factor", "[-]", mdeltah);
        }


        void initialize()
        {
            double num[3] = {0.0, 0.0, 1.0};

            double den[3] = {1.0/(momegah*momegah), 2.0*mdeltah/momegah, 1.0};
            myFilter.initialize(mTimestep, num, den, 0, 0, -mxvmax, mxvmax);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double cp  = mpPP->readNode(NodeHydraulic::WAVEVARIABLE);
            double Zcp = mpPP->readNode(NodeHydraulic::CHARIMP);
            double ct  = mpPT->readNode(NodeHydraulic::WAVEVARIABLE);
            double Zct = mpPT->readNode(NodeHydraulic::CHARIMP);
            double ca  = mpPA->readNode(NodeHydraulic::WAVEVARIABLE);
            double Zca = mpPA->readNode(NodeHydraulic::CHARIMP);
            double cb  = mpPB->readNode(NodeHydraulic::WAVEVARIABLE);
            double Zcb = mpPB->readNode(NodeHydraulic::CHARIMP);
            double xvin  = mpIn->readNode(NodeSignal::VALUE);

            myFilter.update(xvin);
            double xv = myFilter.value();

            double xpanom = std::max(xv-moverlap_pa,0.0);
            double xpbnom = std::max(-xv-moverlap_pb,0.0);
            double xatnom = std::max(-xv-moverlap_at,0.0);
            double xbtnom = std::max(xv-moverlap_bt,0.0);

            double Kcpa = mCq*mf*pi*md*xpanom*sqrt(2.0/890.0);
            double Kcpb = mCq*mf*pi*md*xpbnom*sqrt(2.0/890.0);
            double Kcat = mCq*mf*pi*md*xatnom*sqrt(2.0/890.0);
            double Kcbt = mCq*mf*pi*md*xbtnom*sqrt(2.0/890.0);

            //With TurbulentFlowFunction:
            mQturbpa.setFlowCoefficient(Kcpa);
            mQturbpb.setFlowCoefficient(Kcpb);
            mQturbat.setFlowCoefficient(Kcat);
            mQturbbt.setFlowCoefficient(Kcbt);

            double qpa = mQturbpa.getFlow(cp, ca, Zcp, Zca);
            double qpb = mQturbpb.getFlow(cp, cb, Zcp, Zcb);
            double qat = mQturbat.getFlow(ca, ct, Zca, Zct);
            double qbt = mQturbbt.getFlow(cb, ct, Zcb, Zct);

            double qp, qa, qb, qt;

            if (xv >= 0.0)
            {
                qp = -qpa;
                qa = qpa;
                qb = -qbt;
                qt = qbt;
            }
            else
            {
                qp = -qpb;
                qa = -qat;
                qb = qpb;
                qt = qat;
            }

            double pp = cp + qp*Zcp;
            double pa = ca + qa*Zca;
            double pb = cb + qb*Zcb;
            double pt = ct + qt*Zct;

            double pload;

            if(xv >= 0.0)
            {
                pload = pa;
            }
            else
            {
                pload = pb;
            }

            //Write new values to nodes

            mpPP->writeNode(NodeHydraulic::PRESSURE, pp);
            mpPP->writeNode(NodeHydraulic::FLOW, qp);
            mpPT->writeNode(NodeHydraulic::PRESSURE, pt);
            mpPT->writeNode(NodeHydraulic::FLOW, qt);
            mpPA->writeNode(NodeHydraulic::PRESSURE, pa);
            mpPA->writeNode(NodeHydraulic::FLOW, qa);
            mpPB->writeNode(NodeHydraulic::PRESSURE, pb);
            mpPB->writeNode(NodeHydraulic::FLOW, qb);
            mpPL->writeNode(NodeSignal::VALUE, pload);
        }
    };
}

#endif // HYDRAULIC43LOADSENSINGVALVE_HPP_INCLUDED

