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
        double *v1_ptr, *c1_ptr, *Zc1_ptr, *v2_ptr, *c2_ptr, *Zc2_ptr;
        Port *mpP1, *mpP2;

    public:
        static Component *Creator()
        {
            return new MechanicTranslationalSpring("TranslationalSpring");
        }

        MechanicTranslationalSpring(const std::string name) : ComponentC(name)
        {
            //Set member attributes
            mTypeName = "MechanicTranslationalSpring";
            k = 100.0;

            //Add ports to the component
            mpP1 = addPowerPort("P1", "NodeMechanic");
            mpP2 = addPowerPort("P2", "NodeMechanic");

            //Register changable parameters to the HOPSAN++ core
            registerParameter("k", "Spring Coefficient", "[N/m]",  k);
        }


        void initialize()
        {
            v1_ptr = mpP1->getNodeDataPtr(NodeMechanic::VELOCITY);
            c1_ptr = mpP1->getNodeDataPtr(NodeMechanic::WAVEVARIABLE);
            Zc1_ptr = mpP1->getNodeDataPtr(NodeMechanic::CHARIMP);
            v2_ptr = mpP2->getNodeDataPtr(NodeMechanic::VELOCITY);
            c2_ptr = mpP2->getNodeDataPtr(NodeMechanic::WAVEVARIABLE);
            Zc2_ptr = mpP2->getNodeDataPtr(NodeMechanic::CHARIMP);
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes
            v1 = (*v1_ptr);
            lastc1 = (*c1_ptr);
            v2 = (*v2_ptr);
            lastc2 = (*c2_ptr);

            //Spring equations
            Zc = k*mTimestep;
            c1 = lastc2 + 2.0*Zc*v2;
            c2 = lastc1 + 2.0*Zc*v1;

            //Write new values to nodes
            (*c1_ptr) = c1;
            (*Zc1_ptr) = Zc;
            (*c2_ptr) = c2;
            (*Zc2_ptr) = Zc;
        }
    };
}

#endif // MECHANICTRANSLATIONALSPRING_HPP_INCLUDED


