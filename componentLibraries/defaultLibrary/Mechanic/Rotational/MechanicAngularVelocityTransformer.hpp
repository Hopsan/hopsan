/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
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




