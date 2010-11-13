#ifndef HYDRAULICDUMMYQ_HPP_INCLUDED
#define HYDRAULICDUMMYQ_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"

namespace hopsan {

    class HydraulicDummyQ : public ComponentQ
    {
    private:
        Port *mpP1, *mpP2;
        double *input, *p1, *q1, *c1, *Zc1;

    public:
        static Component *Creator()
        {
            return new HydraulicDummyQ("Dummy Q");
        }

        HydraulicDummyQ(const std::string name) : ComponentQ(name)
        {
            mTypeName = "HydraulicDummyQ";

            mpP1 = addReadPort("in", "NodeSignal");
            mpP2 = addPowerPort("P1", "NodeHydraulic");
        }


        void initialize()
        {
            input = mpP1->getNodeDataPtr(NodeSignal::VALUE);
            p1 = mpP2->getNodeDataPtr(NodeHydraulic::PRESSURE);
            q1 = mpP2->getNodeDataPtr(NodeHydraulic::FLOW);
            c1 = mpP2->getNodeDataPtr(NodeHydraulic::WAVEVARIABLE);
            Zc1 = mpP2->getNodeDataPtr(NodeHydraulic::CHARIMP);
        }


        void simulateOneTimestep()
        {
            (*p1) = (*c1)+(*Zc1);
            for(size_t i=0; i<(*input); ++i)
            {
                (*p1) = (*p1) * i;
            }
            *q1 = *input;
        }
    };
}

#endif // HYDRAULICDUMMYQ_HPP_INCLUDED
