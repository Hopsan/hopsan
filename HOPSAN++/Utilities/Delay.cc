/*
 *  Delay.cc
 *  HOPSAN++
 *
 *  Created by Björn Eriksson on 2009-12-19.
 *  Copyright 2009 LiU. All rights reserved.
 *
 */

#include "Delay.h"


Delay::Delay(double &var, const std::size_t stepDelay)
{
    mVar = &var;
    mStepDelay = stepDelay;
    mValues.resize(mStepDelay+1, 0.0);
}


Delay::Delay(double &var, const double timeDelay, const double Ts)
{
    mVar = &var;
    mStepDelay = (std::size_t)timeDelay/Ts;
    mValues.resize(mStepDelay+1, 0.0);
}


void Delay::simulateOneTimestep()
{
    mValues.push_front(*mVar);
    mValues.pop_back();
}


void Delay::setStepDelay(const std::size_t stepDelay)
{
    mStepDelay = stepDelay;
    mValues.resize(mStepDelay+1, 0.0);
}


void Delay::setTimeDelay(const double timeDelay, const double Ts)
{
    mStepDelay = (std::size_t)timeDelay/Ts;
    mValues.resize(mStepDelay+1, 0.0);
}


double Delay::value()
{
    return mValues.back();
}


double Delay::value(const std::size_t idx)
{
    return mValues[idx];
}
