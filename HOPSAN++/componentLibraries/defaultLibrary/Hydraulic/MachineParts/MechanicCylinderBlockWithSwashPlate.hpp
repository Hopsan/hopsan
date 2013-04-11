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
//! @file   MechanicCylinderBlockWithSwashPlate.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2012-10-12
//!
//! @brief Contains a mechanic cylinder block with swash plate component
//!
//$Id$

#ifndef MECHANICCYLINDERBLOCKWITHSWASHPLATE_HPP_INCLUDED
#define MECHANICCYLINDERBLOCKWITHSWASHPLATE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicCylinderBlockWithSwashPlate : public ComponentQ
    {

    private:
        Port *mpIn1, *mpOut1, *mpOut2, *mpP1, *mpP2;
        size_t mNumPorts1;

        double mNumX[3], mNumV[2];
        double mDenX[3], mDenV[2];

        double *mpND_in1, *mpND_out1, *mpND_out2;
        std::vector<double*> mvpND_f1, mvpND_x1, mvpND_v1, mvpND_c1, mvpND_Zc1, mvpND_me1;
        double *mpND_t1, *mpND_a1, *mpND_w1, *mpND_c1, *mpND_Zx1, *mpND_t2, *mpND_a2, *mpND_w2, *mpND_c2, *mpND_Zx2;
        double *mpOffset, *mpR, *mpB;

        double t2, a2, w2, c2, Zx2;
        std::vector<double> f1, c1, Zc1, x1,v1;

        SecondOrderTransferFunction mFilterX;
        FirstOrderTransferFunction mFilterV;

        double J, mp, rp, startX;

    public:
        static Component *Creator()
        {
            return new MechanicCylinderBlockWithSwashPlate();
        }

        void configure()
        {
            //Set member attributes
            J = 0.1;
            mp = 0.001;
            rp = 0.01;

            //Register changable parameters to the HOPSAN++ core
            addInputVariable("B", "Viscous Friction", "[Nms/rad]", 10.0);
            addInputVariable("r", "Swivel Radius", "[m]", 0.05);
            addInputVariable("theta_offset", "Angle Offset", "[m]", 0.0);
            registerParameter("J", "Moment of Inertia of Cylinder Block", "[kgm^2]", J);
            registerParameter("m_p", "Mass of each Piston", "[kg]", mp);
            registerParameter("r_p", "Piston Radius", "[m]", rp);

            //Add ports to the component
            mpIn1 = addReadPort("angle", "NodeSignal");
            mpOut1 = addWritePort("torque", "NodeSignal", Port::NotRequired);
            mpOut2 = addWritePort("movement", "NodeSignal", Port::NotRequired);
            mpP1 = addPowerMultiPort("P1", "NodeMechanic");
            mpP2 = addPowerPort("P2", "NodeMechanicRotational");
        }


        void initialize()
        {
            mNumPorts1 = mpP1->getNumPorts();
            mvpND_f1.resize(mNumPorts1);
            mvpND_x1.resize(mNumPorts1);
            mvpND_v1.resize(mNumPorts1);
            mvpND_c1.resize(mNumPorts1);
            mvpND_Zc1.resize(mNumPorts1);
            mvpND_me1.resize(mNumPorts1);
            f1.resize(mNumPorts1);
            c1.resize(mNumPorts1);
            Zc1.resize(mNumPorts1);
            x1.resize(mNumPorts1);
            v1.resize(mNumPorts1);

            mpND_t2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::Torque);
            mpND_a2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::Angle);
            mpND_w2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::AngularVelocity);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::WaveVariable);
            mpND_Zx2 = getSafeNodeDataPtr(mpP2, NodeMechanicRotational::CharImpedance);

            mpND_in1 = getSafeNodeDataPtr(mpIn1, NodeSignal::Value);
            mpND_out1 = getSafeNodeDataPtr(mpOut1, NodeSignal::Value);
            mpND_out2 = getSafeNodeDataPtr(mpOut2, NodeSignal::Value);

            mpOffset = getSafeNodeDataPtr("theta_offset", NodeSignal::Value);
            mpR = getSafeNodeDataPtr("r", NodeSignal::Value);
            mpB = getSafeNodeDataPtr("B", NodeSignal::Value);

            //Assign node data pointers
            for (size_t i=0; i<mNumPorts1; ++i)
            {
                mvpND_f1[i] = getSafeMultiPortNodeDataPtr(mpP1, i, NodeMechanic::Force, 0.0);
                mvpND_x1[i] = getSafeMultiPortNodeDataPtr(mpP1, i, NodeMechanic::Position, 0.0);
                mvpND_v1[i] = getSafeMultiPortNodeDataPtr(mpP1, i, NodeMechanic::Velocity, 0.0);
                mvpND_c1[i] = getSafeMultiPortNodeDataPtr(mpP1, i, NodeMechanic::WaveVariable, 0.0);
                mvpND_Zc1[i] = getSafeMultiPortNodeDataPtr(mpP1, i, NodeMechanic::CharImpedance, 0.0);
                mvpND_me1[i] = getSafeMultiPortNodeDataPtr(mpP1, i, NodeMechanic::EquivalentMass, 0.0);
            }

            for (size_t i=0; i<mNumPorts1; ++i)
            {
                (*mvpND_me1[i]) = 0.02;
            }

            double r = (*mpR);
            double B = (*mpB);

            startX = (*mvpND_x1[0]);

            t2 = (*mpND_t2);
            a2 = (*mpND_a2);
            w2 = (*mpND_w2);

            mNumX[0] = 1.0;
            mNumX[1] = 0.0;
            mNumX[2] = 0.0;
            mDenX[0] = 0.0;
            mDenX[1] = B;
            mDenX[2] = J;
            mNumV[0] = 1.0;
            mNumV[1] = 0.0;
            mDenV[0] = B;
            mDenV[1] = J;

            mFilterX.initialize(mTimestep, mNumX, mDenX, -t2, a2);
            mFilterV.initialize(mTimestep, mNumV, mDenV, -t2, w2);

            //Append moment of inertia from pistons
            J = J + mp*(rp*rp/2 + r*r);
        }


        void simulateOneTimestep()
        {
            double offset = (*mpOffset);
            double r = (*mpR);
            double B = (*mpB);

            //Get variable values from nodes
            c2 = (*mpND_c2);
            Zx2 = (*mpND_Zx2);

            //Calculate constants
            double angle = (*mpND_in1);
            double s = r*tan(angle);
            double diff = 2*3.1416/mNumPorts1;

            //Calculate torque
            double cp = 0;
            double Zp = 0;
            double a1 = -a2;
            for(size_t i=0; i<mNumPorts1; ++i)
            {
                c1[i] = (*mvpND_c1[i]);
                Zc1[i] = (*mvpND_Zc1[i]);
                cp += c1[i]*tan(angle)*r*cos(a1-offset-diff*i);
                Zp += Zc1[i]*tan(angle)*r*cos(a1-offset-diff*i)*tan(angle)*r*cos(a1-offset-diff*i);
            }

            //Inertia equations
            mDenX[1] = B+Zp+Zx2;
            mDenV[0] = B+Zp+Zx2;
            mFilterX.setDen(mDenX);
            mFilterV.setDen(mDenV);

            a2 = mFilterX.update(cp-c2);
            w2 = mFilterV.update(cp-c2);

            t2 = c2 + Zx2*w2;
            double w1 = -w2;

            //Calculate positions and velocities
            for(size_t i=0; i<mNumPorts1; ++i)
            {
                v1[i] = s*cos(a1-offset-diff*i)*w1;
                x1[i] = startX+s*sin(a1-offset-diff*i);
            }

            //Piston forces
            for(size_t i=0; i<mNumPorts1; ++i)
            {
                f1[i] = c1[i]+Zc1[i]*v1[i];
            }

            //Write new values to nodes
            (*mpND_out1) = -t2;
            (*mpND_out2) = w1;
            for(size_t i=0; i<mNumPorts1; ++i)
            {
                (*mvpND_f1[i]) = f1[i];
                (*mvpND_x1[i]) = x1[i];
                (*mvpND_v1[i]) = v1[i];
            }
            (*mpND_t2) = t2;
            (*mpND_a2) = a2;
            (*mpND_w2) = w2;
        }
    };
}

#endif // MECHANICCYLINDERBLOCKWITHSWASHPLATE_HPP_INCLUDED

