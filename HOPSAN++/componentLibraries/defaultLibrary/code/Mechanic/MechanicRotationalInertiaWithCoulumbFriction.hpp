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

#include <sstream>

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
        double J, B, ts, tk;     //Changeable parameters
        double *mpND_t1, *mpND_a1, *mpND_w1, *mpND_c1, *mpND_Zx1, *mpND_t2, *mpND_a2, *mpND_w2, *mpND_c2, *mpND_Zx2;  //Node data pointers
        double t1, a1, w1, c1, Zx1, t2, a2, w2, c2, Zx2;                                                    //Node data variables
        //DoubleIntegratorWithDamping mIntegrator;                                                            //External functions
        double mNum[3];
        double mDen[3];
        DoubleIntegratorWithDampingAndCoulombFriction mIntegrator;
//        SecondOrderFilter mFilter;
//        Integrator mInt;
        Port *mpP1, *mpP2;                                                                                  //Ports

    public:
        static Component *Creator()
        {
            return new MechanicRotationalInertiaWithCoulombFriction();
        }

        void configure()
        {
            //Set member attributes
            J = 1.0;
            B = 10;
            ts = 50;
            tk = 45;

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanicRotational");
            mpP2 = addPowerPort("P2", "NodeMechanicRotational");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("J", "Inertia", "[kgm^2]", J);
            registerParameter("B", "Viscous Friction Coefficient", "[Nms/rad]", B);
            registerParameter("t_s", "Static Friction Torque", "[Nm]",  ts);
            registerParameter("t_k", "Kinetic Friction Torque", "[Nm]",  tk);
        }


        void initialize()
        {
            //Assign node data pointers
            mpND_t1 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::TORQUE);
            mpND_a1 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::ANGLE);
            mpND_w1 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::ANGULARVELOCITY);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::WAVEVARIABLE);
            mpND_Zx1 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::CHARIMP);

            mpND_t2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::TORQUE);
            mpND_a2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::ANGLE);
            mpND_w2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::ANGULARVELOCITY);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::WAVEVARIABLE);
            mpND_Zx2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::CHARIMP);

            t1 = (*mpND_t1);
            a1 = (*mpND_a1);
            w1 = (*mpND_w1);
            c1 = (*mpND_c1);
            a2 = (*mpND_a2);
            c2 = (*mpND_c2);

            mIntegrator.initialize(mTimestep, 0, J, ts, tk, 0, 0, 0);

            //Print debug message if start angles or velocities doe not match
            if((*mpND_a1) != -(*mpND_a2))
            {
                std::stringstream ss;
                ss << "Start angles does not match, {" << getName() << "::" << mpP1->getPortName() <<
                        "} and {" << getName() << "::" << mpP2->getPortName() << "}.";
                this->addWarningMessage(ss.str());
            }
            if((*mpND_w1) != -(*mpND_w2))
            {
                std::stringstream ss;
                ss << "Start velocities does not match, {" << getName() << "::" << mpP1->getPortName() <<
                        "} and {" << getName() << "::" << mpP2->getPortName() << "}.";
                this->addWarningMessage(ss.str());
            }
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            a1 = (*mpND_a1);
            c1 = (*mpND_c1);
            Zx1 = (*mpND_Zx1);
            a2 = (*mpND_a2);
            c2 = (*mpND_c2);
            Zx2 = (*mpND_Zx2);

            mIntegrator.setDamping((B+Zx1+Zx2) / J * mTimestep);
            mIntegrator.integrateWithUndo((c1-c2)/J);
            w2 = mIntegrator.valueFirst();
            a2 = mIntegrator.valueSecond();

            w1 = -w2;
            a1 = -a2;
            t1 = c1 + Zx1*w1;
            t2 = c2 + Zx2*w2;

            //Write new values to nodes
            (*mpND_t1) = t1;
            (*mpND_a1) = a1;
            (*mpND_w1) = w1;
            (*mpND_t2) = t2;
            (*mpND_a2) = a2;
            (*mpND_w2) = w2;
        }
    };
}

#endif // MECHANICROTATIONALINERTIAWITHCOULUMBFRICTION_HPP_INCLUDED

