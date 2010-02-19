//!
//! @file   SignalTimeDelay.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-22
//!
//! @brief Contains a Signal Time Delay Component
//!
//$Id$

#ifndef SIGNALTIMEDELAY_HPP_INCLUDED
#define SIGNALTIMEDELAY_HPP_INCLUDED

#include "../../ComponentEssentials.h"

//!
//! @brief
//! @ingroup SignalComponents
//!
class SignalTimeDelay : public ComponentSignal
{

private:
    double mStartY, mTimeDelay;
    Delay mDelay;
    enum {in, out};

public:
    static Component *Creator()
    {
        std::cout << "running TimeDelay creator" << std::endl;
        return new SignalTimeDelay("DefaultTimeDelayName");
    }

    SignalTimeDelay(const string name,
                    const double timedelay = 1.0,
                    const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        mTypeName = "SignalTimeDelay";
        mStartY = 0.0;
        mTimeDelay = timedelay;

        registerParameter("TD", "Time delay", "s", mTimeDelay);

        addReadPort("in", "NodeSignal", in);
        addWritePort("out", "NodeSignal", out);
    }


	void initialize()
	{
	    mDelay.initialize(mTime, mStartY);
        mDelay.setTimeDelay(mTimeDelay, mTimestep, mStartY);
	    ///TODO: Write out values into node as well? (I think so) This is true for all components
	}


    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double u = mPortPtrs[in]->readNode(NodeSignal::VALUE);
mDelay.update(u);
        //Write new values to nodes
        mPortPtrs[out]->writeNode(NodeSignal::VALUE, mDelay.value(u));
    }
};

#endif // SIGNALTIMEDELAY_HPP_INCLUDED


