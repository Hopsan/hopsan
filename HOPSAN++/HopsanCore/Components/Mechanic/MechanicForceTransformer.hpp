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
        double *mpND_signal, *mpND_f, *mpND_c, *mpND_Zx;
        Port *mpIn, *mpP1;

    public:
        static Component *Creator()
        {
            return new MechanicForceTransformer("ForceTransformer");
        }

        MechanicForceTransformer(const std::string name) : ComponentC(name)
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
            std::stringstream ss;
            ss << "C: " << (*mpND_c);
            addInfoMessage(ss.str());
        }
    };
}

#endif // MECHANICFORCETRANSFORMER_HPP_INCLUDED
