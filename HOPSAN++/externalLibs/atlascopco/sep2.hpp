/*****************************************************************

Mechanic contact
Translated from old hopsan gXGCON.

Erik Jakobsson
20100916


Schematic image:

---| |---

*****************************************************************/
#ifndef SEP2_HPP_INCLUDED
#define SEP2_HPP_INCLUDED


#include "../../HopsanCore/ComponentEssentials.h"
#include "../../HopsanCore/ComponentUtilities.h"
//#include "../../HopsanCore/ComponentUtilities/Integrator.hpp"

namespace hopsan {

    class sep2 : public ComponentQ
    {

    private:
        double X1S, X2S, V1S, V2S, F1S, F2S;
        bool CONTACT;
        Integrator XINT1, XINT2;
        Port *pP1, *pP2;

    public:
        static Component *Creator()
        {
            return new sep2("sep2");
        }

        sep2(const std::string name) : ComponentQ(name)
        {
            //Set member attributes
            mTypeName = "sep2";

            //Startvalues
            X2S = 0;
            X1S = 0;
            V1S = 0;
            V2S = 0;
            F1S = 0;
            F2S = 0;


            //Add ports to the component
            pP1 = addPowerPort("P1", "NodeMechanic");
            pP2 = addPowerPort("P2", "NodeMechanic");

            //Register parameters to be seen in simulation environment.
            registerParameter("Node 1 position", "Position", "[m]",   X1S);
            registerParameter("Node 2 position", "Position", "[m]",   X2S);
            registerParameter("Node 1 velocity", "Velocity", "[m/s]",   V1S);
            registerParameter("Node 2 velocity", "Velocity", "[m/s]",   V2S);
            registerParameter("Node 1 force", "Force", "[N]",   F1S);
            registerParameter("Node 2 force", "Force", "[N]",   F2S);

        }


        void initialize()
        {
            //Read all values from ports.
            //STARTVALUEHANDLING NOT COMPLETE, SINCE WE'RE WAITING FOR LiTH!

            double Zx1  = pP1->readNode(NodeMechanic::CHARIMP);
            double Cx1  = pP1->readNode(NodeMechanic::WAVEVARIABLE);
            double Zx2  = pP2->readNode(NodeMechanic::CHARIMP);
            double Cx2  = pP2->readNode(NodeMechanic::WAVEVARIABLE);

            //If impedance = 0, let Characteristic = Force on node.
            if(Zx1==0) Cx1 = F1S;
            if(Zx2==0) Cx2 = F2S;


            //Determine if contact or not.
            CONTACT = ((X1S+X2S <= 0 ? true : false));

            XINT1.initialize(mTimestep, V1S, X1S);
            XINT2.initialize(mTimestep, V2S, X2S);

            //Write new values to nodes               
            pP1->writeNode(NodeMechanic::POSITION, X1S);
            pP2->writeNode(NodeMechanic::POSITION, X2S);
            pP1->writeNode(NodeMechanic::VELOCITY, V1S);
            pP2->writeNode(NodeMechanic::VELOCITY, V2S);
            pP1->writeNode(NodeMechanic::FORCE, F1S);
            pP2->writeNode(NodeMechanic::FORCE, F2S);
            pP1->writeNode(NodeMechanic::WAVEVARIABLE, Cx1);
            pP2->writeNode(NodeMechanic::WAVEVARIABLE, Cx2);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double Zx1  = pP1->readNode(NodeMechanic::CHARIMP);
            double Cx1  = pP1->readNode(NodeMechanic::WAVEVARIABLE);
            double Zx2  = pP2->readNode(NodeMechanic::CHARIMP);
            double Cx2  = pP2->readNode(NodeMechanic::WAVEVARIABLE);

            //Read all valuables from ports.
            double X1, V1=0, F1, X2, V2=0, F2;

            //If nodes are in contact:
            if(CONTACT)
            {
                V2=(Cx1-Cx2)/(Zx1+Zx2);
                V1=-V2;
                F1=Cx1+Zx1*V1;
                F2=Cx2+Zx2*V2;

                if(F1<0 || F2<0) CONTACT=false;

                if(F1<0) F1=0;
                if(F2<0) F2=0;

                X1=XINT1.update(V1);
                X2=XINT2.update(V2);
            }

            //If nodes are NOT in contact:
            else
            {
                //No contact force.
                F1=0;
                F2=0;

                //check to avoid division with 0 when calculatin v.
                if(Zx1>0) V1=-Cx1/Zx1;
                if(Zx2>0) V2=-Cx2/Zx2;

                //Integrate to find x
                X1=XINT1.update(V1);
                X2=XINT2.update(V2);

                //Determine if contact or not.
                CONTACT = ((X1+X2 <= 0 ? true : false));
            }

            //Write new values to nodes
            pP1->writeNode(NodeMechanic::FORCE, F1);
            pP2->writeNode(NodeMechanic::FORCE, F2);
            pP1->writeNode(NodeMechanic::VELOCITY, V1);
            pP2->writeNode(NodeMechanic::VELOCITY, V2);
            pP1->writeNode(NodeMechanic::POSITION, X1);
            pP2->writeNode(NodeMechanic::POSITION, X2);
        }
    };
}

#endif // SEP2_HPP_INCLUDED

