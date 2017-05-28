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
//! @file   SignalUnitDelay.hpp
//! @author Robert Braun <bjorn.eriksson@liu.se>
//! @date   2013-01-11
//!
//! @brief Contains a Signal Time Delay Component
//!
//$Id$

#ifndef SIGNALUNITDELAY_HPP_INCLUDED
#define SIGNALUNITDELAY_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalUnitDelay : public ComponentSignal
    {

    private:
        double *mpND_in, *mpND_out;

    public:
        static Component *Creator()
        {
            return new SignalUnitDelay();
        }

        void configure()
        {
            Port *pIVPort = addInputVariable("in", "", "", 0.0, &mpND_in);
            pIVPort->setSortHint(IndependentDestination);
            addOutputVariable("out", "", "", 0.0, &mpND_out);
        }


        void initialize()
        {
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            (*mpND_out) =  (*mpND_in);
        }
    };
}

#endif // SIGNALUNITDELAY_HPP_INCLUDED
