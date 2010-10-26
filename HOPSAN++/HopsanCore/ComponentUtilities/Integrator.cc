//!
//! @file   Integrator.cc
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-22
//!
//! @brief Contains a Integrator Utility
//!
//$Id$

#include <iostream>
#include <cassert>
#include "../HopsanCore.h"
#include "Integrator.h"

using namespace hopsan;

Integrator::Integrator()
{
    mLastTime = 0.0;
    mIsInitialized = false;
}


void Integrator::initialize(double &rTime, double timestep, double u0, double y0)
{
    mDelayU.setStepDelay(1);
    mDelayY.setStepDelay(1);
    mDelayU.initialize(rTime, u0);
    mDelayY.initialize(rTime, y0);

    mTimeStep = timestep;
    mpTime = &rTime;
    mIsInitialized = true;
}


void Integrator::initializeValues(double u0, double y0)
{
    mDelayU.initializeValues(u0);
    mDelayY.initializeValues(y0);
}


void Integrator::update(double &u)
{
    if (!mIsInitialized)
    {
        std::cout << "Integrator function has to be initialized" << std::endl;
        assert(false);
    }
    else if (mLastTime != *mpTime)
    {
        //Filter equation
        //Bilinear transform is used
        mDelayY.update(mDelayY.value() + mTimeStep/2.0*(u + mDelayU.value()));
        mDelayU.update(u);

        mLastTime = *mpTime;
    }
}


double Integrator::value(double &u)
{
    update(u);

    return mDelayY.value();
}


//! Observe that a call to this method has to be followed by another call to value(double u) or to update(double u)
//! @return The integrated actual value.
//! @see value(double u)
double Integrator::value()
{
    update(mDelayU.valueIdx(1));

    return mDelayY.value();
}












OptimizedIntegrator::OptimizedIntegrator()
{
    mLastTime = 0.0;
    mIsInitialized = false;
}


void OptimizedIntegrator::initialize(double &rTime, double timestep, double *uref, double *yref, double u0, double y0)
{
    //mDelayU.setStepDelay(1);
    //mDelayY.setStepDelay(1);
    //mDelayU.initialize(rTime, u0);
    //mDelayY.initialize(rTime, y0);
    mDelayU = u0;
    mDelayY = y0;

    mTimeStep = timestep;
    mpU = uref;
    mpY = yref;
    mpTime = &rTime;
    mIsInitialized = true;
}


void OptimizedIntegrator::initializeValues(double u0, double y0)
{
    mDelayU = u0;
    mDelayY = y0;
    //mDelayU.initializeValues(u0);
    //mDelayY.initializeValues(y0);
}


void OptimizedIntegrator::update()
{
    if (!mIsInitialized)
    {
        std::cout << "Integrator function has to be initialized" << std::endl;
        assert(false);
    }
    else if (mLastTime != *mpTime)
    {
        //Filter equation
        //Bilinear transform is used
        mDelayY = mDelayY + mTimeStep/2.0*(*mpU + mDelayU);
        mDelayU = *mpU;
        //mDelayU.update(*mpU);

        mLastTime = *mpTime;
    }
}


void OptimizedIntegrator::doTheStuff()
{
    update();

    *mpY = mDelayY;
}
