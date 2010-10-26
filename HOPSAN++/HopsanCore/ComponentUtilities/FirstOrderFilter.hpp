//!
//! @file   FirstOrderFilter.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2009-12-23
//!
//! @brief Contains the Core First Order Filter class
//!
//$Id: FirstOrderFilter.h 1642 2010-08-09 12:54:33Z bjoer $

#ifndef FIRSTORDERFILTER_HPP_INCLUDED
#define FIRSTORDERFILTER_HPP_INCLUDED

#include <deque>
//#include "../win32dll.h"
#include "Delay.hpp"

namespace hopsan {

    /*
            num[0]*s + num[1]
    G = -------------------------
            den[0]*s + den[1]
*/

    class FirstOrderFilter
    {
    public:

        FirstOrderFilter()
        {
            mLastTime = -1.0;
            mIsInitialized = false;
        }

        void initialize(double &rTime, double timestep, double num[2], double den[2], double u0=0.0, double y0=0.0, double min=-1.5E+300, double max=1.5E+300)
        {
            mMin = min;
            mMax = max;
            mValue = y0;
            mDelayU.setStepDelay(1);
            mDelayY.setStepDelay(1);
            mDelayU.initialize(rTime, u0);
            mDelayY.initialize(rTime, std::max(std::min(y0, mMax), mMin));
            mTimeStep = timestep;
            mpTime = &rTime;
            mIsInitialized = true;
            mLastTime = -mTimeStep;

            setNumDen(num, den);
        }

        void initializeValues(double u0, double y0)
        {
            mDelayU.initializeValues(u0);
            mDelayY.initializeValues(y0);
            mValue = y0;
        }

        void setMinMax(double min, double max)
        {
            mMin = min;
            mMax = max;
        }

        void setNumDen(double num[2], double den[2])
        {
        //num =
        //(T + T*q)*(2*a + T*b - 2*a*q + T*b*q)
        //den =
        //(T + T*q)*(2*A - 2*A*q + B*T + B*T*q)

            mCoeffU[0] = num[1]*mTimeStep-2.0*num[0];
            mCoeffU[1] = num[1]*mTimeStep+2.0*num[0];

            mCoeffY[0] = den[1]*mTimeStep-2.0*den[0];
            mCoeffY[1] = den[1]*mTimeStep+2.0*den[0];
        }

        void update(double u)
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

                mValue = 1.0/mCoeffY[1]*(mCoeffU[1]*u + mCoeffU[0]*mDelayU.value(u) - mCoeffY[0]*mDelayY.value());
        //cout << "FILTER: " << "  u: " << u << "  y: " << mValue << endl;

                if (mValue > mMax)
                {
                    mDelayY.initializeValues(mMax);
                    mDelayU.initializeValues(mMax);
                    mValue = mMax;
                }
                else if (mValue < mMin)
                {
                    mDelayY.initializeValues(mMin);
                    mDelayU.initializeValues(mMin);
                    mValue = mMin;
                }
                else
                {
                    mDelayY.update(mValue);
                    mDelayU.update(u);
                }

                mLastTime = *mpTime;
            }
        }

    double value(double u)
    {
        update(u);

        return mValue;
    }

    double value()
    {
        update(mDelayU.valueIdx(1));

        return mValue;
    }

    private:
        double mValue;
        Delay mDelayU, mDelayY;
        double mCoeffU[2];
        double mCoeffY[2];
        double mMin, mMax;
        double mTimeStep;
        double *mpTime;
        double mLastTime;
        bool mIsInitialized;
    };

}
#endif // FIRSTORDERFILTER_H_INCLUDED
