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

#include "HopsanCore.h"

class SignalLP2Filter : public ComponentSignal
{

private:
    double mCofrequency;
    double mTimestep;
    TransferFunction Filter;
    enum {in, out};

public:
    static Component *Creator()
    {
        std::cout << "running lp2 filter creator" << std::endl;
        return new SignalLP2Filter("DefaultLP2FilterName");
    }

    SignalLP2Filter(const string name,
                 const double cofrequency = 100,
                 const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        mTypeName = "SignalLP2Filter";
        mCofrequency = cofrequency;
        mTimestep = timestep;

        addReadPort("in", "NodeSignal", in);
        addWritePort("out", "NodeSignal", out);

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
        double u = mPortPtrs[in]->readNode(NodeSignal::VALUE);

        //Filter equations
		double y = Filter.getValue(u);

        //Write new values to nodes
        mPortPtrs[out]->writeNode(NodeSignal::VALUE, y);

        //Update filter:
        //Filter.update(u);
    }
};

#endif // SIGNALLP2FILTER_HPP_INCLUDED
