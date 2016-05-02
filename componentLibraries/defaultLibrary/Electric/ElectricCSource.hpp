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

//$Id: ElectricCSource.hpp 8974 2016-05-02 07:40:48Z petno25 $

#ifndef ElectricCSOURCE_HPP
#define ElectricCSOURCE_HPP

#include "ComponentEssentials.h"

namespace hopsan {

//!
//! @brief
//! @ingroup ElectricComponents
//!
class ElectricCSource : public ComponentC
{

private:
    double *mpIn_c, *mpIn_Zx;
    double *mpP1_c, *mpP1_Zx;

public:
    static Component *Creator()
    {
        return new ElectricCSource();
    }

    void configure()
    {
        addInputVariable("in_c", "Wave variable input", "Voltage", 0, &mpIn_c);
        addInputVariable("in_z", "Char. impedance variable input", "N s/m", 0, &mpIn_Zx);
        addPowerPort("P1", "NodeElectric");
    }

    void initialize()
    {
        mpP1_c = getSafeNodeDataPtr("P1", NodeElectric::WaveVariable);
        mpP1_Zx = getSafeNodeDataPtr("P1", NodeElectric::CharImpedance);
        simulateOneTimestep();
    }

    void simulateOneTimestep()
    {
        (*mpP1_c) = (*mpIn_c);
        (*mpP1_Zx) = (*mpIn_Zx);
    }
};
}

#endif // ElectricCSOURCE_HPP

