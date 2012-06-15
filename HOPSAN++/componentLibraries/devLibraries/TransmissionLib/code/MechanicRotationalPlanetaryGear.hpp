//!
//! @file   MechanicRotationalPlanetaryGear.hpp
//! @author Karl Pettersson <karl.pettersson@liu.se>
//! @date   2011-02-14
//!
//! @brief Contains a mechanic rotational planetary gear component
//!
//$Id: MechanicRotationalPlanetaryGear.hpp 3683 2011-11-29 11:50:41Z robbr48 $

#ifndef MECHANICROTATIONALPLANETARYGEAR_HPP_INCLUDED
#define MECHANICROTATIONALPLANETARYGEAR_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicRotationalPlanetaryGear : public ComponentQ
    {

    private:
        double J, B, k, R;
        double mNumX[3],mNumV[2];
        double mDenX[3],mDenV[2];
//        DoubleIntegratorWithDamping mIntegrator;
        SecondOrderTransferFunction mFilterX;
        FirstOrderTransferFunction mFilterV;
        Integrator mInt1;
        Integrator mInt2;
        Integrator mInt3;
        double *mpND_t1A, *mpND_a1A, *mpND_w1A, *mpND_c1A, *mpND_Zx1A,
        *mpND_t2B, *mpND_a2B, *mpND_w2B, *mpND_c2B, *mpND_Zx2B,
        *mpND_t3A, *mpND_a3A, *mpND_w3A, *mpND_c3A, *mpND_Zx3A;
        double t1A, a1A, w1A, c1A, Zx1A, t2B, a2B, w2B, c2B, Zx2B, t3A, a3A, w3A, c3A, Zx3A;
        Port *mpP1A, *mpP2B, *mpP3A;

    public:
        static Component *Creator()
        {
            return new MechanicRotationalPlanetaryGear();
        }

        MechanicRotationalPlanetaryGear() : ComponentQ()
        {
            //Set member attributes
            J = 0.1;
            B = 10.0;
            k = 0.0;
            R = -1;

            //Add ports to the component
            mpP1A = addPowerPort("P1A", "NodeMechanicRotational");
            mpP2B = addPowerPort("P2B", "NodeMechanicRotational");
            mpP3A = addPowerPort("P3A", "NodeMechanicRotational");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("J", "Moment of Inertia", "[kgm^2]", J);
            registerParameter("B", "Viscous Friction", "[Nms/rad]", B);
            registerParameter("R", "Gear Ratio", "[Nms/rad]", R);
            registerParameter("k", "Spring Constant", "[Nm/rad]", k);
        }


        void initialize()
        {
            mpND_t1A = getSafeNodeDataPtr(mpP1A, NodeMechanicRotational::TORQUE);
            mpND_a1A = getSafeNodeDataPtr(mpP1A, NodeMechanicRotational::ANGLE);
            mpND_w1A = getSafeNodeDataPtr(mpP1A, NodeMechanicRotational::ANGULARVELOCITY);
            mpND_c1A = getSafeNodeDataPtr(mpP1A, NodeMechanicRotational::WAVEVARIABLE);
            mpND_Zx1A = getSafeNodeDataPtr(mpP1A, NodeMechanicRotational::CHARIMP);

            mpND_t2B = getSafeNodeDataPtr(mpP2B, NodeMechanicRotational::TORQUE);
            mpND_a2B = getSafeNodeDataPtr(mpP2B, NodeMechanicRotational::ANGLE);
            mpND_w2B = getSafeNodeDataPtr(mpP2B, NodeMechanicRotational::ANGULARVELOCITY);
            mpND_c2B = getSafeNodeDataPtr(mpP2B, NodeMechanicRotational::WAVEVARIABLE);
            mpND_Zx2B = getSafeNodeDataPtr(mpP2B, NodeMechanicRotational::CHARIMP);

            mpND_t3A = getSafeNodeDataPtr(mpP3A, NodeMechanicRotational::TORQUE);
            mpND_a3A = getSafeNodeDataPtr(mpP3A, NodeMechanicRotational::ANGLE);
            mpND_w3A = getSafeNodeDataPtr(mpP3A, NodeMechanicRotational::ANGULARVELOCITY);
            mpND_c3A = getSafeNodeDataPtr(mpP3A, NodeMechanicRotational::WAVEVARIABLE);
            mpND_Zx3A = getSafeNodeDataPtr(mpP3A, NodeMechanicRotational::CHARIMP);

            t1A = (*mpND_t1A);
            t2B = (*mpND_t2B);
            t3A = (*mpND_t3A);
            a1A = (*mpND_a1A);
            a2B = (*mpND_a2B);
            a3A = (*mpND_a3A);
            w1A = (*mpND_w1A);

            mNumX[0] = 1.0;
            mNumX[1] = 0.0;
            mNumX[2] = 0.0;
            mDenX[0] = 0.0;
            //mDenX[0] = 0;
            mDenX[2] = J*(Zx2B+Zx3A*pow((-1+R),2));
            mDenX[1] = (B-Zx1A)*(Zx2B+Zx3A*pow((-1+R),2))+pow(R,2)*Zx3A*Zx2B;
            mNumV[0] = 1.0;
            mNumV[1] = 0.0;
            mDenV[1] = J*(Zx2B+Zx3A*pow((-1+R),2));
            mDenV[0] = (B-Zx1A)*Zx2B+Zx3A*pow((-1+R),2)*(B-Zx1A);

            mFilterX.initialize(mTimestep, mNumX, mDenX, t1A+t3A*R, -a1A);
            mFilterV.initialize(mTimestep, mNumV, mDenV, t1A+t3A*R, -w1A);
            mInt2.initialize(mTimestep, -w2B, -a2B);
            mInt3.initialize(mTimestep, -w3A, -a3A);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            c1A = (*mpND_c1A);
            Zx1A =(*mpND_Zx1A);
            c2B = (*mpND_c2B);
            Zx2B =(*mpND_Zx2B);
            c3A = (*mpND_c3A);
            Zx3A =(*mpND_Zx3A);

            //System equations
//            mIntegrator.setDamping((B+Zx1+Zx2)/J*mTimestep);
//            mIntegrator.integrate((c1-c2)/J);
            mDenX[2] = J*(Zx2B+Zx3A*pow((-1+R),2));
            mDenX[1] = (B+Zx1A)*(Zx2B+Zx3A*pow((-1+R),2))-pow(R,2)*Zx3A*Zx2B;
            mDenX[0] = k*Zx2B+k*Zx3A*pow((-1+R),2);
            mDenV[1] = J*(Zx2B+Zx3A*pow((-1+R),2));
            mDenV[0] = (B+Zx1A)*(Zx2B+Zx3A*pow((-1+R),2))-pow(R,2)*Zx3A*Zx2B;
            mFilterX.setDen(mDenX);
            mFilterV.setDen(mDenV);

            a1A = mFilterX.update((-c1A+c3A*R)*Zx2B-(-1+R)*(c1A*(-1+R)+c2B*R)*Zx3A);
            w1A = mFilterV.update((-c1A+c3A*R)*Zx2B-(-1+R)*(c1A*(-1+R)+c2B*R)*Zx3A)+k*a1A;

            w3A = (-(c2B+c3A*(-1+R))*(-1+R)+R*w1A*Zx2B)/(Zx2B+pow((-1+R),2)*Zx3A);
            a3A = mInt2.update(w3A);

            w2B = (-w1A*R+w3A)/(R-1);
            a2B = mInt3.update(w2B);

            t1A = c1A + Zx1A*w1A;
            t2B = c2B + Zx2B*w2B;
            t3A = c3A + Zx3A*w3A;

            //Write new values to nodes
            (*mpND_t1A) = t1A;
            (*mpND_a1A) = a1A;
            (*mpND_w1A) = w1A;
            (*mpND_t2B) = t2B;
            (*mpND_a2B) = a2B;
            (*mpND_w2B) = w2B;
            (*mpND_t3A) = t3A;
            (*mpND_a3A) = a3A;
            (*mpND_w3A) = w3A;
        }
    };
}

#endif // MECHANICROTATIONALPLANETARYGEAR_HPP_INCLUDED

