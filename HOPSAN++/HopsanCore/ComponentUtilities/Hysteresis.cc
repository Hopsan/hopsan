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
//! @file   Hysteresis.cc
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-10-01
//!
//! @brief Contains a hysteresis function
//!
//$Id$

#include <math.h>
#include <iostream>
#include "Hysteresis.h"
#include "ComponentUtilities.h"

using namespace hopsan;
using namespace std;

Hysteresis::Hysteresis()
{
}


void Hysteresis::initialize(double &rTime, double initValue, double hysteresis)
{
    mDelay.initialize(rTime, initValue);
    mDelay.setStepDelay(1, initValue);
    mHysteresis = hysteresis;
}

void Hysteresis::setHysteresis(double hysteresis)
{
    mHysteresis = hysteresis;
}


double Hysteresis::value()
{
    return mDelay.value();
}


double Hysteresis::value(double u)
{
    double old=mDelay.value();
    if (old < u-mHysteresis/2.0)
    {
        cout << "Debug 1" << endl;
        mDelay.update(u-mHysteresis/2.0);
        return u-mHysteresis/2.0;
    }
    else if (old > u+mHysteresis/2.0)
    {
        cout << "Debug 2" << endl;
        mDelay.update(u+mHysteresis/2.0);
        return u+mHysteresis/2.0;
       }
    else
    {
        cout << "Debug 3" << endl;
        mDelay.update(old);
        return old;
    }
}
