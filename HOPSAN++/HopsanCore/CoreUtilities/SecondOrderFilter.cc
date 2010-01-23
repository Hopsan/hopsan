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
#include "HopsanCore.h"
#include "SecondOrderFilter.h"

SecondOrderFilter::SecondOrderFilter()
{
    mLastTime = 0.0;
    mIsInitialized = false;
}


void SecondOrderFilter::initialize(double &rTime, double timestep, double num[3], double den[3], double u0, double y0, double min, double max)
{
    mMin = min;
    mMax = max;
    mDelayU.setStepDelay(4);
    mDelayY.setStepDelay(4);
    mDelayU.initialize(rTime, u0);
    mDelayY.initialize(rTime, std::max(std::min(y0, mMax), mMin));

    mTimeStep = timestep;
    mpTime = &rTime;
    mIsInitialized = true;

    setNumDen(num, den);
}


void SecondOrderFilter::setNumDen(double num[3], double den[3])
{
    mCoeffU[0] = pow(mTimeStep, 2)*(num[2]*pow(mTimeStep, 2) + 2*num[1]*mTimeStep + 4*num[0]); //To newest U
    mCoeffU[1] = pow(mTimeStep, 2)*(4*num[2]*pow(mTimeStep, 2) + 4*num[1]*mTimeStep);
    mCoeffU[2] = -pow(mTimeStep, 2)*(8*num[0] - 6*pow(mTimeStep, 2)*num[2]);
    mCoeffU[3] = -pow(mTimeStep, 2)*(4*mTimeStep*num[1] - 4*pow(mTimeStep, 2)*num[2]);
    mCoeffU[4] = pow(mTimeStep, 2)*(num[2]*pow(mTimeStep, 2) - 2*num[1]*mTimeStep + 4*num[0]);
cout << "U0 " << mCoeffU[0] << endl;
cout << "U1 " << mCoeffU[1] << endl;
cout << "U2 " << mCoeffU[2] << endl;
cout << "U3 " << mCoeffU[3] << endl;
cout << "U4 " << mCoeffU[4] << endl;

    mCoeffY[0] = pow(mTimeStep, 2.0)*(den[2]*pow(mTimeStep, 2.0) + 2*den[1]*mTimeStep + 4*den[0]);
    mCoeffY[1] = pow(mTimeStep, 2.0)*(4*den[2]*pow(mTimeStep, 2.0) + 4*den[1]*mTimeStep);
    mCoeffY[2] = -pow(mTimeStep, 2.0)*(8*den[0] - 6*pow(mTimeStep, 2.0)*den[2]);
    mCoeffY[3] = -pow(mTimeStep, 2.0)*(4*mTimeStep*den[1] - 4*pow(mTimeStep, 2.0)*den[2]);
    mCoeffY[4] = pow(mTimeStep, 2.0)*(den[2]*pow(mTimeStep, 2.0) - 2*den[1]*mTimeStep + 4*den[0]);
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

        double y = 1.0/mCoeffY[0]*(mCoeffU[0]*u + mCoeffU[1]*mDelayU.valueIdx(u, 1) + mCoeffU[2]*mDelayU.valueIdx(u, 2) + mCoeffU[3]*mDelayU.valueIdx(u, 3) + mCoeffU[4]*mDelayU.valueIdx(u, 4) - (mCoeffY[1]*mDelayY.valueIdx(1)+ mCoeffY[2]*mDelayY.valueIdx(2)+ mCoeffY[3]*mDelayY.valueIdx(3)+ mCoeffY[4]*mDelayY.valueIdx(4)));

        if (y >= mMax)
        {
            mDelayY.update(mMax);
        }
        else if (y <= mMin)
        {
            mDelayY.update(mMin);
        }
        else
        {
            mDelayY.update(y);
        }
        mDelayU.update(u);

        mLastTime = *mpTime;
    }
}


double SecondOrderFilter::value(double u)
{
    update(u);

    return mDelayY.value();
}


double SecondOrderFilter::value()
{
    update(mDelayU.value(1));

    return mDelayY.value();
}
