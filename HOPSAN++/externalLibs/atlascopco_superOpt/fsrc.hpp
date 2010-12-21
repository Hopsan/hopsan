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

#include "../../HopsanCore/ComponentEssentials.h"
#include "../../HopsanCore/ComponentUtilities.h"

namespace hopsan {

    class fsrc : public ComponentQ
    {

    private:
        double X1S, F1S, V1S;
        Port *pP1, *pIN;

        double *X1, *V1, *F1, *Cx1, *Zx1, *in;

        double XIntegratorDelayU;
        double XIntegratorDelayY;

    public:
        static Component *Creator()
        {
            return new fsrc("fsrc");
        }

        fsrc(const std::string name) : ComponentQ(name)
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

            X1 = pP1->getNodeDataPtr(NodeMechanic::POSITION);
            V1 = pP1->getNodeDataPtr(NodeMechanic::VELOCITY);
            F1 = pP1->getNodeDataPtr(NodeMechanic::FORCE);
            Cx1 = pP1->getNodeDataPtr(NodeMechanic::WAVEVARIABLE);
            Zx1 = pP1->getNodeDataPtr(NodeMechanic::CHARIMP);
            if(pIN->isConnected())
            {
                in = pIN->getNodeDataPtr(NodeSignal::VALUE);
                F1 = in;
            }
            else
            {
                *F1=F1S;
            }

            //Initiate the integrator
            XIntegratorDelayU = V1S;
            XIntegratorDelayY = X1S;


            *X1 = X1S;
            *V1 = V1S;

        }

        void simulateOneTimestep()
        {
            //If signal port is connected, read the value from the port.
            //else use the start value (F1S never changed).


            //Calculate velocity of source.
            *V1 = (*F1 - *Cx1) / *Zx1;
            //Calculate position by integrating velocity.
            XIntegratorDelayY = XIntegratorDelayY + mTimestep/2 * (*V1 + XIntegratorDelayU);
            XIntegratorDelayU = *V1;
            *X1 = XIntegratorDelayY;
        }
    };
}

#endif // FSRC_HPP_INCLUDED

