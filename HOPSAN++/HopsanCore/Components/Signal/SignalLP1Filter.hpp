#ifndef SIGNALLP1FILTER_HPP_INCLUDED
#define SIGNALLP1FILTER_HPP_INCLUDED

#include "HopsanCore.h"

class SignalLP1Filter : public ComponentSignal
{

private:
    double mcofrequency;
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
        mcofrequency = cofrequency;
        mTimestep = timestep;

        addPort("in", "NodeSignal", in);
        addPort("out", "NodeSignal", out);

        registerParameter("Frequency", "Cut-Off Frequency", "[rad/s]", mcofrequency);
    }


	void initialize()
	{
	    double num [3] = {1.0, 0.0, 0.0};
	    double den [3] = {1.0, 1.0/mcofrequency, 0.0};
        Filter.setCoefficients(num, den, mTimestep);
	}


    void simulateOneTimestep()
    {
        //read fron nodes
   		Node* p1_ptr = mPorts[in].getNodePtr();
   		Node* p2_ptr = mPorts[out].getNodePtr();

        //Get variable values from nodes
        double u = p1_ptr->getData(NodeSignal::VALUE);

        //Filter equations
		double y = Filter.filter(u);

        //Write new values to nodes
        p2_ptr->setData(NodeSignal::VALUE, y);
    }
};

#endif // SIGNALLP1FILTER_HPP_INCLUDED


