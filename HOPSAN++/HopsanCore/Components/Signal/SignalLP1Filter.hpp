#ifndef SIGNALLP1FILTER_HPP_INCLUDED
#define SIGNALLP1FILTER_HPP_INCLUDED

#include "HopsanCore.h"

class SignalLP1Filter : public ComponentSignal
{

private:
    double mCofrequency;
    double mTimestep;
    TransferFunction Filter;
    enum {in, out};

public:
    static Component *Creator()
    {
        std::cout << "running lp1 filter creator" << std::endl;
        return new SignalLP1Filter("DefaultLP1FilterName");
    }

    SignalLP1Filter(const string name,
                 const double cofrequency = 100,
                 const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        mCofrequency = cofrequency;
        mTimestep = timestep;

        addReadPort("in", "NodeSignal", in);
        addWritePort("out", "NodeSignal", out);

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
        double u = mPortPtrs[in]->readNode(NodeSignal::VALUE);

        //Filter equations
		double y = Filter.getValue(u);

        //Write new values to nodes
        mPortPtrs[out]->writeNode(NodeSignal::VALUE, y);

        //Update filter:
        //Filter.update(u);
    }
};

#endif // SIGNALLP1FILTER_HPP_INCLUDED


