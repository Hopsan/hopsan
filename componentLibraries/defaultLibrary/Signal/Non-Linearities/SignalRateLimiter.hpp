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

        // Choose which limit to compare with
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
