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
}


void FirstOrderFilter::initialize(double timestep, double num[2], double den[2], double u0, double y0, double min, double max)
{
    mMin = min;
    mMax = max;
    mValue = y0;
    mDelayU = u0;
    mDelayY = std::max(std::min(y0, mMax), mMin);
    mTimeStep = timestep;
    setNumDen(num, den);
}


void FirstOrderFilter::setMinMax(double min, double max)
{
    mMin = min;
    mMax = max;
}


void FirstOrderFilter::setNum(double num[2])
{
    mCoeffU[0] = num[1]*mTimeStep-2.0*num[0];
    mCoeffU[1] = num[1]*mTimeStep+2.0*num[0];
}


void FirstOrderFilter::setDen(double den[2])
{
    mCoeffY[0] = den[1]*mTimeStep-2.0*den[0];
    mCoeffY[1] = den[1]*mTimeStep+2.0*den[0];
}


void FirstOrderFilter::setNumDen(double num[2], double den[2])
{
    mCoeffU[0] = num[1]*mTimeStep-2.0*num[0];
    mCoeffU[1] = num[1]*mTimeStep+2.0*num[0];

    mCoeffY[0] = den[1]*mTimeStep-2.0*den[0];
    mCoeffY[1] = den[1]*mTimeStep+2.0*den[0];
}


void FirstOrderFilter::initializeValues(double u0, double y0)
{
    mDelayU = u0;
    mDelayY = y0;
    mValue = y0;
}


double FirstOrderFilter::update(double &u)
{
    //Filter equation
    //Bilinear transform is used

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

    return mValue;
}


//! Observe that a call to this method has to be followed by another call to value(double u) or to update(double u)
//! @return The filtered actual value.
//! @see value(double u)
double &FirstOrderFilter::value()
{
    return mValue;
}
