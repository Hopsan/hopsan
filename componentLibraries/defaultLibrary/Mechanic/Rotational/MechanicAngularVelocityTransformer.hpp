/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

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
            addInputVariable("omega", "Generated angular velocity", "AngularVelocity", 0.0, &mpW);
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




