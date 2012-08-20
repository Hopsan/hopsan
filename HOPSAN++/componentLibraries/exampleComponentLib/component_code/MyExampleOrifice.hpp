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
#ifndef MYEXAMPLEORIFICE_H
#define MYEXAMPLEORIFICE_H

// Include the necessary header files from Hopsan
#include "ComponentEssentials.h"

// Put your component class inside the hopsan namespace (optional)
namespace hopsan {

// Define a new Class that inherits from ComponentC, ComponentQ or ComponentS
// This depends on the type of component you want to create, a C, Q or signal component
class MyExampleOrifice : public ComponentQ
{
private:
    // Private member variables
    double mKc;
    Port *mpP1, *mpP2;

public:
    // The creator function that is registered when a component lib is loaded into Hopsan
    static Component *Creator()
    {
        return new MyExampleOrifice();
    }

    // The Constructor function that is run immediately when a new object of the class is created
    // Use this function to set initial member variable values, and to register Ports, Parameters and Startvalues
    void configure()
    {
        // Set initial member variable values
        mKc = 1.0e-11;

        // Add ports to the component
        mpP1 = addPowerPort("P1", "NodeHydraulic");
        mpP2 = addPowerPort("P2", "NodeHydraulic");

        // Register component parameters that can be changed by the user
        registerParameter("Kc", "Pressure-Flow Coefficient", "[m^5/Ns]", mKc);
    }

    // The initialize function is called ONCE before simulation begins
    // In this function you can read or write from/to nodes
    void initialize()
    {
        //Nothing to initialize
    }

    // The simulateOneTimestep() function is called ONCE every time step
    // This function contains the actual component simulation equations
    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double c1 = mpP1->readNode(NodeHydraulic::WAVEVARIABLE);
        double Zc1 = mpP1->readNode(NodeHydraulic::CHARIMP);
        double c2 = mpP2->readNode(NodeHydraulic::WAVEVARIABLE);
        double Zc2 = mpP2->readNode(NodeHydraulic::CHARIMP);

        //Orifice equations
        double q2 = mKc*(c1-c2)/(1.0+mKc*(Zc1+Zc2));
        double q1 = -q2;
        double p1 = c1 + q1*Zc1;
        double p2 = c2 + q2*Zc2;

        //Write new values to nodes
        mpP1->writeNode(NodeHydraulic::PRESSURE, p1);
        mpP1->writeNode(NodeHydraulic::FLOW, q1);
        mpP2->writeNode(NodeHydraulic::PRESSURE, p2);
        mpP2->writeNode(NodeHydraulic::FLOW, q2);
    }

    // The finalize function is called ONCE after simulation ends
    // Use this function to clean up after yourself (if needed)
    void finalize()
    {
        //Nothing to finalize
    }
};

}

#endif // MYEXAMPLEORIFICE_H
