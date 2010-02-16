#ifndef TRANSFERFUNCTION_H_INCLUDED
#define TRANSFERFUNCTION_H_INCLUDED

#include "../win32dll.h"
#include "Delay.h"

class DLLIMPORTEXPORT TransferFunction
{
public:
    TransferFunction();
    TransferFunction(double num [3], double den [3], double timestep);
    void update(double signal);
    void setCoefficients(double num [3], double den [3], double timestep);
    double getValue(double value);
    void initialize(double initValueU, double initValueY, double &rTime);

private:
    double u0, u1, u2;
	double y0, y1, y2;
	double a [3];
	double b [3];
	Delay mDelayu;
	Delay mDelayy;
	double mnum [3];
	double mden [3];
	double mTimestep;
    double *mpTime;
    double mLastTime;
    bool mIsInitialized;
};

#endif // TRANSFERFUNCTION_H_INCLUDED
