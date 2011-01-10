/*****************************************************************

reflfree
Translated from old hopsan noreflex.

Thomas Johansson
20101006

Schematic image:
 -
|+|
 -
Termination giving force and displacement matched to characteristics, avoiding reflections.
*****************************************************************/

#ifndef MECHANICREFLEXFREEROCK_HPP_INCLUDED
#define MECHANICREFLEXFREEROCK_HPP_INCLUDED

//#include "P:/Hopsan_ng/Hopsan_latest/include/ComponentEssentials.h"
//#include "P:/Hopsan_ng/Hopsan_latest/include/ComponentUtilities.h"
#include "../../HopsanCore/ComponentEssentials.h"
#include "../../HopsanCore/ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicReflexfreeRock : public ComponentQ
    {

    private:
        Integrator Int;
        Port *pP1;
        double F1, X1, V1, Cx1, Zx1;
        double *F1_ptr, *X1_ptr, *V1_ptr, *Cx1_ptr, *Zx1_ptr;

    public:
        static Component *Creator()
        {
            return new MechanicReflexfreeRock();
        }

        MechanicReflexfreeRock() : ComponentQ()
        {
            //Set member attributes


            //Add ports to the component
            pP1 = addPowerPort("P1", "NodeMechanic");

            //Register changable parameters to the HOPSAN++ core
        }

        void initialize()
        {
            //Assign node data pointers
            F1_ptr = pP1->getNodeDataPtr(NodeMechanic::FORCE);
            X1_ptr = pP1->getNodeDataPtr(NodeMechanic::POSITION);
            V1_ptr = pP1->getNodeDataPtr(NodeMechanic::VELOCITY);
            Cx1_ptr = pP1->getNodeDataPtr(NodeMechanic::WAVEVARIABLE);
            Zx1_ptr = pP1->getNodeDataPtr(NodeMechanic::CHARIMP);

            //Read values from nodes
            X1 = (*X1_ptr);
            V1 = (*V1_ptr);

            Int.initialize(mTimestep, V1, X1);
        }

        void simulateOneTimestep()
        {
            //Get variable values from nodes
            Cx1 = (*Cx1_ptr);
            Zx1 = (*Zx1_ptr);

            //Caracteristic matching equations
            V1 = -Cx1/(2.0*Zx1);
            X1 = Int.update(V1);
            F1 = Cx1/2;

            //Write new values to nodes
            (*F1_ptr) = F1;
            (*X1_ptr) = X1;
            (*V1_ptr) = V1;
        }
    };
}

#endif // MECHANICFORCETRANSFORMER_HPP_INCLUDED
