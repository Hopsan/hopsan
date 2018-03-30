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
//! @file   SecondOrderTransferFunction.cpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-23
//!
//! @brief Contains the Core Second Transfer Function class
//!
//$Id$

//#include <iostream>
//#include <cassert>
#include <algorithm>
#include "ComponentUtilities/SecondOrderTransferFunction.h"

using namespace hopsan;

//! @class hopsan::SecondOrderTransferFunction
//! @ingroup ComponentUtilityClasses
//! @brief The SecondOrderTransferFunction class implements a second order transfer function using bilinear transform
//!
//! To declare a filter like \f[G=\frac{a_2 s^2 + a_1 s + a_0}{b_2 s^2 + b_1 s + b_0}\f]
//! the syntax is filter.setNumDen(num, den)
//! where \f$num[0]=a_0\f$, \f$num[1]=a_1\f$, \f$num[2]=a_2\f$
//! and \f$den[0]=b_0\f$, \f$den[1]=b_1\f$, \f$den[2]=b_2\f$,
//!


//! @brief Constructor
//! @param timestep Time step
//! @param num Numerator
//! @param den Denominator
//! @param u0 Initial input value
//! @param y0 Initial output value
//! @param min Minimum limit of output value
//! @param max Maximum limit of output value
//! @param sy0 Initial derivative of output value
void SecondOrderTransferFunction::initialize(double timestep, double num[3], double den[3], double u0, double y0, double min, double max, double sy0)
{
    mIsSaturated = false;
    mMin = min;
    mMax = max;
    mDelayedU = u0;
    mDelayed2U = u0;
    mValue = y0;
    mDelayedY = std::max(std::min(y0, mMax), mMin);
    mDelayed2Y = mDelayedY-sy0*mTimeStep;
    mTimeStep = timestep;
    setNumDen(num, den);
    setBackupLength(1);
}


void SecondOrderTransferFunction::setNum(double num[3])
{
    mCoeffU[0] = num[0]*mTimeStep*mTimeStep + 2.0*num[1]*mTimeStep + 4.0*num[2];
    mCoeffU[1] = 2.0*num[0]*mTimeStep*mTimeStep - 8.0*num[2];
    mCoeffU[2] = num[0]*mTimeStep*mTimeStep - 2.0*num[1]*mTimeStep + 4.0*num[2];
}


void SecondOrderTransferFunction::setDen(double den[3])
{
    mCoeffY[0] = den[0]*mTimeStep*mTimeStep + 2.0*den[1]*mTimeStep + 4.0*den[2];
    mCoeffY[1] = 2.0*den[0]*mTimeStep*mTimeStep - 8.0*den[2];
    mCoeffY[2] = den[0]*mTimeStep*mTimeStep - 2.0*den[1]*mTimeStep + 4.0*den[2];
}


void SecondOrderTransferFunction::setNumDen(double num[3], double den[3])
{
    //num = (c*T^2*q^2 + 2*c*T^2*q + c*T^2 - 2*b*T*q^2 + 2*b*T + 4*a*q^2 - 8*a*q + 4*a)
    //den = (C*T^2*q^2 + 2*C*T^2*q + C*T^2 - 2*B*T*q^2 + 2*B*T + 4*A*q^2 - 8*A*q + 4*A)
    mCoeffU[0] = num[0]*mTimeStep*mTimeStep + 2.0*num[1]*mTimeStep + 4.0*num[2];
    mCoeffU[1] = 2.0*num[0]*mTimeStep*mTimeStep - 8.0*num[2];
    mCoeffU[2] = num[0]*mTimeStep*mTimeStep - 2.0*num[1]*mTimeStep + 4.0*num[2];

    mCoeffY[0] = den[0]*mTimeStep*mTimeStep + 2.0*den[1]*mTimeStep + 4.0*den[2];
    mCoeffY[1] = 2.0*den[0]*mTimeStep*mTimeStep - 8.0*den[2];
    mCoeffY[2] = den[0]*mTimeStep*mTimeStep - 2.0*den[1]*mTimeStep + 4.0*den[2];
}


void SecondOrderTransferFunction::setMinMax(double min, double max)
{
    mMin = min;
    mMax = max;
}

void SecondOrderTransferFunction::backup()
{
    mBackupU.update(mDelayed2U);
    mBackupU.update(mDelayedU);
    mBackupY.update(mDelayed2Y);
    mBackupY.update(mDelayedY);
}

void SecondOrderTransferFunction::restoreBackup(size_t nSteps)
{
    if (nSteps > 0)
    {
        nSteps -= 1;
    }
    mDelayedU = mBackupU.getIdx(0+nSteps*2);
    mDelayed2U = mBackupU.getIdx(1+nSteps*2);
    mDelayedY = mBackupY.getIdx(0+nSteps*2);
    mDelayed2Y = mBackupY.getIdx(1+nSteps*2);
}


void SecondOrderTransferFunction::initializeValues(double u0, double y0)
{
    mDelayedU = u0;
    mDelayed2U = u0;
    mDelayedY = y0;
    mDelayed2Y = y0;
    mValue = y0;
}

void SecondOrderTransferFunction::setBackupLength(size_t nStep)
{
    mBackupU.initialize(nStep*2, mDelayedU);
    mBackupY.initialize(nStep*2, mDelayedY);
}


double SecondOrderTransferFunction::update(double u)
{
//    std::cout << "u: " << u << " Value before: " << mY;
    mValue = 1.0/mCoeffY[0]*(mCoeffU[0]*u + mCoeffU[1]*mDelayedU + mCoeffU[2]*mDelayed2U - mCoeffY[1]*mDelayedY - mCoeffY[2]*mDelayed2Y);
//    std::cout << " Value after: " << mY << std::endl;

    //    if (mValue >= mMax)
    //    {
    //        mDelayed2U = mMax;
    //        mDelayedU = mMax;
    //        mDelayed2Y = mMax;
    //        mDelayedY = mMax;
    //        mValue     = mMax;
    //        mIsSaturated = true;
    //    }
    //    else if (mValue <= mMin)
    //    {
    //        mDelayed2U = mMin;
    //        mDelayedU = mMin;
    //        mDelayed2Y = mMin;
    //        mDelayedY = mMin;
    //        mValue     = mMin;
    //        mIsSaturated = true;
    //    }
    //    else
    //    {
    //        mDelayed2U = mDelayedU;
    //        mDelayedU = u;
    //        mDelayed2Y = mDelayedY;
    //        mDelayedY = mValue;
    //        mIsSaturated = false;
    //    }

    if (mValue >= mMax)
    {
        mValue = mMax;
        mIsSaturated = true;
    }
    else if (mValue <= mMin)
    {
        mValue = mMin;
        mIsSaturated = true;
    }
    else
    {
        mIsSaturated = false;
    }

    mDelayed2U = mDelayedU;
    mDelayedU  = u;
    mDelayed2Y = mDelayedY;
    mDelayedY  = mValue;

    return mValue;
}

double SecondOrderTransferFunction::updateWithBackup(double u)
{
    backup();
    return update(u);
}


//! Return current filter output value
//! @return The filtered actual value.
double SecondOrderTransferFunction::value() const
{
    return mValue;
}

double SecondOrderTransferFunction::delayedU() const
{
    return mDelayedU;
}

double SecondOrderTransferFunction::delayed2U() const
{
    return mDelayed2U;
}

double SecondOrderTransferFunction::delayedY() const
{
    return mDelayedY;
}

double SecondOrderTransferFunction::delayed2Y() const
{
    return mDelayed2Y;
}

//! @brief Check if the transfer function is saturated (har reached the set limits)
//! @returns true or false
bool SecondOrderTransferFunction::isSaturated() const
{
    return mIsSaturated;
}





void SecondOrderTransferFunctionVariable::initialize(double *pTimeStep, double num[3], double den[3], double u0, double y0, double min, double max)
{
    mMin = min;
    mMax = max;
    mDelayU[0] = u0;
    mDelayU[1] = u0;
    mValue = y0;
    mDelayY[0] = std::max(std::min(y0, mMax), mMin);
    mDelayY[1] = mDelayY[0];
    mpTimeStep = pTimeStep;
    mPrevTimeStep = (*pTimeStep);
    setNumDen(num, den);
}


void SecondOrderTransferFunctionVariable::setNum(double num[3])
{
    mNum[0] = num[0];
    mNum[1] = num[1];
    mNum[2] = num[2];
    recalculateCoefficients();
}


void SecondOrderTransferFunctionVariable::setDen(double den[3])
{
    mDen[0] = den[0];
    mDen[1] = den[1];
    mDen[2] = den[2];
    recalculateCoefficients();
}


void SecondOrderTransferFunctionVariable::setNumDen(double num[3], double den[3])
{
    mNum[0] = num[0];
    mNum[1] = num[1];
    mNum[2] = num[2];
    mDen[0] = den[0];
    mDen[1] = den[1];
    mDen[2] = den[2];
    recalculateCoefficients();
}


void SecondOrderTransferFunctionVariable::setMinMax(double min, double max)
{
    mMin = min;
    mMax = max;
}


void SecondOrderTransferFunctionVariable::initializeValues(double u0, double y0)
{
    mDelayU[0] = u0;
    mDelayU[1] = u0;
    mDelayY[0] = y0;
    mDelayY[1] = y0;
}


double SecondOrderTransferFunctionVariable::update(double u)
{
    if(mPrevTimeStep != (*mpTimeStep))
    {
        mPrevTimeStep = (*mpTimeStep);
        recalculateCoefficients();
    }

    mValue = 1.0/mCoeffY[0]*(mCoeffU[0]*u + mCoeffU[1]*mDelayU[0] + mCoeffU[2]*mDelayU[1] - (mCoeffY[1]*mDelayY[0] + mCoeffY[2]*mDelayY[1]));

    if (mValue > mMax)
    {
        mDelayU[1] = mMax;
        mDelayU[0] = mMax;
        mDelayY[1] = mMax;
        mDelayY[0] = mMax;
        mValue     = mMax;
    }
    else if (mValue < mMin)
    {
        mDelayU[1] = mMin;
        mDelayU[0] = mMin;
        mDelayY[1] = mMin;
        mDelayY[0] = mMin;
        mValue     = mMin;
    }
    else
    {
        mDelayU[1] = mDelayU[0];
        mDelayU[0] = u;
        mDelayY[1] = mDelayY[0];
        mDelayY[0] = mValue;
    }

    return mValue;
}


//! Return current filter output value
//! @return The filtered actual value.
double SecondOrderTransferFunctionVariable::value()
{
    return mValue;
}

void SecondOrderTransferFunctionVariable::recalculateCoefficients()
{
    mCoeffU[0] = mNum[0]*(*mpTimeStep)*(*mpTimeStep) + 2.0*mNum[1]*(*mpTimeStep) + 4.0*mNum[2];
    mCoeffU[1] = 2.0*mNum[0]*(*mpTimeStep)*(*mpTimeStep) - 8.0*mNum[2];
    mCoeffU[2] = mNum[0]*(*mpTimeStep)*(*mpTimeStep) - 2.0*mNum[1]*(*mpTimeStep) + 4.0*mNum[2];

    mCoeffY[0] = mDen[0]*(*mpTimeStep)*(*mpTimeStep) + 2.0*mDen[1]*(*mpTimeStep) + 4.0*mDen[2];
    mCoeffY[1] = 2.0*mDen[0]*(*mpTimeStep)*(*mpTimeStep) - 8.0*mDen[2];
    mCoeffY[2] = mDen[0]*(*mpTimeStep)*(*mpTimeStep) - 2.0*mDen[1]*(*mpTimeStep) + 4.0*mDen[2];
}
