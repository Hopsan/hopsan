/*****************************************************************

Force
Translated from old hopsan FSRC.

Erik Jakobsson
20101005

Schematic image:

  -->

*****************************************************************/
#ifndef FSRC_HPP_INCLUDED
#define FSRC_HPP_INCLUDED

//#include "P:/Hopsan_ng/Hopsan_latest/include/ComponentEssentials.h"
//#include "P:/Hopsan_ng/Hopsan_latest/include/ComponentUtilities.h"
#include "../../HopsanCore/ComponentEssentials.h"
#include "../../HopsanCore/ComponentUtilities.h"


namespace hopsan {

    class fsrc : public ComponentQ
    {

    private:
        double F1S;
        Integrator XINT;
        double F1, X1, V1, Cx1, Zx1, in;
        double *F1_ptr, *X1_ptr, *V1_ptr, *Cx1_ptr, *Zx1_ptr, *in_ptr;
        Port *pP1, *pIN;

    public:
        static Component *Creator()
        {
            return new fsrc();
        }

        fsrc() : ComponentQ()
        {
            //Set member attributes

            //Startvalues
            F1S = 0;

            //Add ports to the component
            pP1 = addPowerPort("P1", "NodeMechanic");
            pIN = addReadPort("IN", "NodeSignal", Port::NOTREQUIRED);

            //Register parameters to be seen in simulation environment.
            registerParameter("Force", "startvalue", "[N]",   F1S);
        }


        void initialize()
        {
            //Assign node data pointers
            F1_ptr = pP1->getNodeDataPtr(NodeMechanic::FORCE);
            X1_ptr = pP1->getNodeDataPtr(NodeMechanic::POSITION);
            V1_ptr = pP1->getNodeDataPtr(NodeMechanic::VELOCITY);
            Cx1_ptr = pP1->getNodeDataPtr(NodeMechanic::WAVEVARIABLE);
            Zx1_ptr = pP1->getNodeDataPtr(NodeMechanic::CHARIMP);
            if(pIN->isConnected()) { in_ptr = pIN->getNodeDataPtr(NodeSignal::VALUE); }
            else { in_ptr = new double(F1S); }

            //Read values from nodes
            in = (*in_ptr);
            X1 = (*X1_ptr);
            V1 = (*V1_ptr);

            //Initiate the integrator
            XINT.initialize(mTimestep, V1, X1);

            //Write values to nodes
            (*F1_ptr) = in;
        }

        void simulateOneTimestep()
        {
            //Get variable values from nodes
            Cx1 = (*Cx1_ptr);
            Zx1 = (*Zx1_ptr);
            in = (*in_ptr);

            //If signal port is connected, read the value from the port.
            //else use the start value (F1S never changed).
            F1 = in;

            //Calculate velocity of source.
            V1 = (F1-Cx1)/Zx1;
            //Calculate position by integrating velocity.
            X1 = XINT.update(V1);

            //Write new values to nodes
            (*F1_ptr) = F1;
            (*X1_ptr) = X1;
            (*V1_ptr) = V1;

        }
    };
}

#endif // FSRC_HPP_INCLUDED

