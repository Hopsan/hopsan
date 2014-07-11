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
//! @file   SignalRateLimiter.hpp
//! @author Viktor Larsson <vikla551@student.liu.se>
//! @date   2014-03-07
//!
//! @brief Contains a signal rate limiter component
//!
//$Id$
#ifndef SIGNALRATELIMITER_HPP_INCLUDED
#define SIGNALRATELIMITER_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

class SignalRateLimiter : public ComponentSignal
{
private:
    double *mpCup,*mpCdown;
    double *mpIn, *mpOut;

public:
    static Component *Creator()
    {
        return new SignalRateLimiter();
    }

    void configure()
    {
        addInputVariable("c_up", "Maximum increase rate", "unit/s", 1, &mpCup);
        addInputVariable("c_down", "Maximum decrease rate", "unit/s", -1, &mpCdown);
        addInputVariable("in", "", "", 0, &mpIn);
        addOutputVariable("out", "", "", &mpOut);
    }

    void initialize()
    {
        (*mpOut) = (*mpIn);
    }

    void simulateOneTimestep()
    {
        const double diff  = (*mpIn)-(*mpOut);

        // Choose which limit to comare with
        double c;
        if (diff>=0)
        {
            c = *mpCup;
        }
        else
        {
            c = *mpCdown;
        }

        // Compare slope with limit
        if ( fabs(diff)/mTimestep > fabs(c))
        {
            // Limit rate
            (*mpOut) = (*mpOut) + c*mTimestep;
        }
        else
        {
            // No limit imposed
            (*mpOut) = (*mpIn);
        }
    }
};
}

#endif // SIGNALRATELIMITER_HPP_INCLUDED
