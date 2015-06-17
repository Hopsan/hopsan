/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

// Header guard to avoid inclusion of the same code twice
// Every hpp file need to have its own UNIQUE header guard
#ifndef MYEXAMPLEVOLUME_H
#define MYEXAMPLEVOLUME_H

// Include the necessary header files from Hopsan
#include "ComponentEssentials.h"

// Put your component class inside the hopsan namespace (optional)
namespace hopsan {

// Define a new Class that inherits from ComponentC, ComponentQ or ComponentSignal
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
        addConstant("a", "Low pass coefficient to dampen standing delayline waves", "", 0.1, mAlpha);
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
