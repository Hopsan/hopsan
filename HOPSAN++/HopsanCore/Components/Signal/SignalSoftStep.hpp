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
//                       ↓                   //
//                                           //
//                       XXXXXX  ↑           //
//                      X        |           //
//                     X         | Amplitude //
//                     X         |           //
//                    X          |           //
// BaseValue →  XXXXXX           ↓           //
//                                           //
//                   ↑                       //
//               StartTime                   //
///////////////////////////////////////////////

#ifndef SIGNALSOFTSTEP_HPP_INCLUDED
#define SIGNALSOFTSTEP_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "math.h"

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
    Port *mpOut;

public:
    static Component *Creator()
    {
        return new SignalSoftStep("SoftStep");
    }

    SignalSoftStep(const string name) : ComponentSignal(name)
    {
        mTypeName = "SignalSoftStep";
        mStartTime = 1.0;
        mStopTime = 2.0;
        mBaseValue = 0.0;
        mAmplitude = 1.0;
        mFrequency = pi/(mStopTime-mStartTime);       //omega = 2pi/T, T = (stoptime-starttime)*4

        mpOut = addWritePort("out", "NodeSignal");

        registerParameter("StartTime", "Start Time", "s", mStartTime);
        registerParameter("StopTime", "Stop Time", "s", mStopTime);
        registerParameter("BaseValue", "Base Value", "-", mBaseValue);
        registerParameter("Amplitude", "Amplitude", "-", mAmplitude);
    }


    void initialize()
    {
        //Nothing to initilize
    }


    void simulateOneTimestep()
    {
        //Sinewave Equations
        double output;
        mFrequency = pi/(mStopTime-mStartTime);
        if (mTime < mStartTime)
        {
            output = mBaseValue;     //Before start
        }
        else if (mTime > mStartTime && mTime < mStopTime)
        {
            output = mBaseValue + 0.5*mAmplitude*sin((mTime-mStartTime)*mFrequency - 3.141592653589/2) + mAmplitude*0.5;
        }
        else
        {
            output = mBaseValue + mAmplitude;
        }

        //Write new values to nodes
        mpOut->writeNode(NodeSignal::VALUE, output);
    }
};

#endif // SIGNALSOFTSTEP_HPP_INCLUDED
