//!
//! @file   SignalSecondOrderFilter.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-22
//!
//! @brief Contains a Signal Second Order Filter Component using CoreUtilities
//!
//$Id$

#ifndef SIGNALSECONORDERFILTER_HPP_INCLUDED
#define SIGNALSECONORDERFILTER_HPP_INCLUDED

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

//!
//! @brief
//! @ingroup SignalComponents
//!
class SignalSecondOrderFilter : public ComponentSignal
{

private:
    SecondOrderFilter mFilter;
    double mWnum, mDnum, mWden, mDden, mK;
    double mStartY;
    double mMin, mMax;
    enum {in, out};

public:
    static Component *Creator()
    {
        std::cout << "running Second Order Filter creator" << std::endl;
        return new SignalSecondOrderFilter("Filter");
    }

    SignalSecondOrderFilter(const string name,
                            const double min = -1.5E+300,
                            const double max =  1.5E+300,
                            const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        mTypeName = "SignalSecondOrderFilter";
        mStartY = 0.0;

        mMin = min;
        mMax = max;

        addReadPort("in", "NodeSignal", in);
        addWritePort("out", "NodeSignal", out);

        registerParameter("k", "Gain", "-", mK);
        registerParameter("wnum", "Numerator break frequency", "rad/s", mWnum);
        registerParameter("dnum", "Numerator damp coefficient", "-", mDnum);
        registerParameter("wden", "Denumerator break frequency", "rad/s", mWden);
        registerParameter("dden", "Numerator damp coefficient", "-", mDden);
    }


	void initialize()
	{
//	    double u0 = mPortPtrs[in]->readNode(NodeSignal::VALUE);

        double num[3];
        double den[3];

        num[0] = mK/pow(mWnum, 2);
        num[1] = mK*2.0*mDnum/mWnum;
        num[2] = mK;
        den[0] = 1.0/pow(mWden, 2);
        den[1] = 2.0*mDden/mWden;
        den[2] = 1.0;

	    mFilter.initialize(mTime, mTimestep, num, den, mStartY, mStartY, mMin, mMax);
	    ///TODO: Write out values into node as well? (I think so) This is true for all components
	}


    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double u = mPortPtrs[in]->readNode(NodeSignal::VALUE);

        //Write new values to nodes
        mPortPtrs[out]->writeNode(NodeSignal::VALUE, mFilter.value(u));
    }
};

#endif // SIGNALSECONORDERFILTER_HPP_INCLUDED


