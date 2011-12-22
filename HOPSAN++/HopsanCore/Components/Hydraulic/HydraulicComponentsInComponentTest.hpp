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
#include "HydraulicLaminarOrifice.hpp"
#include "HydraulicVolume.hpp"
#include "ComponentSystem.h"
#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief A hydraulic laminar orifice component
    //! @ingroup HydraulicComponents
    //!
    class HydraulicComponentsInComponentTest : public ComponentSystem
    {
    private:
        //Parameters
        double Volume;

        //Components
        HydraulicLaminarOrifice *mpOrifice1;
        HydraulicVolume *mpVolume;
        HydraulicLaminarOrifice *mpOrifice2;

        //External ports
        Port *mpSysPort1, *mpSysPort2, *mpSysPort3, *mpSysPort4;

    public:
        static Component *Creator()
        {
            return new HydraulicComponentsInComponentTest();
        }

        HydraulicComponentsInComponentTest() : ComponentSystem()
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
            mpOrifice1 = new HydraulicLaminarOrifice();
            addComponent(mpOrifice1);
            mpOrifice1->setName("TheFirstOrifice");             //Names are optional (not used yet)

            mpVolume = new HydraulicVolume();
            addComponent(mpVolume);
            mpVolume->setName("TheVolume");

            mpOrifice2 = new HydraulicLaminarOrifice();
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


        bool initialize(const double startT, const double stopT, const size_t nSamples)     //Important, initialize must have these arguments
        {
            //Set parameters
            mpVolume->setParameterValue("V", to_string(Volume), true);

            return ComponentSystem::initialize(startT, stopT, nSamples);
        }


        void simulateOneTimestep()
        {
            //Don't do anything, just call the ComponentSystem::simulate() function
            simulate(mTime, mTime+mTimestep);
        }
    };
}

#endif // HYDRAULICCOMPONENTINCOMPONENTTEST_HPP_INCLUDED
