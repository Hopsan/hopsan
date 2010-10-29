//!
//! @file   Integrator.cc
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-06-30
//!
//! @brief Contiains a second order integrator utility with provision for some damping
//!
//$Id$

#include <iostream>
#include <cassert>
#include "../HopsanCore.h"
#include "DoubleIntegratorWithDamping.h"

using namespace hopsan;

DoubleIntegratorWithDamping::DoubleIntegratorWithDamping()
{
}


void DoubleIntegratorWithDamping::initialize(double timestep, double w0, double u0, double y0, double sy0)
{
    mW0 = w0;
    mDelayU = u0;
    mDelayY = y0;
    mDelaySY = sy0;
    mTimeStep = timestep;
}


void DoubleIntegratorWithDamping::initializeValues(double u0, double y0, double sy0)
{
    mDelayU = u0;
    mDelayY = y0;
    mDelaySY = sy0;
}


void DoubleIntegratorWithDamping::setDamping(double w0)
{
    mW0 = w0;
}


void DoubleIntegratorWithDamping::integrate(double u)
{
    double tempDelaySY = mDelaySY;
    mDelaySY = (2-mW0)/(2+mW0)*tempDelaySY + mTimeStep/(2.0+mW0)*(u + mDelayU);
    mDelayY = mDelayY + mTimeStep/2.0*tempDelaySY;
    mDelayU = u;
}


//! Returns first primitive from double integration
double DoubleIntegratorWithDamping::valueFirst()
{
    return mDelaySY;
}


//! Returns second primitive from double integration
double DoubleIntegratorWithDamping::valueSecond()
{
    return mDelayY;
}
