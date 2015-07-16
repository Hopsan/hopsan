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
//! @file   MechanicAngleSensor.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-08-12
//!
//! @brief Contains a Mechanic Angle Sensor Component
//!
//$Id$

#ifndef MECHANICANGLESENSOR_HPP_INCLUDED
#define MECHANICANGLESENSOR_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicAngleSensor : public ComponentSignal
    {
    private:
        double *mpP1_a, *mpOut;

    public:
        static Component *Creator()
        {
            return new MechanicAngleSensor();
        }

        void configure()
        {
            addReadPort("P1", "NodeMechanicRotational", "", Port::NotRequired);
            addOutputVariable("out", "Angle", "Angle", &mpOut);
        }


        void initialize()
        {
            mpP1_a = getSafeNodeDataPtr("P1", NodeMechanicRotational::Angle);
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            (*mpOut) = (*mpP1_a);
        }
    };
}

#endif // MECHANICANGLESENSOR_HPP_INCLUDED
