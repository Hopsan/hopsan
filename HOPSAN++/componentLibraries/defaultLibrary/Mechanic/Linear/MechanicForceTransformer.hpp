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

#ifndef MECHANICFORCETRANSFORMER_HPP_INCLUDED
#define MECHANICFORCETRANSFORMER_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicForceTransformer : public ComponentC
    {

    private:
        double *mpND_signal, *mpND_f, *mpND_c, *mpND_Zx;
        Port *mpP1;

    public:
        static Component *Creator()
        {
            return new MechanicForceTransformer();
        }

        void configure()
        {
            addInputVariable("F", "Generated force", "[N]", 0.0, &mpND_signal);
            mpP1 = addPowerPort("P1", "NodeMechanic");
            disableStartValue(mpP1, NodeMechanic::Force);
        }


        void initialize()
        {
            mpND_f = getSafeNodeDataPtr(mpP1, NodeMechanic::Force);
            mpND_c = getSafeNodeDataPtr(mpP1, NodeMechanic::WaveVariable);
            mpND_Zx = getSafeNodeDataPtr(mpP1, NodeMechanic::CharImpedance);

            (*mpND_f) = (*mpND_signal);
            (*mpND_Zx) = 0.0;
        }


        void simulateOneTimestep()
        {
            (*mpND_c) = (*mpND_signal);
            (*mpND_Zx) = 0.0;
        }
    };
}

#endif // MECHANICFORCETRANSFORMER_HPP_INCLUDED
