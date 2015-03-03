#ifndef Hopsan2DBodyTest_HPP_INCLUDED
#define Hopsan2DBodyTest_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    class Hopsan2DBodyTest : public ComponentQ
    {
    private:
        //Input variable node data pointers
        double *mpM, *mpJ, *mpB, *mpBrot;

        //Output variable node data pointers
        double *mpDx1, *mpDy1, *mpDx2, *mpDy2, *mpXcx, *mpVcx, *mpXcy, *mpVcy, *mpAngleC, *mpOmegaC, *mpTh0_1, *mpTh0_2, *mpDebug;

        //Power port pointers
        Port *mpP1x, *mpP1y, *mpP1r, *mpP2x, *mpP2y, *mpP2r;

        //Power port node data pointers
        double *mpP1x_x, *mpP1x_v, *mpP1x_F, *mpP1x_me, *mpP1x_c, *mpP1x_Zc;
        double *mpP1y_x, *mpP1y_v, *mpP1y_F, *mpP1y_me, *mpP1y_c, *mpP1y_Zc;
        double *mpP1r_a, *mpP1r_w, *mpP1r_T, *mpP1r_Je, *mpP1r_c, *mpP1r_Zc;
        double *mpP2x_x, *mpP2x_v, *mpP2x_F, *mpP2x_me, *mpP2x_c, *mpP2x_Zc;
        double *mpP2y_x, *mpP2y_v, *mpP2y_F, *mpP2y_me, *mpP2y_c, *mpP2y_Zc;
        double *mpP2r_a, *mpP2r_w, *mpP2r_T, *mpP2r_Je, *mpP2r_c, *mpP2r_Zc;

        DoubleIntegratorWithDamping mIntegratorX; 
        DoubleIntegratorWithDamping mIntegratorY; 
        DoubleIntegratorWithDamping mIntegratorR; 

        double th0_1, th0_2;    //Initial angles from center to each port
        double l1, l2;    //Lever arms from center to each port
    public:
        static Component *Creator()
        {
            return new Hopsan2DBodyTest();
        }

        void configure()
        {
            //Register constant parameters

            //Register input variables
            addInputVariable("M", "Linear inertia (x and y)", "kg", 100, &mpM);
            addInputVariable("J", "Rotational inertia", "kg/m^2", 1, &mpJ);
            addInputVariable("B", "Linear amping", "Ns/m", 10, &mpB);
            addInputVariable("Brot", "Rotational damping", "Nms/rad", 10, &mpBrot);

            //Register output variables
            addOutputVariable("dx1", "X-lever to port 1", "m", -1, &mpDx1);
            addOutputVariable("dy1", "Y-lever to port 1", "m", 0, &mpDy1);
            addOutputVariable("dx2", "X-lever to port 2", "m", 1, &mpDx2);
            addOutputVariable("dy2", "Y-lever to port 2", "m", 0, &mpDy2);
            addOutputVariable("xcx", "X-position of center", "m/s", 0, &mpXcx);
            addOutputVariable("vcx", "X-velocity of center", "m/s", 0, &mpVcx);
            addOutputVariable("xcy", "Y-position of center", "m/s", 0, &mpXcy);
            addOutputVariable("vcy", "Y-velocity of center", "m/s", 0, &mpVcy);
            addOutputVariable("angleC", "Angle of body", "rad/s", 0, &mpAngleC);
            addOutputVariable("omegaC", "Angular velocity of center", "rad/s", 0, &mpOmegaC);
            addOutputVariable("th0_1", "Initial angle between center and port 1", "rad", 0, &mpTh0_1);
            addOutputVariable("th0_2", "Initial angle between center and port 2", "rad", 0, &mpTh0_2);
            addOutputVariable("debug", "", "", 0, &mpDebug);

            //Add power ports
            mpP1x = addPowerPort("P1x", "NodeMechanic", "");
            mpP1y = addPowerPort("P1y", "NodeMechanic", "");
            mpP1r = addPowerPort("P1r", "NodeMechanicRotational", "");
            mpP2x = addPowerPort("P2x", "NodeMechanic", "");
            mpP2y = addPowerPort("P2y", "NodeMechanic", "");
            mpP2r = addPowerPort("P2r", "NodeMechanicRotational", "");

            //Set default power port start values

        }


        void initialize()
        {
            //Get node data pointers from ports
            mpP1x_x = getSafeNodeDataPtr(mpP1x, NodeMechanic::Position);
            mpP1x_v = getSafeNodeDataPtr(mpP1x, NodeMechanic::Velocity);
            mpP1x_F = getSafeNodeDataPtr(mpP1x, NodeMechanic::Force);
            mpP1x_me = getSafeNodeDataPtr(mpP1x, NodeMechanic::EquivalentMass);
            mpP1x_c = getSafeNodeDataPtr(mpP1x, NodeMechanic::WaveVariable);
            mpP1x_Zc = getSafeNodeDataPtr(mpP1x, NodeMechanic::CharImpedance);
            mpP1y_x = getSafeNodeDataPtr(mpP1y, NodeMechanic::Position);
            mpP1y_v = getSafeNodeDataPtr(mpP1y, NodeMechanic::Velocity);
            mpP1y_F = getSafeNodeDataPtr(mpP1y, NodeMechanic::Force);
            mpP1y_me = getSafeNodeDataPtr(mpP1y, NodeMechanic::EquivalentMass);
            mpP1y_c = getSafeNodeDataPtr(mpP1y, NodeMechanic::WaveVariable);
            mpP1y_Zc = getSafeNodeDataPtr(mpP1y, NodeMechanic::CharImpedance);
            mpP1r_a = getSafeNodeDataPtr(mpP1r, NodeMechanicRotational::Angle);
            mpP1r_w = getSafeNodeDataPtr(mpP1r, NodeMechanicRotational::AngularVelocity);
            mpP1r_T = getSafeNodeDataPtr(mpP1r, NodeMechanicRotational::Torque);
            mpP1r_Je = getSafeNodeDataPtr(mpP1r, NodeMechanicRotational::EquivalentInertia);
            mpP1r_c = getSafeNodeDataPtr(mpP1r, NodeMechanicRotational::WaveVariable);
            mpP1r_Zc = getSafeNodeDataPtr(mpP1r, NodeMechanicRotational::CharImpedance);
            mpP2x_x = getSafeNodeDataPtr(mpP2x, NodeMechanic::Position);
            mpP2x_v = getSafeNodeDataPtr(mpP2x, NodeMechanic::Velocity);
            mpP2x_F = getSafeNodeDataPtr(mpP2x, NodeMechanic::Force);
            mpP2x_me = getSafeNodeDataPtr(mpP2x, NodeMechanic::EquivalentMass);
            mpP2x_c = getSafeNodeDataPtr(mpP2x, NodeMechanic::WaveVariable);
            mpP2x_Zc = getSafeNodeDataPtr(mpP2x, NodeMechanic::CharImpedance);
            mpP2y_x = getSafeNodeDataPtr(mpP2y, NodeMechanic::Position);
            mpP2y_v = getSafeNodeDataPtr(mpP2y, NodeMechanic::Velocity);
            mpP2y_F = getSafeNodeDataPtr(mpP2y, NodeMechanic::Force);
            mpP2y_me = getSafeNodeDataPtr(mpP2y, NodeMechanic::EquivalentMass);
            mpP2y_c = getSafeNodeDataPtr(mpP2y, NodeMechanic::WaveVariable);
            mpP2y_Zc = getSafeNodeDataPtr(mpP2y, NodeMechanic::CharImpedance);
            mpP2r_a = getSafeNodeDataPtr(mpP2r, NodeMechanicRotational::Angle);
            mpP2r_w = getSafeNodeDataPtr(mpP2r, NodeMechanicRotational::AngularVelocity);
            mpP2r_T = getSafeNodeDataPtr(mpP2r, NodeMechanicRotational::Torque);
            mpP2r_Je = getSafeNodeDataPtr(mpP2r, NodeMechanicRotational::EquivalentInertia);
            mpP2r_c = getSafeNodeDataPtr(mpP2r, NodeMechanicRotational::WaveVariable);
            mpP2r_Zc = getSafeNodeDataPtr(mpP2r, NodeMechanicRotational::CharImpedance);

            //Declare local variables
            double M, J, B, Brot;
            double dx1, dy1, dx2, dy2;
            double P1x_x, P1x_v, P1x_F, P1x_me, P1x_c, P1x_Zc, P1y_x, P1y_v, P1y_F, P1y_me, P1y_c, P1y_Zc, P1r_a, P1r_w, P1r_T, P1r_Je, P1r_c, P1r_Zc, P2x_x, P2x_v, P2x_F, P2x_me, P2x_c, P2x_Zc, P2y_x, P2y_v, P2y_F, P2y_me, P2y_c, P2y_Zc, P2r_a, P2r_w, P2r_T, P2r_Je, P2r_c, P2r_Zc;

            //Read variable values from nodes
            P1x_F = (*mpP1x_F);
            P1y_F = (*mpP1y_F);
            P1r_T = (*mpP1r_T);            
            P2x_F = (*mpP2x_F);
            P2y_F = (*mpP2y_F);
            M = (*mpM);
            J = (*mpJ);
            dx1 = (*mpDx1);
            dy1 = (*mpDy1);
            dx2 = (*mpDx2);
            dy2 = (*mpDy2);

            //Initialize filteres
            mIntegratorX.initialize(mTimestep, 0, (-P1x_F-P2x_F)/M, 0, 0);
            mIntegratorY.initialize(mTimestep, 0, (-P1y_F-P2y_F)/M, 0, 0);
            mIntegratorR.initialize(mTimestep, 0, (-P1r_T-P2r_T-P1x_F*dy1-P1y_F*dx1-P2x_F*dy2-P2y_F*dx2)/M, 0, 0);

            //Calculate angles from center to port at zero rotation of body
            th0_1 = atan2(dy1,dx1);
            th0_2 = atan2(dy2,dx2);

            //Calculate lever arms at zero rotation of body
            l1 = sqrt(dx1*dx1 + dy1*dy1);
            l2 = sqrt(dx2*dx2 + dy2*dy2);
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double M, J, B, Brot;
            double dx1, dy1, dx2, dy2, xcx, vcx, xcy, vcy, angleC, omegaC;
            double P1x_x, P1x_v, P1x_F, P1x_me, P1x_c, P1x_Zc, P1y_x, P1y_v, P1y_F, P1y_me, P1y_c, P1y_Zc, P1r_a, P1r_w, P1r_T, P1r_Je, P1r_c, P1r_Zc, P2x_x, P2x_v, P2x_F, P2x_me, P2x_c, P2x_Zc, P2y_x, P2y_v, P2y_F, P2y_me, P2y_c, P2y_Zc, P2r_a, P2r_w, P2r_T, P2r_Je, P2r_c, P2r_Zc;

            //Read variable values from nodes
            P1x_c = (*mpP1x_c);
            P1x_Zc = (*mpP1x_Zc);
            P1y_c = (*mpP1y_c);
            P1y_Zc = (*mpP1y_Zc);
            P1r_c = (*mpP1r_c);
            P1r_Zc = (*mpP1r_Zc);
            P2x_c = (*mpP2x_c);
            P2x_Zc = (*mpP2x_Zc);
            P2y_c = (*mpP2y_c);
            P2y_Zc = (*mpP2y_Zc);
            P2r_c = (*mpP2r_c);
            P2r_Zc = (*mpP2r_Zc);
            M = (*mpM);
            J = (*mpJ);
            B = (*mpB);
            Brot = (*mpBrot);
            angleC = (*mpAngleC);

            //WRITE YOUR EQUATIONS HERE

            //Effective lever arms (= current, depending on angle)
            double dx1e = l1*cos(angleC+th0_1);
            double dy1e = l1*sin(angleC+th0_1);
            double dx2e = l2*cos(angleC+th0_2);
            double dy2e = l2*sin(angleC+th0_2);

            //Kinetics (using second order dynamics)
            mIntegratorX.setDamping((B+P1x_Zc+P2x_Zc) / M * mTimestep); 
            mIntegratorX.integrateWithUndo((-P2x_c-P1x_c)/M);
            vcx = mIntegratorX.valueFirst();
            xcx = mIntegratorX.valueSecond();
            
            mIntegratorY.setDamping((B+P1y_Zc+P2y_Zc) / M * mTimestep); 
            mIntegratorY.integrateWithUndo((-P2y_c-P1y_c)/M);
            vcy = mIntegratorY.valueFirst();
            xcy = mIntegratorY.valueSecond();

            double Btot = Brot + P1x_Zc*dy1e*dy1e + P1y_Zc*dx1e*dx1e + P2x_Zc*dy2e*dy2e + P2y_Zc*dx2e*dx2e + P1r_Zc + P2r_Zc; //Total damping factor for rotation
            double ctot = P2r_c + P1r_c + P2x_c*dy2e + P2y_c*dx2e + P1x_c*dy1e + P1y_c*dx1e;	//Total external forces (wave variables) for rotation
            mIntegratorR.setDamping((Btot) / J * mTimestep); 
            mIntegratorR.integrateWithUndo(-ctot/J);
            omegaC = mIntegratorR.valueFirst();
            angleC = mIntegratorR.valueSecond();
	
            //Kinematics
            P1x_v = vcx - omegaC*dy1e;
            P1x_x = xcx + dx1e;   
            P1y_v = vcy + omegaC*dx1e;
            P1y_x = xcy + dy1e;  

            P2x_v = vcx - omegaC*dy2e;
            P2x_x = xcx + dx2e;
            P2y_v = vcy + omegaC*dx2e;
            P2y_x = xcy + dy2e;

            P2r_w = omegaC;
            P2r_a = angleC;
            P1r_w = omegaC;
            P1r_a = angleC;

            //TLM equations
            P1x_F = P1x_c + P1x_v*P1x_Zc;
            P1y_F = P1y_c + P1y_v*P1y_Zc;
            P2x_F = P2x_c + P2x_v*P2x_Zc;
            P2y_F = P2y_c + P2y_v*P2y_Zc;
            P1r_T = P1r_c + P1r_w*P1r_Zc;
            P2r_T = P2r_c + P2r_w*P2r_Zc;

            //Write new values to nodes
            (*mpP1x_x) = P1x_x;
            (*mpP1x_v) = P1x_v;
            (*mpP1x_F) = P1x_F;
            (*mpP1x_me) = P1x_me;
            (*mpP1y_x) = P1y_x;
            (*mpP1y_v) = P1y_v;
            (*mpP1y_F) = P1y_F;
            (*mpP1y_me) = P1y_me;
            (*mpP1r_a) = P1r_a;
            (*mpP1r_w) = P1r_w;
            (*mpP1r_T) = P1r_T;
            (*mpP1r_Je) = P1r_Je;
            (*mpP2x_x) = P2x_x;
            (*mpP2x_v) = P2x_v;
            (*mpP2x_F) = P2x_F;
            (*mpP2x_me) = P2x_me;
            (*mpP2y_x) = P2y_x;
            (*mpP2y_v) = P2y_v;
            (*mpP2y_F) = P2y_F;
            (*mpP2y_me) = P2y_me;
            (*mpP2r_a) = P2r_a;
            (*mpP2r_w) = P2r_w;
            (*mpP2r_T) = P2r_T;
            (*mpP2r_Je) = P2r_Je;
            (*mpDx1) = dx1e;
            (*mpDy1) = dy1e;
            (*mpDx2) = dx2e;
            (*mpDy2) = dy2e;
            (*mpXcx) = xcx;
            (*mpVcx) = vcx;
            (*mpXcy) = xcy;
            (*mpVcy) = vcy;
            (*mpAngleC) = angleC;
            (*mpOmegaC) = omegaC;
            (*mpTh0_1) = th0_1;
            (*mpTh0_2) = th0_2;
            (*mpDebug) = dx2e;
        }


        void finalize()
        {
            //WRITE YOUR FINALIZE CODE HERE (OPTIONAL)
        }


        void deconfigure()
        {
            //WRITE YOUR DECONFIGURATION CODE HERE (OPTIONAL)
        }
    };
}

#endif //Hopsan2DBodyTest_HPP_INCLUDED


