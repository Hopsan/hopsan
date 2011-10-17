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
//! @file   Delay.hpppp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-12-01
//!
//! @brief Contains the Core Utility Delay Circle Buffer class
//!
//$Id$

#ifndef DELAY_HPP_INCLUDED
#define DELAY_HPP_INCLUDED

#include "../win32dll.h"
#include "assert.h"
#include "math.h"
#include <vector>

namespace hopsan {

    class DLLIMPORTEXPORT Delay
    {
    public:
        Delay()
        {
            mpArray = 0;
        }

        void initialize(const double timeDelay, const double Ts, const double init_value)
        {
            //mFracStep = timeDelay/Ts;
            //Calculate stepdelay, round double to int
            //! @todo mayby need to behave differently if we want to use fractions, (rount to ceeling before), however we cant do that allways as we get one extra step every time
            this->initialize((size_t)floor(timeDelay/Ts+0.5), init_value);
        }

        void initialize(const size_t delaySteps, const double init_value)
        {
            if (mpArray != 0)
            {
                delete  mpArray;
            }
            assert(delaySteps > 0);
            mSize = delaySteps+1;

            mpArray = new double[mSize];
            std::cout << "DelayBuffer, size: " << delaySteps << ", mSize: " << mSize << ", init_value: " << init_value << std::endl;

            //! @todo use c++ vector to init, or some smarter init
            for (size_t i=0; i<mSize; ++i)
            {
                mpArray[i] = init_value;
            }

            newest = 0;
            oldest = 1;
        }

        //! @todo This is a stupid temporary function
        void initializeValues(double init_value)
        {
            for (size_t i=0; i<mSize; ++i)
            {
                mpArray[i] = init_value;
            }
        }

        double update(const double new_value)
        {
            //!< @todo is this OK, we increment after we write
            double shifttmp = mpArray[oldest];
            mpArray[newest] = new_value;

            //Increment for next run
            //  newest = (newest + 1) % mSize;  //Bad performance
            //  oldest = (oldest + 1) % mSize;  //Bad performance

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

            return shifttmp;
        }

        double getOldest()
        {
            return mpArray[oldest];
        }

        double getNewest()
        {
            return mpArray[newest];
        }

        //! @brief Returns a specific value, 0=newest, 1=nextnewest, 2=nextnextnewest and so on, no range check is performed
        double getIdx(const size_t i)
        {
            if (((int)newest-(int)i)<0) //!< @todo is this right
            {
                return mpArray[newest-i+mSize];
            }
            else
            {
                return mpArray[newest-i];
            }
        }

        //! @brief Returns a specific value, 0=oldest, 1=nextoldest, 2=nextnextoldest and so on, no range check is performed
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

        size_t getSize()
        {
            return mSize-1;
        }

    private:
        size_t mSize;
        double *mpArray;
        //std::vector<double> mDataVector;
        size_t newest, oldest;
        //double mshifttmp;

    };
}

#endif
