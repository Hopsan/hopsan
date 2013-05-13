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
        // Declare member variables
        double J, B, k, R;
        double mNumX[3],mNumV[2];
        double mDenX[3],mDenV[2];

        SecondOrderTransferFunction mFilterX;
        FirstOrderTransferFunction mFilterV;
        Integrator mInt1;
        Integrator mInt2;
        Integrator mInt3;

        // Declare port and node data pointers
        Port *mpP1A, *mpP2B, *mpP3A;
        double *mpP1A_t, *mpP1A_a, *mpP1A_w, *mpP1A_c, *mpP1A_Zx,
        *mpP2B_t, *mpP2B_a, *mpP2B_w, *mpP2B_c, *mpP2B_Zx,
        *mpP3A_t, *mpP3A_a, *mpP3A_w, *mpP3A_c, *mpP3A_Zx;

    public:
        static Component *Creator()
        {
            return new MechanicRotationalPlanetaryGear();
        }

        void configure()
        {
            //Add ports to the component
            mpP1A = addPowerPort("P1A", "NodeMechanicRotational");
            mpP2B = addPowerPort("P2B", "NodeMechanicRotational");
            mpP3A = addPowerPort("P3A", "NodeMechanicRotational");

            //Register changable parameters to the HOPSAN++ core
            addConstant("J", "Moment of Inertia", "[kgm^2]", 0.1, J);
            addConstant("B", "Viscous Friction", "[Nms/rad]", 10.0, B);
            addConstant("R", "Gear Ratio", "[Nms/rad]", -1, R);
            addConstant("k", "Spring Constant", "[Nm/rad]", 0.0, k);
        }


        void initialize()
        {
            mpP1A_t = getSafeNodeDataPtr(mpP1A, NodeMechanicRotational::Torque);
            mpP1A_a = getSafeNodeDataPtr(mpP1A, NodeMechanicRotational::Angle);
            mpP1A_w = getSafeNodeDataPtr(mpP1A, NodeMechanicRotational::AngularVelocity);
            mpP1A_c = getSafeNodeDataPtr(mpP1A, NodeMechanicRotational::WaveVariable);
            mpP1A_Zx = getSafeNodeDataPtr(mpP1A, NodeMechanicRotational::CharImpedance);

            mpP2B_t = getSafeNodeDataPtr(mpP2B, NodeMechanicRotational::Torque);
            mpP2B_a = getSafeNodeDataPtr(mpP2B, NodeMechanicRotational::Angle);
            mpP2B_w = getSafeNodeDataPtr(mpP2B, NodeMechanicRotational::AngularVelocity);
            mpP2B_c = getSafeNodeDataPtr(mpP2B, NodeMechanicRotational::WaveVariable);
            mpP2B_Zx = getSafeNodeDataPtr(mpP2B, NodeMechanicRotational::CharImpedance);

            mpP3A_t = getSafeNodeDataPtr(mpP3A, NodeMechanicRotational::Torque);
            mpP3A_a = getSafeNodeDataPtr(mpP3A, NodeMechanicRotational::Angle);
            mpP3A_w = getSafeNodeDataPtr(mpP3A, NodeMechanicRotational::AngularVelocity);
            mpP3A_c = getSafeNodeDataPtr(mpP3A, NodeMechanicRotational::WaveVariable);
            mpP3A_Zx = getSafeNodeDataPtr(mpP3A, NodeMechanicRotational::CharImpedance);

            const double t1A = (*mpP1A_t);
            const double a1A = (*mpP1A_a);
            const double w1A = (*mpP1A_w);
            const double Zx1A =(*mpP1A_Zx);
            //const double t2B = (*mpP2B_t);
            const double a2B = (*mpP2B_a);
            const double w2B = (*mpP2B_w);
            const double Zx2B =(*mpP2B_Zx);
            const double t3A = (*mpP3A_t);
            const double a3A = (*mpP3A_a);
            const double w3A = (*mpP3A_w);
            const double Zx3A =(*mpP3A_Zx);

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
            // Local variables
            double t1A, a1A, w1A, t2B, a2B, w2B, t3A, a3A, w3A;

            // Get node data values from nodes
            const double c1A = (*mpP1A_c);
            const double Zx1A =(*mpP1A_Zx);
            const double c2B = (*mpP2B_c);
            const double Zx2B =(*mpP2B_Zx);
            const double c3A = (*mpP3A_c);
            const double Zx3A =(*mpP3A_Zx);

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
            (*mpP1A_t) = t1A;
            (*mpP1A_a) = a1A;
            (*mpP1A_w) = w1A;
            (*mpP2B_t) = t2B;
            (*mpP2B_a) = a2B;
            (*mpP2B_w) = w2B;
            (*mpP3A_t) = t3A;
            (*mpP3A_a) = a3A;
            (*mpP3A_w) = w3A;
        }
    };
}

#endif // MECHANICROTATIONALPLANETARYGEAR_HPP_INCLUDED

