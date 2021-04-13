/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/



//!
//! @file   ElectricNodeSensor.hpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2015-03-07
//!
//! @brief Contains a Electric Node Sensor Component
//!
//$Id$

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
