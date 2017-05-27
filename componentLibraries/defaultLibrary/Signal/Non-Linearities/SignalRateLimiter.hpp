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
