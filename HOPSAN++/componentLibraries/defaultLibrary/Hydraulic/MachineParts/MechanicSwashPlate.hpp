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
//! @file   MechanicSwashPlate.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2012-10-12
//!
//! @brief Contains a mechanic swivel plate component
//!
//$Id$

#ifndef MECHANICSWASHPLATE_HPP_INCLUDED
#define MECHANICSWASHPLATE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicSwashPlate : public ComponentQ
    {

    private:
        Port *mpP1;
        double *mpAngle, *mpMovement, *mpTorque;
        std::vector<double*> mvpND_f1, mvpND_x1, mvpND_v1, mvpND_c1, mvpND_Zc1, mvpND_me1;
        size_t mNumPorts1;

        double *mpR, *mpOffset;

        Integrator mIntegrator;

        double startX;

        std::vector<double> f1, c1, Zc1, x1,v1;

    public:
        static Component *Creator()
        {
            return new MechanicSwashPlate();
        }

        void configure()
        {
            //Register changable parameters to the HOPSAN++ core
            addInputVariable("r", "Swivel Radius", "[m]", 0.05, &mpR);
            addInputVariable("theta_offset", "Angle Offset", "[m]", 0.0, &mpOffset);
            addInputVariable("angle", "Angle", "rad", 0, &mpAngle);
            addInputVariable("movemenet", "?", "?", 0, &mpMovement);
            addOutputVariable("torque", "Torque", "Nm", &mpTorque);

            //Add ports to the component
            mpP1 = addPowerMultiPort("P1", "NodeMechanic");
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

            mIntegrator.initialize(mTimestep, 0.0, 0.0);

            for (size_t i=0; i<mNumPorts1; ++i)
            {
                (*mvpND_me1[i]) = 0.02;
            }

            startX = (*mvpND_x1[0]);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double angle = (*mpAngle);
            double w1 = (*mpMovement);
            double r = (*mpR);
            double offset = (*mpOffset);

            double s = r*tan(angle);
            double diff = 2*3.1416/mNumPorts1;

            double a1 = mIntegrator.update(w1);

            //Calculate positions and velocities
            for(size_t i=0; i<mNumPorts1; ++i)
            {
                v1[i] = s*cos(a1-offset-diff*i)*w1;
                x1[i] = startX+s*sin(a1-offset-diff*i);
            }

            //Piston forces
            for(size_t i=0; i<mNumPorts1; ++i)
            {
                c1[i] = (*mvpND_c1[i]);
                Zc1[i] = (*mvpND_Zc1[i]);
                f1[i] = c1[i]+Zc1[i]*v1[i];
            }

            //Calculate torque
            double torque = 0;
            for(size_t i=0; i<mNumPorts1; ++i)
            {
                torque += f1[i]*tan(angle)*r*cos(a1-offset-diff*i);
            }

            //Write new values to nodes
            (*mpTorque) = torque;
            for(size_t i=0; i<mNumPorts1; ++i)
            {
                (*mvpND_f1[i]) = f1[i];
                (*mvpND_x1[i]) = x1[i];
                (*mvpND_v1[i]) = v1[i];
            }
        }
    };
}

#endif // MECHANICSWASHPLATE_HPP_INCLUDED

