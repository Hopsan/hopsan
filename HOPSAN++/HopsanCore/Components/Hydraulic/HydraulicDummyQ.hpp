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

#ifndef HYDRAULICDUMMYQ_HPP_INCLUDED
#define HYDRAULICDUMMYQ_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"

namespace hopsan {

    class HydraulicDummyQ : public ComponentQ
    {
    private:
        Port *mpP1, *mpP2;
        double *mpND_in, *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1;

    public:
        static Component *Creator()
        {
            return new HydraulicDummyQ("Dummy Q");
        }

        HydraulicDummyQ(const std::string name) : ComponentQ(name)
        {

            mpP1 = addReadPort("in", "NodeSignal");
            mpP2 = addPowerPort("P1", "NodeHydraulic");
        }


        void initialize()
        {
            mpND_in = getSafeNodeDataPtr(mpP1, NodeSignal::VALUE);
            mpND_p1 = getSafeNodeDataPtr(mpP2, NodeHydraulic::PRESSURE);
            mpND_q1 = getSafeNodeDataPtr(mpP2, NodeHydraulic::FLOW);
            mpND_c1 = getSafeNodeDataPtr(mpP2, NodeHydraulic::WAVEVARIABLE);
            mpND_Zc1 = getSafeNodeDataPtr(mpP2, NodeHydraulic::CHARIMP);
        }


        void simulateOneTimestep()
        {
            (*mpND_p1) = (*mpND_c1)+(*mpND_Zc1);
            for(int i=0; i<(*mpND_in); ++i)
            {
                (*mpND_p1) = (*mpND_p1) * i;
            }
            *mpND_q1 = *mpND_in;
        }
    };
}

#endif // HYDRAULICDUMMYQ_HPP_INCLUDED
