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
        double *signal_ptr, *c_ptr, *Zx_ptr;
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
            if(mpIn->isConnected()) { signal_ptr = mpIn->getNodeDataPtr(NodeSignal::VALUE); }
            else { signal_ptr = new double(f); }

            c_ptr = mpP1->getNodeDataPtr(NodeMechanic::WAVEVARIABLE);
            Zx_ptr = mpP1->getNodeDataPtr(NodeMechanic::CHARIMP);
        }


        void simulateOneTimestep()
        {
            (*c_ptr) = (*signal_ptr);
            (*Zx_ptr) = 0.0;
        }
    };
}

#endif // MECHANICFORCETRANSFORMER_HPP_INCLUDED
