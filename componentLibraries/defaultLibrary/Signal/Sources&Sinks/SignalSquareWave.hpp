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
