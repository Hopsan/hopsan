//!
//! @file   SecondOrderFilter.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2009-12-23
//!
//! @brief Contains the Core Second Order Filter class
//!
//$Id$

#ifndef SECONDORDERFILTER_H_INCLUDED
#define SECONDORDERFILTER_H_INCLUDED

#include <deque>
#include "win32dll.h"
#include "Delay.h"

/*
            num[0]*s^2 + num[1]*s + num[2]
    G = --------------------------------------
            den[0]*s^2 + den[1]*s + den[2]
*/

class DLLIMPORTEXPORT SecondOrderFilter
//! @brief The SecondOrderFilter class implements a second order filter
//!
//! To declare a filter like \f[G=\frac{a_2 s^2 + a_1 s + a_0}{b_2 s^2 + b_1 s + b_0}\f]
//! the syntax is myFilter.setNumDen(num, den)
//! where \f$num=\{a_2, a_1, a_0\}\f$
//! and \f$den=\{b_2, b_1, b_0\}\f$
//!
{
public:
    SecondOrderFilter();
    void initialize(double &rTime, double timestep, double num[3], double den[3], double u0=0.0, double y0=0.0, double min=-1.5E+300, double max=1.5E+300);
    void initializeValues(double u0, double y0);
    void setNumDen(double num[3], double den[3]);
    void update(double u);
	double value(double u);
	double value();

private:
    Delay mDelayU, mDelayY;
    double mCoeffU[5];
    double mCoeffY[5];
    double mMin, mMax;
    double mTimeStep;
    double *mpTime;
    double mLastTime;
	bool mIsInitialized;
};

#endif // SECONDORDERFILTER_H_INCLUDED
