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
//! @file   MechanicFixedPosition.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-11-09
//!
//! @brief Contains a Mechanic Fixed Position Component
//!
//$Id$

#ifndef MECHANICFIXEDPOSITION_HPP_INCLUDED
#define MECHANICFIXEDPOSITION_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    class MechanicFixedPosition : public ComponentQ
    {

    private:
        double me;
        Port *mpPm1;
        double *mpPm1_f, *mpPm1_x, *mpPm1_v, *mpPm1_c, *mpPm1_me;

    public:
        static Component *Creator()
        {
            return new MechanicFixedPosition();
        }

        void configure()
        {
            mpPm1 = addPowerPort("Pm1", "NodeMechanic");
            addConstant("m_e", "Equivalent Mass", "kg", 1, me);
        }

        void initialize()
        {
            mpPm1_f = getSafeNodeDataPtr(mpPm1, NodeMechanic::Force);
            mpPm1_x = getSafeNodeDataPtr(mpPm1, NodeMechanic::Position);
            mpPm1_v = getSafeNodeDataPtr(mpPm1, NodeMechanic::Velocity);
            mpPm1_c = getSafeNodeDataPtr(mpPm1, NodeMechanic::WaveVariable);
            mpPm1_me = getSafeNodeDataPtr(mpPm1, NodeMechanic::EquivalentMass);

            (*mpPm1_v) = 0;
            (*mpPm1_me) = me;
        }


        void simulateOneTimestep()
        {
            //Equations
            (*mpPm1_f) = (*mpPm1_c);
        }
    };
}

#endif // MECHANICFIXEDPOSITION_HPP_INCLUDED
