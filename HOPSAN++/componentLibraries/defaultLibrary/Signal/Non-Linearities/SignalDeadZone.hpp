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

//$Id$

#ifndef SIGNALDEADZONE_HPP_INCLUDED
#define SIGNALDEADZONE_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalDeadZone : public ComponentSignal
    {


    private:
        double mStartDead;
        double mEndDead;
        double *mpND_in, *mpND_out;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalDeadZone();
            }

        void configure()
        {
            mStartDead = -1.0;
            mEndDead = 1.0;

            mpIn = addReadPort("in", "NodeSignal");
            mpOut = addWritePort("out", "NodeSignal", Port::NotRequired);

            registerParameter("u_dstart", "Start of Dead Zone", "[-]", mStartDead);
            registerParameter("u_dend", "End of Dead Zone", "[-]", mEndDead);
        }

        void initialize()
        {
            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);
        }

        void simulateOneTimestep()
        {
            //Deadzone equations
            if ( (*mpND_in) < mStartDead)
            {
                (*mpND_out) = (*mpND_in) - mStartDead;
            }
            else if ( (*mpND_in) > mStartDead && (*mpND_in) < mEndDead)
            {
                (*mpND_out) = 0;
            }
            else
            {
                (*mpND_out) = (*mpND_in) - mEndDead;
            }
        }
    };
}

#endif // SIGNALDEADZONE_HPP_INCLUDED
