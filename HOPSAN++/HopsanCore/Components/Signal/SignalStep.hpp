/*
 *  SignalStep.hpp
 *  HOPSAN++
 *
 *  Created by Robert Braun on 2010-01-08.
 *  Copyright 2010 LiU. All rights reserved.
 *
 */

///////////////////////////////////////////
//                    XXXXXX  ↑          //
//                    X       | StepSize //
//                    X       |          //
// StartValue →  XXXXXX       ↓          //
//                                       //
//                    ↑                  //
//                 StepTime              //
///////////////////////////////////////////

#ifndef SIGNALSTEP_HPP_INCLUDED
#define SIGNALSTEP_HPP_INCLUDED

#include "HopsanCore.h"

class SignalStep : public ComponentSignal
{

private:
    double mStartValue;
    double mStepSize;
    double mStepTime;
    enum {out};

public:
    static Component *Creator()
    {
        std::cout << "running Step creator" << std::endl;
        return new SignalStep("DefaultStepName");
    }


    SignalStep(const string name,
                          const double startvalue = 0.0,
                          const double stepsize = 1.0,
                          const double steptime = 1.0,
                          const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        mStartValue = startvalue;
        mStepSize = stepsize;
        mStepTime = steptime;

        addWritePort("out", "NodeSignal", out);

        registerParameter("StartValue", "Start Value", "-", mStartValue);
        registerParameter("StepSize", "Final Value", "-", mStepSize);
        registerParameter("StepTime", "Step Time", "-", mStepTime);
    }


	void initialize()
	{
        //Nothing to initilize
	}


    void simulateOneTimestep()
    {
        //Step Equations
        double output;
        if (mTime < mStepTime)
        {
            output = mStartValue;     //Before step
        }
        else
        {
            output = mStartValue + mStepSize;     //After step
        }

        //Write new values to nodes
        mPortPtrs[out]->writeNode(NodeSignal::VALUE, output);
    }
};

#endif // SIGNALSTEP_HPP_INCLUDED
