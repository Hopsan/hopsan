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

#ifndef HYDRAULICDUMMYC_HPP_INCLUDED
#define HYDRAULICDUMMYC_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    class HydraulicDummyC : public ComponentC
    {

    private:
        Port *mpP2;
        double *mpIn, *mpND_p1, *mpND_q1, *mpND_c1, *mpND_Zc1;

    public:
        static Component *Creator()
        {
            return new HydraulicDummyC();
        }

        void configure()
        {
            addInputVariable("in", "", "", 0.0, &mpIn);
            mpP2 = addPowerPort("P1", "NodeHydraulic");
        }


        void initialize()
        {
            mpND_p1 = getSafeNodeDataPtr(mpP2, NodeHydraulic::Pressure);
            mpND_q1 = getSafeNodeDataPtr(mpP2, NodeHydraulic::Flow);
            mpND_c1 = getSafeNodeDataPtr(mpP2, NodeHydraulic::WaveVariable);
            mpND_Zc1 = getSafeNodeDataPtr(mpP2, NodeHydraulic::CharImpedance);
        }


        void simulateOneTimestep()
        {
            (*mpND_c1) = (*mpND_p1)+(*mpND_q1);
            for(int i=0; i<(*mpIn); ++i)
            {
                (*mpND_c1) = (*mpND_c1) * i;
            }
            *mpND_Zc1 = *mpIn;
        }
    };
}

#endif // HYDRAULICDUMMYC_HPP_INCLUDED
