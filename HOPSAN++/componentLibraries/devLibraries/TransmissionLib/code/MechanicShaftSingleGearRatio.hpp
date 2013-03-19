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
        double gearRatio, J, B, k;
        double num[3];
        double den[3];
        DoubleIntegratorWithDamping mIntegrator;
        double *mpND_t1, *mpND_a1, *mpND_w1, *mpND_c1, *mpND_Zx1,
               *mpND_t2, *mpND_a2, *mpND_w2, *mpND_c2, *mpND_Zx2,
               *mpND_t3, *mpND_a3, *mpND_w3, *mpND_c3, *mpND_Zx3;
        double t1, a1, w1, c1, Zx1,
               t2, a2, w2, c2, Zx2,
               t3, a3, w3, c3, Zx3;
        Port *mpP1, *mpP2, *mpP3;

    public:
        static Component *Creator()
        {
            return new MechanicShaftSingleGearRatio();
        }

        void configure()
        {
            //Set member attributes
            gearRatio = 1;
            J = 1.0;
            B = 10;

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanicRotational");
            mpP2 = addPowerPort("P2", "NodeMechanicRotational");
            mpP3 = addPowerPort("P3", "NodeMechanicRotational");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("omega", "Gear ratio", "[-]", gearRatio);
            registerParameter("J", "Moment of Inertia", "[kgm^2]", J);
            registerParameter("B", "Viscous Friction", "[Nms/rad]", B);
        }


        void initialize()
        {
            mpND_t1 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::Torque);
            mpND_a1 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::Angle);
            mpND_w1 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::AngularVelocity);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::WaveVariable);
            mpND_Zx1 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::CharImpedance);

            mpND_t2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::Torque);
            mpND_a2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::Angle);
            mpND_w2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::AngularVelocity);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::WaveVariable);
            mpND_Zx2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::CharImpedance);

            mpND_t3 = getSafeNodeDataPtr(mpP3, NodeMechanicRotational::Torque);
            mpND_a3 = getSafeNodeDataPtr(mpP3, NodeMechanicRotational::Angle);
            mpND_w3 = getSafeNodeDataPtr(mpP3, NodeMechanicRotational::AngularVelocity);
            mpND_c3 = getSafeNodeDataPtr(mpP3, NodeMechanicRotational::WaveVariable);
            mpND_Zx3 = getSafeNodeDataPtr(mpP3, NodeMechanicRotational::CharImpedance);

            t1 = (*mpND_t1);
            a1 = (*mpND_a1);
            w1 = (*mpND_w1);

            mIntegrator.initialize(mTimestep, 0, 0, 0, 0);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            c1  = (*mpND_c1);
            Zx1 = (*mpND_Zx1);
            c2  = (*mpND_c2);
            Zx2 = (*mpND_Zx2);
            c3 = (*mpND_c3);
            Zx3 = (*mpND_Zx3);

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
            (*mpND_t1) = t1;
            (*mpND_a1) = a1;
            (*mpND_w1) = w1;
            (*mpND_t2) = t2;
            (*mpND_a2) = a2;
            (*mpND_w2) = w2;
            (*mpND_t3) = t3;
            (*mpND_a3) = a3;
            (*mpND_w3) = w3;
        }
    };
}

#endif // MECHANICSHAFTSINGLEGEARRATIO_HPP_INCLUDED

