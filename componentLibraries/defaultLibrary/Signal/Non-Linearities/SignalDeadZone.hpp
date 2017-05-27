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

//$Id$

#ifndef SIGNALDEADZONE_HPP_INCLUDED
#define SIGNALDEADZONE_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalDeadZone : public ComponentSignal
    {


    private:
        double *mpStartDead;
        double *mpEndDead;
        double *mpND_in, *mpND_out;

    public:
        static Component *Creator()
        {
            return new SignalDeadZone();
            }

        void configure()
        {
            addInputVariable("in", "", "", 0.0, &mpND_in);
            addOutputVariable("out", "", "", &mpND_out);

            addInputVariable("u_dstart", "Start of Dead Zone", "", -1.0, &mpStartDead);
            addInputVariable("u_dend", "End of Dead Zone", "", 1.0, &mpEndDead);
        }

        void initialize()
        {
            simulateOneTimestep();
        }

        void simulateOneTimestep()
        {
            //Deadzone equations
            if ( (*mpND_in) < (*mpStartDead))
            {
                (*mpND_out) = (*mpND_in) - (*mpStartDead);
            }
            else if ( (*mpND_in) > (*mpStartDead) && (*mpND_in) < (*mpEndDead))
            {
                (*mpND_out) = 0;
            }
            else
            {
                (*mpND_out) = (*mpND_in) - (*mpEndDead);
            }
        }
    };
}

#endif // SIGNALDEADZONE_HPP_INCLUDED
