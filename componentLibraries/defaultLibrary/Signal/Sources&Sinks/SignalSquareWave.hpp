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
//! @file   SignalSquareWave.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-08
//!
//! @brief Contains a square wave signal generator
//!
//$Id$

///////////////////////////////////////////////////////////
//                â  XXXXX   XXXXX   XXXXX               //
//      Amplitude |  X   X   X   X   X   X               //
//  BaseValue XXXXXXXX   X   X   X   X   XXX             //
//                       X   X   X   X                   //
//                       XXXXX   XXXXX                   //
//                                                       //
//                   â                                   //
//              StartTime                                //
///////////////////////////////////////////////////////////

#ifndef SIGNALSQUAREWAVE_HPP_INCLUDED
#define SIGNALSQUAREWAVE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities/AuxiliarySimulationFunctions.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSquareWave : public ComponentSignal
    {

    private:
        double *mpStartTime;
        double *mpFrequency;
        double *mpAmplitude;
        double *mpBaseValue;
        double *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalSquareWave();
        }

        void configure()
        {
            addInputVariable("y_0", "Base Value", "", 0.0, &mpBaseValue);
            addInputVariable("y_A", "Amplitude", "", 1.0, &mpAmplitude);
            addInputVariable("f", "Frequencty", "Hz", 1.0, &mpFrequency);
            addInputVariable("t_start", "Start Time", "Time", 0.0, &mpStartTime);


            addOutputVariable("out", "Square wave output", "", &mpOut);
        }


        void initialize()
        {
            // Write basevalue value to node
            (*mpOut) = (*mpBaseValue);
        }


        void simulateOneTimestep()
        {
            // Step Equations
            if (mTime < (*mpStartTime))
            {
                (*mpOut) = (*mpBaseValue);
            }
            else
            {
                if ( sin( (mTime-(*mpStartTime))*2.0*pi*(*mpFrequency) ) >= 0.0 )
                {
                    (*mpOut) = (*mpBaseValue) + (*mpAmplitude);
                }
                else
                {
                    (*mpOut) = (*mpBaseValue) - (*mpAmplitude);
                }
            }
        }
    };
}

#endif // SIGNALSQUAREWAVE_HPP_INCLUDED
