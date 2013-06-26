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
        double *mpStartDead;
        double *mpEndDead;
        double *mpND_in, *mpND_out;

    public:
        static Component *Creator()
        {
            return new SignalDeadZone();
            }

        void configure()
        {
            addInputVariable("in", "", "", 0.0, &mpND_in);
            addOutputVariable("out", "", "", &mpND_out);

            addInputVariable("u_dstart", "Start of Dead Zone", "-", -1.0, &mpStartDead);
            addInputVariable("u_dend", "End of Dead Zone", "-", 1.0, &mpEndDead);
        }

        void initialize()
        {

        }

        void simulateOneTimestep()
        {
            //Deadzone equations
            if ( (*mpND_in) < (*mpStartDead))
            {
                (*mpND_out) = (*mpND_in) - (*mpStartDead);
            }
            else if ( (*mpND_in) > (*mpStartDead) && (*mpND_in) < (*mpEndDead))
            {
                (*mpND_out) = 0;
            }
            else
            {
                (*mpND_out) = (*mpND_in) - (*mpEndDead);
            }
        }
    };
}

#endif // SIGNALDEADZONE_HPP_INCLUDED
