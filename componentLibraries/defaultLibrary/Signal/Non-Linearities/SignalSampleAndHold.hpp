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
//! @file   SignalSampleAndHold.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-12-18
//!
//! @brief Contains a signal sample-and-hold component
//!
//$Id$
#ifndef SIGNALSAMPLEANDHOLD_HPP_INCLUDED
#define SIGNALSAMPLEANDHOLD_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

class SignalSampleAndHold : public ComponentSignal
{
private:
    double *mpFs;
    double *mpIn, *mpOut;
    double mNextT, mTs;

public:
    static Component *Creator()
    {
        return new SignalSampleAndHold();
    }

    void configure()
    {
        addInputVariable("f_s", "Sampling Frequency", "Hz", 100, &mpFs);
        addInputVariable("in", "", "", 0, &mpIn);
        addOutputVariable("out", "", "", &mpOut);
    }

    void initialize()
    {
        (*mpOut) = (*mpIn);
        mNextT = mTime;
        mTs = 1.0/(*mpFs);
    }

    void simulateOneTimestep()
    {
        if(mTime >= mNextT)
        {
            (*mpOut) = (*mpIn);
            mNextT += mTs;
        }
    }
};
}

#endif // SIGNALSAMPLEANDHOLD_HPP_INCLUDED
