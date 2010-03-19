//!
//! @file   SignalLP2Filter.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-21
//!
//! @brief Contains a second order low pass filter
//!
//$Id$

#ifndef SIGNALLP2FILTER_HPP_INCLUDED
#define SIGNALLP2FILTER_HPP_INCLUDED

#include "../../ComponentEssentials.h"

//!
//! @brief
//! @ingroup SignalComponents
//!
class SignalLP2Filter : public ComponentSignal
{

private:
    double mCofrequency;
    double mTimestep;
    TransferFunction Filter;
    Port *mpIn, *mpOut;

public:
    static Component *Creator()
    {
        std::cout << "running lp2 filter creator" << std::endl;
        return new SignalLP2Filter("LP2Filter");
    }

    SignalLP2Filter(const string name,
                 const double cofrequency = 100,
                 const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        mTypeName = "SignalLP2Filter";
        mCofrequency = cofrequency;
        mTimestep = timestep;

        mpIn = addReadPort("in", "NodeSignal");
        mpOut = addWritePort("out", "NodeSignal");

        registerParameter("Frequency", "Cut-Off Frequency", "[rad/s]", mCofrequency);
    }


	void initialize()
	{
	    double num [3] = {1.0, 0.0, 0.0};
	    double den [3] = {1.0, 2.0/mCofrequency, 1.0/pow(mCofrequency,2)};
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

#endif // SIGNALLP2FILTER_HPP_INCLUDED
