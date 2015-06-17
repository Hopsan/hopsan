/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

#ifndef Hopsan2DBody2_HPP_INCLUDED
#define Hopsan2DBody2_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    class Hopsan2DBody2 : public ComponentQ
    {
    private:
        //Input variable node data pointers
        double *mpM, *mpJ, *mpB, *mpBrot, *mpGx, *mpGy;

        //Output variable node data pointers
        double *mpDx1, *mpDy1, *mpDx2, *mpDy2, *mpXcx, *mpVcx, *mpXcy, *mpVcy, *mpAngleC, *mpOmegaC, *mpTh0_1, *mpTh0_2, *mpDebug1, *mpDebug2;

        //Power port pointers
        Port *mpP1, *mpP2;

        //Power port node data pointers
        double *mpP1_a, *mpP1_w, *mpP1_T, *mpP1_Je, *mpP1_cr, *mpP1_Zcr;
        double *mpP1_x, *mpP1_vx, *mpP1_Fx, *mpP1_mex, *mpP1_cx, *mpP1_Zcx;
        double *mpP1_y, *mpP1_vy, *mpP1_Fy, *mpP1_mey, *mpP1_cy, *mpP1_Zcy;

        double *mpP2_a, *mpP2_w, *mpP2_T, *mpP2_Je, *mpP2_cr, *mpP2_Zcr;
        double *mpP2_x, *mpP2_vx, *mpP2_Fx, *mpP2_mex, *mpP2_cx, *mpP2_Zcx;
        double *mpP2_y, *mpP2_vy, *mpP2_Fy, *mpP2_mey, *mpP2_cy, *mpP2_Zcy;

        DoubleIntegratorWithDamping mIntegratorX; 
        DoubleIntegratorWithDamping mIntegratorY; 
        DoubleIntegratorWithDamping mIntegratorR; 

        double th0_1, th0_2;    //Initial angles from center to each port
        double l1, l2;    //Lever arms from center to each port
    public:
        static Component *Creator()
        {
            return new Hopsan2DBody2();
        }

        void configure()
        {
            //Register constant parameters

            //Register input variables
            addInputVariable("M", "Linear inertia (x and y)", "kg", 100, &mpM);
            addInputVariable("J", "Rotational inertia", "kg/m^2", 1, &mpJ);
            addInputVariable("B", "Linear amping", "Ns/m", 10, &mpB);
            addInputVariable("Brot", "Rotational damping", "Nms/rad", 10, &mpBrot);
            addInputVariable("gx", "Gravitation in X-direction", "N/kg", 0, &mpGx);
            addInputVariable("gy", "Gravitation in Y-direction", "N/kg", 9.81, &mpGy);

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
            addOutputVariable("debug1", "", "", 0, &mpDebug1);
            addOutputVariable("debug2", "", "", 0, &mpDebug2);

            //Add power ports
            mpP1 = addPowerPort("P1", "NodeMechanic2D", "");
            mpP2 = addPowerPort("P2", "NodeMechanic2D", "");
        }


        void initialize()
        {
            //Get node data pointers from ports
            mpP1_a = getSafeNodeDataPtr(mpP1, NodeMechanic2D::AngleR);
            mpP1_w = getSafeNodeDataPtr(mpP1, NodeMechanic2D::AngularVelocityR);
            mpP1_T = getSafeNodeDataPtr(mpP1, NodeMechanic2D::TorqueR);
            mpP1_Je = getSafeNodeDataPtr(mpP1, NodeMechanic2D::EquivalentInertiaR);
            mpP1_cr = getSafeNodeDataPtr(mpP1, NodeMechanic2D::WaveVariableR);
            mpP1_Zcr = getSafeNodeDataPtr(mpP1, NodeMechanic2D::CharImpedanceR);
            mpP1_x = getSafeNodeDataPtr(mpP1, NodeMechanic2D::PositionX);
            mpP1_vx = getSafeNodeDataPtr(mpP1, NodeMechanic2D::VelocityX);
            mpP1_Fx = getSafeNodeDataPtr(mpP1, NodeMechanic2D::ForceX);
            mpP1_mex = getSafeNodeDataPtr(mpP1, NodeMechanic2D::EquivalentMassX);
            mpP1_cx = getSafeNodeDataPtr(mpP1, NodeMechanic2D::WaveVariableX);
            mpP1_Zcx = getSafeNodeDataPtr(mpP1, NodeMechanic2D::CharImpedanceX);
            mpP1_y = getSafeNodeDataPtr(mpP1, NodeMechanic2D::PositionY);
            mpP1_vy = getSafeNodeDataPtr(mpP1, NodeMechanic2D::VelocityY);
            mpP1_Fy = getSafeNodeDataPtr(mpP1, NodeMechanic2D::ForceY);
            mpP1_mey = getSafeNodeDataPtr(mpP1, NodeMechanic2D::EquivalentMassY);
            mpP1_cy = getSafeNodeDataPtr(mpP1, NodeMechanic2D::WaveVariableY);
            mpP1_Zcy = getSafeNodeDataPtr(mpP1, NodeMechanic2D::CharImpedanceY);
            
            mpP2_a = getSafeNodeDataPtr(mpP2, NodeMechanic2D::AngleR);
            mpP2_w = getSafeNodeDataPtr(mpP2, NodeMechanic2D::AngularVelocityR);
            mpP2_T = getSafeNodeDataPtr(mpP2, NodeMechanic2D::TorqueR);
            mpP2_Je = getSafeNodeDataPtr(mpP2, NodeMechanic2D::EquivalentInertiaR);
            mpP2_cr = getSafeNodeDataPtr(mpP2, NodeMechanic2D::WaveVariableR);
            mpP2_Zcr = getSafeNodeDataPtr(mpP2, NodeMechanic2D::CharImpedanceR);
            mpP2_x = getSafeNodeDataPtr(mpP2, NodeMechanic2D::PositionX);
            mpP2_vx = getSafeNodeDataPtr(mpP2, NodeMechanic2D::VelocityX);
            mpP2_Fx = getSafeNodeDataPtr(mpP2, NodeMechanic2D::ForceX);
            mpP2_mex = getSafeNodeDataPtr(mpP2, NodeMechanic2D::EquivalentMassX);
            mpP2_cx = getSafeNodeDataPtr(mpP2, NodeMechanic2D::WaveVariableX);
            mpP2_Zcx = getSafeNodeDataPtr(mpP2, NodeMechanic2D::CharImpedanceX);
            mpP2_y = getSafeNodeDataPtr(mpP2, NodeMechanic2D::PositionY);
            mpP2_vy = getSafeNodeDataPtr(mpP2, NodeMechanic2D::VelocityY);
            mpP2_Fy = getSafeNodeDataPtr(mpP2, NodeMechanic2D::ForceY);
            mpP2_mey = getSafeNodeDataPtr(mpP2, NodeMechanic2D::EquivalentMassY);
            mpP2_cy = getSafeNodeDataPtr(mpP2, NodeMechanic2D::WaveVariableY);
            mpP2_Zcy = getSafeNodeDataPtr(mpP2, NodeMechanic2D::CharImpedanceY);

            //Declare local variables
            double M, J, B, Brot, angleC, xcx, xcy, gx, gy;
            double dx1, dy1, dx2, dy2;
            
            double P1_a, P1_w, P1_T, P1_Je, P1_cr, P1_Zcr;
            double P1_x, P1_vx, P1_Fx, P1_mex, P1_cx, P1_Zcx;
            double P1_y, P1_vy, P1_Fy, P1_mey, P1_cy, P1_Zcy;

            double P2_a, P2_w, P2_T, P2_Je, P2_cr, P2_Zcr;         
            double P2_x, P2_vx, P2_Fx, P2_mex, P2_cx, P2_Zcx;
            double P2_y, P2_vy, P2_Fy, P2_mey, P2_cy, P2_Zcy;

            //Read variable values from nodes
            P1_Fx = (*mpP1_Fx);
            P1_Fy = (*mpP1_Fy);
            P1_T = (*mpP1_T);            
            P2_Fx = (*mpP2_Fx);
            P2_Fy = (*mpP2_Fy);
            P2_T = (*mpP2_T);
            M = (*mpM);
            J = (*mpJ);
            dx1 = (*mpDx1);
            dy1 = (*mpDy1);
            dx2 = (*mpDx2);
            dy2 = (*mpDy2);
            angleC = (*mpAngleC);
            xcx = (*mpXcx);
            xcy = (*mpXcy);
            gx = (*mpGx);
            gy = (*mpGy);

            //Initialize filteres
            mIntegratorX.initialize(mTimestep, 0, (-P1_Fx-P2_Fx)/M - gx, xcx, 0);
            mIntegratorY.initialize(mTimestep, 0, (-P1_Fy-P2_Fy)/M - gy, xcy, 0);
            mIntegratorR.initialize(mTimestep, 0, (-P1_T-P2_T-P1_Fx*dy1-P1_Fy*dx1-P2_Fx*dy2-P2_Fy*dx2)/M, angleC, 0);

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
            double M, J, B, Brot, gx, gy;
            double dx1, dy1, dx2, dy2, xcx, vcx, xcy, vcy, angleC, omegaC;

            double P1_a, P1_w, P1_T, P1_Je, P1_cr, P1_Zcr;
            double P1_x, P1_vx, P1_Fx, P1_mex, P1_cx, P1_Zcx;
            double P1_y, P1_vy, P1_Fy, P1_mey, P1_cy, P1_Zcy;

            double P2_a, P2_w, P2_T, P2_Je, P2_cr, P2_Zcr;         
            double P2_x, P2_vx, P2_Fx, P2_mex, P2_cx, P2_Zcx;
            double P2_y, P2_vy, P2_Fy, P2_mey, P2_cy, P2_Zcy;

            //Read variable values from nodes
            P1_cr = (*mpP1_cr);
            P1_Zcr = (*mpP1_Zcr);
            P1_cx = (*mpP1_cx);
            P1_Zcx = (*mpP1_Zcx);
            P1_cy = (*mpP1_cy);
            P1_Zcy = (*mpP1_Zcy);

            P2_cr = (*mpP2_cr);
            P2_Zcr = (*mpP2_Zcr);
            P2_cx = (*mpP2_cx);
            P2_Zcx = (*mpP2_Zcx);
            P2_cy = (*mpP2_cy);
            P2_Zcy = (*mpP2_Zcy);

            M = (*mpM);
            J = (*mpJ);
            B = (*mpB);
            Brot = (*mpBrot);
            angleC = (*mpAngleC);
            gx = (*mpGx);
            gy = (*mpGy);

            //WRITE YOUR EQUATIONS HERE

            //Effective lever arms (= current, depending on angle)
            double dx1e = l1*cos(angleC+th0_1+omegaC*mTimestep);
            double dy1e = l1*sin(angleC+th0_1+omegaC*mTimestep);
            double dx2e = l2*cos(angleC+th0_2+omegaC*mTimestep);
            double dy2e = l2*sin(angleC+th0_2+omegaC*mTimestep);

            double debug1 = angleC+th0_1+omegaC*mTimestep;
            double debug2 = l1;

            //Kinetics (using second order dynamics)
            mIntegratorX.setDamping((B+P1_Zcx+P2_Zcx) / M * mTimestep); 
            mIntegratorX.integrateWithUndo((-P2_cx-P1_cx)/M - gx);
            vcx = mIntegratorX.valueFirst();
            xcx = mIntegratorX.valueSecond();
            
            mIntegratorY.setDamping((B+P1_Zcy+P2_Zcy) / M * mTimestep); 
            mIntegratorY.integrateWithUndo((-P2_cy-P1_cy)/M -gy);
            vcy = mIntegratorY.valueFirst();
            xcy = mIntegratorY.valueSecond();

            double Btot = Brot + P1_Zcx*dy1e*dy1e + P1_Zcy*dx1e*dx1e + P2_Zcx*dy2e*dy2e + P2_Zcy*dx2e*dx2e + P1_Zcr + P2_Zcr; //Total damping factor for rotation
            double ctot = P2_cr + P1_cr + P2_cx*dy2e + P2_cy*dx2e + P1_cx*dy1e + P1_cy*dx1e;	//Total external forces (wave variables) for rotation
            mIntegratorR.setDamping((Btot) / J * mTimestep); 
            mIntegratorR.integrateWithUndo(-ctot/J);
            omegaC = mIntegratorR.valueFirst();
            angleC = mIntegratorR.valueSecond();
	
            //Kinematics
            P1_vx = vcx - omegaC*dy1e;
            P1_x = xcx + dx1e;   
            P1_vy = vcy + omegaC*dx1e;
            P1_y = xcy + dy1e;  

            P2_vx = vcx - omegaC*dy2e;
            P2_x = xcx + dx2e;
            P2_vy = vcy + omegaC*dx2e;
            P2_y = xcy + dy2e;

            P2_w = omegaC;
            P2_a = angleC;
            P1_w = omegaC;
            P1_a = angleC;

            //TLM equations
            P1_T = P1_cr + P1_w*P1_Zcr;
            P1_Fx = P1_cx + P1_vx*P1_Zcx;
            P1_Fy = P1_cy + P1_vy*P1_Zcy;

            P2_T = P2_cr + P2_w*P2_Zcr;
            P2_Fx = P2_cx + P2_vx*P2_Zcx;
            P2_Fy = P2_cy + P2_vy*P2_Zcy;

            //Write new values to nodes
            (*mpP1_a) = P1_a;
            (*mpP1_w) = P1_w;
            (*mpP1_T) = P1_T;
            (*mpP1_Je) = P1_Je;
            (*mpP1_x) = P1_x;
            (*mpP1_vx) = P1_vx;
            (*mpP1_Fx) = P1_Fx;
            (*mpP1_mex) = P1_mex;
            (*mpP1_y) = P1_y;
            (*mpP1_vy) = P1_vy;
            (*mpP1_Fy) = P1_Fy;
            (*mpP1_mey) = P1_mey;

            (*mpP2_a) = P2_a;
            (*mpP2_w) = P2_w;
            (*mpP2_T) = P2_T;
            (*mpP2_Je) = P2_Je;
            (*mpP2_x) = P2_x;
            (*mpP2_vx) = P2_vx;
            (*mpP2_Fx) = P2_Fx;
            (*mpP2_mex) = P2_mex;
            (*mpP2_y) = P2_y;
            (*mpP2_vy) = P2_vy;
            (*mpP2_Fy) = P2_Fy;
            (*mpP2_mey) = P2_mey;

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
            (*mpDebug1) = debug1;
            (*mpDebug2) = debug2;
        }
    };
}

#endif //Hopsan2DBody2_HPP_INCLUDED


