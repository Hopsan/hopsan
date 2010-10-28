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
//    mDelayU.setStepDelay(1);
//    mDelayY.setStepDelay(1);
    mDelayU.initialize(1, u0);
    mDelayY.initialize(1, y0);

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
//    if (!mIsInitialized)
//    {
//        std::cout << "Integrator function has to be initialized" << std::endl;
//        assert(false);
//    }
//    else
    if (mLastTime != *mpTime)
    {
        //Filter equation
        //Bilinear transform is used
        mDelayY.update(mDelayY.getOldest() + mTimeStep/2.0*(u + mDelayU.getOldest()));
        mDelayU.update(u);

        mLastTime = *mpTime;
    }
}


double Integrator::value(double &u)
{
    update(u);

    return mDelayY.getOldest();
}


////! Observe that a call to this method has to be followed by another call to value(double u) or to update(double u)
////! @return The integrated actual value.
////! @see value(double u)
//double Integrator::value()
//{
//    update(mDelayU.valueIdx(1));

//    return mDelayY.value();
//}












NoDelayAndPointersIntegrator::NoDelayAndPointersIntegrator()
{
    mLastTime = 0.0;
    mIsInitialized = false;
}


void NoDelayAndPointersIntegrator::initialize(double &rTime, double timestep, double *pInput, double *pOutput, double u0, double y0)
{
    //mDelayU.setStepDelay(1);
    //mDelayY.setStepDelay(1);
    //mDelayU.initialize(rTime, u0);
    //mDelayY.initialize(rTime, y0);
    mDelayU = u0;
    mDelayY = y0;

    mTimeStep = timestep;
    mpU = pInput;
    mpY = pOutput;
    mpTime = &rTime;
    mIsInitialized = true;
}


void NoDelayAndPointersIntegrator::initializeValues(double u0, double y0)
{
    mDelayU = u0;
    mDelayY = y0;
    //mDelayU.initializeValues(u0);
    //mDelayY.initializeValues(y0);
}


void NoDelayAndPointersIntegrator::update()
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


void NoDelayAndPointersIntegrator::integrate()
{
    update();

    *mpY = mDelayY;
}








NoDelayIntegrator::NoDelayIntegrator()
{
    mLastTime = 0.0;
    mIsInitialized = false;
}


void NoDelayIntegrator::initialize(double &rTime, double timestep, double u0, double y0)
{

    mDelayU = u0;
    mDelayY = y0;

    mTimeStep = timestep;
    mpTime = &rTime;
    mIsInitialized = true;
}


void NoDelayIntegrator::initializeValues(double u0, double y0)
{
    mDelayU = u0;
    mDelayY = y0;
}


void NoDelayIntegrator::update(double &u)
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
        mDelayY = mDelayY + mTimeStep/2.0*(u + mDelayU);
        mDelayU = u;

        mLastTime = *mpTime;
    }
}


double NoDelayIntegrator::value(double &u)
{
    update(u);

    return mDelayY;
}


//! Observe that a call to this method has to be followed by another call to value(double u) or to update(double u)
//! @return The integrated actual value.
//! @see value(double u)
double NoDelayIntegrator::value()
{
    update(mDelayU);

    return mDelayY;
}
