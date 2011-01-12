/*****************************************************************

Mechanic contact
Translated from old hopsan gXGCON.

Erik Jakobsson
20100916


Schematic image:

---| |---

*****************************************************************/
#ifndef SEP2_HPP_INCLUDED
#define SEP2_HPP_INCLUDED

//#include "P:/Hopsan_ng/Hopsan_latest/include/ComponentEssentials.h"
//#include "P:/Hopsan_ng/Hopsan_latest/include/ComponentUtilities.h"
#include "../../HopsanCore/ComponentEssentials.h"
#include "../../HopsanCore/ComponentUtilities.h"


namespace hopsan {

    class sep2 : public ComponentQ
    {

    private:
        //Declaration of node data pointers, ND is short for NodeData
        double *mpND_F1, *mpND_X1, *mpND_V1, *mpND_Cx1, *mpND_Zx1, *mpND_F2, *mpND_X2, *mpND_V2, *mpND_Cx2, *mpND_Zx2;
//        double X1S, X2S, V1S, V2S, F1S, F2S;
        bool mCONTACT;
        Integrator mXINT1, mXINT2;
        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new sep2();
        }

        sep2() : ComponentQ()
        {
            //Set member attributes
            mTypeName = "sep2";

//            //Startvalues
//            X2S = 0;
//            X1S = 0;
//            V1S = 0;
//            V2S = 0;
//            F1S = 0;
//            F2S = 0;


            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanic");
            mpP2 = addPowerPort("P2", "NodeMechanic");

            //Register parameters to be seen in simulation environment.
//            registerParameter("Node 1 position", "Position", "[m]",   X1S);
//            registerParameter("Node 2 position", "Position", "[m]",   X2S);
//            registerParameter("Node 1 velocity", "Velocity", "[m/s]",   V1S);
//            registerParameter("Node 2 velocity", "Velocity", "[m/s]",   V2S);
//            registerParameter("Node 1 force", "Force", "[N]",   F1S);
//            registerParameter("Node 2 force", "Force", "[N]",   F2S);

        }


        void initialize()
        {
            //Assign node data pointeres
            mpND_F1 = getSafeNodeDataPtr(mpP1, NodeMechanic::FORCE);
            mpND_X1 = getSafeNodeDataPtr(mpP1, NodeMechanic::POSITION);
            mpND_V1 = getSafeNodeDataPtr(mpP1, NodeMechanic::VELOCITY);
            mpND_Cx1 = getSafeNodeDataPtr(mpP1, NodeMechanic::WAVEVARIABLE);
            mpND_Zx1 = getSafeNodeDataPtr(mpP1, NodeMechanic::CHARIMP);
            mpND_F2 = getSafeNodeDataPtr(mpP2, NodeMechanic::FORCE);
            mpND_X2 = getSafeNodeDataPtr(mpP2, NodeMechanic::POSITION);
            mpND_V2 = getSafeNodeDataPtr(mpP2, NodeMechanic::VELOCITY);
            mpND_Cx2 = getSafeNodeDataPtr(mpP2, NodeMechanic::WAVEVARIABLE);
            mpND_Zx2 = getSafeNodeDataPtr(mpP2, NodeMechanic::CHARIMP);

            //Read node startvalues values from ports.
            double F1 = *mpND_F1;
            double V1 = *mpND_V1;
            double X1 = *mpND_X1;
            double F2 = *mpND_F2;
            double V2 = *mpND_V2;
            double X2 = *mpND_X2;
//            double Zx1  = pP1->readNode(NodeMechanic::CHARIMP);
//            double Cx1  = pP1->readNode(NodeMechanic::WAVEVARIABLE);
//            double Zx2  = pP2->readNode(NodeMechanic::CHARIMP);
//            double Cx2  = pP2->readNode(NodeMechanic::WAVEVARIABLE);
            double Zx1 = *mpND_Zx1;
            double Cx1 = *mpND_Cx1;
            double Zx2 = *mpND_Zx2;
            double Cx2 = *mpND_Cx2;

            //If impedance = 0, let Characteristic = Force on node.
//            if(Zx1==0) Cx1 = F1;
//            if(Zx2==0) Cx2 = F2;
//!         @warning DONT do  Zx1 == 0,  Zx1 is a floating point number, there is no gurante that 0 == 0 is true.
//!         0 might actually be 0.00000000000000000000000001 or -0.00000000000000000000000001 which are not the same
            if(fabs(Zx1) < 1e-10) { Cx1 = F1;}
            if(fabs(Zx2) < 1e-10) { Cx2 = F2;}


            //Determine if contact or not.
            mCONTACT = ((X1+X2 <= 0 ? true : false));

            mXINT1.initialize(mTimestep, V1, X1);
            mXINT2.initialize(mTimestep, V2, X2);

            //Write new values to nodes               
//            pP1->writeNode(NodeMechanic::POSITION, X1S);
//            pP2->writeNode(NodeMechanic::POSITION, X2S);
//            pP1->writeNode(NodeMechanic::VELOCITY, V1S);
//            pP2->writeNode(NodeMechanic::VELOCITY, V2S);
//            pP1->writeNode(NodeMechanic::FORCE, F1S);
//            pP2->writeNode(NodeMechanic::FORCE, F2S);
//            pP1->writeNode(NodeMechanic::WAVEVARIABLE, Cx1);
//            pP2->writeNode(NodeMechanic::WAVEVARIABLE, Cx2);
//!         @note No point in writing the other variables back to nodes as we have not changed them
            *mpND_Cx1 = Cx1;
            *mpND_Cx2 = Cx2;
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
//            double Zx1  = pP1->readNode(NodeMechanic::CHARIMP);
//            double Cx1  = pP1->readNode(NodeMechanic::WAVEVARIABLE);
//            double Zx2  = pP2->readNode(NodeMechanic::CHARIMP);
//            double Cx2  = pP2->readNode(NodeMechanic::WAVEVARIABLE);
            double Zx1 = *mpND_Zx1;
            double Cx1 = *mpND_Cx1;
            double Zx2 = *mpND_Zx2;
            double Cx2 = *mpND_Cx2;

            //Read all valuables from ports.
            double X1, V1=0, F1, X2, V2=0, F2;

            //If nodes are in contact:
            if(mCONTACT)
            {
                V2=(Cx1-Cx2)/(Zx1+Zx2);
                V1=-V2;
                F1=Cx1+Zx1*V1;
                F2=Cx2+Zx2*V2;

                if(F1<0 || F2<0) mCONTACT=false;

                if(F1<0) F1=0;
                if(F2<0) F2=0;

                X1=mXINT1.update(V1);
                X2=mXINT2.update(V2);
            }

            //If nodes are NOT in contact:
            else
            {
                //No contact force.
                F1=0;
                F2=0;

                //check to avoid division with 0 when calculatin v.
                if(Zx1>0) V1=-Cx1/Zx1;
                if(Zx2>0) V2=-Cx2/Zx2;

                //Integrate to find x
                X1=mXINT1.update(V1);
                X2=mXINT2.update(V2);

                //Determine if contact or not.
                mCONTACT = ((X1+X2 <= 0 ? true : false));
            }

            //Write new values to nodes
//            pP1->writeNode(NodeMechanic::FORCE, F1);
//            pP2->writeNode(NodeMechanic::FORCE, F2);
//            pP1->writeNode(NodeMechanic::VELOCITY, V1);
//            pP2->writeNode(NodeMechanic::VELOCITY, V2);
//            pP1->writeNode(NodeMechanic::POSITION, X1);
//            pP2->writeNode(NodeMechanic::POSITION, X2);
            *mpND_F1 = F1;
            *mpND_F2 = F2;
            *mpND_V1 = V1;
            *mpND_V2 = V2;
            *mpND_X1 = X1;
            *mpND_X2 = X2;
        }
    };
}

#endif // SEP2_HPP_INCLUDED

