/*
 *  ValveHysteresis.cc
 *  HOPSAN++
 *
 *  Created by Robert Braun on 2010-01-13.
 *  Copyright 2010 LiU. All rights reserved.
 *
 */

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
