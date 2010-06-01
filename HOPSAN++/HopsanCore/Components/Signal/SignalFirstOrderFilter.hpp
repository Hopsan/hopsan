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

#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

//!
//! @brief
//! @ingroup SignalComponents
//!
class SignalFirstOrderFilter : public ComponentSignal
{

private:
    FirstOrderFilter mFilter;
    double mWnum, mWden, mK;
    double mStartY;
    double mMin, mMax;
    Port *mpIn, *mpOut;

public:
    static Component *Creator()
    {
        return new SignalFirstOrderFilter("Filter");
    }

    SignalFirstOrderFilter(const string name) : ComponentSignal(name)
    {
        mTypeName = "SignalFirstOrderFilter";
        mStartY = 0.0;

        mMin = -1.5E+300;
        mMax = 1.5E+300;

        mpIn = addReadPort("in", "NodeSignal");
        mpOut = addWritePort("out", "NodeSignal");

        registerParameter("k", "Gain", "-", mK);
        registerParameter("wnum", "Numerator break frequency", "rad/s", mWnum);
        registerParameter("wden", "Denominator break frequency", "rad/s", mWden);
        registerParameter("min", "Output Lower limit", "-", mMin);
        registerParameter("max", "Output Upper limit", "-", mMax);
    }


    void initialize()
    {
        double num[2];
        double den[2];

        num[0] = mK/mWnum;
        num[1] = mK;
        den[0] = 1.0/mWden;
        den[1] = 1.0;

        mFilter.initialize(mTime, mTimestep, num, den, mStartY, mStartY, mMin, mMax);

        //Writes out the value for time "zero"
        mpOut->writeNode(NodeSignal::VALUE, mStartY);
    }


    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double u = mpIn->readNode(NodeSignal::VALUE);

        //Write new values to nodes
        mpOut->writeNode(NodeSignal::VALUE, mFilter.value(u));
    }
};

#endif // SIGNALFIRSTORDERFILTER_HPP_INCLUDED


