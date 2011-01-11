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
        double *F1_ptr, *X1_ptr, *V1_ptr, *Cx1_ptr, *Zx1_ptr, *in_ptr;
        //double X1S, F1S, V1S;
        Integrator XINT;
        Port *pP1, *pIN;

    public:
        static Component *Creator()
        {
            return new fsrc();
        }

        fsrc() : ComponentQ()
        {
            //Set member attributes
            mTypeName = "fsrc";

            //Startvalues
//            X1S = 0;
//            V1S = 0;
//            F1S = 0;

            //Add ports to the component
            pP1 = addPowerPort("P1", "NodeMechanic");
            pIN = addReadPort("IN", "NodeSignal", Port::NOTREQUIRED);

            //Register parameters to be seen in simulation environment.
//            registerParameter("Position", "startvalue", "[m]",   X1S);
//            registerParameter("Velocity", "startvalue", "[m/s]",   V1S);
//            registerParameter("Force", "startvalue", "[N]",   F1S);
        }


        void initialize()
        {
            //Assign node data pointers
            F1_ptr = pP1->getNodeDataPtr(NodeMechanic::FORCE);
            X1_ptr = pP1->getNodeDataPtr(NodeMechanic::POSITION);
            V1_ptr = pP1->getNodeDataPtr(NodeMechanic::VELOCITY);
            Cx1_ptr = pP1->getNodeDataPtr(NodeMechanic::WAVEVARIABLE);
            Zx1_ptr = pP1->getNodeDataPtr(NodeMechanic::CHARIMP);
            if(pIN->isConnected())
            {
                in_ptr = pIN->getNodeDataPtr(NodeSignal::VALUE);
            }
            else
            {
                in_ptr = new double(0.0);
            }

            //Read start node values
            double V1 = *V1_ptr;
            double X1 = *X1_ptr;


            //Initiate the integrator
            XINT.initialize(mTimestep, V1, X1);

            //STARTVALUEHANDLING NOT COMPLETE, SINCE WE'RE WAITING FOR LiTH!
//            pP1->writeNode(NodeMechanic::POSITION, X1S);
//            pP1->writeNode(NodeMechanic::VELOCITY, V1S);
//            pP1->writeNode(NodeMechanic::FORCE, F1);
            //*X1_ptr = X1S;
            //*V1_ptr = V1S;
            *F1_ptr = *in_ptr;
        }

        void simulateOneTimestep()
        {
//            double F1;

            //Get variable values from nodes
//            double Zx1  = pP1->readNode(NodeMechanic::CHARIMP);
//            double Cx1  = pP1->readNode(NodeMechanic::WAVEVARIABLE);
            double Zx1 = *Zx1_ptr;
            double Cx1 = *Cx1_ptr;

//            //If signal port is connected, read the value from the port.
//            //else use the start value (F1S never changed).
//            if(pIN->isConnected())
//                //F1  = pIN->readNode(NodeSignal::VALUE);
//                F1 = *F1_ptr;
//            else
//                F1=F1S;
            double F1 = *in_ptr;

            //Calculate velocity of source.
            double V1 = (F1-Cx1)/Zx1;
            //Calculate position by integrating velocity.
            double X1 = XINT.update(V1);

            //Write new values to nodes
//            pP1->writeNode(NodeMechanic::FORCE, F1);
//            pP1->writeNode(NodeMechanic::VELOCITY, V1);
//            pP1->writeNode(NodeMechanic::POSITION, X1);
            *F1_ptr = F1;
            *X1_ptr = X1;
            *V1_ptr = V1;
        }
    };
}

#endif // FSRC_HPP_INCLUDED

