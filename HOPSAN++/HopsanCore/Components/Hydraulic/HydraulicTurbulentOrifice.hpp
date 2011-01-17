//!
//! @file   HydraulicTurbulentOrifice.hpp
//! @author Karl Pettersson
//! @date   2009-12-21
//!
//! @brief Contains a Hydraulic Turbulent Orifice
//! @ingroup HydraulicComponents
//!
#ifndef HYDRAULICTURBULENTORIFICE_HPP_INCLUDED
#define HYDRAULICTURBULENTORIFICE_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"
#include "math.h"

namespace hopsan {

    //!
    //! @brief Hydraulic orifice with turbulent flow of Q-Type. Uses TurbulentFlowFunction to calculate the flow.
    //! @ingroup HydraulicComponents
    //!
    class HydraulicTurbulentOrifice : public ComponentQ
    {
    private:
        double Cq;
        double A;
        double Kc;
        bool cav;
        TurbulentFlowFunction qTurb;

        double *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1, *mpND_p2, *mpND_q2, *mpND_c2, *mpND_Zc2, *A_ptr;
        double p1,q1, p2, q2, c1, Zc1, c2, Zc2;

        Port *mpP1, *mpP2, *mpIn;

    public:
        static Component *Creator()
        {
            return new HydraulicTurbulentOrifice("TurbulentOrifice");
        }

        HydraulicTurbulentOrifice(const std::string name) : ComponentQ(name)
        {
            mTypeName = "HydraulicTurbulentOrificeName";
            Cq = 0.67;
            A = 0.00001;
            Kc = Cq*A*sqrt(2.0/890.0);

            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");
            mpIn = addReadPort("A", "NodeSignal", Port::NOTREQUIRED);

            registerParameter("Cq", "Flow coefficient", "[-]", Cq);
            registerParameter("A", "Area", "m^3", A);
        }


        void initialize()
        {
            mpND_p1 = mpP1->getNodeDataPtr(NodeHydraulic::PRESSURE);
            mpND_q1 = mpP1->getNodeDataPtr(NodeHydraulic::FLOW);
            mpND_c1 = mpP1->getNodeDataPtr(NodeHydraulic::WAVEVARIABLE);
            mpND_Zc1 = mpP1->getNodeDataPtr(NodeHydraulic::CHARIMP);

            mpND_p2 = mpP2->getNodeDataPtr(NodeHydraulic::PRESSURE);
            mpND_q2 = mpP2->getNodeDataPtr(NodeHydraulic::FLOW);
            mpND_c2 = mpP2->getNodeDataPtr(NodeHydraulic::WAVEVARIABLE);
            mpND_Zc2 = mpP2->getNodeDataPtr(NodeHydraulic::CHARIMP);

            if(mpIn->isConnected())
            {
                A_ptr = mpIn->getNodeDataPtr(NodeSignal::VALUE);
            }
            else
            {
                A_ptr = new double(A);
            }
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            c1 = (*mpND_c1);
            Zc1 = (*mpND_Zc1);
            c2 = (*mpND_c2);
            Zc2 = (*mpND_Zc2);
            A = (*A_ptr);

            //Orifice equations
            Kc = Cq*A*sqrt(2.0/890.0);
            qTurb.setFlowCoefficient(Kc);
            q2 = qTurb.getFlow(c1,c2,Zc1,Zc2);
            q1 = -q2;
            p1 = c1 + q1*Zc1;
            p2 = c2 + q2*Zc2;

            //Cavitation check
            cav = false;
            if(p1 < 0.0)
            {
                c1 = 0.0;
                Zc1 = 0.0;
                cav = true;
            }
            if(p2 < 0.0)
            {
                c2 = 0.0;
                Zc2 = 0.0;
                cav = true;
            }
            if(p1 < 0.0 && p2 < 0.0)
            {
                p1 = 0.0;
                q1 = 0.0;
                p2 = 0.0;
                q2 = 0.0;
            }
            if(cav)
            {
                q2 = qTurb.getFlow(c1,c2,Zc1,Zc2);
                q1 = -q2;
                p1 = c1 + q1*Zc1;
                p2 = c2 + q2*Zc2;
                if(p1 < 0.0) { p1 = 0.0; }
                if(p2 < 0.0) { p2 = 0.0; }
            }

            //Write new variables to nodes
            (*mpND_p1) = p1;
            (*mpND_q1) = q1;
            (*mpND_p2) = p2;
            (*mpND_q2) = q2;
        }
    };
}

#endif // HYDRAULICTURBULENTORIFICE_HPP_INCLUDED
