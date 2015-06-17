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

//$Id$

#ifndef HYDRAULICDUMMYQ_HPP_INCLUDED
#define HYDRAULICDUMMYQ_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"

namespace hopsan {

    class HydraulicDummyQ : public ComponentQ
    {
    private:
        Port *mpP2;
        double *mpIn, *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1;

    public:
        static Component *Creator()
        {
            return new HydraulicDummyQ();
        }

        void configure()
        {
            addInputVariable("in", "", "", 0.0, &mpIn);
            mpP2 = addPowerPort("P1", "NodeHydraulic","",Port::NotRequired);
        }


        void initialize()
        {
            mpND_p1 = getSafeNodeDataPtr(mpP2, NodeHydraulic::Pressure);
            mpND_q1 = getSafeNodeDataPtr(mpP2, NodeHydraulic::Flow);
            mpND_c1 = getSafeNodeDataPtr(mpP2, NodeHydraulic::WaveVariable);
            mpND_Zc1 = getSafeNodeDataPtr(mpP2, NodeHydraulic::CharImpedance);
        }


        void simulateOneTimestep()
        {
            (*mpND_p1) = 1;
            for(int i=0; i<(*mpIn); ++i)
            {
                (*mpND_p1) = (*mpND_p1) * i;
            }
        }
    };
}

#endif // HYDRAULICDUMMYQ_HPP_INCLUDED
