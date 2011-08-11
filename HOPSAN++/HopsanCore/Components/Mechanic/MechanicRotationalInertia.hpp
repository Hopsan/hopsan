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
        double mNumX[3], mNumV[2];
        double mDenX[3], mDenV[2];
//        DoubleIntegratorWithDamping mIntegrator;
        SecondOrderTransferFunction mFilterX;
        FirstOrderTransferFunction mFilterV;
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
            B = 10.0;
            k = 0.0;

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanicRotational");
            mpP2 = addPowerPort("P2", "NodeMechanicRotational");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("J", "Moment of Inertia", "[kgm^2]", J);
            registerParameter("B", "Viscous Friction", "[Nms/rad]", B);
            registerParameter("k", "Spring Constant", "[Nm/rad]", k);
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
            t2 = (*mpND_t2);
            a1 = (*mpND_a1);
            a2 = (*mpND_a2);
            w1 = (*mpND_w1);

            mNumX[0] = 1.0;
            mNumX[1] = 0.0;
            mNumX[2] = 0.0;
            mDenX[0] = k;
            mDenX[1] = B;
            mDenX[2] = J;
            mNumV[0] = 1.0;
            mNumV[1] = 0.0;
            mDenV[0] = B;
            mDenV[1] = J;

//            mIntegrator.initialize(mTimestep, 0, 0, 0, 0);
            mFilterX.initialize(mTimestep, mNumX, mDenX, t1-t2, -a1);
            mFilterV.initialize(mTimestep, mNumV, mDenV, t1-t2 - k*a2, -w1);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            c1 = (*mpND_c1);
            Zx1 = (*mpND_Zx1);
            c2 = (*mpND_c2);
            Zx2 = (*mpND_Zx2);

            //Inertia equations
//            mIntegrator.setDamping((B+Zx1+Zx2)/J*mTimestep);
//            mIntegrator.integrate((c1-c2)/J);
            mDenX[1] = B+Zx1+Zx2;
            mDenV[0] = B+Zx1+Zx2;
            mFilterX.setDen(mDenX);
            mFilterV.setDen(mDenV);

            a2 = mFilterX.update(c1-c2);
            w2 = mFilterV.update(c1-c2 - k*a2);

//            w2 = mIntegrator.valueFirst();
//            a2 = mIntegrator.valueSecond();
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

#endif // MECHANICROTATIONALINERTIA_HPP_INCLUDED

