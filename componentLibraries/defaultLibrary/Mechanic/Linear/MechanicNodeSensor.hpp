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
//! @file   MechanicNodeSensor.hpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2015-11-17

//$Id$

#ifndef MECHANICNODESENSOR_HPP
#define MECHANICNODESENSOR_HPP

#include "ComponentEssentials.h"

namespace hopsan {

//!
//! @brief
//! @ingroup MechanicalComponents
//!
class MechanicNodeSensor : public ComponentSignal
{
private:
    MechanicNodeDataPointerStructT mP1;
    double *mpOut_f, *mpOut_v, *mpOut_x, *mpOut_c, *mpOut_z, *mpOut_em;

public:
    static Component *Creator()
    {
        return new MechanicNodeSensor();
    }

    void configure()
    {
        addReadPort("P1", "NodeMechanic", "Sensor port", Port::NotRequired);
        addOutputVariable("out_f", "Force", "Force", &mpOut_f);
        addOutputVariable("out_v", "Velocity", "Velocity", &mpOut_v);
        addOutputVariable("out_x", "Position", "Position", &mpOut_x);
        addOutputVariable("out_c", "Wave variable", "Force", &mpOut_c);
        addOutputVariable("out_z", "Char. impedance", "", &mpOut_z);
        addOutputVariable("out_em", "Equivalent mass", "Mass", &mpOut_em);
    }

    void initialize()
    {
        getMechanicPortNodeDataPointers(getPort("P1"), mP1);
        simulateOneTimestep(); //Set initial output node value
    }

    void simulateOneTimestep()
    {
        writeOutputVariable(mpOut_f, mP1.f());
        writeOutputVariable(mpOut_v, mP1.v());
        writeOutputVariable(mpOut_x, mP1.x());
        writeOutputVariable(mpOut_c, mP1.c());
        writeOutputVariable(mpOut_z, mP1.Zc());
        writeOutputVariable(mpOut_em, mP1.me());
    }
};
}

#endif // MECHANICNODESENSOR_HPP

