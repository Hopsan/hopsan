/*
 *  SignalSineWave.hpp
 *  HOPSAN++
 *
 *  Created by Robert Braun on 2010-01-08.
 *  Copyright 2010 LiU. All rights reserved.
 *
 */

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

#include "HopsanCore.h"
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
        mStartTime = starttime;
        mFrequency = frequency;
        mAmplitude = amplitude;
        mOffset = offset;

        addPort("out", "NodeSignal", out);

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
        //read fron nodes
   		Node* p1_ptr = mPorts[out].getNodePtr();

        //Sinewave Equations
        double outputSignal;
        if (mTime < mStartTime)
        {
            outputSignal = 0.0;     //Before start
        }
        else
        {
            outputSignal = mAmplitude*sin((mTime-mStartTime)*mFrequency*2*3.14159265 - mOffset);
        }

        //Write new values to nodes
        p1_ptr->setData(NodeSignal::VALUE, outputSignal);
    }
};

#endif // SIGNALSINEWAVE_HPP_INCLUDED
