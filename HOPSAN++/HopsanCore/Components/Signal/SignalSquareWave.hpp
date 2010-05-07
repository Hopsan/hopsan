//!
//! @file   SignalSquareWave.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-08
//!
//! @brief Contains a square wave signal generator
//!
//$Id$

///////////////////////////////////////////////////////////
//                ↑  XXXXX   XXXXX   XXXXX                //
//      Amplitude |  X   X   X   X   X   X                //
//                ↓  X   XXXXX   XXXXX   XXX  ← BaseValue //
//                   X                                    //
// Zero →  XXXXXXXXXXX                                   //
//                                                       //
//                   ↑                                   //
//              StartTime                                //
///////////////////////////////////////////////////////////

#ifndef SIGNALSQUAREWAVE_HPP_INCLUDED
#define SIGNALSQUAREWAVE_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "math.h"

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
    Port *mpOut;

public:
    static Component *Creator()
    {
        return new SignalSquareWave("SquareWave");
    }

    SignalSquareWave(const string name) : ComponentSignal(name)
    {
        mTypeName = "SignalSquareWave";
        mStartTime = 0.0;
        mFrequency = 1.0;
        mAmplitude = 1.0;
        mBaseValue = 0.0;

        mpOut = addWritePort("out", "NodeSignal");

        registerParameter("StartTime", "Start Time", "s", mStartTime);
        registerParameter("Frequency", "Frequencty", "Hz", mFrequency);
        registerParameter("Amplitude", "Amplitude", "-", mAmplitude);
        registerParameter("BaseValue", "Base Value", "-", mBaseValue);
    }


    void initialize()
    {
        //Nothing to initilize
    }


    void simulateOneTimestep()
    {
        //Step Equations
        double output;
        int relTimeInt;
        if (mTime < mStartTime)
        {
            output = 0;
        }
        else
        {
            relTimeInt = (int)ceil((mTime-mStartTime)*mFrequency);
            output = mBaseValue + (mAmplitude * (relTimeInt % 2));
        }

        //Write new values to nodes
        mpOut->writeNode(NodeSignal::VALUE, output);
    }
};

#endif // SIGNALSQUAREWAVE_HPP_INCLUDED
