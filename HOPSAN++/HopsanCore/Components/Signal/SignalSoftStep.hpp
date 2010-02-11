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

#include "HopsanCore.h"
#include "math.h"

class SignalSoftStep : public ComponentSignal
{

private:
    double mStartTime;
    double mStopTime;
    double mBaseValue;
    double mAmplitude;
    double mFrequency;
    double mOffset;
    enum {out};

public:
    static Component *Creator()
    {
        std::cout << "running softstep creator" << std::endl;
        return new SignalSoftStep("DefaultSoftStepName");
    }

    SignalSoftStep(const string name,
                   const double starttime = 1.0,
                   const double stoptime = 2.0,
                   const double basevalue = 0.0,
                   const double amplitude = 1.0,
                   const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        mTypeName = "SignalSoftStep";
        mStartTime = starttime;
        mStopTime = stoptime;
        mBaseValue = basevalue;
        mAmplitude = amplitude;
        mFrequency = 3.141592653589/(mStopTime-mStartTime);       //omega = 2pi/T, T = (stoptime-starttime)*4

        addWritePort("out", "NodeSignal", out);

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
        mPortPtrs[out]->writeNode(NodeSignal::VALUE, output);
    }
};

#endif // SIGNALSOFTSTEP_HPP_INCLUDED
