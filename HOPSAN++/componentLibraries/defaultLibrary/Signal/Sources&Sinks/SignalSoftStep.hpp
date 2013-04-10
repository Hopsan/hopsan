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
//! @file   SignalSoftStep.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-15
//!
//! @brief Contains a soft step generator
//!
//$Id$

///////////////////////////////////////////////
//                    StopTime               //
//                       â                   //
//                                           //
//                       XXXXXX  â           //
//                      X        |           //
//                     X         | Amplitude //
//                     X         |           //
//                    X          |           //
// BaseValue â  XXXXXX           â           //
//                                           //
//                   â                       //
//               StartTime                   //
///////////////////////////////////////////////

#ifndef SIGNALSOFTSTEP_HPP_INCLUDED
#define SIGNALSOFTSTEP_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "math.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSoftStep : public ComponentSignal
    {

    private:
        double *mpStartTime;
        double *mpStopTime;
        double *mpBaseValue;
        double *mpAmplitude;
        double *mpOffset;
        double *mpOut;
        Port *mpOutPort;

    public:
        static Component *Creator()
        {
            return new SignalSoftStep();
        }

        void configure()
        {
            addInputVariable("t_start", "Start Time", "[s]", 1.0);
            addInputVariable("t_end", "Stop Time", "[s]", 2.0);
            addInputVariable("y_0", "Base Value", "[-]", 0.0);
            addInputVariable("y_A", "Amplitude", "[-]", 1.0);

            mpOutPort = addOutputVariable("out","","");
        }


        void initialize()
        {
            mpOut = getSafeNodeDataPtr(mpOutPort, NodeSignal::Value);
            mpStartTime = getSafeNodeDataPtr("t_start", NodeSignal::Value);
            mpStopTime = getSafeNodeDataPtr("t_end", NodeSignal::Value);
            mpBaseValue = getSafeNodeDataPtr("y_0", NodeSignal::Value);
            mpAmplitude = getSafeNodeDataPtr("y_A", NodeSignal::Value);

            (*mpOut) = (*mpBaseValue);
        }


        void simulateOneTimestep()
        {
            //Sinewave Equations
            const double startT = (*mpStartTime);
            const double stopT = (*mpStopTime);
            const double frequency = pi/(stopT-startT); //omega = 2pi/T, T = (stoptime-starttime)*4

            if (mTime < startT)
            {
                (*mpOut) = (*mpBaseValue);     //Before start
            }
            else if (mTime >= startT && mTime < stopT)
            {
                (*mpOut) = (*mpBaseValue) + 0.5*(*mpAmplitude)*sin((mTime-startT)*frequency - 3.141592653589/2.0) + (*mpAmplitude)*0.5;
            }
            else
            {
                (*mpOut) = (*mpBaseValue) + (*mpAmplitude);
            }
        }
    };
}

#endif // SIGNALSOFTSTEP_HPP_INCLUDED
