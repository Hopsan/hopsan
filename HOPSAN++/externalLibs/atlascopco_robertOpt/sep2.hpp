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
        bool CONTACT;
        Integrator XINT1, XINT2;
        double F1, X1, V1, Cx1, Zx1, F2, X2, V2, Cx2, Zx2;
        double *F1_ptr, *X1_ptr, *V1_ptr, *Cx1_ptr, *Zx1_ptr, *F2_ptr, *X2_ptr, *V2_ptr, *Cx2_ptr, *Zx2_ptr;
        Port *pP1, *pP2;

    public:
        static Component *Creator()
        {
            return new sep2();
        }

        sep2() : ComponentQ()
        {
            //Set member attributes

            //Add ports to the component
            pP1 = addPowerPort("P1", "NodeMechanic");
            pP2 = addPowerPort("P2", "NodeMechanic");
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

            //Read all values from ports.
            //STARTVALUEHANDLING NOT COMPLETE, SINCE WE'RE WAITING FOR LiTH!

            F1 = (*F1_ptr);
            X1 = (*X1_ptr);
            V1 = (*V1_ptr);
            Cx1 = (*Cx1_ptr);
            Zx1 = (*Zx1_ptr);
            F2 = (*F2_ptr);
            X2 = (*X2_ptr);
            V1 = (*V1_ptr);
            Cx2 = (*Cx2_ptr);
            Zx2 = (*Zx2_ptr);

            //If impedance = 0, let Characteristic = Force on node.
            if(Zx1==0) Cx1 = F1;
            if(Zx2==0) Cx2 = F2;

            //Determine if contact or not.
            CONTACT = ((X1+X2 <= 0 ? true : false));

            XINT1.initialize(mTimestep, V1, X1);
            XINT2.initialize(mTimestep, V2, X2);

            //Write new values to nodes
            (*Cx1_ptr) = Cx1;
            (*Cx2_ptr) = Cx2;
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            Cx1 = (*Cx1_ptr);
            Zx1 = (*Zx1_ptr);
            Cx2 = (*Cx2_ptr);
            Zx2 = (*Zx2_ptr);


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
            (*F1_ptr) = F1;
            (*F2_ptr) = F2;
            (*X1_ptr) = X1;
            (*X2_ptr) = X2;
            (*V1_ptr) = V1;
            (*V2_ptr) = V2;
        }
    };
}

#endif // SEP2_HPP_INCLUDED

