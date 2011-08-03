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
//! @file   MechanicRotationalInertia.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-08-05
//!
//! @brief Contains a mechanic rotational inertia component
//!
//$Id$

#ifndef MECHANICROTATIONALINERTIA_HPP_INCLUDED
#define MECHANICROTATIONALINERTIA_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicRotationalInertia : public ComponentQ
    {

    private:
        double J, B, k;
        double num[3];
        double den[3];
        SecondOrderTransferFunction mFilter;
        Integrator mInt;
        double *mpND_t1, *mpND_a1, *mpND_w1, *mpND_c1, *mpND_Zx1, *mpND_t2, *mpND_a2, *mpND_w2, *mpND_c2, *mpND_Zx2;
        double t1, a1, w1, c1, Zx1, t2, a2, w2, c2, Zx2;
        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new MechanicRotationalInertia("RotationalInertia");
        }

        MechanicRotationalInertia(const std::string name) : ComponentQ(name)
        {
            //Set member attributes
            J = 0.1;
            B = 10;
            k = 0.0;

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanicRotational");
            mpP2 = addPowerPort("P2", "NodeMechanicRotational");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("J", "Moment of Inertia", "[kgm^2]", J);
            registerParameter("B", "Viscous Friction", "[Nms/rad]", B);
            registerParameter("k", "Spring Coefficient", "[Nm/rad]", k);
        }


        void initialize()
        {
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

            num[0] = 0.0;
            num[1] = 1.0;
            num[2] = 0.0;
            den[0] = k;
            den[1] = B;
            den[2] = J;
            mFilter.initialize(mTimestep, num, den, -t1, -w1);
            mInt.initialize(mTimestep, -w1, -a1);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            c1 = (*mpND_c1);
            Zx1 = (*mpND_Zx1);
            c2 = (*mpND_c2);
            Zx2 = (*mpND_Zx2);

            //Mass equations
            den[1] = B+Zx1+Zx2;

            mFilter.setNumDen(num, den);
            w2 = mFilter.update(c1-c2);
            w1 = -w2;
            a2 = mInt.update(w2);
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

#endif // MECHANICROTATIONALINERTIA_HPP_INCLUDED

