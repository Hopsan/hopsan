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

//$Id: ElectricQSource.hpp 8974 2016-05-02 07:40:48Z petno25 $

#ifndef ElectricQSOURCE_HPP
#define ElectricQSOURCE_HPP

#include "ComponentEssentials.h"

namespace hopsan {

//!
//! @brief
//! @ingroup ElectricComponents
//!
class ElectricQSource : public ComponentQ
{

private:
    double *mpIn_u, *mpIn_i;
    Port *mpP1;
    ElectricNodeDataPointerStructT mP1;

public:
    static Component *Creator()
    {
        return new ElectricQSource();
    }

    void configure()
    {
        addInputVariable("in_u", "Voltage variable input", "Voltage", 0, &mpIn_u);
        addInputVariable("in_i", "Current variable input", "Current", 0, &mpIn_i);
        mpP1 = addPowerPort("P1", "NodeElectric");
    }

    void initialize()
    {
        getElectricPortNodeDataPointers(mpP1, mP1);
        simulateOneTimestep();
    }

    void simulateOneTimestep()
    {
        mP1.ru() = readSignal(mpIn_u);
        mP1.ri() = readSignal(mpIn_i);
    }
};
}

#endif

