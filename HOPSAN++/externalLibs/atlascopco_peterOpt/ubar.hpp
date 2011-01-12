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
        double mZx;
//        double Alpha;
//        double Cx1;
//        double Cx2;
        double mCx1old;
        double mCx2old;
//        double Cx1new;
//        double Cx2new;
//        double F1;
//        double F2;
//        double V1;
//        double V2;
        Port *mpP1, *mpP2;

        //Declaration of node data pointers, ND is short for NodeData
        double *mpND_F1, *mpND_V1, *mpND_Cx1, *mpND_Zx1, *mpND_F2, *mpND_V2, *mpND_Cx2, *mpND_Zx2;

    public:
        static Component *Creator()
        {
            return new ubar();
        }

        ubar() : ComponentC()
        {
            //Set member attributes
            mTypeName = "ubar";
//            Alpha=0.0;//Filtering of characteristics if >0

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanic");
            mpP2 = addPowerPort("P2", "NodeMechanic");

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
            mpND_F1 = mpP1->getNodeDataPtr(NodeMechanic::FORCE);
            mpND_V1 = mpP1->getNodeDataPtr(NodeMechanic::VELOCITY);
            mpND_Cx1 = mpP1->getNodeDataPtr(NodeMechanic::WAVEVARIABLE);
            mpND_Zx1 = mpP1->getNodeDataPtr(NodeMechanic::CHARIMP);
            mpND_F2 = mpP2->getNodeDataPtr(NodeMechanic::FORCE);
            mpND_V2 = mpP2->getNodeDataPtr(NodeMechanic::VELOCITY);
            mpND_Cx2 = mpP2->getNodeDataPtr(NodeMechanic::WAVEVARIABLE);
            mpND_Zx2 = mpP2->getNodeDataPtr(NodeMechanic::CHARIMP);

            //Startvalues, read force and velocity from connected Q-types
//            F1 = pP1->readNode(NodeMechanic::FORCE);
//            V1 = pP1->readNode(NodeMechanic::VELOCITY);
//            F2 = pP2->readNode(NodeMechanic::FORCE);
//            V2 = pP2->readNode(NodeMechanic::VELOCITY);
            double F1 = *mpND_F1;
            double V1 = *mpND_V1;
            double F2 = *mpND_F2;
            double V2 = *mpND_V2;

            double Zx=RHOB*sqrt(EB/RHOB)*3.141593*D*D/4.0;
            double Cx1=F1-Zx*V1;
            double Cx2=F2-Zx*V2;

            //Write characteristics to nodes
//            pP1->writeNode(NodeMechanic::WAVEVARIABLE, Cx1);
//            pP1->writeNode(NodeMechanic::CHARIMP,      Zx);
//            pP2->writeNode(NodeMechanic::WAVEVARIABLE, Cx2);
//            pP2->writeNode(NodeMechanic::CHARIMP,      Zx);
            *mpND_Cx1 = Cx1;
            *mpND_Zx1 = Zx;
            *mpND_Cx2 = Cx2;
            *mpND_Zx2 = Zx;

            mCx1old=Cx1;
            mCx2old=Cx2;

        }


        void simulateOneTimestep()
        {
            //Definition of local constants
            const double Alpha=0.0; //Filtering of characteristics if >0

            //Get variable values from nodes
//            double V1 = pP1->readNode(NodeMechanic::VELOCITY);
//            //double F1 = pP1->readNode(NodeMechanic::FORCE);
//            double V2 = pP2->readNode(NodeMechanic::VELOCITY);
//            //double F2 = pP2->readNode(NodeMechanic::FORCE);
            double V1 = *mpND_V1;
            double V2 = *mpND_V2;

            //Delay Line equations
            double Cx1new = mCx2old + 2.*mZx*V2;
            double Cx2new = mCx1old + 2.*mZx*V1;
            double Cx1=Alpha*mCx1old + (1.0-Alpha)*Cx1new; // Filtering if Alpha>0
            double Cx2=Alpha*mCx2old + (1.0-Alpha)*Cx2new; // Filtering if Alpha>0

            //Write new values to nodes
//            mpP1->writeNode(NodeMechanic::WAVEVARIABLE, Cx1);
//            mpP1->writeNode(NodeMechanic::CHARIMP,      mZx);
//            mpP2->writeNode(NodeMechanic::WAVEVARIABLE, Cx2);
//            mpP2->writeNode(NodeMechanic::CHARIMP,      mZx);
            *mpND_Cx1 = Cx2;
            *mpND_Zx1 = mZx;
            *mpND_Cx2 = Cx1;
            *mpND_Zx2 = mZx;

            //Update the delayed variabels
            mCx1old=Cx1;
            mCx2old=Cx2;
        }
    };
}

#endif // UBAR_HPP_INCLUDED
