/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   MechanicRotationalInertiaWithCoulumbFriction.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-09-01
//!
//! @brief Contains a rotational inertia with coulumb friction and damper
//!
//$Id$

#ifndef MECHANICROTATIONALINERTIAWITHCOULUMBFRICTION_HPP_INCLUDED
#define MECHANICROTATIONALINERTIAWITHCOULUMBFRICTION_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicRotationalInertiaWithCoulombFriction : public ComponentQ
    {

    private:
        double *mpJ, *mpB;
        double ts, tk;
        double *mpP1_T, *mpP1_a, *mpP1_w, *mpP1_c, *mpP1_Zx, *mpP2_T, *mpP2_a, *mpP2_w, *mpP2_c, *mpP2_Zx;  //Node data pointers
        DoubleIntegratorWithDampingAndCoulombFriction mIntegrator;
        Port *mpP1, *mpP2;                                                                                  //Ports

    public:
        static Component *Creator()
        {
            return new MechanicRotationalInertiaWithCoulombFriction();
        }

        void configure()
        {
            mpP1 = addPowerPort("P1", "NodeMechanicRotational");
            mpP2 = addPowerPort("P2", "NodeMechanicRotational");
            addInputVariable("J", "Inertia", "[kgm^2]", 1.0, &mpJ);
            addInputVariable("B", "Viscous Friction Coefficient", "[Nms/rad]", 10, &mpB);
            addConstant("t_s", "Static Friction Torque", "[Nm]", 50, ts);
            addConstant("t_k", "Kinetic Friction Torque", "[Nm]", 45, tk);
        }


        void initialize()
        {
            //Assign node data pointers
            mpP1_T = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::Torque);
            mpP1_a = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::Angle);
            mpP1_w = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::AngularVelocity);
            mpP1_c = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::WaveVariable);
            mpP1_Zx = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::CharImpedance);

            mpP2_T = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::Torque);
            mpP2_a = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::Angle);
            mpP2_w = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::AngularVelocity);
            mpP2_c = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::WaveVariable);
            mpP2_Zx = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::CharImpedance);

            // Print debug message if start angles or velocities doe not match
            if( !fuzzyEqual((*mpP1_a), -(*mpP2_a)) )
            {
                addWarningMessage("Start angles does not match in:  "+mpP1->getName()+"  and  "+mpP2->getName());
            }
            if( !fuzzyEqual((*mpP1_w), -(*mpP2_w)) )
            {
                addWarningMessage("Start velocities does not match in:  "+mpP1->getName()+"  and  "+mpP2->getName());
            }

            // Initialize
            //! @todo the integrator is not initialized with correct torque angle and angular vel
            mIntegrator.initialize(mTimestep, 0, (*mpJ), ts, tk, (*mpP1_T)-(*mpP2_T), (*mpP2_a), (*mpP2_w));
        }


        void simulateOneTimestep()
        {
            double t1, a1, w1, c1, Zx1, t2, a2, w2, c2, Zx2, J, B;

            //Get variable values from nodes
            a1 = (*mpP1_a);
            c1 = (*mpP1_c);
            Zx1 = (*mpP1_Zx);
            a2 = (*mpP2_a);
            c2 = (*mpP2_c);
            Zx2 = (*mpP2_Zx);
            J = (*mpJ);
            B = (*mpB);

            mIntegrator.setDamping((B+Zx1+Zx2) / J * mTimestep);
            mIntegrator.integrateWithUndo((c1-c2)/J);
            w2 = mIntegrator.valueFirst();
            a2 = mIntegrator.valueSecond();

            w1 = -w2;
            a1 = -a2;
            t1 = c1 + Zx1*w1;
            t2 = c2 + Zx2*w2;

            //Write new values to nodes
            (*mpP1_T) = t1;
            (*mpP1_a) = a1;
            (*mpP1_w) = w1;
            (*mpP2_T) = t2;
            (*mpP2_a) = a2;
            (*mpP2_w) = w2;
        }
    };
}

#endif // MECHANICROTATIONALINERTIAWITHCOULUMBFRICTION_HPP_INCLUDED

