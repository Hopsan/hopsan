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
    double mStartPressure;
    double mStartFlow;
    double mZc;
    double mAlpha;
    double mVolume;
    double mBulkmodulus;
    Port *mpP1, *mpP2;

    double debug,tid1,tid2;

public:
    // The creator function that is registered when a component lib is loaded into Hopsan
    static Component *Creator()
    {
        return new MyExampleVolume();
    }

    // The Constructor function that is run immediately when a new object of the class is created
    // Use this function to set initial member variable values, and to register Ports, Parameters and Startvalues
    void configure()
    {
        // Set initial member variable values
        mStartPressure = 0.0;
        mStartFlow     = 0.0;
        mBulkmodulus   = 1.0e9;
        mVolume        = 1.0e-3;
        mAlpha         = 0.1;

        // Add ports to the component
        mpP1 = addPowerPort("P1", "NodeHydraulic");
        mpP2 = addPowerPort("P2", "NodeHydraulic");

        // Register component parameters that can be changed by the user
        registerParameter("V", "Volume", "[m^3]",            mVolume);
        registerParameter("Be", "Bulkmodulus", "[Pa]", mBulkmodulus);
        registerParameter("a", "Low pass coeficient to dampen standing delayline waves", "[-]",  mAlpha);
    }


    // The initialize function is called ONCE before simulation begins
    // In this function you can read or write from/to nodes
    void initialize()
    {
        mZc = mBulkmodulus/mVolume*mTimestep/(1-mAlpha); //Need to be updated at simulation start since it is volume and bulk that are set.

        // Write to nodes
        mpP1->writeNode(NodeHydraulic::FLOW,         mStartFlow);
        mpP1->writeNode(NodeHydraulic::PRESSURE,     mStartPressure);
        mpP1->writeNode(NodeHydraulic::WAVEVARIABLE, mStartPressure+mZc*mStartFlow);
        mpP1->writeNode(NodeHydraulic::CHARIMP,      mZc);
        mpP2->writeNode(NodeHydraulic::FLOW,         mStartFlow);
        mpP2->writeNode(NodeHydraulic::PRESSURE,     mStartPressure);
        mpP2->writeNode(NodeHydraulic::WAVEVARIABLE, mStartPressure+mZc*mStartFlow);
        mpP2->writeNode(NodeHydraulic::CHARIMP,      mZc);
    }

    // The simulateOneTimestep() function is called ONCE every time step
    // This function contains the actual component simulation equations
    void simulateOneTimestep()
    {
        // First read the necessary data from nodes
        double q1  = mpP1->readNode(NodeHydraulic::FLOW);
        double c1  = mpP1->readNode(NodeHydraulic::WAVEVARIABLE);
        double q2  = mpP2->readNode(NodeHydraulic::FLOW);
        double c2  = mpP2->readNode(NodeHydraulic::WAVEVARIABLE);

        // Volume equations
        double c10 = c2 + 2.0*mZc * q2;
        double c20 = c1 + 2.0*mZc * q1;

        c1 = mAlpha*c1 + (1.0-mAlpha)*c10;
        c2 = mAlpha*c2 + (1.0-mAlpha)*c20;

        // Write new values back to the nodes
        mpP1->writeNode(NodeHydraulic::WAVEVARIABLE, c1);
        mpP2->writeNode(NodeHydraulic::WAVEVARIABLE, c2);
        mpP1->writeNode(NodeHydraulic::CHARIMP,      mZc);
        mpP2->writeNode(NodeHydraulic::CHARIMP,      mZc);
    }

    // The finalize function is called ONCE after simulation ends
    // Use this function to clean up after yourself (if needed)
    void finalize()
    {

    }
};

}

#endif
