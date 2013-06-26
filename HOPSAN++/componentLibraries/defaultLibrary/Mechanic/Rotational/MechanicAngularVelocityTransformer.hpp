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
//! @file   MechanicAngularVelocityTransformer.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-08-05
//!
//! @brief Contains an angular velocity transformer component
//!
//$Id$

#ifndef MECHANICANGULARVELOCITYTRANSFORMER_HPP_INCLUDED
#define MECHANICANGULARVELOCITYTRANSFORMER_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicAngularVelocityTransformer : public ComponentQ
    {

    private:
        Integrator mInt;
        Port *mpOut;
        double *mpOut_t, *mpOut_a, *mpOut_w, *mpOut_c, *mpOut_Zx;
        double *mpW;

    public:
        static Component *Creator()
        {
            return new MechanicAngularVelocityTransformer();
        }

        void configure()
        {
            mpOut = addPowerPort("out", "NodeMechanicRotational");
            addInputVariable("omega", "Generated angular velocity", "rad/s", 0.0, &mpW);
        }


        void initialize()
        {
            mpOut_t = getSafeNodeDataPtr(mpOut, NodeMechanicRotational::Torque);
            mpOut_a = getSafeNodeDataPtr(mpOut, NodeMechanicRotational::Angle);
            mpOut_w = getSafeNodeDataPtr(mpOut, NodeMechanicRotational::AngularVelocity);
            mpOut_c = getSafeNodeDataPtr(mpOut, NodeMechanicRotational::WaveVariable);
            mpOut_Zx = getSafeNodeDataPtr(mpOut, NodeMechanicRotational::CharImpedance);

            mInt.initialize(mTimestep, (*mpW), (*mpOut_a));
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            double a, t;
            const double w = (*mpW);
            const double c = (*mpOut_c);
            const double Zx = (*mpOut_Zx);

            //Spring equations
            a = mInt.update(w);
            t = c + Zx*w;

            //Write values to nodes
            (*mpOut_t) = t;
            (*mpOut_a) = a;
            (*mpOut_w) = w;
        }
    };
}

#endif // MECHANICANGULARVELOCITYTRANSFORMER_HPP_INCLUDED




