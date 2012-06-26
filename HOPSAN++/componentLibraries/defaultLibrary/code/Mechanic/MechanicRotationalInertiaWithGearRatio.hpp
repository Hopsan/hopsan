//!
//! @file   MechanicRotationalInertiaWithGearRatio.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2011-03-15
//!
//! @brief Contains a mechanic rotational gear ratio with inertia component
//!
//$Id$

#ifndef MECHANICROTATIONALINERTIAWITHGEARRATIO_HPP_INCLUDED
#define MECHANICROTATIONALINERTIAWITHGEARRATIO_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //Verified with AMESim 2011-03-21
    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicRotationalInertiaWithGearRatio : public ComponentQ
    {

    private:
        double gearRatio, J, B, k;
        double mNumTheta[3];
        double mDenTheta[3];
        double mNumOmega[2];
        double mDenOmega[2];
        SecondOrderTransferFunction mFilterTheta;
        FirstOrderTransferFunction mFilterOmega;
        //        DoubleIntegratorWithDamping mIntegrator;
        double *mpND_t1, *mpND_a1, *mpND_w1, *mpND_c1, *mpND_Zx1,
               *mpND_t2, *mpND_a2, *mpND_w2, *mpND_c2, *mpND_Zx2;
        double t1, a1, w1, c1, Zx1,
               t2, a2, w2, c2, Zx2;
        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new MechanicRotationalInertiaWithGearRatio();
        }

        MechanicRotationalInertiaWithGearRatio() : ComponentQ()
        {
            //Set member attributes
            gearRatio = 1;
            J = 1.0;
            B = 10;

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanicRotational");
            mpP2 = addPowerPort("P2", "NodeMechanicRotational");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("omega", "Gear ratio", "[-]", gearRatio);
            registerParameter("J", "Moment of Inertia", "[kgm^2]", J);
            registerParameter("B", "Viscous Friction", "[Nms/rad]", B);
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

            t1 = (*mpND_t1)*gearRatio;
            a1 = (*mpND_a1)/gearRatio;
            w1 = (*mpND_w1)/gearRatio;
            t2 = (*mpND_t2);
            a2 = (*mpND_a2);
            w2 = (*mpND_w2);

            mNumTheta[0] = 1.0;
            mNumTheta[1] = 0.0;
            mNumTheta[2] = 0.0;
            mDenTheta[0] = 0;
            mDenTheta[1] = B;
            mDenTheta[2] = J;
            mNumOmega[0] = 1.0;
            mNumOmega[1] = 0.0;
            mDenOmega[0] = B;
            mDenOmega[1] = J;

            mFilterTheta.initialize(mTimestep, mNumTheta, mDenTheta, t1-t2, -a1);
            mFilterOmega.initialize(mTimestep, mNumOmega, mDenOmega, t1-t2, -w1);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            c1  = (*mpND_c1)*gearRatio;
            Zx1 = (*mpND_Zx1)*pow(gearRatio, 2.0);
            c2  = (*mpND_c2);
            Zx2 = (*mpND_Zx2);

            //Mass equations
            mDenTheta[1] = (B+Zx1+Zx2);
            mDenOmega[0] = (B+Zx1+Zx2);
            mFilterTheta.setDen(mDenTheta);
            mFilterOmega.setDen(mDenOmega);

            a2 = mFilterTheta.update(c1-c2);
            w2 = mFilterOmega.update(c1-c2);
            t2 = c2 + Zx2*w2;

            w1 = -w2*gearRatio;
            a1 = -a2*gearRatio;
            t1 = (c1 + Zx1*w1)/gearRatio;


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

#endif // MECHANICROTATIONALINERTIAWITHGEARRATIO_HPP_INCLUDED

