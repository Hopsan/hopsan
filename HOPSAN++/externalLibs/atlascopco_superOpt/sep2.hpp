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

namespace hopsan {

    class sep2 : public ComponentQ
    {

    private:
        double X1S, X2S, V1S, V2S, F1S, F2S;
        bool CONTACT;
        Port *pP1, *pP2;

        double *Cx1, *Zx1, *Cx2, *Zx2;
        double *X1, *V1, *F1, *X2, *V2, *F2;

        double X1IntegratorDelayU;
        double X1IntegratorDelayY;
        double X2IntegratorDelayU;
        double X2IntegratorDelayY;

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

            Cx1 = pP1->getNodeDataPtr(NodeMechanic::WAVEVARIABLE);
            Zx1 = pP1->getNodeDataPtr(NodeMechanic::CHARIMP);
            Cx2 = pP2->getNodeDataPtr(NodeMechanic::WAVEVARIABLE);
            Zx2 = pP2->getNodeDataPtr(NodeMechanic::CHARIMP);

            X1 = pP1->getNodeDataPtr(NodeMechanic::POSITION);
            V1 = pP1->getNodeDataPtr(NodeMechanic::VELOCITY);
            F1 = pP1->getNodeDataPtr(NodeMechanic::FORCE);
            X2 = pP2->getNodeDataPtr(NodeMechanic::POSITION);
            V2 = pP2->getNodeDataPtr(NodeMechanic::VELOCITY);
            F2 = pP2->getNodeDataPtr(NodeMechanic::FORCE);


            //Read all values from ports.
            //STARTVALUEHANDLING NOT COMPLETE, SINCE WE'RE WAITING FOR LiTH!

            //If impedance = 0, let Characteristic = Force on node.
            if(*Zx1==0) *Cx1 = F1S;
            if(*Zx2==0) *Cx2 = F2S;


            //Determine if contact or not.
            CONTACT = ((X1S+X2S <= 0 ? true : false));

            //XINT1.initialize(mTime, mTimestep, V1S, X1S);
            //XINT2.initialize(mTime, mTimestep, V2S, X2S);

            X1IntegratorDelayU = V1S;
            X1IntegratorDelayY = X1S;
            X2IntegratorDelayU = V2S;
            X2IntegratorDelayY = X2S;

            *X1 = X1S;
            *X2 = X2S;
            *V1 = V1S;
            *V2 = V2S;
            *F1 = F1S;
            *F2 = F2S;
        }


        void simulateOneTimestep()
        {
            *V1 = 0;
            *V2 = 0;




            //If nodes are in contact:
            if(CONTACT)
            {
                *V2=(*Cx1 - *Cx2)/(*Zx1 + *Zx2);
                *V1 = -*V2;
                *F1 = *Cx1 + *Zx1 * *V1;
                *F2 = *Cx2 + *Zx2 * *V2;

                if(*F1<0 || *F2<0) CONTACT=false;

                if(*F1<0) *F1=0;
                if(*F2<0) *F2=0;

                X1IntegratorDelayY = X1IntegratorDelayY + mTimestep/2 * (*V1 + X1IntegratorDelayU);
                X1IntegratorDelayU = *V1;
                *X1 = X1IntegratorDelayY;

                X2IntegratorDelayY = X2IntegratorDelayY + mTimestep/2 * (*V2 + X2IntegratorDelayU);
                X2IntegratorDelayU = *V2;
                *X2 = X2IntegratorDelayY;
            }

            //If nodes are NOT in contact:
            else
            {
                //No contact force.
                *F1=0;
                *F2=0;

                //check to avoid division with 0 when calculatin v.
                if(*Zx1>0) *V1 = -*Cx1 / *Zx1;
                if(*Zx2>0) *V2 = -*Cx2 / *Zx2;

                //Integrate to find x
                X1IntegratorDelayY = X1IntegratorDelayY + mTimestep/2 * (*V1 + X1IntegratorDelayU);
                X1IntegratorDelayU = *V1;
                *X1 = X1IntegratorDelayY;

                X2IntegratorDelayY = X2IntegratorDelayY + mTimestep/2 * (*V2 + X2IntegratorDelayU);
                X2IntegratorDelayU = *V2;
                *X2 = X2IntegratorDelayY;

                //Determine if contact or not.
                CONTACT = ((*X1 + *X2 <= 0 ? true : false));
            }
        }
    };
}

#endif // SEP2_HPP_INCLUDED

