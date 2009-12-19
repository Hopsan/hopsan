/*
 *  Delay.cc
 *  HOPSAN++
 *
 *  Created by Björn Eriksson on 2009-12-19.
 *  Copyright 2009 LiU. All rights reserved.
 *
 */

#include "Delay.h"


Delay::Delay()
{
}


Delay::Delay(const std::size_t stepDelay)
{
    mStepDelay = stepDelay;
    mValues.resize(mStepDelay, 0.0);
}


Delay::Delay(const double timeDelay, const double Ts)
{
    mStepDelay = (std::size_t)timeDelay/Ts; ///TODO: fixa divisionen med avrundningen
    mValues.resize(mStepDelay, 0.0);
}


void Delay::update(const double value)
{
    mValues.push_front(value);
    mValues.pop_back();
}


void Delay::setStepDelay(const std::size_t stepDelay)
{
    mStepDelay = stepDelay;
    mValues.resize(mStepDelay, 0.0);
}


void Delay::setTimeDelay(const double timeDelay, const double Ts)
{
    mStepDelay = (std::size_t)timeDelay/Ts;
    mValues.resize(mStepDelay, 0.0);
}


double Delay::value() ///TODO: interpolera värden
{
    if (mValues.empty()) 
    {
        return 0;
    }
    else 
    {
        return mValues.back();
    }
}


double Delay::value(const std::size_t idx) ///TODO: interpolera värden
{
    return mValues[idx];
}
