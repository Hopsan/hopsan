/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

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
    mDelayY = mDelayY + mTimeStep/2.0*(mDelaySY+tempDelaySY);
    mDelayU = u;
}


//! @brief Integrates one step, but saves previous step in case step has to be re-integrated
void DoubleIntegratorWithDamping::integrateWithUndo(double u)
{
    mDelaySYbackup = mDelaySY;
    mDelayYbackup = mDelayY;
    mDelayUbackup = mDelayU;

    integrate(u);
}


//! @brief Re-integrates last step
//! Last step must have been called with integrateWithUndo() for this to work.
void DoubleIntegratorWithDamping::redoIntegrate(double u)
{
    double tempDelaySY = mDelaySY;
    mDelaySY = (2-mW0)/(2+mW0)*mDelaySYbackup + mTimeStep/(2.0+mW0)*(u + mDelayUbackup);
    mDelayY = mDelayYbackup + mTimeStep/2.0*(mDelaySY+tempDelaySY);
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
