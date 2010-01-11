#ifndef SIGNALSUBTRACT_HPP_INCLUDED
#define SIGNALSUBTRACT_HPP_INCLUDED

#include "HopsanCore.h"

class SignalSubtract : public ComponentSignal
{

private:
    enum {in1, in2, out};

public:
    static Component *Creator()
    {
        std::cout << "running Subtraction creator" << std::endl;
        return new SignalSubtract("DefaultSubtractName");
    }

    SignalSubtract(const string name,
                   const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        addPort("in1", "NodeSignal", in1);
        addPort("in2", "NodeSignal", in2);
        addPort("out", "NodeSignal", out);
    }


	void initialize()
	{
        //Nothing to initilize
	}


    void simulateOneTimestep()
    {
        //read fron nodes
   		Node* p1_ptr = mPorts[in1].getNodePtr();
   		Node* p2_ptr = mPorts[in2].getNodePtr();
   		Node* p3_ptr = mPorts[out].getNodePtr();

        //Get variable values from nodes
        double signal1, signal2;

        if (mPorts[in1].isConnected() && mPorts[in2].isConnected())       //Both ports connected
        {
            signal1 = p1_ptr->getData(NodeSignal::VALUE);
            signal2 = p2_ptr->getData(NodeSignal::VALUE);
        }
        else if (mPorts[in1].isConnected() && !mPorts[in2].isConnected())       //Port 1 connected, port 2 disconnected
        {
            signal1 = p1_ptr->getData(NodeSignal::VALUE);
            signal2 = 0;
        }
        else if (!mPorts[in1].isConnected() && mPorts[in2].isConnected())       //Port 2 connected, port 1 disconnected
        {
            signal1 = 0;
            signal2 = p2_ptr->getData(NodeSignal::VALUE);
        }
        else
        {
            signal1 = 0;                                                     //Nothing connected
            signal2 = 0;
        }


        //Gain equations
		double out = signal1 - signal2;

        //Write new values to nodes
        p3_ptr->setData(NodeSignal::VALUE, out);
    }
};

#endif // SIGNALSUBTRACT_HPP_INCLUDED

