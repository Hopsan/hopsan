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

// Header guard to avoid inclusion of the same code twice
// Every hpp file need to have its own UNIQUE header guard
#ifndef MYEXAMPLEVOLUME_H
#define MYEXAMPLEVOLUME_H

// Include the necessary header files from Hopsan
#include "ComponentEssentials.h"

// Put your component class inside the hopsan namespace (optional)
namespace hopsan {

// Define a new Class that inherits from ComponentC, ComponentQ or ComponentS
// This depends on the type of component you want to create, a C, Q or signal component
class MyExampleVolume : public ComponentC
{
private:
    // Private member variables
    double mAlpha;
    double *mpVolume, *mpBulkmodulus;
    Port *mpP1, *mpP2;

public:
    // The creator function that is registered when a component lib is loaded into Hopsan
    // This static function is mandatory
    static Component *Creator()
    {
        return new MyExampleVolume();
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
        addInputVariable("V", "Volume", "m^3", 1.0e-3, &mpVolume);
        addInputVariable("Be", "Bulkmodulus", "Pa", 1.0e9, &mpBulkmodulus);

        // Register parameters that are constant during simulation
        addConstant("a", "Low pass coeficient to dampen standing delayline waves", "", 0.1, mAlpha);
    }


    // The initialize function is called before simulation begins.
    // It may be called multiple times
    // In this function you can read or write from/to nodes
    // This function is optional but most likely needed
    void initialize()
    {
        // Read from node data ptrs
        const double Be = (*mpBulkmodulus);
        const double V = (*mpVolume);

        // Decide initial Zc
        const double Zc = Be/V*mTimestep/(1.0-mAlpha);

        // Set initial values
        const double startPressure1 = mpP1->readNode(NodeHydraulic::Pressure);
        const double startFlow1     = mpP1->readNode(NodeHydraulic::Flow);
        const double startPressure2 = mpP2->readNode(NodeHydraulic::Pressure);
        const double startFlow2     = mpP2->readNode(NodeHydraulic::Flow);

        // Write to nodes
        mpP1->writeNode(NodeHydraulic::WaveVariable, startPressure2+Zc*startFlow2);
        mpP1->writeNode(NodeHydraulic::CharImpedance,      Zc);

        mpP2->writeNode(NodeHydraulic::WaveVariable, startPressure1+Zc*startFlow1);
        mpP2->writeNode(NodeHydraulic::CharImpedance,      Zc);
    }

    // The simulateOneTimestep() function is called ONCE every time step
    // This function contains the actual component simulation equations
    // This function is mandatory
    void simulateOneTimestep()
    {
        // First read the necessary data from nodes
        double q1  = mpP1->readNode(NodeHydraulic::Flow);
        double c1  = mpP1->readNode(NodeHydraulic::WaveVariable);
        double q2  = mpP2->readNode(NodeHydraulic::Flow);
        double c2  = mpP2->readNode(NodeHydraulic::WaveVariable);
        // Read from inputVariable node-data ptrs
        const double Be = (*mpBulkmodulus);
        const double V = (*mpVolume);

        // Update Zc, Be and V may have changed
        const double Zc = Be/V*mTimestep/(1.0-mAlpha);

        // Volume equations
        double c10 = c2 + 2.0*Zc * q2;
        double c20 = c1 + 2.0*Zc * q1;

        c1 = mAlpha*c1 + (1.0-mAlpha)*c10;
        c2 = mAlpha*c2 + (1.0-mAlpha)*c20;

        // Write new values back to the nodes
        mpP1->writeNode(NodeHydraulic::WaveVariable, c1);
        mpP2->writeNode(NodeHydraulic::WaveVariable, c2);
        mpP1->writeNode(NodeHydraulic::CharImpedance, Zc);
        mpP2->writeNode(NodeHydraulic::CharImpedance, Zc);
    }

    // The finalize function is called after simulation ends
    // It may be called multiple times
    // Use this function to clean up of any custom allocations made in Initialize (if needed)
    // This function is optional
    void finalize()
    {
        // Nothing to finalize
    }

    // The deconfigure function is called just before a component is deleated
    // Use it to clean up after any custom allocation in the configure function
    // This function is optional
    void deconfigure()
    {
        // Nothing to deconfigure
    }
};

}

#endif
