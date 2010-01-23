#include <math.h>
#include <iostream>
#include <cassert>
#include "TransferFunction.h"


TransferFunction::TransferFunction()
{
}

TransferFunction::TransferFunction(double num [3], double den [3], double timestep)
{
    for(int i=0; i<3; i++)
    {
      mnum[i] = num[i];
      mden[i] = den[i];
    }
    mTimestep = timestep;
    mLastTime = 0.0;
    mDelayu.setStepDelay(2);
    mDelayy.setStepDelay(2);
    mIsInitialized = false;
}

void TransferFunction::update(double signal)
{
    if (!mIsInitialized)
    {
        std::cout << "TransferFunction has to be initialized" << std::endl;
        assert(false);
    }

    else if (mLastTime != *mpTime)
    {

      u0 = signal;
      u1 = mDelayu.valueIdx(u0, 1); // Inc. idx +1
      u2 = mDelayu.valueIdx(u0, 2); // Inc. idx +1

      y2 = mDelayy.valueIdx(y0, 2); // Inc. idx +1
      y1 = mDelayy.valueIdx(y0, 1); // Inc. idx +1

      b[0] = 4.0*mnum[2]+mnum[0]*pow(mTimestep,2.0)+2.0*mnum[1]*mTimestep;
      b[1] = -8.0*mnum[2]+2*mnum[0]*pow(mTimestep,2.0);
      b[2] = -2.0*mnum[1]*mTimestep+4.0*mnum[2]+mnum[0]*pow(mTimestep,2.0);

      a[0] = 4.0*mden[2]+mden[0]*pow(mTimestep,2.0)+2.0*mden[1]*mTimestep;
      a[1] = -8.0*mden[2]+2.0*mden[0]*pow(mTimestep,2.0);
      a[2] = -2.0*mden[1]*mTimestep+4.0*mden[2]+mden[0]*pow(mTimestep,2.0);

        /* Equation:
                bo+b1q^-1+b2q^-2
          y=  -------------------- u
                a0+a1q^-1+a2q^-2
        */

        y0 = (b[0]*u0+b[1]*u1+b[2]*u2-a[1]*y1-a[2]*y2)/a[0];

        mLastTime = *mpTime;
    }
}

void TransferFunction::setCoefficients(double num [3], double den [3], double timestep)
{
    for(int i=0; i<3; i++)
    {
      mnum[i] = num[i];
      mden[i] = den[i];
    }
    mTimestep = timestep;
    mDelayu.setStepDelay(2); // Added
    mDelayy.setStepDelay(2); // Added

}

double TransferFunction::getValue(double value)
{
    update(value);
    return y0;
}

void TransferFunction::initialize(double initValueU, double initValueY, double &rTime)
{
    mpTime = &rTime;
    mLastTime = 0.0;
    mDelayu.initialize(rTime, initValueU);
    mDelayy.initialize(rTime, initValueY);
    mIsInitialized = true;
}


