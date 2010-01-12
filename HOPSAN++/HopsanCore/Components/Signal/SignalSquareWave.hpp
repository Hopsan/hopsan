/*
 *  SignalSquareWave.hpp
 *  HOPSAN++
 *
 *  Created by Robert Braun on 2010-01-08.
 *  Copyright 2010 LiU. All rights reserved.
 *
 */

///////////////////////////////////////////////////////////
//               ↑  XXXXX   XXXXX   XXXXX                //
//     Amplitude |  X   X   X   X   X   X                //
//               ↓  X   XXXXX   XXXXX   XXX  ← BaseValue //
//                  X                                    //
// Zero →  XXXXXXXXXXX                                   //
//                                                       //
//                   ↑                                   //
//              StartTime                                //
///////////////////////////////////////////////////////////

#ifndef SIGNALSQUAREWAVE_HPP_INCLUDED
#define SIGNALSQUAREWAVE_HPP_INCLUDED

#include "HopsanCore.h"
#include "math.h"

class SignalSquareWave : public ComponentSignal
{

private:
    double mStartTime;
    double mFrequency;
    double mAmplitude;
    double mBaseValue;
    enum {out};

public:
    static Component *Creator()
    {
        std::cout << "running Squarewave creator" << std::endl;
        return new SignalSquareWave("DefaultSquareWaveName");
    }


    SignalSquareWave(const string name,
                              const double starttime = 0.0,
                              const double frequency = 1.0,
                              const double amplitude = 1.0,
                              const double basevalue = 0.0,
                              const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        mStartTime = starttime;
        mFrequency = frequency;
        mAmplitude = amplitude;
        mBaseValue = basevalue;

        addPort("out", "NodeSignal", out);

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
        //read fron nodes
   		Node* p1_ptr = mPorts[out].getNodePtr();

        //Step Equations
        double outputSignal;
        int relTimeInt;
        if (mTime < mStartTime)
        {
            outputSignal = 0;
        }
        else
        {
            relTimeInt = ceil((mTime-mStartTime)*mFrequency);
            outputSignal = mBaseValue + (mAmplitude * (relTimeInt % 2));
        }

        //Write new values to nodes
        p1_ptr->setData(NodeSignal::VALUE, outputSignal);
    }
};

#endif // SIGNALSQUAREWAVE_HPP_INCLUDED
