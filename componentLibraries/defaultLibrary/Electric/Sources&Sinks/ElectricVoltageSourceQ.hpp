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

#ifndef ELECTRICVOLTAGESOURCEQ_HPP
#define ELECTRICVOLTAGESOURCEQ_HPP

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"


using namespace hopsan;

class ElectricVoltageSourceQ : public ComponentQ
{
private:
     Port *mpPel1;

     double uin;

     double *mpND_uel1;
     double *mpND_iel1;
     double *mpND_cel1;
     double *mpND_Zcel1;

     double *mpU;

public:
     static Component *Creator()
     {
        return new ElectricVoltageSourceQ();
     }

     void configure()
     {
        mpPel1=addPowerPort("Pel1","NodeElectric");

        addInputVariable("U","Voltage","V",12.,&mpU);
     }

    void initialize()
     {
        mpND_uel1=getSafeNodeDataPtr(mpPel1, NodeElectric::Voltage);
        mpND_iel1=getSafeNodeDataPtr(mpPel1, NodeElectric::Current);
        mpND_cel1=getSafeNodeDataPtr(mpPel1, NodeElectric::WaveVariable);
        mpND_Zcel1=getSafeNodeDataPtr(mpPel1, NodeElectric::CharImpedance);
     }
    void simulateOneTimestep()
     {
        (*mpND_iel1) = (*mpU - *mpND_cel1)/(*mpND_Zcel1);
        (*mpND_uel1) = (*mpU);
     }

};

#endif // ELECTRICVOLTAGESOURCEQ_HPP
