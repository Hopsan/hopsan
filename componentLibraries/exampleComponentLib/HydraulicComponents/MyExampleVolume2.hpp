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

// Header guard to avoid inclusion of the same code twice
// Every hpp file need to have its own UNIQUE header guard
#ifndef MYEXAMPLEVOLUME2_H
#define MYEXAMPLEVOLUME2_H

// Include the necessary header files from Hopsan
#include "ComponentEssentials.h"

// Put your component class inside the hopsan namespace (optional)
namespace hopsan {

// Define a new Class that inherits from ComponentC, ComponentQ or ComponentSignal
// This depends on the type of component you want to create, a C, Q or signal component
class MyExampleVolume2 : public ComponentC
{
private:
    // Private member variables
    double mAlpha;
    Port *mpP1, *mpP2, *mpV, *mpBe;

public:
    // The creator function that is registered when a component lib is loaded into Hopsan
    // This static function is mandatory
    static Component *Creator()
    {
        return new MyExampleVolume2();
    }

    // The Configure function that is run ONCE when a new object of the class is created
    // Use this function to set initial member variable values and to register Ports, Parameters and Startvalues
    // This function is mandatory
    void configure()
    {
        // Add ports to the component
        mpP1 = addPowerPort("P1", "NodeHydraulic", "Port1");
        mpP2 = addPowerPort("P2", "NodeHydraulic", "Port2");

        // Add inputVariables, if the ports is not connected the default value is used
        mpV = addInputVariable("V", "Volume", "m^3", 1.0e-3);
        mpBe = addInputVariable("Be", "Bulkmodulus", "Pa", 1.0e9);

        // Register parameters that are constant during simulation
        addConstant("a", "Low pass coefficient to dampen standing delayline waves", "", 0.1, mAlpha);
    }


    // The initialize function is called before simulation begins.
    // It may be called multiple times
    // In this function you can read or write from/to nodes
    // This function is optional but most likely needed
    void initialize()
    {
        // Read from node data ptrs
        const double Be = readInputVariable(mpBe);
        const double V = readInputVariable(mpV);
        double p1, q1, p2, q2;
        readHydraulicPort_pq(mpP1, p1, q1);
        readHydraulicPort_pq(mpP2, p2, q2);

        // Decide initial Zc
        const double Zc = Be/V*mTimestep/(1.0-mAlpha);

        // Write to nodes
        writeHydraulicPort_cZc(mpP1, p2+Zc*q2, Zc);
        writeHydraulicPort_cZc(mpP2, p1+Zc*q1, Zc);
    }

    // The simulateOneTimestep() function is called ONCE every time step
    // This function contains the actual component simulation equations
    // This function is mandatory
    void simulateOneTimestep()
    {
        // First read the necessary data from nodes
        HydraulicNodeDataValueStructT p1, p2;
        readHydraulicPort_all(mpP1, p1);
        readHydraulicPort_all(mpP2, p2);

        // Read from inputVariable node-data ptrs
        const double Be = readInputVariable(mpBe);
        const double V = readInputVariable(mpV);

        // Update Zc, Be and V may have changed
        const double Zc = Be/V*mTimestep/(1.0-mAlpha);

        // Volume equations
        double c10 = p2.c + 2.0*Zc * p2.q;
        double c20 = p1.c + 2.0*Zc * p1.q;
        p1.c = mAlpha*p1.c + (1.0-mAlpha)*c10;
        p2.c = mAlpha*p2.c + (1.0-mAlpha)*c20;

        // Write new values back to the nodes
        writeHydraulicPort_cZc(mpP1, p1.c, p1.Zc);
        writeHydraulicPort_cZc(mpP2, p2.c, p2.Zc);
    }

    // The finalize function is called after simulation ends
    // It may be called multiple times
    // Use this function to clean up of any custom allocations made in Initialize (if needed)
    // This function is optional
    void finalize()
    {
        // Nothing to finalize
    }

    // The deconfigure function is called just before a component is deleted
    // Use it to clean up after any custom allocation in the configure function
    // This function is optional
    void deconfigure()
    {
        // Nothing to deconfigure
    }
};

}

#endif
