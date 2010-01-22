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


void Integrator::initializeValues(double u0, double y0, double timestep, double &rTime)
{
    mDelayU.initializeValues(u0, rTime);
    mDelayY.initializeValues(y0, rTime);

    mTimeStep = timestep;
    mpTime = &rTime;
    mIsInitialized = true;
}


void Integrator::update(double u, double  y)
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
		mDelayY.update(mDelayY.value(y) + mTimeStep/2.0*(u + mDelayU.value(u)));
		mDelayU.update(u);

        mLastTime = *mpTime;
    }
}


double Integrator::value(double u, double y)
{
    update(u, y);
    mDelayU.update(u);
    mDelayY.update(y);

    return mDelayY.value(y);
}
