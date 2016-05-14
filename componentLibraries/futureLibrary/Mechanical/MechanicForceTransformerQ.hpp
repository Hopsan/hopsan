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

//$Id: MechanicForceTransformerQ.hpp 371 2016-04-26 15:07:44Z petno25 $

#ifndef MECHANICFORCETRANSFORMERQ_HPP_INCLUDED
#define MECHANICFORCETRANSFORMERQ_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities/Integrator.h"

namespace hopsan {

//!
//! @brief
//! @ingroup MechanicalComponents
//!
class MechanicForceTransformerQ : public ComponentQ
{

private:
    MechanicNodeDataPointerStructT mP1;
    double *mpF_signal;
    Integrator mIntegrator1;

    Port *mpP1;

public:
    static Component *Creator()
    {
        return new MechanicForceTransformerQ();
    }

    void configure()
    {
        addInputVariable("F", "Generated force", "N", 0.0, &mpF_signal);
        mpP1 = addPowerPort("P1", "NodeMechanic");
        disableStartValue(mpP1, NodeMechanic::Force);
    }


    void initialize()
    {
        getMechanicPortNodeDataPointers(mpP1, mP1);
        mIntegrator1.initialize(mTimestep, mP1.v(), mP1.x());
        simulateOneTimestep();
    }


    void simulateOneTimestep()
    {
        // First calculate the non-contact behavior
        const double f = readSignal(mpF_signal);
        mP1.rf() = f;
        mP1.rv() = (f-mP1.c())/std::max(mP1.Zc(), 1e-12);
        mP1.rx() = mIntegrator1.update(mP1.v());
    }
};
}

#endif // MECHANICFORCETRANSFORMER_HPP_INCLUDED
