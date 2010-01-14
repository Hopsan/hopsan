/*
 *  SignalPulse.hpp
 *  HOPSAN++
 *
 *  Created by Robert Braun on 2010-01-14.
 *  Copyright 2010 LiU. All rights reserved.
 *
 */

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
        mBaseValue = basevalue;
        mStartTime = starttime;
        mStopTime = stoptime;
        mAmplitude = amplitude;

        addPort("out", "NodeSignal", out);

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
        //read fron nodes
   		Node* p1_ptr = mPorts[out].getNodePtr();

        //Step Equations
        double outputSignal;

        if (mTime > mStartTime && mTime < mStopTime)
        {
            outputSignal = mBaseValue + mAmplitude;     //During pulse
        }
        else
        {
            outputSignal = mBaseValue;                   //Not during pulse
        }

        //Write new values to nodes
        p1_ptr->setData(NodeSignal::VALUE, outputSignal);
    }
};

#endif // SIGNALPULSE_HPP_INCLUDED
