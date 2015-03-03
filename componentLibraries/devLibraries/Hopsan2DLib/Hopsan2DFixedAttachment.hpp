#ifndef Hopsan2DFixedAttachment_HPP_INCLUDED
#define Hopsan2DFixedAttachment_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    class Hopsan2DFixedAttachment : public ComponentQ
    {
    private:
        //Power port pointers
        Port *mpP1;

        double Je, mex, mey;

        //Power port node data pointers
        double *mpP1_a, *mpP1_w, *mpP1_T, *mpP1_Je, *mpP1_cr, *mpP1_Zcr;
        double *mpP1_x, *mpP1_vx, *mpP1_Fx, *mpP1_mex, *mpP1_cx, *mpP1_Zcx;
        double *mpP1_y, *mpP1_vy, *mpP1_Fy, *mpP1_mey, *mpP1_cy, *mpP1_Zcy;

    public:
        static Component *Creator()
        {
            return new Hopsan2DFixedAttachment();
        }

        void configure()
        {
            //Register constant parameters
            addConstant("m_ex", "Equivalent Inertia", "kgm^2", 1, Je);
            addConstant("m_ex", "Equivalent Mass in X-direction", "kg", 1, mex);
            addConstant("m_ey", "Equivalent Mass in Y-direction", "kg", 1, mey);

            //Add power ports
            mpP1 = addPowerPort("P1", "NodeMechanic2D", "");
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

            (*mpP1_Je) = Je;
            (*mpP1_vx) = 0;
            (*mpP1_mex) = mex;
            (*mpP1_vy) = 0;
            (*mpP1_mey) = mey;
        }


        void simulateOneTimestep()
        {
            //Declare local variables
            double P1_a, P1_w, P1_T, P1_Je, P1_cr, P1_Zcr;
            double P1_x, P1_vx, P1_Fx, P1_mex, P1_cx, P1_Zcx;
            double P1_y, P1_vy, P1_Fy, P1_mey, P1_cy, P1_Zcy;

            //Read variable values from nodes
            P1_cr = (*mpP1_cr);
            P1_Zcr = (*mpP1_Zcr);
            P1_cx = (*mpP1_cx);
            P1_cy = (*mpP1_cy);
        
            //WRITE YOUR EQUATIONS HERE
            (*mpP1_w) = -P1_cr/P1_Zcr;
            (*mpP1_T) = 0;
            (*mpP1_Fx) = (*mpP1_cx);
            (*mpP1_Fy) = (*mpP1_cx);
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

#endif //Hopsan2DFixedAttachment_HPP_INCLUDED


