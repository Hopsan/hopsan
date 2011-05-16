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

//! Updates the integrator one timestep and returns the new value
double Integrator::update(double &u)
{
    //Filter equation
    //Bilinear transform is used
    mDelayY = mDelayY + mTimeStep/2.0*(u + mDelayU);
    mDelayU = u;

    return mDelayY;
}


//! Returns the integrator value
//! @return The integrated actual value.
//! @see value(double u)
double Integrator::value()
{
    return mDelayY;
}
