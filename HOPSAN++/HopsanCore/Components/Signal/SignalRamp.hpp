/*
 *  SignalRamp.hpp
 *  HOPSAN++
 *
 *  Created by Robert Braun on 2010-01-08.
 *  Copyright 2010 LiU. All rights reserved.
 *
 */

///////////////////////////////////////////////////
//                       StopTime                //
//                          ↓                    //
//                                               //
//                          XXXXXXX  ↑           //
//                        XX         |           //
//                      XX           | Amplitude //
//                    XX             |           //
// BaseValue →  XXXXXX               ↓           //
//                                               //
//                   ↑                           //
//               StartTime                       //
///////////////////////////////////////////////////

#ifndef SIGNALRAMP_HPP_INCLUDED
#define SIGNALRAMP_HPP_INCLUDED

#include "HopsanCore.h"

class SignalRamp : public ComponentSignal
{

private:
    double mBaseValue;
    double mAmplitude;
    double mStartTime;
    double mStopTime;
    enum {out};

public:
    static Component *Creator()
    {
        std::cout << "running ramp creator" << std::endl;
        return new SignalRamp("DefaultRampName");
    }


    SignalRamp(const string name,
                          const double basevalue = 0.0,
                          const double amplitude = 1.0,
                          const double starttime = 1.0,
                          const double stoptime = 2.0,
                          const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        mBaseValue = basevalue;
        mAmplitude = amplitude;
        mStartTime = starttime;
        mStopTime = stoptime;

        addWritePort("out", "NodeSignal", out);

        registerParameter("BaseValue", "Base Value", "-", mBaseValue);
        registerParameter("Amplitude", "Amplitude", "-", mAmplitude);
        registerParameter("StartTime", "Start Time", "s", mStartTime);
        registerParameter("StopTime", "Stop Time", "s", mStopTime);
    }


	void initialize()
	{
        //Nothing to initilize
	}


    void simulateOneTimestep()
    {
        //Step Equations
        double output;

        if (mTime < mStartTime)
        {
            output = mBaseValue;     //Before ramp
        }
        else if (mTime >= mStartTime && mTime < mStopTime)
        {
            output = ((mTime - mStartTime) / (mStopTime - mStartTime)) * mAmplitude + mBaseValue ;     //During ramp
        }
        else
        {
            output = mBaseValue + mAmplitude;     //After ramp
        }

        //Write new values to nodes
        mPortPtrs[out]->writeNode(NodeSignal::VALUE, output);

    }
};

#endif // SIGNALRAMP_HPP_INCLUDED
