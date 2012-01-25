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

//!
//! @file   SignalSum.hpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2011-04-04
//!
//! @brief Contains a mathematical multiport summator component
//!
//$Id$

// Header guard to avoid inclusion of the same code twice
#ifndef SIGNALSUM_HPP_INCLUDED
#define SIGNALSUM_HPP_INCLUDED

// Include the necessary header files from Hopsan
#include "ComponentEssentials.h"

// Put your component class inside the hopsan namespace (optional)
namespace hopsan {

// Define a new Class that inherits from ComponentC, ComponentQ or ComponentS
// This depends on the type of component you want to create, a C, Q or signal component
class SignalSum : public ComponentSignal
{
private:
    // Private member variables
    size_t mnInputs;
    Port *mpMultiInPort, *mpOutPort;

public:
    // The creator function that is registered when a component lib is loaded into Hopsan
    static Component *Creator()
    {
        return new SignalSum();
    }

    // The Constructor function that is run immediately when a new object of the class is created
    // Use this function to set initial member variable values, and to register Ports, Parameters and Startvalues
    SignalSum() : ComponentSignal()
    {
        // Add ports to the component
        mpMultiInPort = addReadMultiPort("in", "NodeSignal", Port::NOTREQUIRED);
        mpOutPort = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);
    }

    // The initialize function is called ONCE before simulation begins
    // In this function you can read or write from/to nodes
    void initialize()
    {
        // We assume that noone will be disconnecting during simulation
        mnInputs = mpMultiInPort->getNumPorts();
    }

    // The simulateOneTimestep() function is called ONCE every time step
    // This function contains the actual component simulation equations
    void simulateOneTimestep()
    {
        // Initialize sum variable
        double sum = 0;

        // Sum all input ports
        // This index i, represents each subport in the multiport
        for (size_t i=0; i<mnInputs; ++i)
        {
            sum += mpMultiInPort->readNode(NodeSignal::VALUE, i);
        }

        //Write value to output node
        mpOutPort->writeNode(NodeSignal::VALUE, sum);
    }
};

}
#endif
