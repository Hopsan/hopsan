//!
//! @file   Integrator.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-06-30
//!
//! @brief Core utility for double integrator with provision for some damping
//!
//$Id$

#ifndef DOUBLEINTEGRATORWITHDAMPING_H_INCLUDED
#define DOUBLEINTEGRATORWITHDAMPING_H_INCLUDED

#include <deque>
#include "../win32dll.h"
#include "Delay.h"

namespace hopsan {

    class DLLIMPORTEXPORT DoubleIntegratorWithDamping
    {
    public:
        DoubleIntegratorWithDamping();
        void initialize(double &rTime, double timestep, double w0, double u0=0.0, double y0=0.0, double sy0=0.0);
        void initializeValues(double u0, double y0, double sy0);
        void setDamping(double w0);
        void integrate(double u);
        double valueFirst();
        double valueSecond();

    private:
        Delay mDelayU, mDelayY, mDelaySY;
        double mTimeStep;
        double *mpTime;
        double mLastTime;
        bool mIsInitialized;
        double mW0;
    };
}

#endif // DOUBLEINTEGRATORWITHDAMPING_H_INCLUDED
