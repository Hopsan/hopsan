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
        double *F1_ptr, *X1_ptr, *V1_ptr, *Cx1_ptr, *Zx1_ptr, *F2_ptr, *X2_ptr, *V2_ptr, *Cx2_ptr, *Zx2_ptr;
//        double X1S, X2S, V1S, V2S, F1S, F2S;
        bool CONTACT;
        Integrator XINT1, XINT2;
        Port *pP1, *pP2;

    public:
        static Component *Creator()
        {
            return new sep2();
        }

        sep2() : ComponentQ()
        {
            //Set member attributes
            mTypeName = "sep2";

            //Startvalues
//            X2S = 0;
//            X1S = 0;
//            V1S = 0;
//            V2S = 0;
//            F1S = 0;
//            F2S = 0;


            //Add ports to the component
            pP1 = addPowerPort("P1", "NodeMechanic");
            pP2 = addPowerPort("P2", "NodeMechanic");

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
            F1_ptr = pP1->getNodeDataPtr(NodeMechanic::FORCE);
            X1_ptr = pP1->getNodeDataPtr(NodeMechanic::POSITION);
            V1_ptr = pP1->getNodeDataPtr(NodeMechanic::VELOCITY);
            Cx1_ptr = pP1->getNodeDataPtr(NodeMechanic::WAVEVARIABLE);
            Zx1_ptr = pP1->getNodeDataPtr(NodeMechanic::CHARIMP);
            F2_ptr = pP2->getNodeDataPtr(NodeMechanic::FORCE);
            X2_ptr = pP2->getNodeDataPtr(NodeMechanic::POSITION);
            V2_ptr = pP2->getNodeDataPtr(NodeMechanic::VELOCITY);
            Cx2_ptr = pP2->getNodeDataPtr(NodeMechanic::WAVEVARIABLE);
            Zx2_ptr = pP2->getNodeDataPtr(NodeMechanic::CHARIMP);

            //Read node startvalues values from ports.
            double F1 = *F1_ptr;
            double V1 = *V1_ptr;
            double X1 = *X1_ptr;
            double F2 = *F2_ptr;
            double V2 = *V2_ptr;
            double X2 = *X2_ptr;

//            double Zx1  = pP1->readNode(NodeMechanic::CHARIMP);
//            double Cx1  = pP1->readNode(NodeMechanic::WAVEVARIABLE);
//            double Zx2  = pP2->readNode(NodeMechanic::CHARIMP);
//            double Cx2  = pP2->readNode(NodeMechanic::WAVEVARIABLE);
            double Zx1 = *Zx1_ptr;
            double Cx1 = *Cx1_ptr;
            double Zx2 = *Zx2_ptr;
            double Cx2 = *Cx2_ptr;

            //If impedance = 0, let Characteristic = Force on node.
            if(Zx1==0) Cx1 = F1;
            if(Zx2==0) Cx2 = F2;


            //Determine if contact or not.
            CONTACT = ((X1+X2 <= 0 ? true : false));

            XINT1.initialize(mTimestep, V1, X1);
            XINT2.initialize(mTimestep, V2, X2);

            //Write new values to nodes               
//            pP1->writeNode(NodeMechanic::POSITION, X1S);
//            pP2->writeNode(NodeMechanic::POSITION, X2S);
//            pP1->writeNode(NodeMechanic::VELOCITY, V1S);
//            pP2->writeNode(NodeMechanic::VELOCITY, V2S);
//            pP1->writeNode(NodeMechanic::FORCE, F1S);
//            pP2->writeNode(NodeMechanic::FORCE, F2S);
//            pP1->writeNode(NodeMechanic::WAVEVARIABLE, Cx1);
//            pP2->writeNode(NodeMechanic::WAVEVARIABLE, Cx2);
            //*X1_ptr = X1S;
            //*X2_ptr = X2S;
            //*V1_ptr = V1S;
            //*V2_ptr = V2S;
            //*F1_ptr = F1S;
            //*F2_ptr = F2S;
            *Cx1_ptr = Cx1;
            *Cx2_ptr = Cx2;
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
//            double Zx1  = pP1->readNode(NodeMechanic::CHARIMP);
//            double Cx1  = pP1->readNode(NodeMechanic::WAVEVARIABLE);
//            double Zx2  = pP2->readNode(NodeMechanic::CHARIMP);
//            double Cx2  = pP2->readNode(NodeMechanic::WAVEVARIABLE);
            double Zx1 = *Zx1_ptr;
            double Cx1 = *Cx1_ptr;
            double Zx2 = *Zx2_ptr;
            double Cx2 = *Cx2_ptr;

            //Read all valuables from ports.
            double X1, V1=0, F1, X2, V2=0, F2;

            //If nodes are in contact:
            if(CONTACT)
            {
                V2=(Cx1-Cx2)/(Zx1+Zx2);
                V1=-V2;
                F1=Cx1+Zx1*V1;
                F2=Cx2+Zx2*V2;

                if(F1<0 || F2<0) CONTACT=false;

                if(F1<0) F1=0;
                if(F2<0) F2=0;

                X1=XINT1.update(V1);
                X2=XINT2.update(V2);
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
                X1=XINT1.update(V1);
                X2=XINT2.update(V2);

                //Determine if contact or not.
                CONTACT = ((X1+X2 <= 0 ? true : false));
            }

            //Write new values to nodes
//            pP1->writeNode(NodeMechanic::FORCE, F1);
//            pP2->writeNode(NodeMechanic::FORCE, F2);
//            pP1->writeNode(NodeMechanic::VELOCITY, V1);
//            pP2->writeNode(NodeMechanic::VELOCITY, V2);
//            pP1->writeNode(NodeMechanic::POSITION, X1);
//            pP2->writeNode(NodeMechanic::POSITION, X2);
            *F1_ptr = F1;
            *F2_ptr = F2;
            *X1_ptr = X1;
            *X2_ptr = X2;
            *V1_ptr = V1;
            *V2_ptr = V2;
        }
    };
}

#endif // SEP2_HPP_INCLUDED

