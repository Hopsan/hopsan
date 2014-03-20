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
