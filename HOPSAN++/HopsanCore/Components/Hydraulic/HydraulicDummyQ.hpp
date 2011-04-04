#ifndef HYDRAULICDUMMYQ_HPP_INCLUDED
#define HYDRAULICDUMMYQ_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"

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
            for(size_t i=0; i<(*mpND_in); ++i)
            {
                (*mpND_p1) = (*mpND_p1) * i;
            }
            *mpND_q1 = *mpND_in;
        }
    };
}

#endif // HYDRAULICDUMMYQ_HPP_INCLUDED
