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
//! @file   SignalCeil.hpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2015-12-08
//!
//$Id$

#ifndef SIGNALCEIL_HPP_INCLUDED
#define SIGNALCEIL_HPP_INCLUDED

#include "ComponentEssentials.h"
#include <cmath>

namespace hopsan {

//!
//! @brief
//! @ingroup SignalComponents
//!
class SignalCeil : public ComponentSignal
{

private:
    double *mpIn, *mpOut;

public:
    static Component *Creator()
    {
        return new SignalCeil();
    }

    void configure()
    {
        addInputVariable("in", "", "", 0.0, &mpIn);
        addOutputVariable("out", "ceil(in)", "", &mpOut);
    }


    void initialize()
    {
        simulateOneTimestep();
    }


    void simulateOneTimestep()
    {
        (*mpOut) = ceil(*mpIn);
    }
};
}
#endif // SIGNALCEIL_HPP_INCLUDED
