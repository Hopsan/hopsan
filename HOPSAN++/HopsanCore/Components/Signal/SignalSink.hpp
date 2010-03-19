//!
//! @file   SignalSink.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-05
//!
//! @brief Contains a Signal Sink Component
//!
//$Id$

#ifndef SIGNALSINK_HPP_INCLUDED
#define SIGNALSINK_HPP_INCLUDED

#include "../../ComponentEssentials.h"

//!
//! @brief
//! @ingroup SignalComponents
//!
class SignalSink : public ComponentSignal
{

private:
    Port *mpIn;

public:
    static Component *Creator()
    {
        std::cout << "running Sink creator" << std::endl;
        return new SignalSink("Sink");
    }


    SignalSink(const string name,
                  const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        mTypeName = "SignalSink";

        mpIn = addReadPort("in", "NodeSignal");
    }


	void initialize()
	{
        //Nothing to initilize
	}


    void simulateOneTimestep()
    {
        //Nothing to do
    }
};

#endif // SIGNALSINK_HPP_INCLUDED
