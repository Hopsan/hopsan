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

        Port *mpP1, *mpP2, *mpP3;
        Integrator mInt;
//        double X10,X20,X30,V10,V20,V30, F10,F20,F30;
        double mX10, mX20, mX30;

        //Declaration of node data pointers, ND is short for NodeData
        double *mpND_Zx1, *mpND_Zx2, *mpND_Zx3, *mpND_Cx1, *mpND_Cx2, *mpND_Cx3,
               *mpND_X1, *mpND_X2, *mpND_X3, *mpND_V1, *mpND_V2, *mpND_V3, *mpND_F1, *mpND_F2, *mpND_F3;

    public:
        static Component *Creator()
        {
            return new con3();
        }

        con3() : ComponentQ()

        {
            mTypeName = "con3";

            mpP1 = addPowerPort("P1", "NodeMechanic");
            mpP2 = addPowerPort("P2", "NodeMechanic");
            mpP3 = addPowerPort("P3", "NodeMechanic");

//            //Start values
//            X10 = 0;
//            X20 = 0;
//            X30 = 0;
//            V10 = 0;
//            V20 = 0;
//            V30 = 0;
//            F10 = 0;
//            F20 = 0;
//            F30 = 0;
//            registerParameter("X1", "Displacement", "[m]", X10);
//            registerParameter("X2", "Displacement", "[m]", X20);
//            registerParameter("X3", "Displacement", "[m]", X30);
//            registerParameter("V1", "Velocity", "[m/s]", V10);
//            registerParameter("V2", "Velocity", "[m/s]", V20);
//            registerParameter("V3", "Velocity", "[m/s]", V30);
        }

        void initialize()
        {
//                //Initialize start values
//            pP1->writeNode(NodeMechanic::POSITION, X10);
//            pP2->writeNode(NodeMechanic::POSITION, X20);
//            pP3->writeNode(NodeMechanic::POSITION, X30);
//            pP1->writeNode(NodeMechanic::VELOCITY, V10);
//            pP2->writeNode(NodeMechanic::VELOCITY, V20);
//            pP3->writeNode(NodeMechanic::VELOCITY, V30);

            //Assign node data pointers
            mpND_Zx1 = mpP1->getNodeDataPtr(NodeMechanic::CHARIMP);
            mpND_Zx2 = mpP2->getNodeDataPtr(NodeMechanic::CHARIMP);
            mpND_Zx3 = mpP3->getNodeDataPtr(NodeMechanic::CHARIMP);
            mpND_Cx1 = mpP1->getNodeDataPtr(NodeMechanic::WAVEVARIABLE);
            mpND_Cx2 = mpP2->getNodeDataPtr(NodeMechanic::WAVEVARIABLE);
            mpND_Cx3 = mpP3->getNodeDataPtr(NodeMechanic::WAVEVARIABLE);
            mpND_X1 = mpP1->getNodeDataPtr(NodeMechanic::POSITION);
            mpND_X2 = mpP2->getNodeDataPtr(NodeMechanic::POSITION);
            mpND_X3 = mpP3->getNodeDataPtr(NodeMechanic::POSITION);
            mpND_V1 = mpP1->getNodeDataPtr(NodeMechanic::VELOCITY);
            mpND_V2 = mpP2->getNodeDataPtr(NodeMechanic::VELOCITY);
            mpND_V3 = mpP3->getNodeDataPtr(NodeMechanic::VELOCITY);
            mpND_F1  = mpP1->getNodeDataPtr(NodeMechanic::FORCE);
            mpND_F2  = mpP2->getNodeDataPtr(NodeMechanic::FORCE);
            mpND_F2  = mpP3->getNodeDataPtr(NodeMechanic::FORCE);

            //Read values from node data pointers
//            double Zx1 = pP1->readNode(NodeMechanic::CHARIMP);
//            double Zx2 = pP2->readNode(NodeMechanic::CHARIMP);
//            double Zx3 = pP2->readNode(NodeMechanic::CHARIMP);
//            double X1  = pP1->readNode(NodeMechanic::POSITION);
//            double V1  = pP1->readNode(NodeMechanic::VELOCITY);
//            double F1  = pP1->readNode(NodeMechanic::FORCE);
//            double F2  = pP1->readNode(NodeMechanic::FORCE);
//            double F3  = pP1->readNode(NodeMechanic::FORCE);
            double Zx1 = *mpND_Zx1;
            double Zx2 = *mpND_Zx2;
            double Zx3 = *mpND_Zx3; //!< @note I use port 3 here instead of 2 like in commented lines above
            double V1 = *mpND_V1;
            double X1 = *mpND_X1;
            double F1 = *mpND_F1;
            double F2 = *mpND_F2; //!< @note I use port 2 here instead of 1 like in commented lines above
            double F3 = *mpND_F3; //!< @note I use port 3 here instead of 1 like in commented lines above
            mX10 = X1;
            mX20 = *mpND_X2;
            mX30 = *mpND_X3;

//!         @warning DONT do  Zx1 == 0,  Zx1 is a floating point number, there is no gurante that 0 == 0 is true.
//!         0 might actually be 0.00000000000000000000000001 or -0.00000000000000000000000001 which are not the same
//            if (Zx1 == 0) pP1 ->writeNode(NodeMechanic::WAVEVARIABLE, F1);
//            if (Zx2 == 0) pP2 ->writeNode(NodeMechanic::WAVEVARIABLE, F2);
//            if (Zx3 == 0) pP2 ->writeNode(NodeMechanic::WAVEVARIABLE, F3);
            if (fabs(Zx1) < 1e-10) {*mpND_Cx1 = F1;}
            if (fabs(Zx2) < 1e-10) {*mpND_Cx2 = F2;}
            if (fabs(Zx3) < 1e-10) {*mpND_Cx3 = F3;} //! @note using pP3 instead of pP2 like in commented line above
            mInt.initialize(mTimestep, V1, X1);

        }

        void simulateOneTimestep() // Here is the actual simulation call.
        {
            //Read values from nodes
//            double Cx1 = pP1->readNode(NodeMechanic::WAVEVARIABLE);
//            double Zx1 = pP1->readNode(NodeMechanic::CHARIMP);
//            double Cx2 = pP2->readNode(NodeMechanic::WAVEVARIABLE);
//            double Zx2 = pP2->readNode(NodeMechanic::CHARIMP);
//            double Cx3 = pP3->readNode(NodeMechanic::WAVEVARIABLE);
//            double Zx3 = pP3->readNode(NodeMechanic::CHARIMP);
            double Cx1 = *mpND_Cx1;
            double Zx1 = *mpND_Zx1;
            double Cx2 = *mpND_Cx2;
            double Zx2 = *mpND_Zx2;
            double Cx3 = *mpND_Cx3;
            double Zx3 = *mpND_Zx3;

            //Equations
            double V1 = (-Cx1+Cx2+Cx3)/((Zx1+Zx2+Zx3)); // !!!!!!!!!!if Zx==0 notify user!!!!!!!!!
            double V2 = -V1;
            double V3 = V2;
            double X1 = mInt.update(V1);
            double X2 = mX20-(X1-mX10);
            double X3 = mX30-(X1-mX10);
            double F1 = Cx1 + V1*Zx1;
            double F2 = Cx2 + V2*Zx2;
            double F3 = Cx3 + V3*Zx3;

            //Write new values to node
//            pP1->writeNode(NodeMechanic::POSITION, X1);
//            pP2->writeNode(NodeMechanic::POSITION, X2);
//            pP3->writeNode(NodeMechanic::POSITION, X3);
//            pP1->writeNode(NodeMechanic::VELOCITY, V1);
//            pP2->writeNode(NodeMechanic::VELOCITY, V2);
//            pP3->writeNode(NodeMechanic::VELOCITY, V3);
//            pP1->writeNode(NodeMechanic::FORCE, F1);
//            pP2->writeNode(NodeMechanic::FORCE, F2);
//            pP3->writeNode(NodeMechanic::FORCE, F3);
            *mpND_X1 = X1;
            *mpND_X2 = X2;
            *mpND_X3 = X3;
            *mpND_V1 = V1;
            *mpND_V2 = V2;
            *mpND_V3 = V3;
            *mpND_F1 = F1;
            *mpND_F2 = F2;
            *mpND_F3 = F3;
        }


        void finalize()
        {
            //Nothing to finalize.
        }
    };
}

#endif
