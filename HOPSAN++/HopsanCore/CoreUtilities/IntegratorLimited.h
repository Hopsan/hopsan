//!
//! @file   IntegratorLimited.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-22
//!
//! @brief Contains the Core Utility Limited Integrator class
//!
//$Id$

#ifndef INTEGRATORLIMITED_H_INCLUDED
#define INTEGRATORLIMITED_H_INCLUDED

#include <deque>
#include "win32dll.h"

class DLLIMPORTEXPORT IntegratorLimited
{
public:
    IntegratorLimited();
    void initialize(double &rTime, double timestep, double u0=0.0, double y0=0.0, double min=-1.5E+300, double max=1.5E+300);
    void initializeValues(double u0, double y0);
    void setMinMax(double min, double max);
    void update(double u);
	double value(double u);

private:
    Delay mDelayU, mDelayY;
    double mMin, mMax;
    double mTimeStep;
    double *mpTime;
    double mLastTime;
	bool mIsInitialized;
};

#endif // INTEGRATOR_H_INCLUDED
