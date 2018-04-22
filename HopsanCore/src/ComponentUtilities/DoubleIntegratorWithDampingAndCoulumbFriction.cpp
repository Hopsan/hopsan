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
//! @file   DoubleIntegratorWithDampingAndCoulumbFriction.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-08-03
//!
//! @brief Contains a second order integrator utility with provision for some damping and coulomb friction
//! Revised by Liselott Eriksson 2013
//!
//$Id$

#include "ComponentUtilities/DoubleIntegratorWithDampingAndCoulumbFriction.h"

using namespace hopsan;



void DoubleIntegratorWithDampingAndCoulombFriction::initialize(double timestep, double w0, double Fs, double Fk, double u0, double y0, double sy0)
{
    mW0 = w0;
    mUs = Fs;
    mUk = Fk;
    mDelayU = u0;
    mDelayY = y0;
    mDelaySY = sy0;
    mTimeStep = timestep;
    movement = 0;
}


void DoubleIntegratorWithDampingAndCoulombFriction::initializeValues(double u0, double y0, double sy0)
{
    mDelayU = u0;
    mDelayY = y0;
    mDelaySY = sy0;
}


void DoubleIntegratorWithDampingAndCoulombFriction::setDamping(double w0)
{
    mW0 = w0;
}


void DoubleIntegratorWithDampingAndCoulombFriction::setFriction(double Fs, double Fk)
{
    mUs = Fs;
    mUk = Fk;
}


void DoubleIntegratorWithDampingAndCoulombFriction::integrate(double u)
{
    double tempDelaySY = mDelaySY;
    double ue;    //Effective acceleration
    double ues = -(2.0-mW0)/mTimeStep*tempDelaySY-mDelayU;

    switch (movement)
    {
    case 0:     // Standing still
        if((u >= mUs))
        {
            movement = 1;
        }
        else if((u <= -mUs))
        {
            movement = -1;
        }
        break;
    case 1:     // Moving with positive velocity
        if (ues > (u-mUk))
        {
            mDelaySY = 0.0;
#ifdef Q_OS_OSX
            /* mDelayY = mDelayY; */ //Note: this is strange. /magse
#else
            mDelayY = mDelayY; //Note: the movement is skipped for the partial timestep
#endif
            movement = 0;
        }
        else
        {
            ue = u-mUk;
            mDelaySY = (2.0-mW0)/(2.0+mW0)*tempDelaySY + mTimeStep/(2.0+mW0)*(ue+mDelayU);
            mDelayY = mDelayY + mTimeStep/2.0*(mDelaySY+tempDelaySY);
            mDelayU = ue;
        }
        break;
    case -1:     // Moving with negative velocity
        if (ues<(u+mUk))
        {
            mDelaySY = 0.0;
#ifdef Q_OS_OSX
            /* mDelayY = mDelayY; */ //Note: this is strange. /magse
#else
            mDelayY = mDelayY;  //Note: the movement is skipped for the partial timestep
#endif
            movement = 0;
        }
        else
        {
            ue = u+mUk;
            mDelaySY = (2.0-mW0)/(2.0+mW0)*tempDelaySY + mTimeStep/(2.0+mW0)*(ue+mDelayU);
            mDelayY = mDelayY + mTimeStep/2.0*(mDelaySY+tempDelaySY);
            mDelayU = ue;
        }
        break;

        //Note: the code will always stop at velocity zero

    default:
        movement = 0;
        break;
    }
}


//! @brief Integrates one step, but saves previous step in case step has to be re-integrated
void DoubleIntegratorWithDampingAndCoulombFriction::integrateWithUndo(double u)
{
    mDelaySYbackup = mDelaySY;
    mDelayYbackup = mDelayY;
    mDelayUbackup = mDelayU;

    integrate(u);
}


//! @brief Re-integrates last step
//! Last step must have been called with integrateWithUndo() for this to work.
void DoubleIntegratorWithDampingAndCoulombFriction::redoIntegrate(double u)
{
    mDelaySY = (2.0-mW0)/(2.0+mW0)*mDelaySYbackup + mTimeStep/(2.0+mW0)*(u + mDelayUbackup);
    mDelayY = mDelayYbackup + mTimeStep/2.0*(mDelaySY+mDelaySYbackup);
    mDelayU = u;
}


//! Returns first primitive from double integration
double DoubleIntegratorWithDampingAndCoulombFriction::valueFirst()
{
    return mDelaySY;
}


//! Returns second primitive from double integration
double DoubleIntegratorWithDampingAndCoulombFriction::valueSecond()
{
    return mDelayY;
}
