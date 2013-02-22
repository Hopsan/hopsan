/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   Delay.hpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-12-01
//!
//! @brief Contains the Component Utility Delay circle buffer class
//!
//$Id$

#ifndef DELAY_HPP_INCLUDED
#define DELAY_HPP_INCLUDED

#include "stddef.h"

namespace hopsan {

//! @brief The Old Delay template class, implementing a circle buffer
//! @ingroup ComponentUtilityClasses
//! @deprecated
template<typename T>
class Delay_old
{
public:
    Delay_old()
    {
        mpArray = 0;
    }

    ~Delay_old()
    {
        if (mpArray != 0)
        {
            delete mpArray;
        }
    }

    //! @brief Initialize Delay buffer size based on timeDelay and timestep, Td/Ts must be multiple of 1
    //! @param [in] timeDelay The total time delay for a value to come out on the other side of the circle buffer
    //! @param [in] Ts The timestep between each call
    //! @param [in] initValue The initial value of all buffer elements
    void initialize(const double timeDelay, const double Ts, const T initValue)
    {
        //mFracStep = timeDelay/Ts;
        //Calculate stepdelay, round double to int
        //! @todo mayby need to behave differently if we want to use fractions, (rount to ceeling before), however we cant do that allways as we get one extra step every time
        //We let truncation round downwards, +0.5 to be sure we dont fall bellow integer value in float
        //! @todo should we have -1 step also so 1sec delay at 10Hz would be 9 delay slots, (right now 10 slots)
        this->initialize( size_t(timeDelay/Ts+0.5), initValue);
    }

    //! @brief Initialize Delay size based on known number of delay steps
    //! @param [in] delaySteps The number of delay steps, must be >= 1
    //! @param [in] initValue The initial value of all buffer elements
    void initialize(const size_t delaySteps, const T initValue)
    {
        if (mpArray != 0)
        {
            delete  mpArray;
        }

        mSize = delaySteps+1;

        // Make sure we will not crash if someone entered 0 delaysteps
        if (mSize < 2)
        {
            mSize = 2;
        }

        mpArray = new T[mSize];

        //! @todo maybe use c++ vector to init, or some smarter init, does not really matter I think
        for (size_t i=0; i<mSize; ++i)
        {
            mpArray[i] = initValue;
        }

        newest = 0;
        oldest = 1;
    }

    //! @brief updates delay with a new value
    //! @param [in] newValue The new value to insert into delay buffer
    //! @return The odlest value of the delay buffer (After update this value will have been overwriten in the buffer)
    T update(const T newValue)
    {
        // First get the oldes value
        T oldestValue = mpArray[oldest];

        // Increment the pointers, newest will after incrementaion overwrite previous oldest
        ++newest;
        ++oldest;

        if (oldest >= mSize)
        {
            oldest = 0;
        }
        if (newest >= mSize)
        {
            newest = 0;
        }

        // Overwrite previous oldest with the new_value
        mpArray[newest] = newValue;

        return oldestValue;
    }

    //! @brief Get the oldest value inte the buffer
    //! @return The oldest value in the buffer
    T getOldest()
    {
        return mpArray[oldest];
    }

    //! @brief Get the newest value inte the buffer
    //! @return The newest value in the buffer
    T getNewest()
    {
        return mpArray[newest];
    }

    //! @brief Returns a specific value, 0=newest, 1=nextnewest, 2=nextnextnewest and so on, no range check is performed
    //! @param [in] i Index of value to return
    //! @return Value of specified index
    T getIdx(const size_t i)
    {
        if ( (int(newest)-int(i)) < 0 )
        {
            return mpArray[mSize+newest-i];
        }
        else
        {
            return mpArray[newest-i];
        }
    }

    //! @brief Returns a specific value, 0=oldest, 1=nextoldest, 2=nextnextoldest and so on, no range check is performed
    //! @param [in] i Index of value to return
    //! @return Value of specified index
    T getOldIdx(const size_t i)
    {
        if (oldest+i >= mSize)
        {
            return mpArray[oldest+i-mSize];
        }
        else
        {
            return mpArray[oldest+i];
        }
    }

    //! @brief Get the size of the delay (the number of delaySteps)
    //! @return The size of the delay (the number of delaySteps)
    size_t getSize() const
    {
        return mSize-1;
    }

private:
    size_t mSize, newest, oldest;
    T *mpArray;
};

//! @brief Delay template class, implementing a circle buffer
//! @ingroup ComponentUtilityClasses
template<typename T>
class Delay_
{
public:
    Delay_()
    {
        mpArray = 0;
        mSize = 0;
    }

    ~Delay_()
    {
        clear();
    }

    //! @brief Initialize Delay buffer size based on timeDelay and timestep, Td/Ts must be multiple of 1 and >= 1
    //! @param [in] timeDelay The total time delay for a value to come out on the other side of the circle buffer
    //! @param [in] Ts The timestep between each call
    //! @param [in] initValue The initial value of all buffer elements
    void initialize(const double timeDelay, const double Ts, const T initValue)
    {
        //We let truncation round downwards, +0.5 to be sure we dont fall bellow integer value in float
        this->initialize( int(timeDelay/Ts+0.5), initValue);
    }

    //! @brief Initialize Delay size based on known number of delay steps
    //! @param [in] delaySteps The number of delay steps, must be >= 1
    //! @param [in] initValue The initial value of all buffer elements
    void initialize(const int delaySteps, const T initValue)
    {
        // First clear old data
        clear();

        // Make sure we will not crash if someone entered < 1 delaysteps
        if (delaySteps < 1)
        {
            mSize = 1;
        }
        else
        {
            mSize = size_t(delaySteps);
        }

        mpArray = new T[mSize];
        for (size_t i=0; i<mSize; ++i)
        {
            mpArray[i] = initValue;
        }

        mOldest = 0;
        mNewest = mSize-1;
    }

    //! @brief Updates delay with a new value, "pop old", "push new". You should likely run this at the end of each time step
    //! @param [in] newValue The new value to insert into delay buffer
    //! @return The odlest value in the delay buffer (After update this value will have been overwriten in the buffer)
    T update(const T newValue)
    {
        // First get the oldes value
        T oldestValue = mpArray[mOldest];

        // Increment the pointers, newest will after incrementaion overwrite previous oldest
        ++mOldest;
        ++mNewest;

        if (mOldest >= mSize)
        {
            mOldest = 0;
        }
        if (mNewest >= mSize)
        {
            mNewest = 0;
        }

        // Overwrite previous oldest with the new_value
        mpArray[mNewest] = newValue;

        return oldestValue;
    }

    //! @brief Get the oldest value inte the buffer
    //! @return The oldest value in the buffer
    T getOldest() const
    {
        return mpArray[mOldest];
    }

    //! @brief Get the newest value inte the buffer
    //! @return The newest value in the buffer
    T getNewest() const
    {
        return mpArray[mNewest];
    }

    //! @brief Returns a specific value, 0=newest, 1=nextnewest, 2=nextnextnewest and so on, no range check is performed
    //! @param [in] i Index of value to return
    //! @return Value of specified index
    T getIdx(const size_t i) const
    {
        if ( (int(mNewest)-int(i)) < 0 )
        {
            return mpArray[mSize+mNewest-i];
        }
        else
        {
            return mpArray[mNewest-i];
        }
    }

    //! @brief Returns a specific value, 0=oldest, 1=nextoldest, 2=nextnextoldest and so on, no range check is performed
    //! @param [in] i Index of value to return
    //! @return Value of specified index
    T getOldIdx(const size_t i) const
    {
        if (mOldest+i >= mSize)
        {
            return mpArray[mOldest+i-mSize];
        }
        else
        {
            return mpArray[mOldest+i];
        }
    }

    //! @brief Get the size of the delay (the number of delaySteps)
    //! @return The size of the delay (the number of delaySteps)
    size_t getSize() const
    {
        return mSize;
    }

    //! @brief Clear the delay buffer, deleting all data
    void clear()
    {
        if (mpArray != 0)
        {
            delete mpArray;
            mpArray=0;
            mSize=0;
        }
    }


private:
    size_t mSize, mNewest, mOldest;
    T *mpArray;
};

//! @ingroup ComponentUtilityClasses
typedef Delay_<double> Delay; //!< @todo My we should only have template class and let users choose data type themselfs
typedef Delay_old<double> DelayOld;

}

#endif
