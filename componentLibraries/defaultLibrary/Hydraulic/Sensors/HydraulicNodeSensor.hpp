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
            addOutputVariable("p", "Pressure", "Pressure", &mpOut_p);
            addOutputVariable("q", "Flow", "Flow", &mpOut_q);
            addOutputVariable("c", "WaveVariable", "Pressure",  &mpOut_c);
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
