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

#ifndef MECHANICFORCETRANSFORMER_HPP_INCLUDED
#define MECHANICFORCETRANSFORMER_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

//!
//! @brief
//! @ingroup MechanicalComponents
//!
class MechanicForceTransformer : public ComponentC
{

private:
    double *mpF_signal, *mpP1_f, *mpP1_c;
    Port *mpP1;

public:
    static Component *Creator()
    {
        return new MechanicForceTransformer();
    }

    void configure()
    {
        addInputVariable("F", "Generated force", "N", 0.0, &mpF_signal);
        mpP1 = addPowerPort("P1", "NodeMechanic");
        disableStartValue(mpP1, NodeMechanic::Force);
        setDefaultStartValue(mpP1, NodeMechanic::CharImpedance, 0);
        disableStartValue(mpP1, NodeMechanic::CharImpedance);
    }


    void initialize()
    {
        mpP1_f = getSafeNodeDataPtr(mpP1, NodeMechanic::Force);
        mpP1_c = getSafeNodeDataPtr(mpP1, NodeMechanic::WaveVariable);

        (*mpP1_f) = (*mpF_signal);
        if ((*mpP1_c) == 0)
        {
            (*mpP1_c) = (*mpF_signal);
        }
    }


    void simulateOneTimestep()
    {
        (*mpP1_c) = (*mpF_signal);
    }
};
}

#endif // MECHANICFORCETRANSFORMER_HPP_INCLUDED
