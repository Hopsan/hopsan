/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   SignalSineWave.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-08
//!
//! @brief Contains a sine wave signal generator
//!
//$Id$

//////////////////////////////////////////////////////////
//                                                      //
//              Offset â                                //
//                                                      //
//                   XXX         XXX        â           //
//                  X   X       X   X       | Amplitude //
// Zero â  XXXXX   X     X     X     X      â           //
//                X       X   X       X                 //
//              XX         XXX         XXX              //
//                                                      //
//              â           â1/Frequencyâ               //
//          StartTime                                   //
//////////////////////////////////////////////////////////

#ifndef SIGNALSINEWAVE_HPP_INCLUDED
#define SIGNALSINEWAVE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "math.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSineWave : public ComponentSignal
    {

    private:
        double *mpStartTime;
        double *mpFrequency;
        double *mpAmplitude;
        double *mpPhaseTOffset;
        double *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalSineWave();
        }

        void configure()
        {
            addInputVariable("t_start", "Start Time", "s", 0.0, &mpStartTime);
            addInputVariable("f", "Frequencty", "Hz", 1.0, &mpFrequency);
            addInputVariable("y_A", "Amplitude", "-", 1.0, &mpAmplitude);
            addInputVariable("y_offset", "(Phase) Offset", "s", 0.0, &mpPhaseTOffset);

            addOutputVariable("out", "Sinus wave output", "", &mpOut);
        }


        void initialize()
        {
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            // Sinewave Equations
            if (mTime < (*mpStartTime))
            {
                (*mpOut) = 0.0;     //Before start
            }
            else
            {
                // out = A * sin( (T-Tstart-Toffset)*2*pi*f )
                (*mpOut) = (*mpAmplitude) * sin( (mTime-(*mpStartTime)-(*mpPhaseTOffset)) * 2.0*M_PI*(*mpFrequency) );
            }
        }
    };
}

#endif // SIGNALSINEWAVE_HPP_INCLUDED
