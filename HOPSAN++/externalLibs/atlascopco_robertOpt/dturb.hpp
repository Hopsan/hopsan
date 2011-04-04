/*****************************************************************

Turbulent orifice
Translated from old hopsan D_ORIFIT.

Simon Magnusson
20101005


Schematic image:

  ===---===

*****************************************************************/


#ifndef DTURB_HPP_INCLUDED //
#define DTURB_HPP_INCLUDED //

//#include "P:/Hopsan_ng/Hopsan_latest/include/ComponentEssentials.h"
//#include "P:/Hopsan_ng/Hopsan_latest/include/ComponentUtilities.h"
#include "../../HopsanCore/ComponentEssentials.h"
#include "../../HopsanCore/ComponentUtilities.h"
#include "math.h"

namespace hopsan {

    class dturb : public ComponentQ

    {
    private:
        double P1, Q1, Cx1, Zx1, P2, Q2, Cx2, Zx2;
        double *P1_ptr, *Q1_ptr, *Cx1_ptr, *Zx1_ptr, *P2_ptr, *Q2_ptr, *Cx2_ptr, *Zx2_ptr;
        Port *pP1, *pP2;
        double Cq,D,rho,K;
        bool cav;
        TurbulentFlowFunction qTurb;

    public:
        static Component *Creator()
        {
            return new dturb();
        }

        dturb() : ComponentQ()
        {

                //Add port to the component
            pP1 = addPowerPort("P1", "NodeHydraulic");
            pP2 = addPowerPort("P2", "NodeHydraulic");

		//Start values
            cav = false;

                //Parameters
            D   = 1e-3;
            Cq  = 0.6;
            rho = 890;
            registerParameter("D", "Orifice diameter", "[m]", D);
            registerParameter("Cq", "Flow-pressure coefficient", "[-]", Cq);
            registerParameter("rho", "Fluid density", "[kg/m^3]", rho);
        }


        void initialize() 
        {            
            //Assign port pointers
            P1_ptr = pP1->getNodeDataPtr(NodeHydraulic::PRESSURE);
            Q1_ptr = pP1->getNodeDataPtr(NodeHydraulic::FLOW);
            Cx1_ptr = pP1->getNodeDataPtr(NodeHydraulic::WAVEVARIABLE);
            Zx1_ptr = pP1->getNodeDataPtr(NodeHydraulic::CHARIMP);
            P2_ptr = pP2->getNodeDataPtr(NodeHydraulic::PRESSURE);
            Q2_ptr = pP2->getNodeDataPtr(NodeHydraulic::FLOW);
            Cx2_ptr = pP2->getNodeDataPtr(NodeHydraulic::WAVEVARIABLE);
            Zx2_ptr = pP2->getNodeDataPtr(NodeHydraulic::CHARIMP);

            //Read data from nodes
            P1 = (*P1_ptr);
            Zx1 = (*Zx1_ptr);
            P2 = (*P2_ptr);
            Zx2 = (*Zx2_ptr);

            if (Zx1 == 0) Cx1 = P1;
            if (Zx2 == 0) Cx2 = P2;
            K = Cq*D*D*3.14159265/4*sqrt(2/rho);
            if(K < 0) K = 0;    //!!!!! Notify user !!!!!
            qTurb.setFlowCoefficient(K);

            //Write data to nodes
            (*Cx1_ptr) = Cx1;
            (*Cx2_ptr) = Cx2;
        }

        void simulateOneTimestep() // Here is the actual simulation call.
        {
            //Read data from nodes
            Cx1 = (*Cx1_ptr);
            Zx1 = (*Zx1_ptr);
            Cx2 = (*Cx2_ptr);
            Zx2 = (*Zx2_ptr);

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
            cav = false;

            //Write new values to nodes
            (*P1_ptr) = P1;
            (*Q1_ptr) = Q1;
            (*P2_ptr) = P2;
            (*Q2_ptr) = Q2;
        }


        void finalize()
        {
            //Nothing to finalize.
        }
    };
}

#endif
