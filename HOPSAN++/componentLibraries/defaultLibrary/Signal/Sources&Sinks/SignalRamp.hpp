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
//! @file   SignalRamp.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-08
//!
//! @brief Contains a ramp signal generator
//!
//$Id$

///////////////////////////////////////////////////
//                       StopTime                //
//                          â                    //
//                                               //
//                          XXXXXXX  â           //
//                        XX         |           //
//                      XX           | Amplitude //
//                    XX             |           //
// BaseValue â  XXXXXX               â           //
//                                               //
//                   â                           //
//               StartTime                       //
///////////////////////////////////////////////////

#ifndef SIGNALRAMP_HPP_INCLUDED
#define SIGNALRAMP_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalRamp : public ComponentSignal
    {

    private:
        double *mpBaseValue;
        double *mpAmplitude;
        double *mpStartTime;
        double *mpStopTime;
        double *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalRamp();
        }

        void configure()
        {
            addInputVariable("y_0", "Base Value", "[-]", 0.0, &mpBaseValue);
            addInputVariable("y_A", "Amplitude", "[-]", 1.0, &mpAmplitude);
            addInputVariable("t_start", "Start Time", "[s]", 1.0, &mpStartTime);
            addInputVariable("t_end", "Stop Time", "[s]", 2.0, &mpStopTime);

            addOutputVariable("out", "Ramp output", "-", &mpOut);
        }


        void initialize()
        {
            (*mpOut) = (*mpBaseValue);
        }


        void simulateOneTimestep()
        {
            const double startT = (*mpStartTime);
            const double stopT = (*mpStopTime);

            // Step Equations
            if (mTime < startT)
            {
                (*mpOut) = (*mpBaseValue);     //Before ramp
            }
            else if (mTime >= startT && mTime < stopT)
            {
                (*mpOut) = ((mTime - startT) / (stopT - startT)) * (*mpAmplitude) + (*mpBaseValue);     //During ramp
            }
            else
            {
                (*mpOut) = (*mpBaseValue) + (*mpAmplitude);     //After ramp
            }
        }
    };
}

#endif // SIGNALRAMP_HPP_INCLUDED
