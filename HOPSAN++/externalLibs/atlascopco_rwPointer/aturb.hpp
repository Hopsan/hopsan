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

        Port *pP1, *pP2;
        double Q10,Q20,P10,P20;
        double Cq,A,rho,K;
        bool cav;
        TurbulentFlowFunction qTurb;

    public:
        static Component *Creator()
        {
            return new aturb();
        }

        aturb() : ComponentQ()
        {

                //Add port to the component
            pP1 = addPowerPort("P1", "NodeHydraulic");
            pP2 = addPowerPort("P2", "NodeHydraulic");

		//Start values
                cav = false;
                P10 = 0;
                P20 = 0;
                Q10 = 0;
                Q20 = 0;
            registerParameter("P1", "Pressure", "[Pa]", P10);
            registerParameter("P2", "Pressure", "[Pa]", P20);
            registerParameter("Q1", "Flow", "[m^3/s]", Q10);
            registerParameter("Q2", "Flow", "[m^3/s]", Q20);

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
		//Initialize start values
            pP1->writeNode(NodeHydraulic::PRESSURE, P10);
            pP2->writeNode(NodeHydraulic::PRESSURE, P20);
            pP1->writeNode(NodeHydraulic::FLOW, Q10);
            pP2->writeNode(NodeHydraulic::FLOW, Q20);
            double Zx1 = pP1->readNode(NodeHydraulic::CHARIMP);
            double Zx2 = pP2->readNode(NodeHydraulic::CHARIMP);
            double P1  = pP1->readNode(NodeHydraulic::PRESSURE);
            double P2  = pP2->readNode(NodeHydraulic::PRESSURE);
            if (Zx1 == 0) pP1 ->writeNode(NodeHydraulic::WAVEVARIABLE, P1);
            if (Zx2 == 0) pP2 ->writeNode(NodeHydraulic::WAVEVARIABLE, P2);
            K = Cq*A*sqrt(2/rho);
            if(K < 0) K = 0;    //!!!!! Notify user !!!!!
            qTurb.setFlowCoefficient(K);
        }

        void simulateOneTimestep() // Here is the actual simulation call.
        {
            double Cx1 = pP1->readNode(NodeHydraulic::WAVEVARIABLE);
            double Zx1 = pP1->readNode(NodeHydraulic::CHARIMP);
            double Cx2 = pP2->readNode(NodeHydraulic::WAVEVARIABLE);
            double Zx2 = pP2->readNode(NodeHydraulic::CHARIMP);

            //Equations
            double Q2 = qTurb.getFlow(Cx1,Cx2,Zx1,Zx2);
            double Q1 = -Q2;
            double P1 = Cx1 + Q1*Zx1;
            double P2 = Cx2 + Q2*Zx2;

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
            pP1->writeNode(NodeHydraulic::PRESSURE, P1);
            pP2->writeNode(NodeHydraulic::PRESSURE, P2);
            pP1->writeNode(NodeHydraulic::FLOW, Q1);
            pP2->writeNode(NodeHydraulic::FLOW, Q2);
        }


        void finalize()
        {
            //Nothing to finalize.
        }
    };
}

#endif
