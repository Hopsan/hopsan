//!
//! @file   FirstOrderFilter.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2009-12-23
//!
//! @brief Contains the Core First Order Filter class
//!
//$Id$

#ifndef FIRSTORDERFILTER_H_INCLUDED
#define FIRSTORDERFILTER_H_INCLUDED

#include <deque>
#include "win32dll.h"

/*
            num[0]*s + num[1]
    G = -------------------------
            den[0]*s + den[1]
*/

class DLLIMPORTEXPORT FirstOrderFilter
//! @brief The FirstOrderFilter class implements a first order filter
//!
//! To declare a filter like \f[G=\frac{a_1 s + a_0}{b_1 s + b_0}\f]
//! the syntax is myFilter.setNumDen(num, den)
//! where \f$num=\{a_1, a_0\}\f$
//! and \f$den=\{b_1, b_0\}\f$
//!
{
public:
    FirstOrderFilter();
    void initialize(double &rTime, double timestep, double num[2], double den[2], double u0=0.0, double y0=0.0, double min=-1.5E+300, double max=1.5E+300);
    void initializeValues(double u0, double y0);
    void setNumDen(double num[2], double den[2]);
    void update(double u);
	double value(double u);
	double value();

private:
    Delay mDelayU, mDelayY;
    double mCoeffU[2];
    double mCoeffY[2];
    double mMin, mMax;
    double mTimeStep;
    double *mpTime;
    double mLastTime;
	bool mIsInitialized;
};

#endif // FIRSTORDERFILTER_H_INCLUDED
