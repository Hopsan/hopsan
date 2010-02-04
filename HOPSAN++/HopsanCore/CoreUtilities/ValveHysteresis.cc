//!
//! @file   ValveHysteresis.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-13
//!
//! @brief Contains a hysteresis function for valves and signals
//!
//$Id$

#include <math.h>
#include "ValveHysteresis.h"

ValveHysteresis::ValveHysteresis()
{
}

double ValveHysteresis::getValue(double xs, double xh, double xd)
{
    if (xd < xs-xh/2)
    {
        return xs-xh/2;
    }
    else if (xd > xs+xh/2)
    {
        return xs+xh/2;
       }
    else
    {
        return xd;
    }
}
