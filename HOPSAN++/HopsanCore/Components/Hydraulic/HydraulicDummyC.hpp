//$Id: HydraulicVolume.hpp 2132 2010-11-11 18:27:52Z bjoer $

#ifndef HYDRAULICDUMMYC_HPP_INCLUDED
#define HYDRAULICDUMMYC_HPP_INCLUDED

#include "../../ComponentEssentials.h"

namespace hopsan {

    class HydraulicDummyC : public ComponentC
    {

    private:
        Port *mpP1, *mpP2;
        double *input, *p1, *q1, *c1, *Zc1;

    public:
        static Component *Creator()
        {
            return new HydraulicDummyC("Dummy C");
        }

        HydraulicDummyC(const std::string name) : ComponentC(name)
        {
            //Set member attributes
            mTypeName = "HydraulicDummyC";

            //Add ports to the component
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
            (*c1) = (*p1)+(*q1);
            for(size_t i=0; i<(*input); ++i)
            {
                (*c1) = (*c1) * i;
            }
            *Zc1 = *input;
        }

        void finalize()
        {

        }
    };
}

#endif // HYDRAULICDUMMYC_HPP_INCLUDED
