//!
//! @file   Delay.cc
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2009-12-19
//!
//! @brief Contains the Core Utility Delay class
//!
//$Id$

#include <math.h>
#include <iostream>
#include <cassert>
#include "Delay.h"


Delay::Delay()
{
    mStepDelay = 1;
    mInitialValue = 0.0;
    mValues.resize(mStepDelay+1, mInitialValue);
    mLastTime =0.0;
    mIsInitialized = false;
}


Delay::Delay(const std::size_t stepDelay, const double initValue)
{
    mStepDelay = stepDelay;
    mFracStep = mStepDelay;
    mInitialValue = initValue;
    mValues.resize(mStepDelay+1, mInitialValue);
    mLastTime =0.0;
    mIsInitialized = false;
}


Delay::Delay(const double timeDelay, const double Ts, const double initValue)
{
    mFracStep = timeDelay/Ts;
    //avrundar uppat
    mStepDelay = (std::size_t) ceil(((double) timeDelay)/Ts); ///TODO: kolla att det verkligen ar ratt
    mInitialValue = initValue;
    mValues.resize(mStepDelay+1, mInitialValue);
    mLastTime =0.0;
    mIsInitialized = false;
}


void Delay::initialize(double &rTime, const double initValue)
{
    mInitialValue = initValue;
    mValues.assign(mValues.size(), mInitialValue);
    mLastTime =0.0;
    mpTime = &rTime;
    mIsInitialized = true;
}


void Delay::initializeValues(const double initValue)
{
    mInitialValue = initValue;
    mValues.assign(mValues.size(), mInitialValue);
}


void Delay::update(const double value)
{
    if (!mIsInitialized)
    {
        std::cout << "Delay function has to be initialized" << std::endl;
        assert(false);
    }
    else if (mLastTime != *mpTime)
    {
        mValues.push_front(value);
        mValues.pop_back();
        mLastTime = *mpTime;
    }
    else
    {
        mValues[0] = value;
    }
}


void Delay::setStepDelay(const std::size_t stepDelay, const double initValue)
{
    mStepDelay = stepDelay;
    mFracStep = mStepDelay;
    if (initValue != 0)
    {
        mInitialValue = initValue;
    }
    mValues.resize(mStepDelay+1, mInitialValue);
}


void Delay::setTimeDelay(const double timeDelay, const double Ts, const double initValue)
{
    mFracStep = timeDelay/Ts;
    //avrundar uppat
    mStepDelay = (std::size_t) ceil(((double) timeDelay)/Ts); ///TODO: kolla att det verkligen ar ratt
    if (initValue != 0)
    {
        mInitialValue = initValue;
    }
    mValues.resize(mStepDelay+1, mInitialValue);
}


double Delay::value()
{
    update(mValues.front());

    if (mValues.empty())
    {
        return mInitialValue;
    }
    else if ((mFracStep < mStepDelay) && (mValues.size() >= 2))
    {
        return ((1 - (mStepDelay - mFracStep)) * mValues[mValues.size()-2] + (mStepDelay - mFracStep) * mValues.back()); //interpolerar
    }
    else
    {
        return mValues.back();
    }

}


double Delay::value(double value)
{
    update(value);
    if (mValues.empty())
    {
        return mInitialValue;
    }
    else if ((mFracStep < mStepDelay) && (mValues.size() >= 2))
    {
        return ((1 - (mStepDelay - mFracStep)) * mValues[mValues.size()-2] + (mStepDelay - mFracStep) * mValues.back()); //interpolerar
    }
    else
    {
        return mValues.back();
    }

}


double Delay::valueIdx(double value, const int idx) ///TODO: interpolera värden
{
    update(value);
    if (((size_t)idx < 0) || ((size_t)idx > mValues.size()))
    {
        std::cout << "Indexed outside Delay-vector" << "  Index: " << idx << "  Length: " << mValues.size() << std::endl;
        assert(false);
    }
    else
    {
        return mValues[idx];
    }
}


double Delay::valueIdx(const int idx) ///TODO: interpolera värden
{
    update(mValues.front());
    if (((size_t)idx < 1) || ((size_t)idx > mValues.size()))
    {
        std::cout << "Indexed outside Delay-vector" << "  Index: " << idx << "  Delay vector index rage is " << "[1, " << mValues.size() << "]" << std::endl;
        assert(false);
    }
    else
    {
        return mValues[idx];
    }
}
