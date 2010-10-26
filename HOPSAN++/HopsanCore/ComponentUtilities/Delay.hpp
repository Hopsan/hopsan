//!
//! @file   Delay.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2009-12-19
//!
//! @brief Contains the Core Utility Delay class
//!
//$Id: Delay.h 1995 2010-10-20 10:59:29Z bjoer $

#ifndef DELAY_H_INCLUDED
#define DELAY_H_INCLUDED

#include <deque>
#include <assert.h>
//#include "../win32dll.h"

namespace hopsan {

    class Delay
    {
    public:
        Delay()
        {
            mStepDelay = 1;
            mInitialValue = 0.0;
            mValues.resize(mStepDelay+1, mInitialValue);
            mLastTime =-1.0;
            mIsInitialized = false;
        }

        Delay(const std::size_t stepDelay, const double initValue=0.0)
        {
            mStepDelay = stepDelay;
            mFracStep = mStepDelay;
            mInitialValue = initValue;
            mValues.resize(mStepDelay+1, mInitialValue);
            mLastTime =-1.0;
            mIsInitialized = false;
        }

        Delay(const double timeDelay, const double Ts, const double initValue=0.0)
        {
            mFracStep = timeDelay/Ts;
            //avrundar uppat
            mStepDelay = (std::size_t) ceil(((double) timeDelay)/Ts); //! @todo kolla att det verkligen ar ratt
            mInitialValue = initValue;
            mValues.resize(mStepDelay+1, mInitialValue);
            mLastTime = -1.0;
            mIsInitialized = false;
        }

        void initialize(double &rTime, const double initValue)
        {
            mInitialValue = initValue;
            mValues.assign(mValues.size(), mInitialValue);
            mLastTime = -1.0;
            mpTime = &rTime;
            mIsInitialized = true;
        }

        void initialize(double &rTime)
        {
            initialize(rTime, mInitialValue);
        }

        void initializeValues(const double initValue)
        {
            mInitialValue = initValue;
            mValues.assign(mValues.size(), mInitialValue);
        }

        void update(const double value)
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

        void setStepDelay(const std::size_t stepDelay)
        {
            setStepDelay(stepDelay, mInitialValue);
        }

        void setStepDelay(const std::size_t stepDelay, const double initValue)
        {
            mStepDelay = stepDelay;
            mFracStep = mStepDelay;
            if (initValue != 0)
            {
                mInitialValue = initValue;
            }
            mValues.resize(mStepDelay+1, mInitialValue);
        }


        void setTimeDelay(const double timeDelay, const double Ts)
        {
            setTimeDelay(timeDelay, Ts, mInitialValue);
        }

        void setTimeDelay(const double timeDelay, const double Ts, const double initValue)
        {
            mFracStep = timeDelay/Ts;
            //avrundar uppat
            mStepDelay = (std::size_t) ceil(((double) timeDelay)/Ts); //! @todo kolla att det verkligen ar ratt
            if (initValue != 0)
            {
                mInitialValue = initValue;
            }
            mValues.resize(mStepDelay+1, mInitialValue);
        }

    double value()
    {
        update(mValues.front());

        if (mValues.empty())
        {
            return mInitialValue;
        }
        else if ((mFracStep < mStepDelay) && (mValues.size() >= 2))
        {
            return ((1.0 - (mStepDelay - mFracStep)) * mValues[mValues.size()-2] + (mStepDelay - mFracStep) * mValues.back()); //interpolerar
        }
        else
        {
            return mValues.back();
        }

    }

    double value(double value)
    {
        update(value);
        if (mValues.empty())
        {
            return mInitialValue;
        }
        else if ((mFracStep < mStepDelay) && (mValues.size() >= 2))
        {
            return ((1.0 - (mStepDelay - mFracStep)) * mValues[mValues.size()-2] + (mStepDelay - mFracStep) * mValues.back()); //interpolerar
        }
        else
        {
            return mValues.back();
        }

    }

    double valueIdx(const int idx)
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

    double valueIdx(double value, const int idx)
    {
        update(value);
        if ((idx < 0) || ((size_t)idx > mValues.size()))
        {
            std::cout << "Indexed outside Delay-vector" << "  Index: " << idx << "  Length: " << mValues.size() << std::endl;
            assert(false);
        }
        else
        {
            return mValues[idx];
        }
    }
        //! @todo Implement void valueTime(double time); A function which returns delayed value of time
    private:
        double *mpTime;
        double mLastTime;
        double mInitialValue;
	double mFracStep;
	std::size_t mStepDelay;
	std::deque<double> mValues;
	bool mIsInitialized;
    };

}

#endif // DELAY_H_INCLUDED
