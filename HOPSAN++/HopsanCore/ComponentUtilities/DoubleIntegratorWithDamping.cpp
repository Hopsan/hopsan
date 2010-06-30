//!
//! @file   Integrator.cc
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-06-30
//!
//! @brief Contiains a second order integrator utility with provision for some damping
//!
//$Id$

#include <iostream>
#include <cassert>
#include "../HopsanCore.h"
#include "DoubleIntegratorWithDamping.h"

DoubleIntegratorWithDamping::DoubleIntegratorWithDamping()
{
    mLastTime = 0.0;
    mIsInitialized = false;
}


void DoubleIntegratorWithDamping::initialize(double &rTime, double timestep, double w0, double u0, double y0, double sy0)
{
    mW0 = w0;

    mDelayU.setStepDelay(1);
    mDelayY.setStepDelay(1);
    mDelaySY.setStepDelay(1);
    mDelayU.initialize(rTime, u0);
    mDelayY.initialize(rTime, y0);
    mDelaySY.initialize(rTime, sy0);

    mTimeStep = timestep;
    mpTime = &rTime;
    mIsInitialized = true;
}


void DoubleIntegratorWithDamping::initializeValues(double u0, double y0, double sy0)
{
    mDelayU.initializeValues(u0);
    mDelayY.initializeValues(y0);
    mDelaySY.initializeValues(sy0);
}


void DoubleIntegratorWithDamping::setDamping(double w0)
{
    mW0 = w0;
}


void DoubleIntegratorWithDamping::update(double u)
{
    if (!mIsInitialized)
    {
        std::cout << "Integrator function has to be initialized" << std::endl;
        assert(false);
    }
    else if (mLastTime != *mpTime)
    {
        double tempDelaySY = mDelaySY.value();
        mDelaySY.update( (2-mW0)/(2+mW0)*tempDelaySY + mTimeStep/(2.0+mW0)*(u + mDelayU.value()) );
        mDelayY.update( mDelayY.value() + mTimeStep/2.0*tempDelaySY );
        mDelayU.update(u);

        mLastTime = *mpTime;
    }
}


//! Returns first primitive from double integration
double DoubleIntegratorWithDamping::valueFirst(double u)
{
    update(u);
    return mDelaySY.value();
}


//! Returns second primitive from double integration
double DoubleIntegratorWithDamping::valueSecond(double u)
{
    update(u);
    return mDelayY.value();
}


//! Returns first primitive from double integration
double DoubleIntegratorWithDamping::valueFirst()
{
    update(mDelayU.valueIdx(1));
    return mDelaySY.value();
}


//! Returns second primitive from double integration
double DoubleIntegratorWithDamping::valueSecond()
{
    update(mDelayU.valueIdx(1));
    return mDelayY.value();
}
