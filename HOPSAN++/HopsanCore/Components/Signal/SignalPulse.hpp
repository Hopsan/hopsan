//!
//! @file   SignalPulse.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-14
//!
//! @brief Contains a pulse signal generator
//!
//$Id$

////////////////////////////////////////////////
//                    XXXXX       ↑           //
//                    X   X       | Amplitude //
//                    X   X       |           //
// BaseValue →  XXXXXXX   XXXXXXX ↓           //
//                    ↑   ↑                   //
//            StartTime   StopTime            //
////////////////////////////////////////////////

#ifndef SIGNALPULSE_HPP_INCLUDED
#define SIGNALPULSE_HPP_INCLUDED

#include "../../ComponentEssentials.h"

//!
//! @brief
//! @ingroup SignalComponents
//!
class SignalPulse : public ComponentSignal
{

private:
    double mBaseValue;
    double mStartTime;
    double mStopTime;
    double mAmplitude;
    Port *mpOut;

public:
    static Component *Creator()
    {
        return new SignalPulse("Pulse");
    }

    SignalPulse(const std::string name) : ComponentSignal(name)
    {
        mTypeName = "SignalPulse";
        mBaseValue = 0.0;
        mStartTime = 1.0;
        mStopTime = 2.0;
        mAmplitude = 1.0;

        mpOut = addWritePort("out", "NodeSignal");

        registerParameter("BaseValue", "Base Value", "-", mBaseValue);
        registerParameter("StartTime", "Start Time", "-", mStartTime);
        registerParameter("StopTime", "Stop Time", "-", mStopTime);
        registerParameter("Amplitude", "Amplitude", "-", mAmplitude);
    }


    void initialize()
    {
        //Nothing to initilize
    }


    void simulateOneTimestep()
    {

        //Step Equations
        double output;

        if (mTime > mStartTime && mTime < mStopTime)
        {
            output = mBaseValue + mAmplitude;     //During pulse
        }
        else
        {
            output = mBaseValue;                   //Not during pulse
        }

        //Write new values to nodes
        mpOut->writeNode(NodeSignal::VALUE, output);

    }
};

#endif // SIGNALPULSE_HPP_INCLUDED
