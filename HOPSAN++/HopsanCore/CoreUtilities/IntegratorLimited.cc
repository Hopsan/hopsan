//!
//! @file   IntegratorLimited.cc
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-22
//!
//! @brief Contains a Limited Integrator Utility
//!
//$Id$

#include <iostream>
#include <cassert>
#include <math.h>
#include "HopsanCore.h"
#include "IntegratorLimited.h"

//! @class IntegratorLimited
//! @brief The IntegratorLimited class integrates a variable with limited output signal and wind-up protection
//!
//! The class implements
//!  \f[y=\left\{\begin{array}{ll} y_{min} & \mbox{if } \int u \, dt + y_0 < y_{min} \\ y_{max} & \mbox{if } \int u \, dt + y_0 > y_{max} \\ \displaystyle{\int_{0}^{t} u \, dt + y_0} & \mbox{otherwise} \end{array} \right.\f]
//!

IntegratorLimited::IntegratorLimited()
{
    mLastTime = 0.0;
    mIsInitialized = false;
}


void IntegratorLimited::initialize(double &rTime, double timestep, double u0, double y0, double min, double max)
{
    mMin = min;
    mMax = max;
    mDelayU.setStepDelay(1);
    mDelayY.setStepDelay(1);
    mDelayU.initialize(rTime, u0);
    mDelayY.initialize(rTime, std::max(std::min(y0, mMax), mMin));

    mTimeStep = timestep;
    mpTime = &rTime;
    mIsInitialized = true;
}


void IntegratorLimited::initializeValues(double u0, double y0)
{
    mDelayU.initializeValues(u0);
    mDelayY.initializeValues(y0);
}


void IntegratorLimited::setMinMax(double min, double max)
{
    mMin = min;
    mMax = max;
}


void IntegratorLimited::update(double u)
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

        double y = mDelayY.value() + mTimeStep/2.0*(u + mDelayU.value());

        if (y >= mMax)
        {
            mDelayY.update(mMax);
        }
        else if (y <= mMin)
        {
            mDelayY.update(mMin);
        }
        else
        {
            mDelayY.update(y);
        }
        mDelayU.update(u);

        mLastTime = *mpTime;
    }
}


double IntegratorLimited::value(double u)
{
    update(u);

    return mDelayY.value();
}


double IntegratorLimited::value()
{
    update(mDelayU.valueIdx(1));

    return mDelayY.value();
}
