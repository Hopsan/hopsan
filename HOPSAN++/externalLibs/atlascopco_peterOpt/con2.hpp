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

//#include "P:/Hopsan_ng/Hopsan_latest/include/ComponentEssentials.h"
//#include "P:/Hopsan_ng/Hopsan_latest/include/ComponentUtilities.h"
#include "../../HopsanCore/ComponentEssentials.h"
#include "../../HopsanCore/ComponentUtilities.h"

namespace hopsan {

    class con2 : public ComponentQ
    {
    private:

        Port *mpP1, *mpP2;
        Integrator Int;
        //double X10,X20,V10,V20,F10,F20;
        double mX10, mX20;

        //Declaration of node data pointers, ND is short for NodeData
        double *mpND_Zx1, *mpND_Zx2, *mpND_Cx1, *mpND_Cx2, *mpND_X1, *mpND_X2, *mpND_V1, *mpND_V2, *mpND_F1, *mpND_F2;

    public:
        static Component *Creator()
        {
            return new con2();
        }

        con2() : ComponentQ()
        {
            mTypeName = "con2";

            //Add port to the component
            mpP1 = addPowerPort("P1", "NodeMechanic");
            mpP2 = addPowerPort("P2", "NodeMechanic");

//            //Start values
//            X10 = 0;
//            X20 = 0;
//            V10 = 0;
//            V20 = 0;
//            F10 = 0;
//            F20 = 0;
//            registerParameter("X1", "Displacement", "[m]", X10);
//            registerParameter("X2", "Displacement", "[m]", X20);
//            registerParameter("V1", "Velocity", "[m/s]", V10);
//            registerParameter("V2", "Velocity", "[m/s]", V20);
//            registerParameter("F1", "Force", "[N]", F10);
//            registerParameter("F2", "Force", "[N]", F20);
        }


        void initialize() 
        {
            //Initialize start values
//            mpP1->writeNode(NodeMechanic::POSITION, X10);
//            mpP2->writeNode(NodeMechanic::POSITION, X20);
//            mpP1->writeNode(NodeMechanic::VELOCITY, V10);
//            mpP2->writeNode(NodeMechanic::VELOCITY, V20);
//            mpP1->writeNode(NodeMechanic::FORCE, F10);
//            mpP2->writeNode(NodeMechanic::FORCE, F20);

            //Declaration of local variables
            double Zx1, Zx2, F1, F2, V1, X1;

            //Assign node data pointers
            mpND_Zx1 = mpP1->getNodeDataPtr(NodeMechanic::CHARIMP);
            mpND_Zx2 = mpP2->getNodeDataPtr(NodeMechanic::CHARIMP);
            mpND_Cx1 = mpP1->getNodeDataPtr(NodeMechanic::WAVEVARIABLE);
            mpND_Cx2 = mpP2->getNodeDataPtr(NodeMechanic::WAVEVARIABLE);
            mpND_X1 = mpP1->getNodeDataPtr(NodeMechanic::POSITION);
            mpND_X2 = mpP2->getNodeDataPtr(NodeMechanic::POSITION);
            mpND_V1 = mpP1->getNodeDataPtr(NodeMechanic::VELOCITY);
            mpND_V2 = mpP2->getNodeDataPtr(NodeMechanic::VELOCITY);
            mpND_F1  = mpP1->getNodeDataPtr(NodeMechanic::FORCE);
            mpND_F2  = mpP2->getNodeDataPtr(NodeMechanic::FORCE);

            //Read values from node data pointers
//            double Zx1 = mpP1->readNode(NodeMechanic::CHARIMP);
//            double Zx2 = mpP2->readNode(NodeMechanic::CHARIMP);
//            double X1  = mpP1->readNode(NodeMechanic::POSITION);
//            double V1  = mpP1->readNode(NodeMechanic::VELOCITY);
//            double F1  = mpP1->readNode(NodeMechanic::FORCE);
//            double F2  = mpP1->readNode(NodeMechanic::FORCE);
            Zx1 = *mpND_Zx1;
            Zx2 = *mpND_Zx2;
            F1 = *mpND_F1;
            F2 = *mpND_F2;  //!< @note should we access port one or two for F2, see commented line 4 lines above
            V1 = *mpND_V1;
            X1 = *mpND_X1;
            mX10 = X1;
            mX20 = *mpND_X2;

//!         @warning DONT do  Zx1 == 0,  Zx1 is a floating point number, there is no gurante that 0 == 0 is true.
//!         0 might actually be 0.00000000000000000000000001 or -0.00000000000000000000000001 whiche are not the same
//            if (Zx1 == 0) mpP1 ->writeNode(NodeMechanic::WAVEVARIABLE, F1);
//            if (Zx2 == 0) mpP2 ->writeNode(NodeMechanic::WAVEVARIABLE, F2);
            if (fabs(Zx1) < 1e-10) {*mpND_Cx1 = F1;}
            if (fabs(Zx2) < 1e-10) {*mpND_Cx2 = F2;}
            Int.initialize(mTimestep, V1, X1);

        }

        void simulateOneTimestep() // Here is the actual simulation call.
        {
            //Declaration of local variables
            double Zx1, Zx2, Cx1, Cx2, F1, F2, V1, V2, X1, X2;

            //Read values from nodes
//            double Cx1 = mpP1->readNode(NodeMechanic::WAVEVARIABLE);
//            double Zx1 = mpP1->readNode(NodeMechanic::CHARIMP);
//            double Cx2 = mpP2->readNode(NodeMechanic::WAVEVARIABLE);
//            double Zx2 = mpP2->readNode(NodeMechanic::CHARIMP);
            Cx1 = *mpND_Cx1;
            Zx1 = *mpND_Zx1;
            Cx2 = *mpND_Cx2;
            Zx2 = *mpND_Zx2;

            //Equations
            V2 = (Cx1-Cx2)/((Zx1+Zx2)); // !!!!!!!!!!if Zx==0 notify user!!!!!!!!!
            V1 = -V2;
            X1 = Int.update(V1);
            X2 = mX20-(X1-mX10);
            F1 = Cx1 + V1*Zx1;
            F2 = Cx2 + V2*Zx2;

            //Write new values to nodes
//            mpP1->writeNode(NodeMechanic::POSITION, X1);
//            mpP2->writeNode(NodeMechanic::POSITION, X2);
//            mpP1->writeNode(NodeMechanic::VELOCITY, V1);
//            mpP2->writeNode(NodeMechanic::VELOCITY, V2);
//            mpP1->writeNode(NodeMechanic::FORCE, F1);
//            mpP2->writeNode(NodeMechanic::FORCE, F2);
            *mpND_X1 = X1;
            *mpND_X2 = X2;
            *mpND_V1 = V1;
            *mpND_V2 = V2;
            *mpND_F1 = F1;
            *mpND_F2 = F2;
        }


        void finalize() // In this method (function) you can finalize anything for your component. It will be called after each time a simulation has finished.
        {
            //Nothing to finalize.
        }
    };
}

#endif
