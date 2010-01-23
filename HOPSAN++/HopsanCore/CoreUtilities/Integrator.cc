//!
//! @file   Integrator.cc
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-22
//!
//! @brief Contains a Integrator Utility
//!
//$Id$

#include <iostream>
#include <cassert>
#include "HopsanCore.h"
#include "Integrator.h"

Integrator::Integrator()
{
    mLastTime = 0.0;
    mIsInitialized = false;
}


void Integrator::initialize(double &rTime, double timestep, double u0, double y0)
{
    mDelayU.setStepDelay(1);
    mDelayY.setStepDelay(1);
    mDelayU.initialize(rTime, u0);
    mDelayY.initialize(rTime, y0);

    mTimeStep = timestep;
    mpTime = &rTime;
    mIsInitialized = true;
}


void Integrator::initializeValues(double u0, double y0)
{
    mDelayU.initializeValues(u0);
    mDelayY.initializeValues(y0);
}


void Integrator::update(double u)
{
    if (!mIsInitialized)
    {
        std::cout << "Integrator function has to be initialized" << std::endl;
        assert(false);
    }
    else if (mLastTime != *mpTime)
    {
        //Filter equation
        //Bilinear transform is used
		mDelayY.update(mDelayY.value() + mTimeStep/2.0*(u + mDelayU.value()));
		mDelayU.update(u);

        mLastTime = *mpTime;
    }
}


double Integrator::value(double u)
{
    update(u);

    return mDelayY.value();
}


double Integrator::value()
{
    update(mDelayU.value(1));

    return mDelayY.value();
}
