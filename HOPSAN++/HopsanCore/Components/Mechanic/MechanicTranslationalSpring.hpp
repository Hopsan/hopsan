//$Id$

#ifndef MECHANICTRANSLATIONALSPRING_HPP_INCLUDED
#define MECHANICTRANSLATIONALSPRING_HPP_INCLUDED

#include "../../ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicTranslationalSpring : public ComponentC
    {

    private:
        double k;
        double v1, c1, lastc1, v2, c2, lastc2, Zc;
        double *mpND_v1, *mpND_c1, *mpND_Zc1, *mpND_v2, *mpND_c2, *mpND_Zc2;
        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new MechanicTranslationalSpring("TranslationalSpring");
        }

        MechanicTranslationalSpring(const std::string name) : ComponentC(name)
        {
            //Set member attributes
            k = 100.0;

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanic");
            mpP2 = addPowerPort("P2", "NodeMechanic");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("k", "Spring Coefficient", "[N/m]",  k);
        }


        void initialize()
        {
            mpND_v1 = getSafeNodeDataPtr(mpP1, NodeMechanic::VELOCITY);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeMechanic::WAVEVARIABLE);
            mpND_Zc1 = getSafeNodeDataPtr(mpP1, NodeMechanic::CHARIMP);
            mpND_v2 = getSafeNodeDataPtr(mpP2, NodeMechanic::VELOCITY);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeMechanic::WAVEVARIABLE);
            mpND_Zc2 = getSafeNodeDataPtr(mpP2, NodeMechanic::CHARIMP);

            Zc = k*mTimestep;

            (*mpND_Zc1) = Zc;
            (*mpND_Zc2) = Zc;
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            v1 = (*mpND_v1);
            lastc1 = (*mpND_c1);
            v2 = (*mpND_v2);
            lastc2 = (*mpND_c2);

            //Spring equations
            Zc = k*mTimestep;
            c1 = lastc2 + 2.0*Zc*v2;
            c2 = lastc1 + 2.0*Zc*v1;

            //Write new values to nodes
            (*mpND_c1) = c1;
            (*mpND_Zc1) = Zc;
            (*mpND_c2) = c2;
            (*mpND_Zc2) = Zc;
        }
    };
}

#endif // MECHANICTRANSLATIONALSPRING_HPP_INCLUDED


