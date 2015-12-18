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
