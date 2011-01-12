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
        Port *mpP1;

        //Declaration of node data pointers, ND is short for NodeData
        double *mpND_Zx1, *mpND_Cx1, *mpND_X1, *mpND_V1, *mpND_F1;

    public:
        static Component *Creator()
        {
            return new MechanicReflexfreeRock();
        }

        MechanicReflexfreeRock() : ComponentQ()
        {
            //Set member attributes


            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanic");

            //Register changable parameters to the HOPSAN++ core
        }

        void initialize()
        {
            //Assign node data pointers
            mpND_Zx1 = mpP1->getNodeDataPtr(NodeMechanic::CHARIMP);
            mpND_Cx1 = mpP1->getNodeDataPtr(NodeMechanic::WAVEVARIABLE);
            mpND_X1 = mpP1->getNodeDataPtr(NodeMechanic::POSITION);
            mpND_V1 = mpP1->getNodeDataPtr(NodeMechanic::VELOCITY);
            mpND_F1 = mpP1->getNodeDataPtr(NodeMechanic::FORCE);

            //Read values from nodes
//            double X1  = mpP1->readNode(NodeMechanic::POSITION);
//            double V1  = mpP1->readNode(NodeMechanic::VELOCITY);
//!         @note In this case we read directly from the nodedata pointers withouth creating a pointless local variable in between
            Int.initialize(mTimestep, *mpND_V1, *mpND_X1);
        }

        void simulateOneTimestep()
        {
            //Get variable values from nodes
//            double Zx1  = mpP1->readNode(NodeMechanic::CHARIMP);
//            double c1  = mpP1->readNode(NodeMechanic::WAVEVARIABLE);
            double Zx1 = *mpND_Zx1;
            double c1 = *mpND_Cx1;

            //Caracteristic matching equations
            double V1 = -c1/(2.0*Zx1);
            double X1 = Int.update(V1);
            double F1 = c1/2;

            //Write new values to nodes
//            mpP1->writeNode(NodeMechanic::FORCE, F1);
//            mpP1->writeNode(NodeMechanic::VELOCITY, V1);
//            mpP1->writeNode(NodeMechanic::POSITION, X1);
            *mpND_F1 = F1;
            *mpND_V1 = V1;
            *mpND_X1 = X1;
        }
    };
}

#endif // MECHANICFORCETRANSFORMER_HPP_INCLUDED
