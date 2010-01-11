#ifndef HYDRAULICFLOWSOURCEQ_HPP_INCLUDED
#define HYDRAULICFLOWSOURCEQ_HPP_INCLUDED

#include <iostream>
#include "HopsanCore.h"

class HydraulicFlowSourceQ : public ComponentQ
{
private:
    double mFlow;
    enum {P1,in};

public:
    static Component *Creator()
    {
        std::cout << "running flowsourceQ creator" << std::endl;
        return new HydraulicFlowSourceQ("DefaultFlowSourceQName");
    }

    HydraulicFlowSourceQ(const string name,
                         const double flow     = 1.0e-3,
                         const double timestep = 0.001)
	: ComponentQ(name, timestep)
    {
        mFlow = flow;

        addPort("P1", "NodeHydraulic", P1);
        addPort("in", "NodeSignal", in);

        registerParameter("Flow", "Flöde", "m^3/s", mFlow);
    }


	void initialize()
	{
        //Nothing to initilize
	}


    void simulateOneTimestep()
    {
        //Get the nodes
   		Node* p1_ptr = mPorts[P1].getNodePtr();
        Node* p2_ptr = mPorts[in].getNodePtr();

        //Get variable values from nodes
		double c  = p1_ptr->getData(NodeHydraulic::WAVEVARIABLE);
        double Zc = p1_ptr->getData(NodeHydraulic::CHARIMP);

        //Flow source equations
        double q;
        if (mPorts[in].isConnected())
        {
            q = p2_ptr->getData(NodeSignal::VALUE);         //Control signal exist!
        }
        else
        {
            q = mFlow;              //No control signal, use parameter...
        }
        double p = c + mFlow*Zc;

        //Write new values to nodes
        p1_ptr->setData(NodeHydraulic::MASSFLOW, q);
        p1_ptr->setData(NodeHydraulic::PRESSURE, p);
    }
};

#endif // HYDRAULICFLOWSOURCEQ_HPP_INCLUDED
