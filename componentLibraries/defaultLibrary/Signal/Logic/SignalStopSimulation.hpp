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
//! @file   SignalStopSimulation.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-10-15
//!
//! @brief Contains a component for stopping a simulation
//!
//$Id$

#ifndef SIGNALSTOPSIMULATION_HPP_INCLUDED
#define SIGNALSTOPSIMULATION_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalStopSimulation : public ComponentSignal
    {

    private:
        double *mpIn;
        HString mMessage;

    public:
        static Component *Creator()
        {
            return new SignalStopSimulation();
        }

        void configure()
        {
            addInputVariable("in", "Stop simulation if >0.5", "", boolToDouble(false), &mpIn);
            addConstant("message", "Message to show when stopping", "", "", mMessage);
        }


        void initialize()
        {
            // Nothing
        }


        void simulateOneTimestep()
        {
            if(doubleToBool(*mpIn))
            {
                if (mMessage.empty())
                {
                    stopSimulation(HString("Caused by component: ")+getName());
                }
                else
                {
                    stopSimulation(mMessage);
                }
            }
        }
    };
}
#endif // SIGNALSTOPSIMULATION_HPP_INCLUDED
