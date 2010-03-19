//!
//! @file   SignalSource.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-05
//!
//! @brief Contains a Signal Source Component
//!
//$Id$

#ifndef SIGNALSOURCE_HPP_INCLUDED
#define SIGNALSOURCE_HPP_INCLUDED

#include "../../ComponentEssentials.h"

//!
//! @brief
//! @ingroup SignalComponents
//!
class SignalSource : public ComponentSignal
{

private:
    double mValue;
    Port *mpOut;

public:
    static Component *Creator()
    {
        std::cout << "running Source creator" << std::endl;
        return new SignalSource("Source");
    }


    SignalSource(const string name,
                 const double value    = 1.0,
                 const double timestep = 0.001)
                     : ComponentSignal(name, timestep)
    {
        mTypeName = "SignalSource";
        mValue = value;

        mpOut = addWritePort("out", "NodeSignal");

        registerParameter("Value", "Source Value", "-", mValue);
    }


    void initialize()
    {
        //Initialize value to the node
        mpOut->writeNode(NodeSignal::VALUE, mValue);
    }


    void simulateOneTimestep()
    {
        //Write new values to nodes
        mpOut->writeNode(NodeSignal::VALUE, mValue);
    }
};

#endif // SIGNALSOURCE_HPP_INCLUDED
