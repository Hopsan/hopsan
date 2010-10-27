//!
//! @file   FirstOrderFilter.cc
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-23
//!
//! @brief Contains the Core First Order Filter class
//!
//$Id$

#include <iostream>
#include <cassert>
#include <math.h>
#include "../HopsanCore.h"
#include "FirstOrderFilter.h"

using namespace hopsan;

//! @class FirstOrderFilter
//! @brief The FirstOrderFilter class implements a first order filter using bilinear transform
//!
//! To declare a filter like \f[G=\frac{a_1 s + a_0}{b_1 s + b_0}\f]
//! the syntax is myFilter.setNumDen(num, den)
//! where \f$num=\{a_1, a_0\}\f$
//! and \f$den=\{b_1, b_0\}\f$
//!

FirstOrderFilter::FirstOrderFilter()
{
    mLastTime = -1.0;
    mIsInitialized = false;
}


void FirstOrderFilter::initialize(double &rTime, double timestep, double num[2], double den[2], double u0, double y0, double min, double max)
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


void FirstOrderFilter::setMinMax(double min, double max)
{
    mMin = min;
    mMax = max;
}


void FirstOrderFilter::setNumDen(double num[2], double den[2])
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


void FirstOrderFilter::initializeValues(double u0, double y0)
{
    mDelayU.initializeValues(u0);
    mDelayY.initializeValues(y0);
    mValue = y0;
}


void FirstOrderFilter::update(double &u)
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


double FirstOrderFilter::value(double &u)
{
    update(u);

    return mValue;
}


//! Observe that a call to this method has to be followed by another call to value(double u) or to update(double u)
//! @return The filtered actual value.
//! @see value(double u)
double FirstOrderFilter::value()
{
    double tmp = mDelayU.valueIdx(1);
    update(tmp);

    return mValue;
}









OptimizedFirstOrderFilter::OptimizedFirstOrderFilter()
{
    mLastTime = -1.0;
    mIsInitialized = false;
}


void OptimizedFirstOrderFilter::initialize(double &rTime, double timestep, double num[2], double den[2], double *uref, double *yref, double u0, double y0, double min, double max)
{
    mMin = min;
    mMax = max;
    mpU = uref;
    mpY = yref;
    mValue = y0;
    mDelayU = u0;
    mDelayY = std::max(std::min(y0, mMax), mMin);
    mTimeStep = timestep;
    mpTime = &rTime;
    mIsInitialized = true;
    mLastTime = -mTimeStep;

    setNumDen(num, den);
}


void OptimizedFirstOrderFilter::setMinMax(double min, double max)
{
    mMin = min;
    mMax = max;
}


void OptimizedFirstOrderFilter::setNumDen(double num[2], double den[2])
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


void OptimizedFirstOrderFilter::initializeValues(double u0, double y0)
{
    mDelayU = u0;
    mDelayY = y0;
    mValue = y0;
}


void OptimizedFirstOrderFilter::update()
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

        mDelayU = *mpU;
        mValue = 1.0/mCoeffY[1]*(mCoeffU[1] * *mpU + mCoeffU[0]*mDelayU - mCoeffY[0]*mDelayY);

        if (mValue > mMax)
        {
            mDelayY = mMax;
            mDelayU = mMax;
            mValue = mMax;
        }
        else if (mValue < mMin)
        {
            mDelayY = mMin;
            mDelayU = mMin;
            mValue = mMin;
        }
        else
        {
            mDelayY = mValue;
            mDelayU = *mpU;
        }

        mLastTime = *mpTime;
    }
}


void OptimizedFirstOrderFilter::filter()
{
    update();

    *mpY = mValue;
}











NoDelayFirstOrderFilter::NoDelayFirstOrderFilter()
{
    mLastTime = -1.0;
    mIsInitialized = false;
}


void NoDelayFirstOrderFilter::initialize(double &rTime, double timestep, double num[2], double den[2], double u0, double y0, double min, double max)
{
    mMin = min;
    mMax = max;
    mValue = y0;
    mDelayU = u0;
    mDelayY = std::max(std::min(y0, mMax), mMin);
    mTimeStep = timestep;
    mpTime = &rTime;
    mIsInitialized = true;
    mLastTime = -mTimeStep;

    setNumDen(num, den);
}


void NoDelayFirstOrderFilter::setMinMax(double min, double max)
{
    mMin = min;
    mMax = max;
}


void NoDelayFirstOrderFilter::setNumDen(double num[2], double den[2])
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


void NoDelayFirstOrderFilter::initializeValues(double u0, double y0)
{
    mDelayU = u0;
    mDelayY = y0;
    mValue = y0;
}


void NoDelayFirstOrderFilter::update(double &u)
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

        mDelayU = u;

        mValue = 1.0/mCoeffY[1]*(mCoeffU[1]*u + mCoeffU[0]*mDelayU - mCoeffY[0]*mDelayY);

        if (mValue > mMax)
        {
            mDelayY = mMax;
            mDelayU = mMax;
            mValue = mMax;
        }
        else if (mValue < mMin)
        {
            mDelayY = mMin;
            mDelayU = mMin;
            mValue = mMin;
        }
        else
        {
            mDelayY = mValue;
            mDelayU = u;
        }

        mLastTime = *mpTime;
    }
}


double NoDelayFirstOrderFilter::value(double &u)
{
    update(u);

    return mValue;
}


//! Observe that a call to this method has to be followed by another call to value(double u) or to update(double u)
//! @return The filtered actual value.
//! @see value(double u)
double NoDelayFirstOrderFilter::value()
{
    double tmp = mDelayU;
    update(tmp);

    return mValue;
}
