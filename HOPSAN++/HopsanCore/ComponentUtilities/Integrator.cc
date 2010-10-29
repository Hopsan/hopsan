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
}


void Integrator::initialize(double timestep, double u0, double y0)
{

    mDelayU = u0;
    mDelayY = y0;

    mTimeStep = timestep;
}


void Integrator::initializeValues(double u0, double y0)
{
    mDelayU = u0;
    mDelayY = y0;
}


double Integrator::update(double &u)
{
    //Filter equation
    //Bilinear transform is used
    mDelayY = mDelayY + mTimeStep/2.0*(u + mDelayU);
    mDelayU = u;

    return mDelayY;
}


//! Observe that a call to this method has to be followed by another call to value(double u) or to update(double u)
//! @return The integrated actual value.
//! @see value(double u)
double Integrator::value()
{
    return mDelayY;
}
