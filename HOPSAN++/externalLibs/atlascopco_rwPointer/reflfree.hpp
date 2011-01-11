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
            double X1  = pP1->readNode(NodeMechanic::POSITION);
            double V1  = pP1->readNode(NodeMechanic::VELOCITY);
            Int.initialize(mTimestep, V1, X1);
        }

        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double Zx1  = pP1->readNode(NodeMechanic::CHARIMP);
            double c1  = pP1->readNode(NodeMechanic::WAVEVARIABLE);

            //Caracteristic matching equations
            double V1 = -c1/(2.0*Zx1);
            double X1 = Int.update(V1);
            double F1 = c1/2;


            //Write new values to nodes
            pP1->writeNode(NodeMechanic::FORCE, F1);
            pP1->writeNode(NodeMechanic::VELOCITY, V1);
            pP1->writeNode(NodeMechanic::POSITION, X1);

        }
    };
}

#endif // MECHANICFORCETRANSFORMER_HPP_INCLUDED
