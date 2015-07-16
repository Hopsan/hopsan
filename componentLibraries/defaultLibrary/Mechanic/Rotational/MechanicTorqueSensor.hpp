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
//! @file   MechanicTorqueSensor.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-02-23
//!
//! @brief Contains a Mechanic Torque Sensor Component
//!
//$Id$

#ifndef MECHANICTORQUESENSOR_HPP_INCLUDED
#define MECHANICTORQUESENSOR_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicTorqueSensor : public ComponentSignal
    {
    private:
        double *mpP1_T, *mpOut;

    public:
        static Component *Creator()
        {
            return new MechanicTorqueSensor();
        }

        void configure()
        {
            addReadPort("P1", "NodeMechanicRotational", "", Port::NotRequired);
            addOutputVariable("out","Torque","Torque",&mpOut);
        }


        void initialize()
        {
            mpP1_T = getSafeNodeDataPtr("mpP1", NodeMechanicRotational::Torque);
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            (*mpOut) = (*mpP1_T);
        }
    };
}

#endif // MECHANICTORQUESENSOR_HPP_INCLUDED
