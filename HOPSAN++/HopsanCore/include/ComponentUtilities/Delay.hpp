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
//! @brief Contains the Core Utility Delay Circle Buffer class
//!
//$Id$

#ifndef DELAY_HPP_INCLUDED
#define DELAY_HPP_INCLUDED

#include "win32dll.h"
#include "assert.h"
#include <cmath>
#include <iostream>
//#include <vector>

namespace hopsan {

    class DLLIMPORTEXPORT Delay
    {
    public:
        Delay()
        {
            mpArray = 0;
        }

        ~Delay()
        {
            if (mpArray != 0)
            {
                delete mpArray;
            }
        }

        //! @brief Initialize Delay size based on timeDelay at a given timestep
        //! @param [in] timeDelay The total time delay for a value to come out on the other side of the circle buffer
        //! @param [in] Ts The timestep between each call
        //! @param [in] initValue The initial value of all buffer elements
        void initialize(const double timeDelay, const double Ts, const double initValue)
        {
            //mFracStep = timeDelay/Ts;
            //Calculate stepdelay, round double to int
            //! @todo mayby need to behave differently if we want to use fractions, (rount to ceeling before), however we cant do that allways as we get one extra step every time
            this->initialize( size_t(floor(timeDelay/Ts+0.5)), initValue);
        }

        //! @brief Initialize Delay size based on known number of delay steps
        //! @param [in] delaySteps The number of delay steps
        //! @param [in] initValue The initial value of all buffer elements
        void initialize(const size_t delaySteps, const double initValue)
        {
            if (mpArray != 0)
            {
                delete  mpArray;
            }
            assert(delaySteps > 0);
            mSize = delaySteps+1;

            mpArray = new double[mSize];
            std::cout << "DelayBuffer, size: " << delaySteps << ", mSize: " << mSize << ", init_value: " << initValue << std::endl;

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
        double update(const double newValue)
        {
            // First get the oldes value
            double oldestValue = mpArray[oldest];

            // Increment the pointers, newets will after incrementaion overwrite previous oldest
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
        double getOldest()
        {
            return mpArray[oldest];
        }

        //! @brief Get the newest value inte the buffer
        //! @return The newest value in the buffer
        double getNewest()
        {
            return mpArray[newest];
        }

        //! @brief Returns a specific value, 0=newest, 1=nextnewest, 2=nextnextnewest and so on, no range check is performed
        //! @param [in] i Index of value to return
        //! @return Value of specified index
        double getIdx(const size_t i)
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
        double getOldIdx(const size_t i)
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

        //! @brief Get the size of the ring buffer (the number of elements)
        //! @return The size of the buffer (the number of elements)
        size_t getSize() const
        {
            return mSize-1;
        }

    private:
        size_t mSize;
        double *mpArray;
        size_t newest, oldest;

    };
}

#endif
