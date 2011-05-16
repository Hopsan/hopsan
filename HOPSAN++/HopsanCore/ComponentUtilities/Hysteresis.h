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
//! @file   Hysteresis.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-10-01
//!
//! @brief Contains a hysteresis function
//!
//$Id$

#ifndef HYSTERESIS_H_INCLUDED
#define HYSTERESIS_H_INCLUDED

#include <deque>
#include "../win32dll.h"

#include "Delay.h"

namespace hopsan {

class DLLIMPORTEXPORT Hysteresis
{
public:
    Hysteresis();
    void initialize(double &rTime, double initValue=0.0, double hysteresis=0.0);
    void setHysteresis(double hysteresis);
    double value(double u);
    double value();
    void update(double u);

private:
    Delay mDelay;
    double mHysteresis;
};
}

#endif // VALVEHYSTERESIS_H_INCLUDED
