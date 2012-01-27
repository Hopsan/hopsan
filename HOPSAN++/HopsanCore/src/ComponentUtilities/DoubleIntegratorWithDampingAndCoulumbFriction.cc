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
//! @file   DoubleIntegratorWithDampingAndCoulumbFriction.cc
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-08-03
//!
//! @brief Contiains a second order integrator utility with provision for some damping and coulumb friction
//! Revised by Robert Braun 2011-11-04
//!
//$Id$

//#include <iostream>
//#include <cassert>
#include "ComponentUtilities/DoubleIntegratorWithDampingAndCoulumbFriction.h"

using namespace hopsan;



void DoubleIntegratorWithDampingAndCoulombFriction::initialize(double timestep, double w0, double mass, double Fs, double Fk, double u0, double y0, double sy0)
{
    mW0 = w0;
    mUs = Fs/mass;
    mUk = Fk/mass;
    mDelayU = u0;
    mDelayY = y0;
    mDelaySY = sy0;
    mTimeStep = timestep;
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


void DoubleIntegratorWithDampingAndCoulombFriction::integrate(double u)
{
    double tempDelaySY = mDelaySY;
    double ue;    //Effective acceleration
    double ues = -(2-mW0)/mTimeStep*tempDelaySY-mDelayU;

    ue = ues;           //First assume no movement
    if(ues>(u-mUs) && ues<(u+mUs))
    {
      mDelaySY = 0;
      mDelayY = mDelayY;
    }
    else
    {
        if(ues<=(u-mUs))   //Movement, so change ue
        {
            ue = u-mUk;
        }
        else if(ues>=(u+mUs))
        {
            ue = u+mUk;
        }
        mDelaySY = (2-mW0)/(2+mW0)*tempDelaySY + mTimeStep/(2+mW0)*(ue+mDelayU);
        mDelayY = mDelayY + mTimeStep/2*(mDelaySY+tempDelaySY);
    }
    mDelayU = ue;
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
    mDelaySY = (2-mW0)/(2+mW0)*mDelaySYbackup + mTimeStep/(2.0+mW0)*(u + mDelayUbackup);
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
