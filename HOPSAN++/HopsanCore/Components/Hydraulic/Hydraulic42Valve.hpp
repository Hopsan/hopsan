//!
//! @file   Hydraulic42Valve.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-12-20
//!
//! @brief Contains a hydraulic 4/2-valve of Q-type

#ifndef HYDRAULIC42VALVE_HPP_INCLUDED
#define HYDRAULIC42VALVE_HPP_INCLUDED

#define pi 3.14159

#include <iostream>
#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief Hydraulic 4/2-valve of Q-type.
    //! @ingroup HydraulicComponents
    //!
    class Hydraulic42Valve : public ComponentQ
    {
    private:
        double mCq;
        double md;
        double mf;
        double mxvmax;
        double mOverlap_pa;
        double mOverlap_pb;
        double mOverlap_at;
        double mOverlap_bt;
        double momegah;
        double mdeltah;
        SecondOrderFilter myFilter;
        TurbulentFlowFunction mQturb_pa;
        TurbulentFlowFunction mQturb_bt;
        Port *mpPP, *mpPT, *mpPA, *mpPB, *mpIn;

    public:
        static Component *Creator()
        {
            return new Hydraulic42Valve("Hydraulic 4/2 Valve");
        }

        Hydraulic42Valve(const std::string name) : ComponentQ(name)
        {
            mTypeName = "Hydraulic42Valve";
            mCq = 0.67;
            md = 0.01;
            mf = 1.0;
            mxvmax = 0.01;
            mOverlap_pa = 0.0;
            mOverlap_pb = 0.0;
            mOverlap_at = 0.0;
            mOverlap_bt = 0.0;
            momegah = 100.0;
            mdeltah = 0.0;

            mpPP = addPowerPort("PP", "NodeHydraulic");
            mpPT = addPowerPort("PT", "NodeHydraulic");
            mpPA = addPowerPort("PA", "NodeHydraulic");
            mpPB = addPowerPort("PB", "NodeHydraulic");
            mpIn = addReadPort("in", "NodeSignal");

            registerParameter("Cq", "Flow Coefficient", "[-]", mCq);
            registerParameter("d", "Diameter", "[m]", md);
            registerParameter("f", "Spool Fraction of the Diameter", "[-]", mf);
            registerParameter("xvmax", "Maximum Spool Displacement", "[m]", mxvmax);
            registerParameter("overlap", "Spool Overlap From Port P To A", "[m]", mOverlap_pa);
            registerParameter("overlap", "Spool Overlap From Port P To A", "[m]", mOverlap_pb);
            registerParameter("overlap", "Spool Overlap From Port P To A", "[m]", mOverlap_at);
            registerParameter("overlap", "Spool Overlap From Port P To A", "[m]", mOverlap_bt);
            registerParameter("omegah", "Resonance Frequency", "[rad/s]", momegah);
            registerParameter("deltah", "Damping Factor", "[-]", mdeltah);
        }


        void initialize()
        {
            //Initiate second order low pass filter
            double num[3] = {0.0, 0.0, 1.0};
            double den[3] = {1.0/(momegah*momegah), 2.0*mdeltah/momegah, 1.0};
            myFilter.initialize(mTimestep, num, den, 0, 0, 0, mxvmax);
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

            //Dynamics of spool position (second order low pass filter)
            myFilter.update(xvin);
            double xv = myFilter.value();

            //Determine flow coefficient
            double Kc = mCq*mf*pi*md*xv*sqrt(2.0/890.0);

            //Calculate flow
            mQturb_pa.setFlowCoefficient(Kc);
            mQturb_bt.setFlowCoefficient(Kc);
            double qpa = mQturb_pa.getFlow(cp, ca, Zcp, Zca);
            double qbt = mQturb_bt.getFlow(cb, ct, Zcb, Zct);
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
                qp = 0;
                qa = 0;
                qb = 0;
                qt = 0;
            }

            //Calculate pressures from flow and impedance
            double pp = cp + qp*Zcp;
            double pa = ca + qa*Zca;
            double pb = cb + qb*Zcb;
            double pt = ct + qt*Zct;

            //Write new values to nodes
            mpPP->writeNode(NodeHydraulic::PRESSURE, pp);
            mpPP->writeNode(NodeHydraulic::FLOW, qp);
            mpPT->writeNode(NodeHydraulic::PRESSURE, pt);
            mpPT->writeNode(NodeHydraulic::FLOW, qt);
            mpPA->writeNode(NodeHydraulic::PRESSURE, pa);
            mpPA->writeNode(NodeHydraulic::FLOW, qa);
            mpPB->writeNode(NodeHydraulic::PRESSURE, pb);
            mpPB->writeNode(NodeHydraulic::FLOW, qb);
        }
    };
}

#endif // HYDRAULIC42VALVE_HPP_INCLUDED
