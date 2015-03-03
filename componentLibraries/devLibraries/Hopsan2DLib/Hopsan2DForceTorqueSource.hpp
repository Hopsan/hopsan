#ifndef Hopsan2DForceTorqueSource_HPP_INCLUDED
#define Hopsan2DForceTorqueSource_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    class Hopsan2DForceTorqueSource : public ComponentC
    {
    private:
        //Input variable node data pointers
        double *mpT, *mpFx, *mpFy;

        //Power port pointers
        Port *mpP1;

        //Power port node data pointers
        double *mpP1_a, *mpP1_w, *mpP1_T, *mpP1_Je, *mpP1_cr, *mpP1_Zcr;
        double *mpP1_x, *mpP1_vx, *mpP1_Fx, *mpP1_mex, *mpP1_cx, *mpP1_Zcx;
        double *mpP1_y, *mpP1_vy, *mpP1_Fy, *mpP1_mey, *mpP1_cy, *mpP1_Zcy;

    public:
        static Component *Creator()
        {
            return new Hopsan2DForceTorqueSource();
        }

        void configure()
        {
            addInputVariable("T", "Torque", "Nm", 0, &mpT);
            addInputVariable("Fx", "Force in X-direction", "N", 0, &mpFx);
            addInputVariable("Fy", "Force in y-direction", "N", 0, &mpFy);

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

            (*mpP1_T) = (*mpT);
            (*mpP1_Fx) = (*mpFx);
            (*mpP1_Fy) = (*mpFy);

            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            (*mpP1_cr) = (*mpT);
            (*mpP1_Zcr) = 0.0;
            (*mpP1_cx) = (*mpFx);
            (*mpP1_Zcx) = 0.0;
            (*mpP1_cy) = (*mpFy);
            (*mpP1_Zcy) = 0.0;
        }
    };
}

#endif //Hopsan2DForceTorqueSource_HPP_INCLUDED


