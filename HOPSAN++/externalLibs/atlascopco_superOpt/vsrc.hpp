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

#include "../../HopsanCore/ComponentEssentials.h"
#include "../../HopsanCore/ComponentUtilities.h"

namespace hopsan {

    class vsrc : public ComponentQ
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
            return new vsrc("vsrc");
        }

        vsrc(const std::string name) : ComponentQ(name)
        {
            //Set member attributes
            mTypeName = "vsrc";

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
                V1 = in;
            }
            else
            {
                *V1=V1S;
            }

            //Initiate the integrator
            XIntegratorDelayU = *V1;
            XIntegratorDelayY = X1S;
        }

        void simulateOneTimestep()
        {
            //Calculate position by integrating velocity.
            XIntegratorDelayY = XIntegratorDelayY + mTimestep/2 * (*V1 + XIntegratorDelayU);
            XIntegratorDelayU = *V1;
            *X1 = XIntegratorDelayY;

            //Calculate force of source.
            *F1 = *Cx1 + *Zx1 *  *V1;
        }
    };
}

#endif // VSRC_HPP_INCLUDED

