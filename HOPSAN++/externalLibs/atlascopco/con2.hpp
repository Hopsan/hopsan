/*****************************************************************

Mechanic contact
Translated from old hopsan XGCON.

Simon Magnusson
20100916


Schematic image:

------

*****************************************************************/


#ifndef CON2_HPP_INCLUDED // 
#define CON2_HPP_INCLUDED //

#include "../../HopsanCore/ComponentEssentials.h"
#include "../../HopsanCore/ComponentUtilities.h"
//#include "../../HopsanCore/ComponentUtilities/Integrator.hpp"

namespace hopsan {

    class con2 : public ComponentQ

    {
    private:

        Port *pP1, *pP2;
        Integrator Int;
        double X10,X20,V10,V20,F10,F20;

    public:
        static Component *Creator()
        {
            return new con2("con2");
        }

        con2(const std::string name) : ComponentQ(name)

        {
            mTypeName = "con2";

            //Add port to the component
            pP1 = addPowerPort("P1", "NodeMechanic");
            pP2 = addPowerPort("P2", "NodeMechanic");

            //Start values
            X10 = 0;
            X20 = 0;
            V10 = 0;
            V20 = 0;
            F10 = 0;
            F20 = 0;
            registerParameter("X1", "Displacement", "[m]", X10);                     
            registerParameter("X2", "Displacement", "[m]", X20); 
            registerParameter("V1", "Velocity", "[m/s]", V10);
            registerParameter("V2", "Velocity", "[m/s]", V20);
            registerParameter("F1", "Force", "[N]", F10);
            registerParameter("F2", "Force", "[N]", F20);
        }


        void initialize() 
        {
            //Initialize start values
            pP1->writeNode(NodeMechanic::POSITION, X10);
            pP2->writeNode(NodeMechanic::POSITION, X20);
            pP1->writeNode(NodeMechanic::VELOCITY, V10);
            pP2->writeNode(NodeMechanic::VELOCITY, V20);
            pP1->writeNode(NodeMechanic::FORCE, F10);
            pP2->writeNode(NodeMechanic::FORCE, F20);
            double Zx1 = pP1->readNode(NodeMechanic::CHARIMP);
            double Zx2 = pP2->readNode(NodeMechanic::CHARIMP);
            double X1  = pP1->readNode(NodeMechanic::POSITION);
            double V1  = pP1->readNode(NodeMechanic::VELOCITY);
            double F1  = pP1->readNode(NodeMechanic::FORCE);
            double F2  = pP1->readNode(NodeMechanic::FORCE);
            if (Zx1 == 0) pP1 ->writeNode(NodeMechanic::WAVEVARIABLE, F1);
            if (Zx2 == 0) pP2 ->writeNode(NodeMechanic::WAVEVARIABLE, F2);
            Int.initialize(mTimestep, V1, X1);

        }

        void simulateOneTimestep() // Here is the actual simulation call.
        {

            double Cx1 = pP1->readNode(NodeMechanic::WAVEVARIABLE);
            double Zx1 = pP1->readNode(NodeMechanic::CHARIMP);
            double Cx2 = pP2->readNode(NodeMechanic::WAVEVARIABLE);
            double Zx2 = pP2->readNode(NodeMechanic::CHARIMP);

            //Equations
            double V2 = (Cx1-Cx2)/((Zx1+Zx2)); // !!!!!!!!!!if Zx==0 notify user!!!!!!!!!
            double V1 = -V2;
            double X1 = Int.update(V1);
            double X2 = X20-(X1-X10);
            double F1 = Cx1 + V1*Zx1;
            double F2 = Cx2 + V2*Zx2;

            //Write new values to nodes
            pP1->writeNode(NodeMechanic::POSITION, X1);
            pP2->writeNode(NodeMechanic::POSITION, X2);
            pP1->writeNode(NodeMechanic::VELOCITY, V1);
            pP2->writeNode(NodeMechanic::VELOCITY, V2);
            pP1->writeNode(NodeMechanic::FORCE, F1);
            pP2->writeNode(NodeMechanic::FORCE, F2);
        }


        void finalize() // In this method (function) you can finalize anything for your component. It will be called after each time a simulation has finished.
        {
            //Nothing to finalize.
        }
    };
}

#endif
