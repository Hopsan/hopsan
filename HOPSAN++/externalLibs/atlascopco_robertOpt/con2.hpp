/*****************************************************************

Mechanic contact
Translated from old hopsan XGCON.

Simon Magnusson
20100916


Schematic image:

------

*****************************************************************/


#ifndef CON2_HPP_INCLUDED // 
#define CON2_HPP_INCLUDED //

//#include "P:/Hopsan_ng/Hopsan_latest/include/ComponentEssentials.h"
//#include "P:/Hopsan_ng/Hopsan_latest/include/ComponentUtilities.h"
#include "../../HopsanCore/ComponentEssentials.h"
#include "../../HopsanCore/ComponentUtilities.h"

namespace hopsan {

    class con2 : public ComponentQ
    {
    private:
        double F1, X1, V1, Cx1, Zx1, F2, X2, V2, Cx2, Zx2;
        double *F1_ptr, *X1_ptr, *V1_ptr, *Cx1_ptr, *Zx1_ptr, *F2_ptr, *X2_ptr, *V2_ptr, *Cx2_ptr, *Zx2_ptr;
        Port *pP1, *pP2;
        Integrator Int;
        double length;

    public:
        static Component *Creator()
        {
            return new con2();
        }

        con2() : ComponentQ()
        {

            //Add port to the component
            pP1 = addPowerPort("P1", "NodeMechanic");
            pP2 = addPowerPort("P2", "NodeMechanic");
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

            //Read data from nodes
            F1 = (*F1_ptr);
            X1 = (*X1_ptr);
            V1 = (*V1_ptr);
            Zx1 = (*Zx1_ptr);
            F2 = (*F2_ptr);
            X2 = (*X2_ptr);
            Zx2 = (*Zx2_ptr);

            if (Zx1 == 0) Cx1 = F1;
            if (Zx2 == 0) Cx2 = F2;
            Int.initialize(mTimestep, V1, X1);
            length = X2-X1;

            //Write data to nodes
            (*Cx1_ptr) = Cx1;
            (*Cx2_ptr) = Cx2;
        }

        void simulateOneTimestep() // Here is the actual simulation call.
        {
            //Get variable values from nodes
            Cx1 = (*Cx1_ptr);
            Zx1 = (*Zx1_ptr);
            Cx2 = (*Cx2_ptr);
            Zx2 = (*Zx2_ptr);

            //Equations
            V2 = (Cx1-Cx2)/((Zx1+Zx2)); // !!!!!!!!!!if Zx==0 notify user!!!!!!!!!
            V1 = -V2;
            X1 = Int.update(V1);
            X2 = length-X1;
            F1 = Cx1 + V1*Zx1;
            F2 = Cx2 + V2*Zx2;

            //Write new values to nodes
            (*F1_ptr) = F1;
            (*X1_ptr) = X1;
            (*F1_ptr) = V1;
            (*F2_ptr) = F2;
            (*X2_ptr) = X2;
            (*V2_ptr) = V2;
        }


        void finalize() // In this method (function) you can finalize anything for your component. It will be called after each time a simulation has finished.
        {
            //Nothing to finalize.
        }
    };
}

#endif
