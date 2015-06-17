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
//! @file   HydraulicNodeSensor.hpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2015-03-07
//!
//! @brief Contains a Hydraulic Node Sensor Component
//!
//$Id$

#ifndef HYDRAULICNODESENSOR_HPP_INCLUDED
#define HYDRAULICNODESENSOR_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup HydraulicComponents
    //!
    class HydraulicNodeSensor : public ComponentSignal
    {
    private:
        Port *mpP1;
        double *mpOut_p, *mpOut_q, *mpOut_c, *mpOut_Zc;

    public:
        static Component *Creator()
        {
            return new HydraulicNodeSensor();
        }

        void configure()
        {
            mpP1 = addReadPort("P1", "NodeHydraulic", "", Port::NotRequired);
            addOutputVariable("p", "Pressure", "Pa", &mpOut_p);
            addOutputVariable("q", "Flow", "m^3/s", &mpOut_q);
            addOutputVariable("c", "WaveVariable", "",  &mpOut_c);
            addOutputVariable("Zc", "Charateristc Impedance", "", &mpOut_Zc);
        }


        void initialize()
        {
            simulateOneTimestep(); //Set initial output node value
        }


        void simulateOneTimestep()
        {
            *mpOut_p = mpP1->readNode(NodeHydraulic::Pressure);
            *mpOut_q = mpP1->readNode(NodeHydraulic::Flow);
            *mpOut_c = mpP1->readNode(NodeHydraulic::WaveVariable);
            *mpOut_Zc = mpP1->readNode(NodeHydraulic::CharImpedance);
        }
    };
}

#endif
