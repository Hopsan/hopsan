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
        SecondOrderFilter mFilter;
        Integrator mInt;
        double *t1_ptr, *a1_ptr, *w1_ptr, *c1_ptr, *Zx1_ptr, *t2_ptr, *a2_ptr, *w2_ptr, *c2_ptr, *Zx2_ptr;
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
            mTypeName = "MechanicRotationalInertia";
            J = 1.0;
            B    = 10;
            k   = 0.0;

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanicRotational");
            mpP2 = addPowerPort("P2", "NodeMechanicRotational");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("J", "Moment of Inertia", "[kgm^2]", J);
            registerParameter("B", "Viscous Friction", "[Ns/rad]", B);
            registerParameter("k", "Spring Coefficient", "[N/rad]", k);
        }


        void initialize()
        {
            t1_ptr = mpP1->getNodeDataPtr(NodeMechanicRotational::TORQUE);
            a1_ptr = mpP1->getNodeDataPtr(NodeMechanicRotational::ANGLE);
            w1_ptr = mpP1->getNodeDataPtr(NodeMechanicRotational::ANGULARVELOCITY);
            c1_ptr = mpP1->getNodeDataPtr(NodeMechanicRotational::WAVEVARIABLE);
            Zx1_ptr = mpP1->getNodeDataPtr(NodeMechanicRotational::CHARIMP);

            t2_ptr = mpP2->getNodeDataPtr(NodeMechanicRotational::TORQUE);
            a2_ptr = mpP2->getNodeDataPtr(NodeMechanicRotational::ANGLE);
            w2_ptr = mpP2->getNodeDataPtr(NodeMechanicRotational::ANGULARVELOCITY);
            c2_ptr = mpP2->getNodeDataPtr(NodeMechanicRotational::WAVEVARIABLE);
            Zx2_ptr = mpP2->getNodeDataPtr(NodeMechanicRotational::CHARIMP);

            t1 = (*t1_ptr);
            a1 = (*a1_ptr);
            w1 = (*w1_ptr);

            num[0] = 0.0;
            num[1] = 1.0;
            num[2] = 0.0;
            den[0] = J;
            den[1] = B;
            den[2] = k;
            mFilter.initialize(mTimestep, num, den, -t1, -w1);
            mInt.initialize(mTimestep, -w1, -a1);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            c1 = (*c1_ptr);
            Zx1 = (*Zx1_ptr);
            c2 = (*c2_ptr);
            Zx2 = (*Zx2_ptr);

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
            (*t1_ptr) = t1;
            (*a1_ptr) = a1;
            (*w1_ptr) = w1;
            (*t2_ptr) = t2;
            (*a2_ptr) = a2;
            (*w2_ptr) = w2;
        }
    };
}

#endif // MECHANICROTATIONALINERTIA_HPP_INCLUDED

