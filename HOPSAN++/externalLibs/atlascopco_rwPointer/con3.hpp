/*****************************************************************

Mechanic connection
Translated from old hopsan X3GCON.

Simon Magnusson
20100916


Schematic image:
    |---
----|
    |---

*****************************************************************/

#ifndef CON3__HPP_INCLUDED
#define CON3__HPP_INCLUDED

//#include "P:/Hopsan_ng/Hopsan_latest/include/ComponentEssentials.h"
//#include "P:/Hopsan_ng/Hopsan_latest/include/ComponentUtilities.h"
#include "../../HopsanCore/ComponentEssentials.h"
#include "../../HopsanCore/ComponentUtilities.h"

namespace hopsan {

    class con3 : public ComponentQ
    {
    private:

        Port *pP1, *pP2, *pP3;
        Integrator Int;
        double X10,X20,X30,V10,V20,V30, F10,F20,F30;

    public:
        static Component *Creator()
        {
            return new con3();
        }

        con3() : ComponentQ()

        {

            pP1 = addPowerPort("P1", "NodeMechanic");
            pP2 = addPowerPort("P2", "NodeMechanic");
            pP3 = addPowerPort("P3", "NodeMechanic");

                //Start values
            X10 = 0;
            X20 = 0;
            X30 = 0;
            V10 = 0;
            V20 = 0;
            V30 = 0;
            F10 = 0;
            F20 = 0;
            F30 = 0;
            registerParameter("X1", "Displacement", "[m]", X10);
            registerParameter("X2", "Displacement", "[m]", X20);
            registerParameter("X3", "Displacement", "[m]", X30);
            registerParameter("V1", "Velocity", "[m/s]", V10);
            registerParameter("V2", "Velocity", "[m/s]", V20);
            registerParameter("V3", "Velocity", "[m/s]", V30);
        }

        void initialize()
        {
                //Initialize start values
            pP1->writeNode(NodeMechanic::POSITION, X10);
            pP2->writeNode(NodeMechanic::POSITION, X20);
            pP3->writeNode(NodeMechanic::POSITION, X30);
            pP1->writeNode(NodeMechanic::VELOCITY, V10);
            pP2->writeNode(NodeMechanic::VELOCITY, V20);
            pP3->writeNode(NodeMechanic::VELOCITY, V30);
            double Zx1 = pP1->readNode(NodeMechanic::CHARIMP);
            double Zx2 = pP2->readNode(NodeMechanic::CHARIMP);
            double Zx3 = pP2->readNode(NodeMechanic::CHARIMP);
            double X1  = pP1->readNode(NodeMechanic::POSITION);
            double V1  = pP1->readNode(NodeMechanic::VELOCITY);
            double F1  = pP1->readNode(NodeMechanic::FORCE);
            double F2  = pP1->readNode(NodeMechanic::FORCE);
            double F3  = pP1->readNode(NodeMechanic::FORCE);
            if (Zx1 == 0) pP1 ->writeNode(NodeMechanic::WAVEVARIABLE, F1);
            if (Zx2 == 0) pP2 ->writeNode(NodeMechanic::WAVEVARIABLE, F2);
            if (Zx3 == 0) pP2 ->writeNode(NodeMechanic::WAVEVARIABLE, F3);
            Int.initialize(mTimestep, V1, X1);

        }

        void simulateOneTimestep() // Here is the actual simulation call.
        {

            double Cx1 = pP1->readNode(NodeMechanic::WAVEVARIABLE);
            double Zx1 = pP1->readNode(NodeMechanic::CHARIMP);
            double Cx2 = pP2->readNode(NodeMechanic::WAVEVARIABLE);
            double Zx2 = pP2->readNode(NodeMechanic::CHARIMP);
            double Cx3 = pP3->readNode(NodeMechanic::WAVEVARIABLE);
            double Zx3 = pP3->readNode(NodeMechanic::CHARIMP);

            //Equations
            double V1 = (-Cx1+Cx2+Cx3)/((Zx1+Zx2+Zx3)); // !!!!!!!!!!if Zx==0 notify user!!!!!!!!!
            double V2 = -V1;
            double V3 = V2;
            double X1 = Int.update(V1);
            double X2 = X20-(X1-X10);
            double X3 = X30-(X1-X10);
            double F1 = Cx1 + V1*Zx1;
            double F2 = Cx2 + V2*Zx2;
            double F3 = Cx3 + V3*Zx3;

            //Write new values to node
            pP1->writeNode(NodeMechanic::POSITION, X1);
            pP2->writeNode(NodeMechanic::POSITION, X2);
            pP3->writeNode(NodeMechanic::POSITION, X3);
            pP1->writeNode(NodeMechanic::VELOCITY, V1);
            pP2->writeNode(NodeMechanic::VELOCITY, V2);
            pP3->writeNode(NodeMechanic::VELOCITY, V3);
            pP1->writeNode(NodeMechanic::FORCE, F1);
            pP2->writeNode(NodeMechanic::FORCE, F2);
            pP3->writeNode(NodeMechanic::FORCE, F3);
        }


        void finalize()
        {
            //Nothing to finalize.
        }
    };
}

#endif
