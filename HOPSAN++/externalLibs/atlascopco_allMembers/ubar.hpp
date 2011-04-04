/*****************************************************************

Shortest possible mechanical delay.

Maria Pettersson
20101005

*****************************************************************/
#ifndef UBAR_HPP_INCLUDED
#define UBAR_HPP_INCLUDED

//#include <iostream>
#include <math.h>
//#include "P:/Hopsan_ng/Hopsan_latest/include/ComponentEssentials.h"
//#include "P:/Hopsan_ng/Hopsan_latest/include/ComponentUtilities.h"
#include "../../HopsanCore/ComponentEssentials.h"
#include "../../HopsanCore/ComponentUtilities.h"

namespace hopsan {

    class ubar : public ComponentC
    {

    private:
        double D;
        double RHOB;
        double EB;
        double Zx;
        double Alpha;
        double Cx1;
        double Cx2;
        double Cx1old;
        double Cx2old;
        double Cx1new;
        double Cx2new;
        double F1;
        double F2;
        double V1;
        double V2;
        Port *pP1, *pP2;

    public:
        static Component *Creator()
        {
            return new ubar();
        }

        ubar() : ComponentC()
        {
            //Set member attributes
            Alpha=0.0;//Filtering of characteristics if >0

            //Add ports to the component
            pP1 = addPowerPort("P1", "NodeMechanic");
            pP2 = addPowerPort("P2", "NodeMechanic");

            //Register changable parameters to the HOPSAN++ core
            D=0.005;
            RHOB=7800.;
            EB=2.1e11;
            registerParameter("D", "Diameter", "m",  D);
            registerParameter("RHOB", "Density", "kg/m3", RHOB);
            registerParameter("EB", "Young's modulus", "Pa", EB);
        }


        void initialize()
        {
            //Startvalues, read force and velocity from connected Q-types
            F1 = pP1->readNode(NodeMechanic::FORCE);
            V1 = pP1->readNode(NodeMechanic::VELOCITY);
            F2 = pP2->readNode(NodeMechanic::FORCE);
            V2 = pP2->readNode(NodeMechanic::VELOCITY);
            Zx=RHOB*sqrt(EB/RHOB)*3.141593*D*D/4.0;
            Cx1=F1-Zx*V1;
            Cx2=F2-Zx*V2;

            //Write characteristics to nodes
            pP1->writeNode(NodeMechanic::WAVEVARIABLE, Cx1);
            pP1->writeNode(NodeMechanic::CHARIMP,      Zx);
            pP2->writeNode(NodeMechanic::WAVEVARIABLE, Cx2);
            pP2->writeNode(NodeMechanic::CHARIMP,      Zx);
            Cx1old=Cx1;
            Cx2old=Cx2;

        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double V1 = pP1->readNode(NodeMechanic::VELOCITY);
            //double F1 = pP1->readNode(NodeMechanic::FORCE);
            double V2 = pP2->readNode(NodeMechanic::VELOCITY);
            //double F2 = pP2->readNode(NodeMechanic::FORCE);

            //Delay Line equations
            double Cx1new = Cx2old + 2.*Zx*V2;
            double Cx2new = Cx1old + 2.*Zx*V1;
            double Cx1=Alpha*Cx1old + (1.0-Alpha)*Cx1new; // Filtering if Alpha>0
            double Cx2=Alpha*Cx2old + (1.0-Alpha)*Cx2new; // Filtering if Alpha>0


            //Write new values to nodes
            pP1->writeNode(NodeMechanic::WAVEVARIABLE, Cx1);
            pP1->writeNode(NodeMechanic::CHARIMP,      Zx);
            pP2->writeNode(NodeMechanic::WAVEVARIABLE, Cx2);
            pP2->writeNode(NodeMechanic::CHARIMP,      Zx);

            //Update the delayed variabels
            Cx1old=Cx1;
            Cx2old=Cx2;
        }
    };
}

#endif // UBAR_HPP_INCLUDED
