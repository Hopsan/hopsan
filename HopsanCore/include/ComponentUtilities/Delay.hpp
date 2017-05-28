/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

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

//! @brief Delay template class, implementing a circular buffer containing values of specified type
//! @ingroup ComponentUtilityClasses
template<typename T>
class DelayTemplate
{
public:
    DelayTemplate()
    {
        mpArray = 0;
        mSize = 0;
    }

    ~DelayTemplate()
    {
        clear();
    }

    //! @brief Initialize delay buffer size based on timeDelay and timestep, Td/Ts must be multiple of 1 and >= 1
    //! @param [in] timeDelay The total time delay for a value to come out on the other side of the circle buffer
    //! @param [in] Ts The timestep between each call
    //! @param [in] initValue The initial value of all buffer elements
    void initialize(const double timeDelay, const double Ts, const T initValue)
    {
        //We let truncation round downwards, +0.5 to be sure we don't fall bellow integer value in float
        initialize( int(timeDelay/Ts+0.5), initValue);
    }

    //! @brief Initialize delay size based on known number of delay steps
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
    //! @return The oldest value in the delay buffer (After update, this value has been overwritten in the buffer)
    inline T update(const T newValue)
    {
        // First get the oldest value
        T oldestValue = mpArray[mOldest];

        // Increment the pointers, newest will after incrementation overwrite previous oldest
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

    //! @brief Get the oldest value in the buffer
    //! @return The oldest value in the buffer
    inline T getOldest() const
    {
        return mpArray[mOldest];
    }

    //! @brief Get the newest value in the buffer
    //! @return The newest value in the buffer
    inline T getNewest() const
    {
        return mpArray[mNewest];
    }

    //! @brief Returns a specific value, 0=newest, 1=nextnewest, 2=nextnextnewest and so on, no range check is performed
    //! @param [in] i Index of value to return
    //! @return Value at specified index
    inline T getIdx(const size_t i) const
    {
        //if ( (int(mNewest)-int(i)) < 0 )
        if ( i > mNewest )
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
    //! @return Value at specified index
    inline T getOldIdx(const size_t i) const
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

    //! @brief Get the size of the delay buffer (the number of buffer elements)
    //! @return The size of the delay buffer
    inline size_t getSize() const
    {
        return mSize;
    }

    //! @brief Clear the delay buffer, deleting all data
    void clear()
    {
        if (mpArray != 0)
        {
            delete[] mpArray;
            mpArray=0;
            mSize=0;
        }
    }


private:
    size_t mSize, mNewest, mOldest;
    T *mpArray;
};

//! @ingroup ComponentUtilityClasses
typedef DelayTemplate<double> Delay;

}

#endif
