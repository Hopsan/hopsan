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
        double X1S, F1S, V1S;
        Integrator XINT;
        double F1,Zx1,Cx1,V1,X1;
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
            X1S = 0;
            V1S = 0;
            F1S = 0;

            //Add ports to the component
            pP1 = addPowerPort("P1", "NodeMechanic");
            pIN = addReadPort("IN", "NodeSignal", Port::NOTREQUIRED);

            //Register parameters to be seen in simulation environment.
            registerParameter("Position", "startvalue", "[m]",   X1S);
            registerParameter("Velocity", "startvalue", "[m/s]",   V1S);
            registerParameter("Force", "startvalue", "[N]",   F1S);
        }


        void initialize()
        {
            double F1;

            if(pIN->isConnected())
                F1  = pIN->readNode(NodeSignal::VALUE);
            else
                F1=F1S;

            //Initiate the integrator
            XINT.initialize(mTimestep, V1S, X1S);

            //STARTVALUEHANDLING NOT COMPLETE, SINCE WE'RE WAITING FOR LiTH!
            pP1->writeNode(NodeMechanic::POSITION, X1S);
            pP1->writeNode(NodeMechanic::VELOCITY, V1S);
            pP1->writeNode(NodeMechanic::FORCE, F1);
        }

        void simulateOneTimestep()
        {
            //Get variable values from nodes
            Zx1  = pP1->readNode(NodeMechanic::CHARIMP);
            Cx1  = pP1->readNode(NodeMechanic::WAVEVARIABLE);

            //If signal port is connected, read the value from the port.
            //else use the start value (F1S never changed).
            if(pIN->isConnected())
                F1  = pIN->readNode(NodeSignal::VALUE);
            else
                F1=F1S;

            //Calculate velocity of source.
            V1 = (F1-Cx1)/Zx1;
            //Calculate position by integrating velocity.
            X1 = XINT.update(V1);

            //Write new values to nodes
            pP1->writeNode(NodeMechanic::FORCE, F1);
            pP1->writeNode(NodeMechanic::VELOCITY, V1);
            pP1->writeNode(NodeMechanic::POSITION, X1);

        }
    };
}

#endif // FSRC_HPP_INCLUDED

