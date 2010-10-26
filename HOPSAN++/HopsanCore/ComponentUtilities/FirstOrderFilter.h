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
#include "../win32dll.h"
#include "Delay.h"

namespace hopsan {

    /*
            num[0]*s + num[1]
    G = -------------------------
            den[0]*s + den[1]
*/

    class DLLIMPORTEXPORT FirstOrderFilter
    {
    public:
        FirstOrderFilter();
        void initialize(double &rTime, double timestep, double num[2], double den[2], double u0=0.0, double y0=0.0, double min=-1.5E+300, double max=1.5E+300);
        void initializeValues(double u0, double y0);
        void setMinMax(double min, double max);
        void setNumDen(double num[2], double den[2]);
        void update(double u);
        double value(double u);
	double value();

    private:
        double mValue;
        Delay mDelayU, mDelayY;
        double mCoeffU[2];
        double mCoeffY[2];
        double mMin, mMax;
        double mTimeStep;
        double *mpTime;
        double mLastTime;
	bool mIsInitialized;
    };


class DLLIMPORTEXPORT OptimizedFirstOrderFilter
    {
    public:
        OptimizedFirstOrderFilter();
        void initialize(double &rTime, double timestep, double num[2], double den[2], double *uref, double *yref, double u0=0.0, double y0=0.0, double min=-1.5E+300, double max=1.5E+300);
        void initializeValues(double u0, double y0);
        void setMinMax(double min, double max);
        void setNumDen(double num[2], double den[2]);
        void update();
        void doTheStuff();

    private:
        double mValue;
        double mDelayU, mDelayY;
        double mCoeffU[2];
        double mCoeffY[2];
        double *mpU, *mpY;
        double mMin, mMax;
        double mTimeStep;
        double *mpTime;
        double mLastTime;
        bool mIsInitialized;
    };

}
#endif // FIRSTORDERFILTER_H_INCLUDED
