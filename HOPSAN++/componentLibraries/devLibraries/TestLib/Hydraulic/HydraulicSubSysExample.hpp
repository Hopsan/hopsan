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

#ifndef HYDRAULICCOMPONENTINCOMPONENTTEST_HPP_INCLUDED
#define HYDRAULICCOMPONENTINCOMPONENTTEST_HPP_INCLUDED

#include <iostream>
#include "ComponentSystem.h"
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "HopsanEssentials.h"

namespace hopsan {

    //!
    //! @brief A hydraulic laminar orifice component
    //! @ingroup HydraulicComponents
    //!
    class HydraulicComponentsInComponentTest : public ComponentSystem
    {
    private:
        // Parameters
        double Volume;

        // Components
        Component *mpOrifice1, *mpVolume, *mpOrifice2;

        //! @brief Helpfunction to create components and abort safely if that fails
        //! @returns Pointer to created component or dummy
        Component* createSafeComponent(const std::string type)
        {
            Component* pComp = getHopsanEssentials()->createComponent(type.c_str());
            if (pComp == 0)
            {
                addErrorMessage("Could not create subcomponent: " + type);
                pComp = getHopsanEssentials()->createComponent("DummyComponent");
                stopSimulation();
            }
            return pComp;
        }

        //! @brief Helpfunction to safely get the internal parameter data ptr from a subcomponent, the type needs to be known
        //! If parameter or component NULL, then error message instead of crash
        //! @note circumvents the ordinary parameter system, use only if you know what you are doing
        //! @returns A pointer to the parameter or a dummy parameter (to avoid crash on further use)
        template<typename T>
        T* getParameterSafeDataPtr(Component *pComp, const std::string paramName)
        {
            double* pTmp = 0;
            std::string compType = "NULL";

            // First handle if component ptr is null
            if (pComp != 0)
            {
                pTmp = static_cast<T*>(pComp->getParameterDataPtr(paramName));
                compType = pComp->getTypeName();
            }

            // Now check if we found the parameter, if not return dummy, error message and stop simulation
            if (pTmp == 0)
            {
                addErrorMessage("Could not get parameter data ptr from subcomponent: " + compType);
                pTmp = new T;
                stopSimulation();
            }
            return pTmp;
        }

        // External port pointers
        Port *mpSysPort1, *mpSysPort2, *mpSysPort3, *mpSysPort4;

    public:
        static Component *Creator()
        {
            return new HydraulicComponentsInComponentTest();
        }

        void configure()
        {
            //Initialize parameters
            Volume = 1e-3;


            //Register parameters
            registerParameter("V", "Volume", "[m^3]", Volume);


            //Initialize external ports
            mpSysPort1 = addSystemPort("P1");
            mpSysPort2 = addSystemPort("P2");
            mpSysPort3 = addSystemPort("Kc1");
            mpSysPort4 = addSystemPort("Kc2");


            //Initialize sub components
            mpOrifice1 = createSafeComponent("HydraulicLaminarOrifice");
            addComponent(mpOrifice1);
            mpOrifice1->setName("TheFirstOrifice");             //Names are optional (not used yet)

            mpVolume = createSafeComponent("HydraulicVolume");
            addComponent(mpVolume);
            mpVolume->setName("TheVolume");

            mpOrifice2 = createSafeComponent("HydraulicLaminarOrifice");
            addComponent(mpOrifice2);
            mpOrifice2->setName("TheSecondOrifice");

            //Initialize connections
            connect(mpSysPort1, mpOrifice1->getPort("P1"));
            connect(mpSysPort3, mpOrifice1->getPort("Kc"));
            connect(mpOrifice1->getPort("P2"), mpVolume->getPort("P1"));
            connect(mpVolume->getPort("P2"), mpOrifice2->getPort("P1"));
            connect(mpSysPort4, mpOrifice2->getPort("Kc"));
            connect(mpSysPort2, mpOrifice2->getPort("P2"));
        }


        bool initialize(const double startT, const double stopT)     //Important, initialize must have these arguments
        {
            //Set parameters
            mpVolume->setParameterValue("V", to_string(Volume), true);

            return ComponentSystem::initialize(startT, stopT);
        }


        void simulateOneTimestep()
        {
            //Don't do anything, just call the ComponentSystem::simulate() function
            simulate(mTime);
        }
    };
}

#endif // HYDRAULICCOMPONENTINCOMPONENTTEST_HPP_INCLUDED
