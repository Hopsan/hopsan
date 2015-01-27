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
//! @file   SignalCounter.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-12-18
//!
//! @brief Contains a signal flank counter component
//!
//$Id$
#ifndef SIGNALCOUNTER_HPP
#define SIGNALCOUNTER_HPP


#include "ComponentEssentials.h"

namespace hopsan {

class SignalCounter : public ComponentSignal
{
private:
    bool mR, mF;
    double *mpIn, *mpOut;
    double mNextT, mTs;

    bool mPrev;

public:
    static Component *Creator()
    {
        return new SignalCounter();
    }

    void configure()
    {
        addConstant("r", "Count rising flags", "", true, mR);
        addConstant("f", "Count falling flags", "", true, mF);
        addInputVariable("in", "", "", 0, &mpIn);
        addOutputVariable("out", "", "", &mpOut);
    }

    void initialize()
    {
        (*mpOut) = 0;
        mPrev = false;
    }

    void simulateOneTimestep()
    {
        bool in = doubleToBool(*mpIn);

        if(mR && in && !mPrev)
        {
            (*mpOut) += 1;

        }
        else if(mF && !in && mPrev)
        {
            (*mpOut) += 1;
        }

        mPrev = in;
    }
};
}

#endif // SIGNALCOUNTER_HPP
