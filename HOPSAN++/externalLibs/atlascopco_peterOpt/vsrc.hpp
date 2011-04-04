/*****************************************************************

Force
Translated from old hopsan stvsxsrc.

Erik Jakobsson
20101005

Schematic image:

  -->

*****************************************************************/
#ifndef VSRC_HPP_INCLUDED
#define VSRC_HPP_INCLUDED

//#include "P:/Hopsan_ng/Hopsan_latest/include/ComponentEssentials.h"
//#include "P:/Hopsan_ng/Hopsan_latest/include/ComponentUtilities.h"
#include "../../HopsanCore/ComponentEssentials.h"
#include "../../HopsanCore/ComponentUtilities.h"

namespace hopsan {

    class vsrc : public ComponentQ
    {

    private:
        //Declaration of node data pointers, ND is short for NodeData
        double *mpND_F1, *mpND_X1, *mpND_V1, *mpND_Cx1, *mpND_Zx1, *mpND_in;
        double mV1S;
        Integrator mXINT;
        Port *mpP1, *mpIN;

    public:
        static Component *Creator()
        {
            return new vsrc();
        }

        vsrc() : ComponentQ()
        {
            //Set member attributes

//            //Startvalues
//            X1S = 0;
            mV1S = 0;
//            F1S = 0;

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanic");
            mpIN = addReadPort("IN", "NodeSignal", Port::NOTREQUIRED);

            //Register parameters to be seen in simulation environment.
//            registerParameter("Position", "startvalue", "[m]",   X1S);
            registerParameter("Velocity", "If NC", "[m/s]",   mV1S);
//            registerParameter("Force", "startvalue", "[N]",   F1S);
        }


        void initialize()
        {
            //Assign node data pointeres
            mpND_F1 = getSafeNodeDataPtr(mpP1, NodeMechanic::FORCE);
            mpND_X1 = getSafeNodeDataPtr(mpP1, NodeMechanic::POSITION);
            mpND_V1 = getSafeNodeDataPtr(mpP1, NodeMechanic::VELOCITY);
            mpND_Cx1 = getSafeNodeDataPtr(mpP1, NodeMechanic::WAVEVARIABLE);
            mpND_Zx1 = getSafeNodeDataPtr(mpP1, NodeMechanic::CHARIMP);
//            double V1;
//            if(pIN->isConnected())
//                V1  = pIN->readNode(NodeSignal::VALUE);
//            else
//                V1=V1S;
            mpND_in = getSafeNodeDataPtr(mpIN, NodeSignal::VALUE, mV1S);

            //read start node values
            double V1 = *mpND_in;
            double X1 = *mpND_X1;

            //Initiate the integrator
            mXINT.initialize(mTimestep, V1, X1);

//            //STARTVALUEHANDLING NOT COMPLETE, SINCE WE'RE WAITING FOR LiTH!
//            pP1->writeNode(NodeMechanic::POSITION, X1S);
//            pP1->writeNode(NodeMechanic::VELOCITY, V1);
//            pP1->writeNode(NodeMechanic::FORCE, F1S);
        }

        void simulateOneTimestep()
        {
//            double V1;

            //Get variable values from nodes
//            double Zx1  = pP1->readNode(NodeMechanic::CHARIMP);
//            double Cx1  = pP1->readNode(NodeMechanic::WAVEVARIABLE);
            double Zx1 = *mpND_Zx1;
            double Cx1 = *mpND_Cx1;

//            //If signal port is connected, read the value from the port.
//            //else use the start value (V1S never changed).
//            if(pIN->isConnected())
////                V1  = pIN->readNode(NodeSignal::VALUE);
//                V1 = *in_ptr;
//            else
//                V1=V1S;
            double V1 = *mpND_in;

            //Calculate position by integrating velocity.
            double X1 = mXINT.update(V1);

            //Calculate force of source.
            double F1 = Cx1 + Zx1 * V1;

            //Write new values to nodes
//            pP1->writeNode(NodeMechanic::FORCE, F1);
//            pP1->writeNode(NodeMechanic::VELOCITY, V1);
//            pP1->writeNode(NodeMechanic::POSITION, X1);
            *mpND_F1 = F1;
            *mpND_V1 = V1;
            *mpND_X1 = X1;
        }
    };
}

#endif // VSRC_HPP_INCLUDED

