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

#ifndef MECHANICTRANSLATIONALQSOURCE_HPP
#define MECHANICTRANSLATIONALQSOURCE_HPP

#include "ComponentEssentials.h"

namespace hopsan {

//!
//! @brief
//! @ingroup MechanicalComponents
//!
class MechanicTranslationalQSource : public ComponentQ
{

private:
    double *mpIn_f, *mpIn_v, *mpIn_x, *mpIn_em;
    Port *mpMP1;
    MechanicNodeDataPointerStructT mMP1;

public:
    static Component *Creator()
    {
        return new MechanicTranslationalQSource();
    }

    void configure()
    {
        addInputVariable("in_f", "Force variable input", "Force", 0, &mpIn_f);
        addInputVariable("in_v", "Velocity variable input", "Velocity", 0, &mpIn_v);
        addInputVariable("in_x", "Position variable input", "Position", 0, &mpIn_x);
        addInputVariable("in_em", "Equivalent mass variable input", "Mass", 1, &mpIn_em);
        mpMP1 = addPowerPort("MP1", "NodeMechanic");
    }

    void initialize()
    {
        getMechanicPortNodeDataPointers(mpMP1, mMP1);
        simulateOneTimestep();
    }

    void simulateOneTimestep()
    {
        mMP1.rf() = readSignal(mpIn_f);
        mMP1.rv() = readSignal(mpIn_v);
        mMP1.rx() = readSignal(mpIn_x);
        mMP1.rMe() = readSignal(mpIn_em);
    }
};
}

#endif // MECHANICTRANSLATIONALQSOURCE_HPP

