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

#ifndef MECHANICTRANSLATIONALSPRING_HPP_INCLUDED
#define MECHANICTRANSLATIONALSPRING_HPP_INCLUDED

#include "ComponentEssentials.h"

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
        double *mpND_f1, *mpND_f2, *mpND_v1, *mpND_c1, *mpND_Zc1, *mpND_v2, *mpND_c2, *mpND_Zc2;
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
            mpND_f1 =  getSafeNodeDataPtr(mpP1, NodeMechanic::FORCE);
            mpND_f2 =  getSafeNodeDataPtr(mpP2, NodeMechanic::FORCE);
            mpND_v1 = getSafeNodeDataPtr(mpP1, NodeMechanic::VELOCITY);
            mpND_c1 = getSafeNodeDataPtr(mpP1, NodeMechanic::WAVEVARIABLE);
            mpND_Zc1 = getSafeNodeDataPtr(mpP1, NodeMechanic::CHARIMP);
            mpND_v2 = getSafeNodeDataPtr(mpP2, NodeMechanic::VELOCITY);
            mpND_c2 = getSafeNodeDataPtr(mpP2, NodeMechanic::WAVEVARIABLE);
            mpND_Zc2 = getSafeNodeDataPtr(mpP2, NodeMechanic::CHARIMP);

            Zc = k*mTimestep;

            //! @todo Is this correct? Ask Petter!
            //(*mpND_c1) = (*mpND_f2)+2.0*Zc*(*mpND_v2);
            //(*mpND_c2) = (*mpND_f1)+2.0*Zc*(*mpND_v1);
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


