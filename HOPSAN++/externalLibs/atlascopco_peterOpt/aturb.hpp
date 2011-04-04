/*****************************************************************

Turbulent orifice
Translated from old hopsan D_ORIFIT.
Using opening area as input parameter.

Simon Magnusson
20101005


Schematic image:

  ===---===

*****************************************************************/


#ifndef ATURB_HPP_INCLUDED //
#define ATURB_HPP_INCLUDED //

//#include "P:/Hopsan_ng/Hopsan_latest/include/ComponentEssentials.h"
//#include "P:/Hopsan_ng/Hopsan_latest/include/ComponentUtilities.h"
#include "../../HopsanCore/ComponentEssentials.h"
#include "../../HopsanCore/ComponentUtilities.h"
#include "math.h"

namespace hopsan {

    class aturb : public ComponentQ
    {
    private:

        Port *mpP1, *mpP2;
//        double Q10,Q20,P10,P20;
        double Cq,A,rho,K;
        bool cav;
        TurbulentFlowFunction qTurb;

        //Declaration of node data pointers, ND is short for NodeData
        double *mpND_Zx1, *mpND_Zx2, *mpND_Cx1, *mpND_Cx2, *mpND_P1, *mpND_P2, *mpND_Q1, *mpND_Q2;

    public:
        static Component *Creator()
        {
            return new aturb();
        }

        aturb() : ComponentQ()
        {

            //Add port to the component
            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");

            //Start values
            cav = false;
//          P10 = 0;
//          P20 = 0;
//          Q10 = 0;
//          Q20 = 0;
//          registerParameter("P1", "Pressure", "[Pa]", P10);
//          registerParameter("P2", "Pressure", "[Pa]", P20);
//          registerParameter("Q1", "Flow", "[m^3/s]", Q10);
//          registerParameter("Q2", "Flow", "[m^3/s]", Q20);

            //Parameters
            A   = 1e-5;
            Cq  = 0.6;
            rho = 890;
            registerParameter("A", "Orifice area", "[m^2]", A);
            registerParameter("Cq", "Flow-pressure coefficient", "[-]", Cq);
            registerParameter("rho", "Fluid density", "[kg/m^3]", rho);
        }


        void initialize() 
        {
            //Declaration of local variables
            double Zx1, Zx2, P1, P2;

//            //Initialize start values
//            pP1->writeNode(NodeHydraulic::PRESSURE, P10);
//            pP2->writeNode(NodeHydraulic::PRESSURE, P20);
//            pP1->writeNode(NodeHydraulic::FLOW, Q10);
//            pP2->writeNode(NodeHydraulic::FLOW, Q20);

            //Assign node data pointers
            mpND_Cx1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::WAVEVARIABLE);
            mpND_Zx1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::CHARIMP);
            mpND_P1  = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE);
            mpND_Cx2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::WAVEVARIABLE);
            mpND_Zx2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::CHARIMP);
            mpND_P2  = getSafeNodeDataPtr(mpP2, NodeHydraulic::PRESSURE);

            //Read values from node data pointers
//            double Zx1 = pP1->readNode(NodeHydraulic::CHARIMP);
//            double Zx2 = pP2->readNode(NodeHydraulic::CHARIMP);
//            double P1  = pP1->readNode(NodeHydraulic::PRESSURE);
//            double P2  = pP2->readNode(NodeHydraulic::PRESSURE);
            Zx1 = *mpND_Zx1;
            Zx2 = *mpND_Zx2;
            P1 = *mpND_P1;
            P2 = *mpND_P2;

//!         @warning DONT do  Zx1 == 0,  Zx1 is a floating point number, there is no gurante that 0 == 0 is true.
//!         0 might actually be 0.00000000000000000000000001 or -0.999999999999999999999999999999 whiche are not the same
//            if (Zx1 == 0) pP1->writeNode(NodeHydraulic::WAVEVARIABLE, P1);
//            if (Zx2 == 0) pP2->writeNode(NodeHydraulic::WAVEVARIABLE, P2);
            if (fabs(Zx1) < 1e-10) {*mpND_Cx1 = P1;}
            if (fabs(Zx2) < 1e-10) {*mpND_Cx2 = P2;}

            K = Cq*A*sqrt(2.0/rho);
            if(K < 0) K = 0;    //!!!!! Notify user !!!!!
            qTurb.setFlowCoefficient(K);
        }

        void simulateOneTimestep() // Here is the actual simulation call.
        {
            //Declaration of local variables
            double Cx1,Cx2,Zx1,Zx2,P1,P2,Q1,Q2;

            //Read values from nodes
//            double Cx1 = pP1->readNode(NodeHydraulic::WAVEVARIABLE);
//            double Zx1 = pP1->readNode(NodeHydraulic::CHARIMP);
//            double Cx2 = pP2->readNode(NodeHydraulic::WAVEVARIABLE);
//            double Zx2 = pP2->readNode(NodeHydraulic::CHARIMP);
            Cx1 = *mpND_Cx1;
            Zx1 = *mpND_Zx1;
            Cx2 = *mpND_Cx2;
            Zx2 = *mpND_Zx2;

            //Equations
            Q2 = qTurb.getFlow(Cx1,Cx2,Zx1,Zx2);
            Q1 = -Q2;
            P1 = Cx1 + Q1*Zx1;
            P2 = Cx2 + Q2*Zx2;

            // Cavitation handling
            if( P1 <= 0) {
                Cx1 = 0;
                Zx1 = 0;
                cav = true;
            }
            if( P2 <= 0) {
                Cx2 = 0;
                Zx2 = 0;
                cav = true;
            }
            if(cav) {
                Q2 = qTurb.getFlow(Cx1,Cx2,Zx1,Zx2);
                Q1 = -Q2;
                P1 = Cx1 + Q1*Zx1;
                P2 = Cx2 + Q2*Zx2;
                if(P1<0) P1 = 0;
                if(P2<0) P2 = 0;
            }

            //Write new values to nodes
//            pP1->writeNode(NodeHydraulic::PRESSURE, P1);
//            pP2->writeNode(NodeHydraulic::PRESSURE, P2);
//            pP1->writeNode(NodeHydraulic::FLOW, Q1);
//            pP2->writeNode(NodeHydraulic::FLOW, Q2);
            *mpND_P1 = P1;
            *mpND_P2 = P2;
            *mpND_Q1 = Q1;
            *mpND_Q2 = Q2;
        }


        void finalize()
        {
            //Nothing to finalize.
        }
    };
}

#endif
