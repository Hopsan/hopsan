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

#include "HopsanCore.h"

class SignalSink : public ComponentSignal
{

private:
    enum {in};

public:
    static Component *Creator()
    {
        std::cout << "running Sink creator" << std::endl;
        return new SignalSink("DefaultSinkName");
    }


    SignalSink(const string name,
                  const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        addReadPort("in", "NodeSignal", in);
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
