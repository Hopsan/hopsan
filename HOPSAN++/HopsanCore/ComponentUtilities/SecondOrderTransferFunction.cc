#include <math.h>
#include <iostream>
#include <cassert>
#include "SecondOrderTransferFunction.h"

using namespace hopsan;

SecondOrderTransferFunction::SecondOrderTransferFunction()
{
}

SecondOrderTransferFunction::SecondOrderTransferFunction(double num [3], double den [3], double timestep)
{
    for(int i=0; i<3; i++)
    {
      mnum[i] = num[i];
      mden[i] = den[i];
    }
    mTimestep = timestep;
//    mDelayu.setStepDelay(2);
//    mDelayy.setStepDelay(2);
    mDelayu[0] = 0;
    mDelayu[1] = 0;
    mDelayy[0] = 0;
    mDelayy[1] = 0;
}

void SecondOrderTransferFunction::update(double signal)
{
    u0 = signal;
    u1 = mDelayu[0]; // Inc. idx +1
    u2 = mDelayu[1]; // Inc. idx +1

    y1 = mDelayy[0]; // Inc. idx +1
    y2 = mDelayy[1]; // Inc. idx +1

    mDelayu[1] = mDelayu[0];
    mDelayu[0] = u0;
    mDelayy[1] = mDelayy[0];
    mDelayy[0] = y0;

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
}

void SecondOrderTransferFunction::setCoefficients(double num [3], double den [3], double timestep)
{
    for(int i=0; i<3; i++)
    {
      mnum[i] = num[i];
      mden[i] = den[i];
    }
    mTimestep = timestep;

    mDelayu[0] = 0;
    mDelayu[1] = 0;
    mDelayy[0] = 0;
    mDelayy[1] = 0;

}

double SecondOrderTransferFunction::value()
{
    return y0;
}

void SecondOrderTransferFunction::initialize(double initValueU, double initValueY)
{
    y0=initValueY;

    mDelayu[0] = initValueU;
    mDelayu[1] = initValueU;
    mDelayy[0] = initValueY;
    mDelayy[1] = initValueY;
}


