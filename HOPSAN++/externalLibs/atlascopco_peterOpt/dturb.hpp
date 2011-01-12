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

        Port *mpP1, *mpP2;
//        double Q10,Q20,P10,P20;
        double Cq,D,rho,K;
        bool cav;
        TurbulentFlowFunction qTurb;

        //Declaration of node data pointers, ND is short for NodeData
        double *mpND_Zx1, *mpND_Zx2, *mpND_Cx1, *mpND_Cx2, *mpND_P1, *mpND_P2, *mpND_Q1, *mpND_Q2;

    public:
        static Component *Creator()
        {
            return new dturb();
        }

        dturb() : ComponentQ()
        {
            mTypeName = "dturb";

                //Add port to the component
            mpP1 = addPowerPort("P1", "NodeHydraulic");
            mpP2 = addPowerPort("P2", "NodeHydraulic");

		//Start values
                cav = false;
//                P10 = 0;
//                P20 = 0;
//                Q10 = 0;
//                Q20 = 0;
//            registerParameter("P1", "Pressure", "[Pa]", P10);
//            registerParameter("P2", "Pressure", "[Pa]", P20);
//            registerParameter("Q1", "Flow", "[m^3/s]", Q10);
//            registerParameter("Q2", "Flow", "[m^3/s]", Q20);

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
//		//Initialize start values
//            mpP1->writeNode(NodeHydraulic::PRESSURE, P10);
//            mpP2->writeNode(NodeHydraulic::PRESSURE, P20);
//            mpP1->writeNode(NodeHydraulic::FLOW, Q10);
//            mpP2->writeNode(NodeHydraulic::FLOW, Q20);

            //Assign node data pointers
            mpND_Cx1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::WAVEVARIABLE);
            mpND_Zx1 = getSafeNodeDataPtr(mpP1, NodeHydraulic::CHARIMP);
            mpND_P1  = getSafeNodeDataPtr(mpP1, NodeHydraulic::PRESSURE);
            mpND_Cx2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::WAVEVARIABLE);
            mpND_Zx2 = getSafeNodeDataPtr(mpP2, NodeHydraulic::CHARIMP);
            mpND_P2  = getSafeNodeDataPtr(mpP2, NodeHydraulic::PRESSURE);

            //Read values from nodes
//            double Zx1 = mpP1->readNode(NodeHydraulic::CHARIMP);
//            double Zx2 = mpP2->readNode(NodeHydraulic::CHARIMP);
//            double P1  = mpP1->readNode(NodeHydraulic::PRESSURE);
//            double P2  = mpP2->readNode(NodeHydraulic::PRESSURE);
            double Zx1 = *mpND_Zx1;
            double Zx2 = *mpND_Zx2;
            double P1 = *mpND_P1;
            double P2 = *mpND_P2;

//!         @warning DONT do  Zx1 == 0,  Zx1 is a floating point number, there is no gurante that 0 == 0 is true.
//!         0 might actually be 0.00000000000000000000000001 or -0.00000000000000000000000001 which are not the same
//            if (Zx1 == 0) mpP1 ->writeNode(NodeHydraulic::WAVEVARIABLE, P1);
//            if (Zx2 == 0) mpP2 ->writeNode(NodeHydraulic::WAVEVARIABLE, P2);
            if (fabs(Zx1) < 1e-10) { *mpND_Cx1 = P1;}
            if (fabs(Zx2) < 1e-10) { *mpND_Cx2 = P2;}

//          K = Cq*D*D*3.14159265/4*sqrt(2/rho);
//!         @note Be careful when using division, allways make sure that you indicate that you want to do floating point division, otherwise some compilers may assume you mean integer division which means truncation
//!         @note You can use  the  M_PI_4  definition from math.h instead of writing 3.14159265/4.0 (if you want)
            K = Cq*D*D*3.14159265/4.0*sqrt(2.0/rho);
            if(K < 0) K = 0;    //!!!!! Notify user !!!!!
            qTurb.setFlowCoefficient(K);
        }

        void simulateOneTimestep() // Here is the actual simulation call.
        {
            //Read values from nodes
//            double Cx1 = mpP1->readNode(NodeHydraulic::WAVEVARIABLE);
//            double Zx1 = mpP1->readNode(NodeHydraulic::CHARIMP);
//            double Cx2 = mpP2->readNode(NodeHydraulic::WAVEVARIABLE);
//            double Zx2 = mpP2->readNode(NodeHydraulic::CHARIMP);
            double Cx1 = *mpND_Cx1;
            double Zx1 = *mpND_Zx1;
            double Cx2 = *mpND_Cx2;
            double Zx2 = *mpND_Zx2;

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
            cav = false;

            //Write new values to nodes
//            mpP1->writeNode(NodeHydraulic::PRESSURE, P1);
//            mpP2->writeNode(NodeHydraulic::PRESSURE, P2);
//            mpP1->writeNode(NodeHydraulic::FLOW, Q1);
//            mpP2->writeNode(NodeHydraulic::FLOW, Q2);
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
