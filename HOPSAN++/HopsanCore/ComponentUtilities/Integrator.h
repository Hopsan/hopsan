//!
//! @file   Integrator.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-22
//!
//! @brief Contains the Core Utility Integrator class
//!
//$Id$

#ifndef INTEGRATOR_H_INCLUDED
#define INTEGRATOR_H_INCLUDED

#include <deque>
#include "../win32dll.h"
#include "Delay.h"

namespace hopsan {

    class DLLIMPORTEXPORT Integrator
    {
    public:
        Integrator();
        void initialize(double &rTime, double timestep, double u0=0.0, double y0=0.0);
        void initializeValues(double u0, double y0);
        void update(double &u);
        double value(double &u);
	double value();

    private:
        Delay mDelayU, mDelayY;
        double mTimeStep;
        double *mpTime;
        double mLastTime;
        bool mIsInitialized;
    };


    class DLLIMPORTEXPORT OptimizedIntegrator
    {
    public:
        OptimizedIntegrator();
        void initialize(double &rTime, double timestep, double *pInput, double *pOutput, double u0=0.0, double y0=0.0);
        void initializeValues(double u0, double y0);
        void update();
        void integrate();

    private:
        //Delay mDelayU, mDelayY;
        double mDelayU, mDelayY;
        double mTimeStep;
        double *mpU;
        double *mpY;
        double *mpTime;
        double mLastTime;
        bool mIsInitialized;
    };


    class DLLIMPORTEXPORT NoDelayIntegrator
    {
    public:
        NoDelayIntegrator();
        void initialize(double &rTime, double timestep, double u0=0.0, double y0=0.0);
        void initializeValues(double u0, double y0);
        void update(double &u);
        double value(double &u);
        double value();

    private:
        double mDelayU, mDelayY;
        double mTimeStep;
        double *mpTime;
        double mLastTime;
        bool mIsInitialized;
    };

}

#endif // INTEGRATOR_H_INCLUDED
