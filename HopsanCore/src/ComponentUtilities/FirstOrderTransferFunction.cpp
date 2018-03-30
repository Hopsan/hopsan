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
//! @file   FirstOrderTransferFunction.cpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-23
//!
//! @brief Contains the Core First Order Transfer Function class
//!
//$Id$

//#include <iostream>
#include <algorithm>
#include "ComponentUtilities/FirstOrderTransferFunction.h"

using namespace hopsan;

//! @class hopsan::FirstOrderTransferFunction
//! @ingroup ComponentUtilityClasses
//! @brief The FirstOrderTransferFunction class implements a first order time discrete transfer function using bilinear transform
//!
//! To declare a filter like \f[G=\frac{a_1 s + a_0}{b_1 s + b_0}\f]
//! the syntax is filter.setNumDen(num, den)
//! where \f$num[0]=a_0\f$, \f$num[1]=a_1\f$
//! and \f$den[0]=b_0\f$, \f$den[1]=b_1\f$
//!


void FirstOrderTransferFunction::initialize(double timestep, double num[2], double den[2], double u0, double y0, double min, double max)
{
    mIsSaturated = false;
    mMin = min;
    mMax = max;
    mValue = y0;
    mDelayedU = u0;
    mDelayedY = std::max(std::min(y0, mMax), mMin);
    mTimeStep = timestep;
    setNumDen(num, den);
    setBackupLength(1);
}


void FirstOrderTransferFunction::setMinMax(double min, double max)
{
    mMin = min;
    mMax = max;
}


void FirstOrderTransferFunction::setNum(double num[2])
{
    mCoeffU[0] = num[0]*mTimeStep-2.0*num[1];
    mCoeffU[1] = num[0]*mTimeStep+2.0*num[1];
    //std::cout << "DiscNum: " << mCoeffU[1] << " " << mCoeffU[0] << std::endl;

}


void FirstOrderTransferFunction::setDen(double den[2])
{
    mCoeffY[0] = den[0]*mTimeStep-2.0*den[1];
    mCoeffY[1] = den[0]*mTimeStep+2.0*den[1];
    //std::cout << "DiscDen: " << mCoeffY[1] << " " << mCoeffY[0] << std::endl;
}


void FirstOrderTransferFunction::setNumDen(double num[2], double den[2])
{
    mCoeffU[0] = num[0]*mTimeStep-2.0*num[1];
    mCoeffU[1] = num[0]*mTimeStep+2.0*num[1];

    mCoeffY[0] = den[0]*mTimeStep-2.0*den[1];
    mCoeffY[1] = den[0]*mTimeStep+2.0*den[1];
}

//! @brief Restore the backup at the given step
//! @param[in] nSteps The number of steps backwards in time to restore (1=last step) must be >=1
//! @note The function assumes that the backup buffer has been allocated
//! @see setBackupLength
void FirstOrderTransferFunction::restoreBackup(size_t nSteps)
{
    if (nSteps > 0)
    {
        nSteps -= 1;
    }
    mDelayedU = mBackupU.getIdx(nSteps);
    mDelayedY = mBackupY.getIdx(nSteps);
}

//! @brief Pushes a backup of transfer function states into the backup buffer
//! @note Only the delayed states are backed up, not the current value or the coefficients
//! @todo Maybe we should backup more things like coefficients, saturated flag, current value, but that will take time at every timestep
void FirstOrderTransferFunction::backup()
{
    mBackupU.update(mDelayedU);
    mBackupY.update(mDelayedY);
}


void FirstOrderTransferFunction::initializeValues(double u0, double y0)
{
    mDelayedU = u0;
    mDelayedY = y0;
    mValue = y0;
}

//! @brief Setup the number of backup steps to remember (size of the backup buffer)
//! @param[in] nSteps The number of steps to remember
void FirstOrderTransferFunction::setBackupLength(size_t nSteps)
{
    mBackupU.initialize(nSteps, mDelayedU);
    mBackupY.initialize(nSteps, mDelayedY);
}


//! @brief Updates the transfer function
//! @param[in] u The new input value
//! @returns The current transfer function output value after update
double FirstOrderTransferFunction::update(double u)
{
    //Filter equation
    //Bilinear transform is used

    mValue = 1.0/mCoeffY[1]*(mCoeffU[1]*u + mCoeffU[0]*mDelayedU - mCoeffY[0]*mDelayedY);

//    if (mValue >= mMax)
//    {
//        mDelayY = mMax;
//        mDelayU = mMax;
//        mValue = mMax;
//        mIsSaturated = true;
//    }
//    else if (mValue <= mMin)
//    {
//        mDelayY = mMin;
//        mDelayU = mMin;
//        mValue = mMin;
//        mIsSaturated = true;
//    }
//    else
//    {
//        mDelayY = mValue;
//        mDelayU = u;
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

    mDelayedY = mValue;
    mDelayedU = u;

    return mValue;
}

//! @brief Make a backup of states and then calls update
//! @param[in] u The new input value
//! @returns The current transfer function output value after update
double FirstOrderTransferFunction::updateWithBackup(double u)
{
    backup();
    return update(u);
}


//! @brief Read current transfer function output value
//! @return The filtered actual value.
double FirstOrderTransferFunction::value() const
{
    return mValue;
}

double FirstOrderTransferFunction::delayedU() const
{
    return mDelayedU;
}

double FirstOrderTransferFunction::delayedY() const
{
    return mDelayedY;
}

//! @brief Check if the transfer function is saturated (has reached the set limits)
//! @returns true or false
bool FirstOrderTransferFunction::isSaturated() const
{
    return mIsSaturated;
}








void FirstOrderTransferFunctionVariable::initialize(double *pTimestep, double num[2], double den[2], double u0, double y0, double min, double max)
{
    mMin = min;
    mMax = max;
    mValue = y0;
    mDelayU = u0;
    mDelayY = std::max(std::min(y0, mMax), mMin);
    mpTimeStep = pTimestep;
    mPrevTimeStep = *pTimestep;
    mNum[0] = num[0];
    mNum[1] = num[1];
    mDen[0] = den[0];
    mDen[1] = den[1];

    recalculateCoefficients();
}


void FirstOrderTransferFunctionVariable::setMinMax(double min, double max)
{
    mMin = min;
    mMax = max;
}


void FirstOrderTransferFunctionVariable::setNum(double num[2])
{
    mNum[0] = num[0];
    mNum[1] = num[1];
    recalculateCoefficients();
}


void FirstOrderTransferFunctionVariable::setDen(double den[2])
{
    mDen[0] = den[0];
    mDen[1] = den[1];
    recalculateCoefficients();
}


void FirstOrderTransferFunctionVariable::setNumDen(double num[2], double den[2])
{
    mNum[0] = num[0];
    mNum[1] = num[1];
    mDen[0] = den[0];
    mDen[1] = den[1];
    recalculateCoefficients();
}

void FirstOrderTransferFunctionVariable::recalculateCoefficients()
{
    mCoeffU[0] = mNum[0]*(*mpTimeStep)-2.0*mNum[1];
    mCoeffU[1] = mNum[0]*(*mpTimeStep)+2.0*mNum[1];

    mCoeffY[0] = mDen[0]*(*mpTimeStep)-2.0*mDen[1];
    mCoeffY[1] = mDen[0]*(*mpTimeStep)+2.0*mDen[1];
}


void FirstOrderTransferFunctionVariable::initializeValues(double u0, double y0)
{
    mDelayU = u0;
    mDelayY = y0;
    mValue = y0;
}


double FirstOrderTransferFunctionVariable::update(double u)
{
    //Filter equation
    //Bilinear transform is used

    if((*mpTimeStep) != mPrevTimeStep)
    {
        mPrevTimeStep = (*mpTimeStep);
        recalculateCoefficients();
    }

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


//! @brief Read current filter output value
//! @return The filtered actual value.
double FirstOrderTransferFunctionVariable::value()
{
    return mValue;
}


//! @class hopsan::FirstOrderLowPassFilter
//! @ingroup ComponentUtilityClasses
//! @brief The FirstOrderLowpassFilter utility is derived from the FirstOrderTransferFunction and extends it with functions useful when creating low-pass filters of the first order


//! @brief Initialize the filter utility
//! @param [in] timestep The (fixed) simulation timestep used
//! @param [in] wc The break frequency in rad/s
//! @param [in] timestep The (fixed) simulation timestep used
//! @param [in] u0 Initial input signal
//! @param [in] y0 Initial output value
//! @param [in] min Filter minimum value (saturation)
//! @param [in] max Filter maximum value (saturation)
void FirstOrderLowPassFilter::initialize(double timestep, double wc, double u0, double y0, double min, double max)
{
    // G = 1 / ( 1/wc + 1 )
    double num[2], den[2];
    num[1] = 0.0;    num[0] = 1.0;
    den[1] = 1.0/wc; den[0] = 1.0;
    FirstOrderTransferFunction::initialize(timestep, num, den, u0, y0, min, max);
}

//! @brief Return the break frequency for this filter
double FirstOrderLowPassFilter::breakFrequency() const
{
    return 4.0/(mCoeffY[1]-mCoeffY[0]);
}
