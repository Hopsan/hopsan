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
//              Offset →                                //
//                                                      //
//                   XXX         XXX        ↑           //
//                  X   X       X   X       | Amplitude //
// Zero →  XXXXX   X     X     X     X      ↓           //
//                X       X   X       X                 //
//              XX         XXX         XXX              //
//                                                      //
//              ↑           ←1/Frequency→               //
//          StartTime                                   //
//////////////////////////////////////////////////////////

#ifndef SIGNALSINEWAVE_HPP_INCLUDED
#define SIGNALSINEWAVE_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "math.h"

class SignalSineWave : public ComponentSignal
{

private:
    double mStartTime;
    double mFrequency;
    double mAmplitude;
    double mOffset;
    enum {out};

public:
    static Component *Creator()
    {
        std::cout << "running Sinewave creator" << std::endl;
        return new SignalSineWave("DefaultSineWaveName");
    }


    SignalSineWave(const string name,
                   const double starttime = 0.0,
                   const double frequency = 1.0,
                   const double amplitude = 1.0,
                   const double offset = 0.0,
                   const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        mTypeName = "SignalSineWave";
        mStartTime = starttime;
        mFrequency = frequency;
        mAmplitude = amplitude;
        mOffset = offset;

        addWritePort("out", "NodeSignal", out);

        registerParameter("StartTime", "Start Time", "s", mStartTime);
        registerParameter("Frequency", "Frequencty", "Hz", mFrequency);
        registerParameter("Amplitude", "Amplitude", "-", mAmplitude);
        registerParameter("Offset", "Offset", "s", mOffset);
    }


	void initialize()
	{
        //Nothing to initilize
	}


    void simulateOneTimestep()
    {

        //Sinewave Equations
        double output;
        if (mTime < mStartTime)
        {
            output = 0.0;     //Before start
        }
        else
        {
            output = mAmplitude*sin((mTime-mStartTime)*mFrequency*2*3.14159265 - mOffset);
        }

        //Write new values to nodes
        mPortPtrs[out]->writeNode(NodeSignal::VALUE, output);

    }
};

#endif // SIGNALSINEWAVE_HPP_INCLUDED
