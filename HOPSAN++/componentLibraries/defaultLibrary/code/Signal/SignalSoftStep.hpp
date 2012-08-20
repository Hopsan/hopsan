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
        double mStartTime;
        double mStopTime;
        double mBaseValue;
        double mAmplitude;
        double mFrequency;
        double mOffset;
        double *mpND_out;
        Port *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalSoftStep();
        }

        void configure()
        {
            mStartTime = 1.0;
            mStopTime = 2.0;
            mBaseValue = 0.0;
            mAmplitude = 1.0;
            mFrequency = pi/(mStopTime-mStartTime);       //omega = 2pi/T, T = (stoptime-starttime)*4

            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

            registerParameter("t_start", "Start Time", "[s]", mStartTime);
            registerParameter("t_end", "Stop Time", "[s]", mStopTime);
            registerParameter("y_0", "Base Value", "[-]", mBaseValue);
            registerParameter("y_A", "Amplitude", "[-]", mAmplitude);

            disableStartValue(mpOut, NodeSignal::VALUE);
        }


        void initialize()
        {
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            (*mpND_out) = mBaseValue;
        }


        void simulateOneTimestep()
        {
            //Sinewave Equations

            mFrequency = pi/(mStopTime-mStartTime);
            if (mTime < mStartTime)
            {
                (*mpND_out) = mBaseValue;     //Before start
            }
            else if (mTime > mStartTime && mTime < mStopTime)
            {
                (*mpND_out) = mBaseValue + 0.5*mAmplitude*sin((mTime-mStartTime)*mFrequency - 3.141592653589/2) + mAmplitude*0.5;
            }
            else
            {
                (*mpND_out) = mBaseValue + mAmplitude;
            }
        }
    };
}

#endif // SIGNALSOFTSTEP_HPP_INCLUDED
