//!
//! @file   SignalFirstOrderFilter.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-22
//!
//! @brief Contains a Signal First Order Filter Component using CoreUtilities
//!
//$Id$

#ifndef SIGNALFIRSTORDERFILTER_HPP_INCLUDED
#define SIGNALFIRSTORDERFILTER_HPP_INCLUDED

#include "HopsanCore.h"
#include "CoreUtilities/FirstOrderFilter.h"

class SignalFirstOrderFilter : public ComponentSignal
{

private:
    FirstOrderFilter mFilter;
    double mWnum, mWden, mK;
    double mStartY;
    double mMin, mMax;
    enum {in, out};

public:
    static Component *Creator()
    {
        std::cout << "running First Order Filter creator" << std::endl;
        return new SignalFirstOrderFilter("DefaultFilterName");
    }

    SignalFirstOrderFilter(const string name,
                           const double min = -1.5E+300,
                           const double max =  1.5E+300,
                           const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        mStartY = 0.0;

        mMin = min;
        mMax = max;

        addReadPort("in", "NodeSignal", in);
        addWritePort("out", "NodeSignal", out);

        registerParameter("k", "Gain", "-", mK);
        registerParameter("wnum", "Numerator break frequency", "rad/s", mWnum);
        registerParameter("wden", "Denumerator break frequency", "rad/s", mWden);
    }


	void initialize()
	{
	    double u0 = mPortPtrs[in]->readNode(NodeSignal::VALUE);

        double num[2];
        double den[2];

        num[0] = mK/mWnum;
        num[1] = mK;
        den[0] = 1.0/mWden;
        den[1] = 1.0;

	    mFilter.initialize(mTime, mTimestep, num, den, u0, mStartY, mMin, mMax);
	    ///TODO: Write out values into node as well? (I think so) This is true for all components
	}


    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double u = mPortPtrs[in]->readNode(NodeSignal::VALUE);

        //Filter equation
        //Get variable values from nodes

        //Write new values to nodes
        mPortPtrs[out]->writeNode(NodeSignal::VALUE, mFilter.value(u));
    }
};

#endif // SIGNALFIRSTORDERFILTER_HPP_INCLUDED


