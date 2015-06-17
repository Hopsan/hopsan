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

#ifndef PNEUMATICPSENSOR_HPP_INCLUDED
#define PNEUMATICPSENSOR_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

//!
//! @file PneumaticPsensor.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date Wed 13 Mar 2013 16:11:02
//! @brief Pneumatic pressure and temperature source
//! @ingroup PneumaticComponents
//!
//$Id$

using namespace hopsan;

class PneumaticPsensor : public ComponentSignal
{
private:
     Port *mpPp1;
     double *mpPp1_P, *mpOut;

public:
     static Component *Creator()
     {
        return new PneumaticPsensor();
     }

     void configure()
     {
        mpPp1=addReadPort("Pp1","NodePneumatic", "", Port::NotRequired);
        addOutputVariable("out", "Pressure", "Pa", &mpOut);
     }

    void initialize()
     {
        mpPp1_P=getSafeNodeDataPtr(mpPp1, NodePneumatic::Pressure);
        simulateOneTimestep();
     }

    void simulateOneTimestep()
     {
        (*mpOut) = (*mpPp1_P);
     }
};
#endif // PNEUMATICPSENSOR_HPP_INCLUDED
