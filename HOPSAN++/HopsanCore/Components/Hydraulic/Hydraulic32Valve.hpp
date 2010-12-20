//!
//! @file   Hydraulic32Valve.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-12-06
//!
//! @brief Contains a hydraulic 3/2-valve of Q-type

#ifndef HYDRAULIC32VALVE_HPP_INCLUDED
#define HYDRAULIC32VALVE_HPP_INCLUDED

#define pi 3.14159

#include <iostream>
#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief Hydraulic 3/2-valve (closed centre) of Q-type.
    //! @ingroup HydraulicComponents
    //!
    class Hydraulic32Valve : public ComponentQ
    {
    private:
        double mCq;
        double md;
        double mf;
        double mxvmax;
        double moverlap_pa;
        double moverlap_at;
        double moverlap_bt;
        double momegah;
        double mdeltah;
        SecondOrderFilter myFilter;
        TurbulentFlowFunction mQturbpa;
        TurbulentFlowFunction mQturbat;
        Port *mpPP, *mpPT, *mpPA, *mpPB, *mpIn;

    public:
        static Component *Creator()
        {
            return new Hydraulic32Valve("Hydraulic 3/2 Valve");
        }

        Hydraulic32Valve(const std::string name) : ComponentQ(name)
        {
            mTypeName = "Hydraulic32Valve";
            mCq = 0.67;
            md = 0.01;
            mf = 1.0;
            mxvmax = 0.01;
            moverlap_pa = 0.0;
            moverlap_at = 0.0;
            momegah = 100.0;
            mdeltah = 0.0;

            mpPP = addPowerPort("PP", "NodeHydraulic");
            mpPT = addPowerPort("PT", "NodeHydraulic");
            mpPA = addPowerPort("PA", "NodeHydraulic");
            mpIn = addReadPort("in", "NodeSignal");

            registerParameter("Cq", "Flow Coefficient", "[-]", mCq);
            registerParameter("d", "Diameter", "[m]", md);
            registerParameter("f", "Spool Fraction of the Diameter", "[-]", mf);
            registerParameter("xvmax", "Maximum Spool Displacement", "[m]", mxvmax);
            registerParameter("overlap_pa", "Spool Overlap From Port P To A", "[m]", moverlap_pa);
            registerParameter("overlap_at", "Spool Overlap From Port A To T", "[m]", moverlap_at);
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
            double xvin  = mpIn->readNode(NodeSignal::VALUE);

            myFilter.update(xvin);
            double xv = myFilter.value();



            double xpanom = std::max((mxvmax+xv)/2-moverlap_pa,0.0);
            double xatnom = std::max((mxvmax-xv)/2-moverlap_at,0.0);

            double Kcpa = mCq*mf*pi*md*xpanom*sqrt(2.0/890.0);
            double Kcat = mCq*mf*pi*md*xatnom*sqrt(2.0/890.0);

            //With TurbulentFlowFunction:
            mQturbpa.setFlowCoefficient(Kcpa);
            mQturbat.setFlowCoefficient(Kcat);

            double qpa = mQturbpa.getFlow(cp, ca, Zcp, Zca);
            double qat = mQturbat.getFlow(ca, ct, Zca, Zct);

            double qp, qa, qt;
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

            double pp = cp + qp*Zcp;
            double pa = ca + qa*Zca;
            double pt = ct + qt*Zct;

            //Write new values to nodes

            mpPP->writeNode(NodeHydraulic::PRESSURE, pp);
            mpPP->writeNode(NodeHydraulic::FLOW, qp);
            mpPT->writeNode(NodeHydraulic::PRESSURE, pt);
            mpPT->writeNode(NodeHydraulic::FLOW, qt);
            mpPA->writeNode(NodeHydraulic::PRESSURE, pa);
            mpPA->writeNode(NodeHydraulic::FLOW, qa);
        }
    };
}

#endif // HYDRAULIC32VALVE_HPP_INCLUDED

