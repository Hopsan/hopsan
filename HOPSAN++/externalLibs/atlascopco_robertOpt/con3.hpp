/*****************************************************************

Mechanic connection
Translated from old hopsan X3GCON.

Simon Magnusson
20100916


Schematic image:
    |---
----|
    |---

*****************************************************************/

#ifndef CON3__HPP_INCLUDED
#define CON3__HPP_INCLUDED

//#include "P:/Hopsan_ng/Hopsan_latest/include/ComponentEssentials.h"
//#include "P:/Hopsan_ng/Hopsan_latest/include/ComponentUtilities.h"
#include "../../HopsanCore/ComponentEssentials.h"
#include "../../HopsanCore/ComponentUtilities.h"

namespace hopsan {

    class con3 : public ComponentQ
    {
    private:
        double F1, X1, V1, Cx1, Zx1, F2, X2, V2, Cx2, Zx2, F3, X3, V3, Cx3, Zx3;
        double *F1_ptr, *X1_ptr, *V1_ptr, *Cx1_ptr, *Zx1_ptr;
        double *F2_ptr, *X2_ptr, *V2_ptr, *Cx2_ptr, *Zx2_ptr;
        double *F3_ptr, *X3_ptr, *V3_ptr, *Cx3_ptr, *Zx3_ptr;
        Port *pP1, *pP2, *pP3;
        Integrator Int;
        double length21, length31;

    public:
        static Component *Creator()
        {
            return new con3();
        }

        con3() : ComponentQ()

        {
            mTypeName = "con3";

            pP1 = addPowerPort("P1", "NodeMechanic");
            pP2 = addPowerPort("P2", "NodeMechanic");
            pP3 = addPowerPort("P3", "NodeMechanic");
        }

        void initialize()
        {
            //Assign node data pointers
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
            F3_ptr = pP3->getNodeDataPtr(NodeMechanic::FORCE);
            X3_ptr = pP3->getNodeDataPtr(NodeMechanic::POSITION);
            V3_ptr = pP3->getNodeDataPtr(NodeMechanic::VELOCITY);
            Cx3_ptr = pP3->getNodeDataPtr(NodeMechanic::WAVEVARIABLE);
            Zx3_ptr = pP3->getNodeDataPtr(NodeMechanic::CHARIMP);

            //Read data from nodes
            F1 = (*F1_ptr);
            X1 = (*X1_ptr);
            V1 = (*V1_ptr);
            Zx1 = (*Zx1_ptr);
            F2 = (*F2_ptr);
            X2 = (*X2_ptr);
            Zx2 = (*Zx2_ptr);
            F3 = (*F3_ptr);
            X3 = (*X3_ptr);
            Zx3 = (*Zx3_ptr);

            if (Zx1 == 0) Cx1 = F1;
            if (Zx2 == 0) Cx2 = F2;
            if (Zx3 == 0) Cx3 = F3;
            Int.initialize(mTimestep, V1, X1);
            length21 = X2-X1;
            length31 = X3-X1;

            //Write data to nodes
            (*Cx1_ptr) = Cx1;
            (*Cx2_ptr) = Cx2;
            (*Cx3_ptr) = Cx3;
        }

        void simulateOneTimestep() // Here is the actual simulation call.
        {
            //Get variable values from nodes
            Cx1 = (*Cx1_ptr);
            Zx1 = (*Zx1_ptr);
            Cx2 = (*Cx2_ptr);
            Zx2 = (*Zx2_ptr);
            Cx3 = (*Cx3_ptr);
            Zx3 = (*Zx3_ptr);

            //Equations
            V1 = (-Cx1+Cx2+Cx3)/((Zx1+Zx2+Zx3)); // !!!!!!!!!!if Zx==0 notify user!!!!!!!!!
            V2 = -V1;
            V3 = V2;
            X1 = Int.update(V1);
            X2 = length21-X1;
            X3 = length31-X1;
            F1 = Cx1 + V1*Zx1;
            F2 = Cx2 + V2*Zx2;
            F3 = Cx3 + V3*Zx3;

            //Write new values to nodes
            (*F1_ptr) = F1;
            (*X1_ptr) = X1;
            (*F1_ptr) = V1;
            (*F2_ptr) = F2;
            (*X2_ptr) = X2;
            (*V2_ptr) = V2;
            (*F3_ptr) = F3;
            (*X3_ptr) = X3;
            (*V3_ptr) = V3;
        }


        void finalize()
        {
            //Nothing to finalize.
        }
    };
}

#endif
