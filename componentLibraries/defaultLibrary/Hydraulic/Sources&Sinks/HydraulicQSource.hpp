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

//$Id$

#ifndef HydraulicQSOURCE_HPP
#define HydraulicQSOURCE_HPP

#include "ComponentEssentials.h"

namespace hopsan {

//!
//! @brief
//! @ingroup MechanicalComponents
//!
class HydraulicQSource : public ComponentQ
{

private:
    double *mpIn_p, *mpIn_q;
    Port *mpP1;
    HydraulicNodeDataPointerStructT mP1;

public:
    static Component *Creator()
    {
        return new HydraulicQSource();
    }

    void configure()
    {
        addInputVariable("in_p", "Pressure variable input", "Force", 0, &mpIn_p);
        addInputVariable("in_q", "Flow variable input", "Velocity", 0, &mpIn_q);
        mpP1 = addPowerPort("P1", "NodeHydraulic");
    }

    void initialize()
    {
        getHydraulicPortNodeDataPointers(mpP1, mP1);
        simulateOneTimestep();
    }

    void simulateOneTimestep()
    {
        mP1.rp() = readSignal(mpIn_p);
        mP1.rq() = readSignal(mpIn_q);
    }
};
}

#endif

