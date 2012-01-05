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
//! @file   SignalSquareWave.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-08
//!
//! @brief Contains a square wave signal generator
//!
//$Id$

///////////////////////////////////////////////////////////
//                â  XXXXX   XXXXX   XXXXX                //
//      Amplitude |  X   X   X   X   X   X                //
//                â  X   XXXXX   XXXXX   XXX  â BaseValue //
//                   X                                    //
// Zero â  XXXXXXXXXXX                                   //
//                                                       //
//                   â                                   //
//              StartTime                                //
///////////////////////////////////////////////////////////

#ifndef SIGNALSQUAREWAVE_HPP_INCLUDED
#define SIGNALSQUAREWAVE_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "math.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalSquareWave : public ComponentSignal
    {

    private:
        double mStartTime;
        double mFrequency;
        double mAmplitude;
        double mBaseValue;
        int relTimeInt;
        double *mpND_out;
        Port *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalSquareWave();
        }

        SignalSquareWave() : ComponentSignal()
        {
            mStartTime = 0.0;
            mFrequency = 1.0;
            mAmplitude = 1.0;
            mBaseValue = 0.0;

            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);

            registerParameter("t_start", "Start Time", "[s]", mStartTime);
            registerParameter("f", "Frequencty", "[Hz]", mFrequency);
            registerParameter("y_A", "Amplitude", "[-]", mAmplitude);
            registerParameter("y_0", "Base Value", "[-]", mBaseValue);

            disableStartValue(mpOut, NodeSignal::VALUE);
        }


        void initialize()
        {
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            //Write basevalue value to node
            (*mpND_out) = mBaseValue;
        }


        void simulateOneTimestep()
        {
            //Step Equations
            if (mTime < mStartTime)
            {
                (*mpND_out) = 0;
            }
            else
            {
                relTimeInt = (int)ceil((mTime-mStartTime)*mFrequency);
                (*mpND_out) = mBaseValue + (2*mAmplitude * (relTimeInt % 2)) - mAmplitude;
            }
        }
    };
}

#endif // SIGNALSQUAREWAVE_HPP_INCLUDED
