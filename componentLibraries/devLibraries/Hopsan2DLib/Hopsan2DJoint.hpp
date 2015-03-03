#ifndef Hopsan2DJoint_HPP_INCLUDED
#define Hopsan2DJoint_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    class Hopsan2DJoint : public ComponentC
    {
    private:
        //Input variable node data pointers
        double *mpK, *mpAlpha;

        //Power port pointers
        Port *mpP1, *mpP2;

        //Power port node data pointers
        double *mpP1_a, *mpP1_w, *mpP1_T, *mpP1_Je, *mpP1_cr, *mpP1_Zcr;
        double *mpP1_x, *mpP1_vx, *mpP1_Fx, *mpP1_mex, *mpP1_cx, *mpP1_Zcx;
        double *mpP1_y, *mpP1_vy, *mpP1_Fy, *mpP1_mey, *mpP1_cy, *mpP1_Zcy;

        double *mpP2_a, *mpP2_w, *mpP2_T, *mpP2_Je, *mpP2_cr, *mpP2_Zcr;
        double *mpP2_x, *mpP2_vx, *mpP2_Fx, *mpP2_mex, *mpP2_cx, *mpP2_Zcx;
        double *mpP2_y, *mpP2_vy, *mpP2_Fy, *mpP2_mey, *mpP2_cy, *mpP2_Zcy;

    public:
        static Component *Creator()
        {
            return new Hopsan2DJoint();
        }

        void configure()
        {
            //Register constant parameters
            //Register input variables
            addInputVariable("k", "Spring coefficient", "N/m", 10000, &mpK);
            addInputVariable("alpha", "Damping factor", "-", 0.3, &mpAlpha);
            //Register output variables
            //Add power ports
            mpP1 = addPowerPort("P1", "NodeMechanic2D", "");
            mpP2 = addPowerPort("P2", "NodeMechanic2D", "");
            //Set default power port start values
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
            double k;
            
            double P1_a, P1_w, P1_T, P1_Je, P1_cr, P1_Zcr;
            double P1_x, P1_vx, P1_Fx, P1_mex, P1_cx, P1_Zcx;
            double P1_y, P1_vy, P1_Fy, P1_mey, P1_cy, P1_Zcy;

            double P2_a, P2_w, P2_T, P2_Je, P2_cr, P2_Zcr;         
            double P2_x, P2_vx, P2_Fx, P2_mex, P2_cx, P2_Zcx;
            double P2_y, P2_vy, P2_Fy, P2_mey, P2_cy, P2_Zcy;

            //Initialize rotational wave variables and impedance to zero torque
            P1_cr = 0.0;
            P1_Zcr = 0.0;
            P2_cr = 0.0;
            P2_Zcr = 0.0;

            //Write new values to nodes
            (*mpP1_cr) = P1_cr;
            (*mpP1_Zcr) = P1_Zcr;

            (*mpP2_cr) = P2_cr;
            (*mpP2_Zcr) = P2_Zcr;
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double k, alpha;
            
            double P1_a, P1_w, P1_T, P1_Je, P1_cr, P1_Zcr;
            double P1_x, P1_vx, P1_Fx, P1_mex, P1_cx, P1_Zcx;
            double P1_y, P1_vy, P1_Fy, P1_mey, P1_cy, P1_Zcy;

            double P2_a, P2_w, P2_T, P2_Je, P2_cr, P2_Zcr;         
            double P2_x, P2_vx, P2_Fx, P2_mex, P2_cx, P2_Zcx;
            double P2_y, P2_vy, P2_Fy, P2_mey, P2_cy, P2_Zcy;

            //Read variable values from nodes
            k = (*mpK);
            alpha = (*mpAlpha);

            P1_a = (*mpP1_a);
            P1_w = (*mpP1_w);
            P1_T = (*mpP1_T);
            P1_Je = (*mpP1_Je);
            P1_x = (*mpP1_x);
            P1_vx = (*mpP1_vx);
            P1_Fx = (*mpP1_Fx);
            P1_mex = (*mpP1_mex);
            P1_y = (*mpP1_y);
            P1_vy = (*mpP1_vy);
            P1_Fy = (*mpP1_Fy);
            P1_mey = (*mpP1_mey);
            P1_cr = (*mpP1_cr);
            P1_cx = (*mpP1_cx);
            P1_cy = (*mpP1_cy);
            
            P2_a = (*mpP2_a);
            P2_w = (*mpP2_w);
            P2_T = (*mpP2_T);
            P2_Je = (*mpP2_Je);
            P2_x = (*mpP2_x);
            P2_vx = (*mpP2_vx);
            P2_Fx = (*mpP2_Fx);
            P2_mex = (*mpP2_mex);
            P2_y = (*mpP2_y);
            P2_vy = (*mpP2_vy);
            P2_Fy = (*mpP2_Fy);
            P2_mey = (*mpP2_mey);
            P2_cr = (*mpP2_cr);
            P2_cx = (*mpP2_cx);
            P2_cy = (*mpP2_cy);
            
            //Calculate impedance
            const double Zc = (*mpK)*mTimestep/(1.0-alpha);
            
            //Calculate ideal pressures using TLM equations
            double P1_cx0 = -P2_cx - 2.0*Zc*P2_vx;
            double P2_cx0 = -P1_cx - 2.0*Zc*P1_vx;
            double P1_cy0 = -P2_cy - 2.0*Zc*P2_vy;
            double P2_cy0 = -P1_cy - 2.0*Zc*P1_vy;
            
            //Apply damping factor
            P1_cx = alpha*P1_cx + (1.0-alpha)*P1_cx0;
            P2_cx = alpha*P2_cx + (1.0-alpha)*P2_cx0;            
            P1_cy = alpha*P1_cy + (1.0-alpha)*P1_cy0;
            P2_cy = alpha*P2_cy + (1.0-alpha)*P2_cy0;            

            //Write new values to nodes
            (*mpP1_cr) = 0;
            (*mpP1_Zcr) = 0;
            (*mpP1_cx) = P1_cx;
            (*mpP1_Zcx) = Zc;
            (*mpP1_cy) = P1_cy;
            (*mpP1_Zcy) = Zc;

            (*mpP2_cr) = 0;
            (*mpP2_Zcr) = 0;
            (*mpP2_cx) = P2_cx;
            (*mpP2_Zcx) = Zc;
            (*mpP2_cy) = P2_cy;
            (*mpP2_Zcy) = Zc;
        }
    };
}

#endif //Hopsan2DJoint_HPP_INCLUDED
