/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

//!
//! @file   DoubleIntegratorWithDamping.cc
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-06-30
//!
//! @brief Contiains a second order integrator utility with provision for some damping
//!
//$Id$

//#include <iostream>
//#include <cassert>
#include "ComponentUtilities/DoubleIntegratorWithDamping.h"

using namespace hopsan;

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
    double tempDelaySY = mDelaySYbackup;
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
