#ifndef SIGNALSATURATION_HPP_INCLUDED
#define SIGNALSATURATION_HPP_INCLUDED

#include "HopsanCore.h"

class SignalSaturation : public ComponentSignal
{

private:
    double mUpperLimit;
    double mLowerLimit;
    enum {in, out};

public:
    static Component *Creator()
    {
        std::cout << "running Saturation creator" << std::endl;
        return new SignalSaturation("DefaultSaturationName");
    }

    SignalSaturation(const string name,
                     const double upperlimit = 1.0,
                     const double lowerlimit = -1.0,
                     const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        mUpperLimit = upperlimit;
        mLowerLimit = lowerlimit;

        addPort("in", "NodeSignal", in);
        addPort("out", "NodeSignal", out);

        registerParameter("UpperLimit", "Upper Limit", "-", mUpperLimit);
        registerParameter("LowerLimit", "Lower Limit", "-", mLowerLimit);
    }


	void initialize()
	{
        //Nothing to initilize
	}


    void simulateOneTimestep()
    {
        //read fron nodes
   		Node* p1_ptr = mPorts[in].getNodePtr();
   		Node* p2_ptr = mPorts[out].getNodePtr();

        //Get variable values from nodes
        double input = p1_ptr->getData(NodeSignal::VALUE);

        //Gain equations
		double output;
		if (input > mUpperLimit)
		{
		    output = mUpperLimit;
		}
		else if (input < mLowerLimit)
		{
		    output = mLowerLimit;
		}
		else
		{
		    output = input;
		}

        //Write new values to nodes
        p2_ptr->setData(NodeSignal::VALUE, output);
    }
};

#endif // SIGNALSATURATION_HPP_INCLUDED

