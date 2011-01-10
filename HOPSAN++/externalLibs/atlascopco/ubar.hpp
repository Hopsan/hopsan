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
        double Cx1old;
        double Cx2old;
        double Cx1new;
        double Cx2new;
        Port *pP1, *pP2;
        double F1, X1, V1, Cx1, Zx1, F2, X2, V2, Cx2, Zx2;
        double *F1_ptr, *X1_ptr, *V1_ptr, *Cx1_ptr, *Zx1_ptr, *F2_ptr, *X2_ptr, *V2_ptr, *Cx2_ptr, *Zx2_ptr;

    public:
        static Component *Creator()
        {
            return new ubar();
        }

        ubar() : ComponentC()
        {
            //Set member attributes
            mTypeName = "ubar";
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
            //Assign node data pointeres
            F1_ptr = pP1->getNodeDataPtr(NodeMechanic::FORCE);
            X1_ptr = pP1->getNodeDataPtr(NodeMechanic::POSITION);
            V1_ptr = pP1->getNodeDataPtr(NodeMechanic::VELOCITY);
            Cx1_ptr = pP1->getNodeDataPtr(NodeMechanic::WAVEVARIABLE);
            Zx1_ptr = pP1->getNodeDataPtr(NodeMechanic::CHARIMP);
            F2_ptr = pP2->getNodeDataPtr(NodeMechanic::FORCE);
            X2_ptr = pP2->getNodeDataPtr(NodeMechanic::POSITION);
            V2_ptr = pP2->getNodeDataPtr(NodeMechanic::VELOCITY);
            Cx2_ptr = pP2->getNodeDataPtr(NodeMechanic::WAVEVARIABLE);
            Zx2_ptr = pP2->getNodeDataPtr(NodeMechanic::CHARIMP);

            //Startvalues, read force and velocity from connected Q-types
            F1 = (*F1_ptr);
            V1 = (*V1_ptr);
            F2 = (*F2_ptr);
            V2 = (*V2_ptr);

            Zx=RHOB*sqrt(EB/RHOB)*3.141593*D*D/4.0;
            Cx1=F1-Zx*V1;
            Cx2=F2-Zx*V2;

            //Write characteristics to nodes
            (*Cx1_ptr) = Cx1;
            (*Zx1_ptr) = Zx;
            (*Cx2_ptr) = Cx2;
            (*Zx2_ptr) = Zx;

            Cx1old=Cx1;
            Cx2old=Cx2;
        }


        void simulateOneTimestep()
        {
            //Read values from nodes
            V1 = (*V1_ptr);
            V2 = (*V2_ptr);

            //Delay Line equations
            Cx1new = Cx2old + 2.*Zx*V2;
            Cx2new = Cx1old + 2.*Zx*V1;
            Cx1=Alpha*Cx1old + (1.0-Alpha)*Cx1new; // Filtering if Alpha>0
            Cx2=Alpha*Cx2old + (1.0-Alpha)*Cx2new; // Filtering if Alpha>0

            //Write new values to nodes
            (*Cx1_ptr) = Cx1;
            (*Zx1_ptr) = Zx;
            (*Cx2_ptr) = Cx2;
            (*Zx2_ptr) = Zx;

            //Update the delayed variabels
            Cx1old=Cx1;
            Cx2old=Cx2;
        }
    };
}

#endif // UBAR_HPP_INCLUDED
