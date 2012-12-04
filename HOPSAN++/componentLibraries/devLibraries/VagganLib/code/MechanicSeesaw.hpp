//!
//! @file   MechanicRotationalInertiaWithGearRatio.hpp
//! @author Karl Pettersson <karl.pettersson@liu.se>
//! @date   2011-03-15
//!
//! @brief Contains a mechanic rotational shaft with gear ratio
//!
//$Id$

#ifndef MECHANICSEESAW_HPP_INCLUDED
#define MECHANICSEESAW_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //
    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicSeesaw : public ComponentQ
    {

    private:
        double num[3];
        double den[3];
        DoubleIntegratorWithDamping mIntegrator;
        double *mpND_f1, *mpND_x1, *mpND_v1, *mpND_c1, *mpND_Zx1, *mpND_me1;
        double *mpND_f2, *mpND_x2, *mpND_v2, *mpND_c2, *mpND_Zx2, *mpND_me2;
        double *mpND_out;
        double f1, x1, v1, c1, Zx1;
        double f2, x2, v2, c2, Zx2;
        Port *mpP1, *mpP2, *mpOut;
        double l1, l2, J, B;
        double start1, start2;

    public:
        static Component *Creator()
        {
            return new MechanicSeesaw();
        }

        void configure()
        {
            //Set member attributes
            l1=0.5;
            l2=0.5;
            J = 1.0;
            B = 10;

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanic");
            mpP2 = addPowerPort("P2", "NodeMechanic");
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

            //Register changable parameters to the HOPSAN++ core
            registerParameter("l_1", "Length 1", "[m]", l1);
            registerParameter("l_2", "Length 2", "[m]", l2);
            registerParameter("J", "Moment of Inertia", "[kgm^2]", J);
            registerParameter("B", "Viscous Friction", "[Nms/rad]", B);
        }


        void initialize()
        {
            mpND_f1 = getSafeNodeDataPtr(mpP1, NodeMechanic::FORCE);
            mpND_x1 = getSafeNodeDataPtr(mpP1, NodeMechanic::POSITION);
            mpND_v1 = getSafeNodeDataPtr(mpP1, NodeMechanic::VELOCITY);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeMechanic::WAVEVARIABLE);
            mpND_Zx1 = getSafeNodeDataPtr(mpP1, NodeMechanic::CHARIMP);
            mpND_me1 = getSafeNodeDataPtr(mpP1, NodeMechanic::EQMASS);

            mpND_f2 = getSafeNodeDataPtr(mpP2, NodeMechanic::FORCE);
            mpND_x2 = getSafeNodeDataPtr(mpP2, NodeMechanic::POSITION);
            mpND_v2 = getSafeNodeDataPtr(mpP2, NodeMechanic::VELOCITY);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeMechanic::WAVEVARIABLE);
            mpND_Zx2 = getSafeNodeDataPtr(mpP2, NodeMechanic::CHARIMP);
            mpND_me2 = getSafeNodeDataPtr(mpP2, NodeMechanic::EQMASS);

            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            mIntegrator.initialize(mTimestep, 0, 0, 0, 0);

            start1 = mpP1->getConnectedPorts()[0]->getStartValue(NodeMechanic::POSITION);
            start2 = mpP2->getConnectedPorts()[0]->getStartValue(NodeMechanic::POSITION);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            c1  = (*mpND_c1);
            Zx1 = (*mpND_Zx1);
            c2  = (*mpND_c2);
            Zx2 = (*mpND_Zx2);

            //Mass equations

            double c_e=l1*c1-l2*c2;
            double Be = B+l1*l1*Zx1+l2*l2*Zx2;
            mIntegrator.setDamping(Be/J*mTimestep);
            mIntegrator.integrateWithUndo(c_e/J);
            double w = mIntegrator.valueFirst();
            double a = mIntegrator.valueSecond();

            double x1 = start1-l1*a;
            double v1 = -l1*w;
            double f1 = c1 + Zx1*v1;

            double x2 = start2+l2*a;
            double v2 = l2*w;
            double f2 = c2 + Zx2*v2;

            //Write new values to nodes
            (*mpND_f1) = f1;
            (*mpND_x1) = x1;
            (*mpND_v1) = v1;
            (*mpND_f2) = f2;
            (*mpND_x2) = x2;
            (*mpND_v2) = v2;
            (*mpND_me1) = 1;
            (*mpND_me2) = 1;

            (*mpND_out) = a;
        }
    };
}

#endif // MECHANICSEESAW_HPP_INCLUDED

