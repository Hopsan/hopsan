//$Id$

#ifndef MECHANICFORCETRANSFORMER_HPP_INCLUDED
#define MECHANICFORCETRANSFORMER_HPP_INCLUDED

#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicForceTransformer : public ComponentC
    {

    private:
        double f;
        double *mpND_signal, *mpND_c, *mpND_Zx;
        Port *mpIn, *mpP1;

    public:
        static Component *Creator()
        {
            return new MechanicForceTransformer("ForceTransformer");
        }

        MechanicForceTransformer(const std::string name) : ComponentC(name)
        {
            //Set member attributes
            mTypeName = "MechanicForceTransformer";
            f = 0.0;

            //Add ports to the component
            mpIn = addReadPort("in", "NodeSignal", Port::NOTREQUIRED);
            mpP1 = addPowerPort("P1", "NodeMechanic");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("f", "Generated force", "[N]", f);
        }


        void initialize()
        {
            if(mpIn->isConnected()) { mpND_signal = mpIn->getNodeDataPtr(NodeSignal::VALUE); }
            else { mpND_signal = new double(f); }

            mpND_c = mpP1->getNodeDataPtr(NodeMechanic::WAVEVARIABLE);
            mpND_Zx = mpP1->getNodeDataPtr(NodeMechanic::CHARIMP);
        }


        void simulateOneTimestep()
        {
            (*mpND_c) = (*mpND_signal);
            (*mpND_Zx) = 0.0;
        }
    };
}

#endif // MECHANICFORCETRANSFORMER_HPP_INCLUDED
