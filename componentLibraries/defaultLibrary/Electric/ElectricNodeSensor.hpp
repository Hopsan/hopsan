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
//! @file   ElectricNodeSensor.hpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2015-03-07
//!
//! @brief Contains a Electric Node Sensor Component
//!
//$Id: ElectricNodeSensor.hpp 8207 2015-07-16 18:49:37Z petno25 $

#ifndef ElectricNODESENSOR_HPP_INCLUDED
#define ElectricNODESENSOR_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup ElectricComponents
    //!
    class ElectricNodeSensor : public ComponentSignal
    {
    private:
        Port *mpP1;
        double *mpOut_u, *mpOut_i, *mpOut_c, *mpOut_Zc;

    public:
        static Component *Creator()
        {
            return new ElectricNodeSensor();
        }

        void configure()
        {
            mpP1 = addReadPort("P1", "NodeElectric", "", Port::NotRequired);
            addOutputVariable("u", "Voltage", "Voltage", &mpOut_u);
            addOutputVariable("i", "Current", "Current", &mpOut_i);
            addOutputVariable("c", "WaveVariable", "Voltage",  &mpOut_c);
            addOutputVariable("Zc", "Charateristc Impedance", "", &mpOut_Zc);
        }


        void initialize()
        {
            simulateOneTimestep(); //Set initial output node value
        }


        void simulateOneTimestep()
        {
            *mpOut_u = mpP1->readNode(NodeElectric::Voltage);
            *mpOut_i = mpP1->readNode(NodeElectric::Current);
            *mpOut_c = mpP1->readNode(NodeElectric::WaveVariable);
            *mpOut_Zc = mpP1->readNode(NodeElectric::CharImpedance);
        }
    };
}

#endif
