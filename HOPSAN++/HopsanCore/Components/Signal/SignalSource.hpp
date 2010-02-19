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
    enum {out};

public:
    static Component *Creator()
    {
        std::cout << "running Source creator" << std::endl;
        return new SignalSource("DefaultSourceName");
    }


    SignalSource(const string name,
                    const double value    = 1.0,
                    const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        mTypeName = "SignalSource";
        mValue = value;

        addWritePort("out", "NodeSignal", out);

        registerParameter("Value", "Värde", "-", mValue);
    }


	void initialize()
	{
	    //Initialize value to the node
        mPortPtrs[out]->writeNode(NodeSignal::VALUE, mValue);
	}


    void simulateOneTimestep()
    {
        //Write new values to nodes
        mPortPtrs[out]->writeNode(NodeSignal::VALUE, mValue);
    }
};

#endif // SIGNALSOURCE_HPP_INCLUDED
