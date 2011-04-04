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
        Port *mpP1, *mpIN;

        //Declaration of node data pointers, ND is short for NodeData
        double *mpND_F1, *mpND_X1, *mpND_V1, *mpND_Cx1, *mpND_Zx1, *mpND_in;

    public:
        static Component *Creator()
        {
            return new fsrc();
        }

        fsrc() : ComponentQ()
        {
            //Set member attributes

            //Startvalues
//            X1S = 0;
//            V1S = 0;
            F1S = 0;

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanic");
            mpIN = addReadPort("IN", "NodeSignal", Port::NOTREQUIRED);

            //Register parameters to be seen in simulation environment.
//            registerParameter("Position", "startvalue", "[m]",   X1S);
//            registerParameter("Velocity", "startvalue", "[m/s]",   V1S);
            registerParameter("Force", "if NC", "[N]",   F1S);
        }


        void initialize()
        {
            //Assign node data pointers
            mpND_F1 = getSafeNodeDataPtr(mpP1, NodeMechanic::FORCE);
            mpND_X1 = getSafeNodeDataPtr(mpP1, NodeMechanic::POSITION);
            mpND_V1 = getSafeNodeDataPtr(mpP1, NodeMechanic::VELOCITY);
            mpND_Cx1 = getSafeNodeDataPtr(mpP1, NodeMechanic::WAVEVARIABLE);
            mpND_Zx1 = getSafeNodeDataPtr(mpP1, NodeMechanic::CHARIMP);
            mpND_in = getSafeNodeDataPtr(mpIN, NodeSignal::VALUE, F1S);

            //Read start node values
            double V1 = *mpND_V1;
            double X1 = *mpND_X1;


            //Initiate the integrator
            XINT.initialize(mTimestep, V1, X1);

            //STARTVALUEHANDLING NOT COMPLETE, SINCE WE'RE WAITING FOR LiTH!
//            pP1->writeNode(NodeMechanic::POSITION, X1S);
//            pP1->writeNode(NodeMechanic::VELOCITY, V1S);
//            pP1->writeNode(NodeMechanic::FORCE, F1);
            //*X1_ptr = X1S;
            //*V1_ptr = V1S;
            *mpND_F1 = *mpND_in;
        }

        void simulateOneTimestep()
        {
//            double F1;

            //Get variable values from nodes
//            double Zx1  = pP1->readNode(NodeMechanic::CHARIMP);
//            double Cx1  = pP1->readNode(NodeMechanic::WAVEVARIABLE);
            double Zx1 = *mpND_Zx1;
            double Cx1 = *mpND_Cx1;

//            //If signal port is connected, read the value from the port.
//            //else use the start value (F1S never changed).
//            if(pIN->isConnected())
//                //F1  = pIN->readNode(NodeSignal::VALUE);
//                F1 = *F1_ptr;
//            else
//                F1=F1S;
            double F1 = *mpND_in;

            //Calculate velocity of source.
            double V1 = (F1-Cx1)/Zx1;
            //Calculate position by integrating velocity.
            double X1 = XINT.update(V1);

            //Write new values to nodes
//            pP1->writeNode(NodeMechanic::FORCE, F1);
//            pP1->writeNode(NodeMechanic::VELOCITY, V1);
//            pP1->writeNode(NodeMechanic::POSITION, X1);
            *mpND_F1 = F1;
            *mpND_X1 = X1;
            *mpND_V1 = V1;
        }
    };
}

#endif // FSRC_HPP_INCLUDED

