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
// Every hpp file in your library need to have its own UNIQUE header guard
#ifndef MYEXAMPLEORIFICE_H
#define MYEXAMPLEORIFICE_H

// Include the necessary header files from Hopsan
#include "ComponentEssentials.h"

// Put your component class inside the hopsan namespace (optional)
namespace hopsan {

// Define a new Class that inherits from ComponentC, ComponentQ or ComponentSignal
// This depends on the type of component you want to create, a C, Q or signal component
class MyExampleOrifice : public ComponentQ
{
private:
    // Private member variables
    double *mpKc;
    Port *mpP1, *mpP2;

public:
    // The creator function that is registered when a component lib is loaded into Hopsan
    // This static function is mandatory
    static Component *Creator()
    {
        return new MyExampleOrifice();
    }

    // The Configure function that is run ONCE when a new object of the class is created
    // Use this function to set initial member variable values and to register Ports, Parameters and Startvalues
    // This function is mandatory
    void configure()
    {

        // Add ports to the component
        mpP1 = addPowerPort("P1", "NodeHydraulic", "Port 1");
        mpP2 = addPowerPort("P2", "NodeHydraulic", "Port 2");

        // Add inputVariable, if the port is not connected the default value is used
        addInputVariable("Kc", "Pressure-Flow Coefficient", "[m^5/Ns]", 1.0e-11, &mpKc);
    }

    // The initialize function is called before simulation begins.
    // It may be called multiple times
    // In this function you can read or write from/to nodes
    // This function is optional but most likely needed
    void initialize()
    {
        // Nothing to initialize
    }

    // The simulateOneTimestep() function is called ONCE every time step
    // This function contains the actual component simulation equations
    // This function is mandatory
    void simulateOneTimestep()
    {
        //Get variable values from nodes
        const double c1 = mpP1->readNode(NodeHydraulic::WaveVariable);
        const double Zc1 = mpP1->readNode(NodeHydraulic::CharImpedance);
        const double c2 = mpP2->readNode(NodeHydraulic::WaveVariable);
        const double Zc2 = mpP2->readNode(NodeHydraulic::CharImpedance);
        const double Kc = (*mpKc); //Copy directly from data ptr

        //Orifice equations
        double q2 = Kc*(c1-c2)/(1.0+Kc*(Zc1+Zc2));
        double q1 = -q2;
        double p1 = c1 + q1*Zc1;
        double p2 = c2 + q2*Zc2;

        //Write new values to nodes
        mpP1->writeNode(NodeHydraulic::Pressure, p1);
        mpP1->writeNode(NodeHydraulic::Flow, q1);
        mpP2->writeNode(NodeHydraulic::Pressure, p2);
        mpP2->writeNode(NodeHydraulic::Flow, q2);
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

#endif // MYEXAMPLEORIFICE_H
