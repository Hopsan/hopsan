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

#ifndef PNEUMATICQMSENSOR_HPP_INCLUDED
#define PNEUMATICQMSENSOR_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

//!
//! @file PneumaticQmsensor.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date Wed 13 Mar 2013 16:55:00
//! @brief Pneumatic massflow sensor
//! @ingroup PneumaticComponents
//!
//$Id$

using namespace hopsan;

class PneumaticQmsensor : public ComponentSignal
{
private:
     Port *mpPp1;
     double *mpPp1_Qm, *mpOut;

public:
     static Component *Creator()
     {
        return new PneumaticQmsensor();
     }

     void configure()
     {
        mpPp1=addReadPort("Pp1","NodePneumatic", "", Port::NotRequired);
        addOutputVariable("out", "Flow", "kg/s", &mpOut);
     }

    void initialize()
     {
        mpPp1_Qm=getSafeNodeDataPtr(mpPp1, NodePneumatic::MassFlow);
        simulateOneTimestep();
     }

    void simulateOneTimestep()
     {
        (*mpOut) = (*mpPp1_Qm);
     }
};
#endif // PNEUMATICQMSENSOR_HPP_INCLUDED
