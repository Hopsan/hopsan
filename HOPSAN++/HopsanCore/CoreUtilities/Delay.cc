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

//! @class Delay
//! @brief The Delay class delayes a variable in time (A \f$\mbox{\LaTeX}\f$ example: \f$e^{\pi}=-1\f$)
//!
//! Delay delayes a variable, the following example
//! delay a variable 5 timesteps:
//!
//! Delay delayVar;
//!
//! delayVar.initialize(time);
//!
//! delayVar.setStepDelay(5);
//!
//! In every loop a Delay instance has to be called with
//! an argument (a new value like "delayVar.value(newValue)"
//! and/or it can be updated trough the call "delayVar.update(newValue)".
//!

Delay::Delay()
{
    mStepDelay = 1;
    mInitialValue = 0.0;
    mValues.resize(mStepDelay+1, mInitialValue);
    mLastTime =-1.0;
    mIsInitialized = false;
}


Delay::Delay(const std::size_t stepDelay, const double initValue)
{
    mStepDelay = stepDelay;
    mFracStep = mStepDelay;
    mInitialValue = initValue;
    mValues.resize(mStepDelay+1, mInitialValue);
    mLastTime =-1.0;
    mIsInitialized = false;
}


Delay::Delay(const double timeDelay, const double Ts, const double initValue)
{
    mFracStep = timeDelay/Ts;
    //avrundar uppat
    mStepDelay = (std::size_t) ceil(((double) timeDelay)/Ts); ///TODO: kolla att det verkligen ar ratt
    mInitialValue = initValue;
    mValues.resize(mStepDelay+1, mInitialValue);
    mLastTime = -1.0;
    mIsInitialized = false;
}


void Delay::initialize(double &rTime, const double initValue)
{
    mInitialValue = initValue;
    mValues.assign(mValues.size(), mInitialValue);
    mLastTime = -1.0;
    mpTime = &rTime;
    mIsInitialized = true;
}


void Delay::initialize(double &rTime)
{
    initialize(rTime, mInitialValue);
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


void Delay::setStepDelay(const std::size_t stepDelay)
{
    setStepDelay(stepDelay, mInitialValue);
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


void Delay::setTimeDelay(const double timeDelay, const double Ts)
{
    setTimeDelay(timeDelay, Ts, mInitialValue);
}


double Delay::value()
//! Returns the oldest delayed value and update with the last value.
//! @see value(double value)
//! @see valueIdx(const int idx)
//! @see valueIdx(double value, const int idx)
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
//! Get the oldest delayed value and update with a new value.
//! @param[in] value is the new value of the delayed variable.
//! @return The delayed value of the Delay object.
//! @see value()
//! @see valueIdx(const int idx)
//! @see valueIdx(double value, const int idx)
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


double Delay::valueIdx(double value, const int idx)
//! Get the delayed value at a specified index and update with a new value.
//! \f[ [returnValue] = [delayedVariable] z^{-idx} \f]
//! @param value is the new value of the delayed variable.
//! @param idx tell which value to return, 1 is the last timestep's value 2 is the value from two timsteps ago and so on.
//! @return The value delayed idx time steps of the Delay object.
//! @see value()
//! @see value(double value)
//! @see valueIdx(const int idx)
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
//! Get the delayed value at a specified index.
//! @param idx tell which value to return, 1 is the last timestep's value 2 is the value from two timsteps ago and so on.
//! @return The value delayed idx time steps of the Delay object.
//! @see value(double value)
//! @see valueIdx(const int idx)
//! @see valueIdx(double value, const int idx)
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
