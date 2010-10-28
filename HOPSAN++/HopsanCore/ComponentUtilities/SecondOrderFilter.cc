//!
//! @file   SecondOrderFilter.cc
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-23
//!
//! @brief Contains the Core Second Order Filter class
//!
//$Id$

#include <iostream>
#include <cassert>
#include <math.h>
#include "../HopsanCore.h"
#include "SecondOrderFilter.h"

using namespace hopsan;

//! @class SecondOrderFilter
//! @brief The SecondOrderFilter class implements a second order filter using bilinear transform
//!
//! To declare a filter like \f[G=\frac{a_2 s^2 + a_1 s + a_0}{b_2 s^2 + b_1 s + b_0}\f]
//! the syntax is myFilter.setNumDen(num, den)
//! where \f$num=\{a_2, a_1, a_0\}\f$
//! and \f$den=\{b_2, b_1, b_0\}\f$
//!

SecondOrderFilter::SecondOrderFilter()
{
    mLastTime = -1.0;
    mIsInitialized = false;
}


void SecondOrderFilter::initialize(double &rTime, double timestep, double num[3], double den[3], double u0, double y0, double min, double max)
{
    mMin = min;
    mMax = max;
    mDelayU.setStepDelay(2);
    mDelayY.setStepDelay(2);
    mDelayU.initialize(rTime, u0);
    mDelayY.initialize(rTime, std::max(std::min(y0, mMax), mMin));

    mTimeStep = timestep;
    mpTime = &rTime;
    mIsInitialized = true;

    setNumDen(num, den);
}


void SecondOrderFilter::setNumDen(double num[3], double den[3])
{
//num =
//(c*T^2*q^2 + 2*c*T^2*q + c*T^2 - 2*b*T*q^2 + 2*b*T + 4*a*q^2 - 8*a*q + 4*a)
//den =
//(C*T^2*q^2 + 2*C*T^2*q + C*T^2 - 2*B*T*q^2 + 2*B*T + 4*A*q^2 - 8*A*q + 4*A)

    mCoeffU[0] = num[2]*mTimeStep*mTimeStep - 2.0*num[1]*mTimeStep + 4.0*num[0];
    mCoeffU[1] = 2.0*num[2]*mTimeStep*mTimeStep - 8.0*num[0];
    mCoeffU[2] = num[2]*mTimeStep*mTimeStep + 2.0*num[1]*mTimeStep + 4.0*num[0];

    mCoeffY[0] = den[2]*mTimeStep*mTimeStep - 2.0*den[1]*mTimeStep + 4.0*den[0];
    mCoeffY[1] = 2.0*den[2]*mTimeStep*mTimeStep - 8.0*den[0];
    mCoeffY[2] = den[2]*mTimeStep*mTimeStep + 2.0*den[1]*mTimeStep + 4.0*den[0];

//    mCoeffU[0] = mTimeStep*mTimeStep*(num[2]*mTimeStep*mTimeStep - 2*num[1]*mTimeStep + 4*num[0]);
//    mCoeffU[1] = -mTimeStep*mTimeStep*(4*mTimeStep*num[1] - 4*mTimeStep*mTimeStep*num[2]);
//    mCoeffU[2] = -mTimeStep*mTimeStep*(8*num[0] - 6*mTimeStep*mTimeStep*num[2]);
//    mCoeffU[3] = mTimeStep*mTimeStep*(4*num[2]*mTimeStep*mTimeStep + 4*num[1]*mTimeStep);
//    mCoeffU[4] = mTimeStep*mTimeStep*(num[2]*mTimeStep*mTimeStep + 2*num[1]*mTimeStep + 4*num[0]); //To newest U
//
//    mCoeffY[0] = NoDelayAndPointersIntegrator*(den[2]*NoDelayAndPointersIntegrator - 2*den[1]*mTimeStep + 4*den[0]);
//    mCoeffY[1] = -NoDelayAndPointersIntegrator*(4*mTimeStep*den[1] - 4*NoDelayAndPointersIntegrator*den[2]);
//    mCoeffY[2] = -NoDelayAndPointersIntegrator*(8*den[0] - 6*NoDelayAndPointersIntegrator*den[2]);
//    mCoeffY[3] = NoDelayAndPointersIntegrator*(4*den[2]*NoDelayAndPointersIntegrator + 4*den[1]*mTimeStep);
//    mCoeffY[4] = NoDelayAndPointersIntegrator*(den[2]*NoDelayAndPointersIntegrator + 2*den[1]*mTimeStep + 4*den[0]);
}


void SecondOrderFilter::setMinMax(double min, double max)
{
    mMin = min;
    mMax = max;
}


void SecondOrderFilter::initializeValues(double u0, double y0)
{
    mDelayU.initializeValues(u0);
    mDelayY.initializeValues(y0);
}


void SecondOrderFilter::update(double u)
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

        mValue = 1.0/mCoeffY[2]*(mCoeffU[2]*u + mCoeffU[1]*mDelayU.valueIdx(u, 1) + mCoeffU[0]*mDelayU.value(u) - (mCoeffY[1]*mDelayY.valueIdx(1) + mCoeffY[0]*mDelayY.value()));

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


double SecondOrderFilter::value(double u)
{
    update(u);

    return mValue;

}


//! Observe that a call to this method has to be followed by another call to value(double u) or to update(double u)
//! @return The filtered actual value.
//! @see value(double u)
double SecondOrderFilter::value()
{
    update(mDelayU.valueIdx(1));

    return mValue;
}
