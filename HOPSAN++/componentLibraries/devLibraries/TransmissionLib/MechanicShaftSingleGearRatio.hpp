//!
//! @file   MechanicRotationalInertiaWithGearRatio.hpp
//! @author Karl Pettersson <karl.pettersson@liu.se>
//! @date   2011-03-15
//!
//! @brief Contains a mechanic rotational shaft with gear ratio
//!
//$Id: MechanicRotationalShaftGearRatio.hpp 3683 2011-11-29 11:50:41Z robbr48 $

#ifndef MECHANICSHAFTSINGLEGEARRATIO_HPP_INCLUDED
#define MECHANICSHAFTSINGLEGEARRATIO_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //
    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicShaftSingleGearRatio : public ComponentQ
    {
    private:
        // Declare member variables
        double gearRatio, J, B, k;
        double num[3];
        double den[3];
        DoubleIntegratorWithDamping mIntegrator;

        // Declare port and node data pointers
        Port *mpP1, *mpP2, *mpP3;
        double *mpP1_t, *mpP1_a, *mpP1_w, *mpP1_c, *mpP1_Zx,
               *mpP2_t, *mpP2_a, *mpP2_w, *mpP2_c, *mpP2_Zx,
               *mpP3_t, *mpP3_a, *mpP3_w, *mpP3_c, *mpP3_Zx;

    public:
        static Component *Creator()
        {
            return new MechanicShaftSingleGearRatio();
        }

        void configure()
        {
            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanicRotational");
            mpP2 = addPowerPort("P2", "NodeMechanicRotational");
            mpP3 = addPowerPort("P3", "NodeMechanicRotational");

            //Register changable parameters to the HOPSAN++ core
            addConstant("omega", "Gear ratio", "[-]", 1, gearRatio);
            addConstant("J", "Moment of Inertia", "[kgm^2]", 1.0, J);
            addConstant("B", "Viscous Friction", "[Nms/rad]", 10, B);
        }


        void initialize()
        {
            mpP1_t = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::Torque);
            mpP1_a = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::Angle);
            mpP1_w = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::AngularVelocity);
            mpP1_c = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::WaveVariable);
            mpP1_Zx = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::CharImpedance);

            mpP2_t = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::Torque);
            mpP2_a = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::Angle);
            mpP2_w = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::AngularVelocity);
            mpP2_c = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::WaveVariable);
            mpP2_Zx = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::CharImpedance);

            mpP3_t = getSafeNodeDataPtr(mpP3, NodeMechanicRotational::Torque);
            mpP3_a = getSafeNodeDataPtr(mpP3, NodeMechanicRotational::Angle);
            mpP3_w = getSafeNodeDataPtr(mpP3, NodeMechanicRotational::AngularVelocity);
            mpP3_c = getSafeNodeDataPtr(mpP3, NodeMechanicRotational::WaveVariable);
            mpP3_Zx = getSafeNodeDataPtr(mpP3, NodeMechanicRotational::CharImpedance);

            //! @todo The integrator should be initialized (angel vel and angle and intial torque)
            mIntegrator.initialize(mTimestep, 0, 0, 0, 0);
        }


        void simulateOneTimestep()
        {
            // Declare local variables
            double t1, a1, w1,
                   t2, a2, w2,
                   t3, a3, w3;

            //Get variable values from nodes
            const double c1  = (*mpP1_c);
            const double Zx1 = (*mpP1_Zx);
            const double c2  = (*mpP2_c);
            const double Zx2 = (*mpP2_Zx);
            const double c3 = (*mpP3_c);
            const double Zx3 = (*mpP3_Zx);

            //Mass equations
            mIntegrator.setDamping((B+pow(gearRatio,2)*(Zx1+Zx2)-Zx3) / J * mTimestep);
            mIntegrator.integrateWithUndo(c3+(c2-c1)*gearRatio/J);
            w3 = mIntegrator.valueFirst();
            a3 = mIntegrator.valueSecond();
            w1 = w3*gearRatio;
            a1 = a3*gearRatio;
            w2 = -w1;
            a2 = -a1;

            t1 = c1 + Zx1*w1;
            t2 = c2 + Zx2*w2;
            t3 = c3 + Zx3*w3;

            //Write new values to nodes
            (*mpP1_t) = t1;
            (*mpP1_a) = a1;
            (*mpP1_w) = w1;
            (*mpP2_t) = t2;
            (*mpP2_a) = a2;
            (*mpP2_w) = w2;
            (*mpP3_t) = t3;
            (*mpP3_a) = a3;
            (*mpP3_w) = w3;
        }
    };
}

#endif // MECHANICSHAFTSINGLEGEARRATIO_HPP_INCLUDED

