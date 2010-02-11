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

#include "HopsanCore.h"

class SignalPulse : public ComponentSignal
{

private:
    double mBaseValue;
    double mStartTime;
    double mStopTime;
    double mAmplitude;
    enum {out};

public:
    static Component *Creator()
    {
        std::cout << "running Pulse creator" << std::endl;
        return new SignalPulse("DefaultPulseName");
    }


    SignalPulse(const string name,
                          const double basevalue = 0.0,
                          const double starttime = 1.0,
                          const double stoptime = 2.0,
                          const double amplitude = 1.0,
                          const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        mTypeName = "SignalPulse";
        mBaseValue = basevalue;
        mStartTime = starttime;
        mStopTime = stoptime;
        mAmplitude = amplitude;

        addWritePort("out", "NodeSignal", out);

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
        mPortPtrs[out]->writeNode(NodeSignal::VALUE, output);

    }
};

#endif // SIGNALPULSE_HPP_INCLUDED
