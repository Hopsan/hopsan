#ifndef TRANSFERFUNCTION_H_INCLUDED
#define TRANSFERFUNCTION_H_INCLUDED

#include <deque>
#include "win32dll.h"

class DLLIMPORTEXPORT TransferFunction
{
public:
    TransferFunction();
    TransferFunction(double num [3], double den [3], double timestep);
    void setCoefficients(double num [3], double den [3], double timestep);
    double filter(double signal);
private:
    double u [3];
	double y [3];
	double a [3];
	double b [3];
	double mnum [3];
	double mden [3];
	double mTimestep;
};

#endif // TRANSFERFUNCTION_H_INCLUDED
