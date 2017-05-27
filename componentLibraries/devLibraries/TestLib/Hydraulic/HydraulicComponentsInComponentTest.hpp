/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//$Id$

#ifndef HYDRAULICCOMPONENTINCOMPONENTTEST_HPP_INCLUDED
#define HYDRAULICCOMPONENTINCOMPONENTTEST_HPP_INCLUDED

#include <iostream>
#include "ComponentSystem.h"
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "HopsanEssentials.h"
#include "ComponentUtilities/HopsanPowerUser.h"

namespace hopsan {

    class HydraulicComponentsInComponentTest : public ComponentSystem
    {
    private:
        // Constants
        double mVolume;

        // Components
        Component *mpOrifice1, *mpVolume, *mpOrifice2, *mpSinus;

        // External port pointers
        Port *mpSysPort1, *mpSysPort2, *mpSysPortKc1, *mpSysPortKc2, *mpSysPortVolPressureOut, *mpSinOut;

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
            addConstant("V", "Volume", "m^3", 1e-3, mVolume);

            // Add Input Variables
            mpSysPortKc1 = addInputVariable("Kc1", "", "", 1e-11);
            mpSysPortKc2 = addInputVariable("Kc2", "", "", 1e-11);

            // Add external ports
            mpSysPort1 = addPowerPort("P1", "NodeHydraulic", "Hydraulic port 1");
            mpSysPort2 = addPowerPort("P2", "NodeHydraulic", "Hydraulic port 2");
            mpSysPortVolPressureOut = addOutputVariable("out", "Internal volume pressure", "Pa");
            mpSinOut = addOutputVariable("sinOut", "Unrelated sinus wave", "");

            // Create and add sub components
            mpOrifice1 = createSafeComponent(this, "HydraulicLaminarOrifice");
            addComponent(mpOrifice1);
            mpOrifice1->setName("TheFirstOrifice");             //Names are not required, but recommended

            mpVolume = createSafeComponent(this, "HydraulicVolume");
            addComponent(mpVolume);
            mpVolume->setName("TheVolume");
            mpVolume->setConstantValue("V", "V");

            mpOrifice2 = createSafeComponent(this, "HydraulicLaminarOrifice");
            addComponent(mpOrifice2);
            mpOrifice2->setName("TheSecondOrifice");

            mpSinus = createSafeComponent(this, "SignalSineWave");
            addComponent(mpSinus);
            mpSinus->setName("TheSinus");

            // Create sub component connections
            connect(mpSysPortKc1,               mpOrifice1->getPort("Kc"));
            connect(mpSysPortKc2,               mpOrifice2->getPort("Kc"));

            connect(mpSysPort1,                 mpOrifice1->getPort("P1"));
            connect(mpOrifice1->getPort("P2"),  mpVolume->getPort("P1"));
            connect(mpVolume->getPort("P2"),    mpOrifice2->getPort("P1"));
            connect(mpSysPort2,                 mpOrifice2->getPort("P2"));

            connect(mpSinus->getPort("out"),    mpSinOut);

            // Setup default startvalues in ports
            mpSinus->setDefaultStartValue("t_start", "Value", 0.35);

            // Turn off warnings for system parameters (since they may not be directly used in sub components)
            mWarnIfUnusedSystemParameters = false;
        }

        // preInitialize (optional) is called in all components before initialize (in any component) begins
        bool preInitialize()
        {
            const double v = mpSinus->getDefaultStartValue("t_start", "Value")*2.0;

            // Manually set new default startvalues (you may have calculated them)
            mpSinus->setDefaultStartValue("t_start", "Value", v);

            return true;
        }

        // Overload the ComponentSystem initialize(startT, stopT) function
        // Important! initialize must have these arguments (const double startT, const double stopT),
        // otherwise the wrong function will be overloaded
        bool initialize(const double startT, const double stopT)
        {
            // Note! It is to late to use setDefaultStartValue() here, but setting constants will work
            // Example, you may want to calculate internal constants, you can set them like this:
            // mpVolume->setConstantValue("V", calculatedVolume);

            // Get a node data pointer to the internal volume pressure,
            // We can read (and write) the pointer, but DO NOT write to it if the port is connected
            mpInternalVolumePressure = mpVolume->getSafeNodeDataPtr("P2", NodeHydraulic::Pressure);

            // We check that all submodels have been connected OK first, before we initialize the simulation
            if (checkModelBeforeSimulation())
            {
                // Here we call the actual initialize(startT, stopT) function, to initialize all submodels
                bool isOK = ComponentSystem::initialize(startT, stopT);

                // If initialization is successful then proceed with setting initial output values
                if (isOK)
                {
                    std::cout << (*mpInternalVolumePressure) << " " << mpSysPortVolPressureOut->readNode(NodeSignal::Value) << std::endl;
                    // Initialize the output signal value, in this case we read from the volume pressure node data pointer)
                    mpSysPortVolPressureOut->writeNode(NodeSignal::Value, (*mpInternalVolumePressure));

                    return true;
                }
            }

            // If there was a problem in submodel initialization then we will get here
            // we must abort the simulation and return false
            stopSimulation();
            return false;
        }


        void simulate(const double stopTime)
        {
            // Do some magic calculations here if you wish
            // Note! if you use mTime before the call to simulate below, mTime = previousTime

            // Call the ComponentSystem::simulate() function to increment mTime and simulate all subcomponents
            // Note! Will simulate from mTime to stopTime (this could include multiple timesteps (mTimestep))
            ComponentSystem::simulate(stopTime);

            // Write any output variables after the simulation step when all submodels have finished
            mpSysPortVolPressureOut->writeNode(NodeSignal::Value, (*mpInternalVolumePressure));
        }
    };
}

#endif // HYDRAULICCOMPONENTINCOMPONENTTEST_HPP_INCLUDED
