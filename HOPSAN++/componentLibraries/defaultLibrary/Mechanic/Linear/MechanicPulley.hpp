//!
//! @file   MechanicPulley.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2013-02-28
//!
//! @brief Contains a mechanic pulley
//!
//$Id$

#ifndef MECHANICPULLEY_HPP_INCLUDED
#define MECHANICPULLEY_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicPulley : public ComponentQ
    {

    private:
        double m;
        double *mpB;
        double mNumTheta[3];
        double mDenTheta[3];
        double mNumOmega[2];
        double mDenOmega[2];
        SecondOrderTransferFunction mFilterTheta;
        FirstOrderTransferFunction mFilterOmega;
        //        DoubleIntegratorWithDamping mIntegrator;
        double *mpND_f1, *mpND_x1, *mpND_v1, *mpND_me1, *mpND_c1, *mpND_Zx1,
               *mpND_f2, *mpND_x2, *mpND_v2, *mpND_me2, *mpND_c2, *mpND_Zx2;
        double f1, x1, v1, c1, Zx1,
               f2, x2, v2, c2, Zx2;
        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new MechanicPulley();
        }

        void configure()
        {
            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanic");
            mpP2 = addPowerPort("P2", "NodeMechanic");

            //Register changable parameters to the HOPSAN++ core
            addConstant("m", "Inertia", "kg", 1.0, m);
            addInputVariable("B", "Viscous Friction", "Nms/rad", 10, &mpB);
        }


        void initialize()
        {
            mpND_f1 = getSafeNodeDataPtr(mpP1, NodeMechanic::Force);
            mpND_x1 = getSafeNodeDataPtr(mpP1, NodeMechanic::Position);
            mpND_v1 = getSafeNodeDataPtr(mpP1, NodeMechanic::Velocity);
            mpND_me1 = getSafeNodeDataPtr(mpP1, NodeMechanic::EquivalentMass);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeMechanic::WaveVariable);
            mpND_Zx1 = getSafeNodeDataPtr(mpP1, NodeMechanic::CharImpedance);

            mpND_f2 = getSafeNodeDataPtr(mpP2, NodeMechanic::Force);
            mpND_x2 = getSafeNodeDataPtr(mpP2, NodeMechanic::Position);
            mpND_v2 = getSafeNodeDataPtr(mpP2, NodeMechanic::Velocity);
            mpND_me2 = getSafeNodeDataPtr(mpP2, NodeMechanic::EquivalentMass);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeMechanic::WaveVariable);
            mpND_Zx2 = getSafeNodeDataPtr(mpP2, NodeMechanic::CharImpedance);

            f1 = (*mpND_f1)*2.0;
            x1 = (*mpND_x1)/2.0;
            v1 = (*mpND_v1)/2.0;
            f2 = (*mpND_f2);
            x2 = (*mpND_x2);
            v2 = (*mpND_v2);

            mNumTheta[0] = 1.0;
            mNumTheta[1] = 0.0;
            mNumTheta[2] = 0.0;
            mDenTheta[0] = 0;
            mDenTheta[1] = (*mpB);
            mDenTheta[2] = m;
            mNumOmega[0] = 1.0;
            mNumOmega[1] = 0.0;
            mDenOmega[0] = (*mpB);
            mDenOmega[1] = m;

            mFilterTheta.initialize(mTimestep, mNumTheta, mDenTheta, f1-f2, -x1);
            mFilterOmega.initialize(mTimestep, mNumOmega, mDenOmega, f1-f2, -v1);

            (*mpND_me1) = m;
            (*mpND_me2) = m;
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            c1  = (*mpND_c1)*2;
            Zx1 = (*mpND_Zx1)*4;
            c2  = (*mpND_c2);
            Zx2 = (*mpND_Zx2);

            //Mass equations
            mDenTheta[1] = ((*mpB)+Zx1+Zx2);
            mDenOmega[0] = ((*mpB)+Zx1+Zx2);
            mFilterTheta.setDen(mDenTheta);
            mFilterOmega.setDen(mDenOmega);

            x2 = mFilterTheta.update(c1-c2);
            v2 = mFilterOmega.update(c1-c2);
            f2 = c2 + Zx2*v2;

            v1 = -v2*2;
            x1 = -x2*2;
            f1 = (c1 + Zx1*v1)/2.0;


            //Write new values to nodes
            (*mpND_f1) = f1;
            (*mpND_x1) = x1;
            (*mpND_v1) = v1;
            (*mpND_f2) = f2;
            (*mpND_x2) = x2;
            (*mpND_v2) = v2;
        }
    };
}

#endif // MECHANICPULLEY_HPP_INCLUDED

