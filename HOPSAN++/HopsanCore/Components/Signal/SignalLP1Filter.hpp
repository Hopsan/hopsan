#ifndef SIGNALLP1FILTER_HPP_INCLUDED
#define SIGNALLP1FILTER_HPP_INCLUDED

#include "../../ComponentEssentials.h"

//!
//! @brief
//! @ingroup SignalComponents
//!
class SignalLP1Filter : public ComponentSignal
{

private:
    double mCofrequency;
    double mTimestep;
    TransferFunction Filter;
    Port *mpIn, *mpOut;

public:
    static Component *Creator()
    {
        return new SignalLP1Filter("LP1Filter");
    }

    SignalLP1Filter(const string name) : ComponentSignal(name)
    {
        mTypeName = "SignalLP1Filter";
        mCofrequency = 100;

        mpIn = addReadPort("in", "NodeSignal");
        mpOut = addWritePort("out", "NodeSignal");

        registerParameter("Frequency", "Cut-Off Frequency", "[rad/s]", mCofrequency);
    }


    void initialize()
    {
        double num [3] = {1.0, 0.0, 0.0};
        double den [3] = {1.0, 1.0/mCofrequency, 0.0};
        Filter.initialize(0.0, 0.0, mTime);
        Filter.setCoefficients(num, den, mTimestep);
    }


    void simulateOneTimestep()
    {

        //Get variable values from nodes
        double u = mpIn->readNode(NodeSignal::VALUE);

        //Filter equations
		double y = Filter.getValue(u);

        //Write new values to nodes
        mpOut->writeNode(NodeSignal::VALUE, y);

        //Update filter:
        //Filter.update(u);
    }
};

#endif // SIGNALLP1FILTER_HPP_INCLUDED


