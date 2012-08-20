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
        double f;
        double *mpND_signal, *mpND_f, *mpND_c, *mpND_Zx;
        Port *mpIn, *mpP1;

    public:
        static Component *Creator()
        {
            return new MechanicForceTransformer();
        }

        void configure()
        {
            //Set member attributes
            f = 0.0;

            //Add ports to the component
            mpIn = addReadPort("in", "NodeSignal", Port::NOTREQUIRED);
            mpP1 = addPowerPort("P1", "NodeMechanic");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("F", "Generated force", "[N]", f);

            disableStartValue(mpP1, NodeMechanic::FORCE);
        }


        void initialize()
        {
            mpND_signal = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE, f);

            mpND_f = getSafeNodeDataPtr(mpP1, NodeMechanic::FORCE);
            mpND_c = getSafeNodeDataPtr(mpP1, NodeMechanic::WAVEVARIABLE);
            mpND_Zx = getSafeNodeDataPtr(mpP1, NodeMechanic::CHARIMP);

            (*mpND_f) = (*mpND_signal);
            (*mpND_Zx) = 0.0;
        }


        void simulateOneTimestep()
        {
            (*mpND_c) = (*mpND_signal);
//            if(mpIn->isConnected())				//Temporary RT solution
//                (*mpND_c) = (*mpND_signal);
//            else
//                (*mpND_c) = f;
            (*mpND_Zx) = 0.0;

//            std::stringstream ss;
//            ss << "C: " << (*mpND_c);
//            addInfoMessage(ss.str());
        }
    };
}

#endif // MECHANICFORCETRANSFORMER_HPP_INCLUDED
