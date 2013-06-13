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

//$Id: HydraulicComponentsInComponentTest.hpp 3640 2011-11-20 15:29:53Z robbr48 $

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
        Component* createSafeComponent(const HString type)
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
        T* getParameterSafeDataPtr(Component *pComp, const HString paramName)
        {
            double* pTmp = 0;
            HString compType = "NULL";

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
        Port *mpSysPort1, *mpSysPort2, *mpSysPortKc1, *mpSysPortKc2, *mpSysPortVolPressureOut;

        // Node data ptrs
        double *mpInternalVolumePressure;

    public:
        static Component *Creator()
        {
            return new HydraulicComponentsInComponentTest();
        }

        void configure()
        {
            // Add Constant Parameters
            addConstant("V", "Volume", "[m^3]", 1e-3, Volume);

            // Add Input Variables
            mpSysPortKc1 = addInputVariable("Kc1", "", "", 1e-11);
            mpSysPortKc2 = addInputVariable("Kc2", "", "", 1e-11);

            // Add external ports
            mpSysPort1 = addSystemPort("P1", "Hydraulic port 1");
            mpSysPort2 = addSystemPort("P2", "Hydraulic port 2");
            mpSysPortVolPressureOut = addOutputVariable("out", "Internal volume pressure", "bar");


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
            connect(mpSysPortKc1, mpOrifice1->getPort("Kc"));
            connect(mpOrifice1->getPort("P2"), mpVolume->getPort("P1"));
            connect(mpVolume->getPort("P2"), mpOrifice2->getPort("P1"));
            connect(mpSysPortKc2, mpOrifice2->getPort("Kc"));
            connect(mpSysPort2, mpOrifice2->getPort("P2"));
        }


        bool initialize(const double startT, const double stopT)     //Important, initialize must have these arguments
        {
            // Propagate constant parameters into respective components
            mpVolume->setParameterValue("V", to_hstring(Volume), true);
            mpInternalVolumePressure = mpVolume->getSafeNodeDataPtr("P2", NodeHydraulic::Pressure);

            if (checkModelBeforeSimulation())
            {
                return ComponentSystem::initialize(startT, stopT);
            }
            else
            {
                stopSimulation();
                return false;
            }

            // Initialize the output signal value
            //! @todo this is not working, value from last simulation remains allways
            mpSysPortVolPressureOut->writeNode(NodeSignal::Value, (*mpInternalVolumePressure));
        }


        void simulate(const double stopTime)
        {
            // Do some magic stuff
            // Note! if you use mTime before the call to simulate below, mTime = previousTime

            // Call the ComponentSystem::simulate() function to increment mTime and simulate all subcomponents
            // Note! Will simulate from mTime to stopTime (this could include multiple timesteps (mTimestep))
            ComponentSystem::simulate(stopTime);

            // Write any output variables after simulation completes
            mpSysPortVolPressureOut->writeNode(NodeSignal::Value, (*mpInternalVolumePressure));
        }
    };
}

#endif // HYDRAULICCOMPONENTINCOMPONENTTEST_HPP_INCLUDED
