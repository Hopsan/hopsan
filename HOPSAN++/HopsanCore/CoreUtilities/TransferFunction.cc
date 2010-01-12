#include <math.h>
#include "TransferFunction.h"


TransferFunction::TransferFunction()
{
}

void TransferFunction::Initialize()
{

}

double TransferFunction::Filter(double signal, const double num [3], const double den [3], double timestep )
{
        u[2] = u[1];
        u[1] = u[0];
        u[0] = signal;

        y[2] = y[1];
        y[1] = y[0];

        b[0] = 4.0*num[2]+num[0]*pow(timestep,2.0)+2.0*num[1]*timestep;
        b[1] = -8.0*num[2]+2*num[0]*pow(timestep,2.0);
        b[2] = -2.0*num[1]*timestep+4.0*num[2]+num[0]*pow(timestep,2.0);

        a[0] = 4.0*den[2]+den[0]*pow(timestep,2.0)+2.0*den[1]*timestep;
        a[1] = -8.0*den[2]+2.0*den[0]*pow(timestep,2.0);
        a[2] = -2.0*den[1]*timestep+4.0*den[2]+den[0]*pow(timestep,2.0);

        /* Equation:
                bo+b1q^-1+b2q^-2
          y=  -------------------- u
                a0+a1q^-1+a2q^-2
        */

        y[0] = (b[0]*u[0]+b[1]*u[1]+b[2]*u[2]-a[1]*y[1]-a[2]*y[2])/a[0];
        return y[0];
}

