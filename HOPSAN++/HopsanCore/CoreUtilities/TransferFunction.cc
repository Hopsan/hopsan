#include <math.h>
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
    mDelayu.setStepDelay(2);
    mDelayy.setStepDelay(2);
}

void TransferFunction::update(double signal)
{
    u2 = mDelayu.value(2); // Inc. idx +1
    u1 = mDelayu.value(1); // Inc. idx +1
    u0 = signal;

    y2 = mDelayy.value(2); // Inc. idx +1
    y1 = mDelayy.value(1); // Inc. idx +1

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

        mDelayu.update(u0);
        mDelayy.update(y0);
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

double TransferFunction::getValue()
{
    return y0;
}

void TransferFunction::initializeValues(double initValueU, double initValueY)
{
    mDelayu.initializeValues(initValueU);
    mDelayy.initializeValues(initValueY);
}


