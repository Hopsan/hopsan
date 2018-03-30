/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   DoubleIntegratorWithDamping.cpp
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
