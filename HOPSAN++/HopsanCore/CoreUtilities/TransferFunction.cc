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
}

double TransferFunction::filter(double signal)
{
    u[2] = u[1];
    u[1] = u[0];
    u[0] = signal;

    y[2] = y[1];
    y[1] = y[0];

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

        y[0] = (b[0]*u[0]+b[1]*u[1]+b[2]*u[2]-a[1]*y[1]-a[2]*y[2])/a[0];
        return y[0];
}

void TransferFunction::setCoefficients(double num [3], double den [3], double timestep)
{
    for(int i=0; i<3; i++)
    {
      mnum[i] = num[i];
      mden[i] = den[i];
    }
    mTimestep = timestep;
}
