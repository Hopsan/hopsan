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
    Port *mpIn, *mpOut;

public:
    static Component *Creator()
    {
        std::cout << "running Second Order Filter creator" << std::endl;
        return new SignalSecondOrderFilter("Filter");
    }

    SignalSecondOrderFilter(const string name) : ComponentSignal(name)
    {
        mTypeName = "SignalSecondOrderFilter";
        mStartY = 0.0;

        mMin = -1.5E+300;
        mMax = 1.5E+300;

        mK = 1.0;
        mWnum = 1e10;
        mDnum = 1.0;
        mWden = 1.0*2.0*3.1415;
        mDden = 0.7;

        mpIn = addReadPort("in", "NodeSignal");
        mpOut = addWritePort("out", "NodeSignal");

        registerParameter("k", "Gain", "-", mK);
        registerParameter("wnum", "Numerator break frequency", "rad/s", mWnum);
        registerParameter("dnum", "Numerator damp coefficient", "-", mDnum);
        registerParameter("wden", "Denumerator break frequency", "rad/s", mWden);
        registerParameter("dden", "Numerator damp coefficient", "-", mDden);
        registerParameter("min", "Lower limit for output", "-", mMin);
        registerParameter("max", "Upper limit for output", "-", mMax);
    }


    void initialize()
    {
        //double u0 = mpIn->readNode(NodeSignal::VALUE);

        double num[3];
        double den[3];

        num[0] = mK/pow(mWnum, 2);
        num[1] = mK*2.0*mDnum/mWnum;
        num[2] = mK;
        den[0] = 1.0/pow(mWden, 2);
        den[1] = 2.0*mDden/mWden;
        den[2] = 1.0;

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

#endif // SIGNALSECONORDERFILTER_HPP_INCLUDED


