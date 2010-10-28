#include <math.h>
#include <iostream>
#include <cassert>
#include "TransferFunction.h"

using namespace hopsan;

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
//    mDelayu.setStepDelay(2);
//    mDelayy.setStepDelay(2);
    mDelayu.initialize(2,0);
    mDelayy.initialize(2,0);
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
      u1 = mDelayu.getIdx(1); // Inc. idx +1
      u2 = mDelayu.getIdx(2); // Inc. idx +1

      y1 = mDelayy.getIdx(1); // Inc. idx +1
      y2 = mDelayy.getIdx(2); // Inc. idx +1

      mDelayu.update(u0);
      mDelayy.update(y0);

      b[0] = 4.0*mnum[2]+mnum[0]*mTimestep*mTimestep+2.0*mnum[1]*mTimestep;
      b[1] = -8.0*mnum[2]+2*mnum[0]*mTimestep*mTimestep;
      b[2] = -2.0*mnum[1]*mTimestep+4.0*mnum[2]+mnum[0]*mTimestep*mTimestep;

      a[0] = 4.0*mden[2]+mden[0]*mTimestep*mTimestep+2.0*mden[1]*mTimestep;
      a[1] = -8.0*mden[2]+2.0*mden[0]*mTimestep*mTimestep;
      a[2] = -2.0*mden[1]*mTimestep+4.0*mden[2]+mden[0]*mTimestep*mTimestep;

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

    //! @todo why are theses bellow needed?
//    mDelayu.setStepDelay(2); // Added
//    mDelayy.setStepDelay(2); // Added
    mDelayu.initialize(2,0); //This will reset the entire data buffer to 0
    mDelayy.initialize(2,0);

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
//    mDelayu.initialize(rTime, initValueU);
//    mDelayy.initialize(rTime, initValueY);
    mDelayu.initialize(2, initValueU);
    mDelayy.initialize(2, initValueY);
    //! @todo is it allways 2 in this class, i think it should be possible to have mor than 2, there are hardcoded 2 in other places in this file as well
    mIsInitialized = true;
}


