/*
 *  FlowSourceQ.hpp
 *  HOPSAN++
 *
 *  Created by Björn Eriksson on 2009-12-20.
 *  Copyright 2009 LiU. All rights reserved.
 *
 */


#ifndef FLOWSOURCEQ_HPP_INCLUDED
#define FLOWSOURCEQ_HPP_INCLUDED

class ComponentFlowSourceQ : public ComponentQ
{

private:
    double mFlow;
    enum {P1};

public:
    ComponentFlowSourceQ(const string name,
                         const double flow     = 1.0e-3,
                         const double timestep = 0.001)
	: ComponentQ(name, timestep)
    {
        mFlow = flow;

        addPort("P1", "NodeHydraulic", P1);

        registerParameter("Flöde", "m^3/s", mFlow);
    }


	void initialize()
	{
        //Nothing to initilize
	}


    void simulateOneTimestep()
    {
        //Get the nodes
   		Node* p1_ptr = mPorts[P1].getNodePtr();

        //Get variable values from nodes
		double c  = p1_ptr->getData(NodeHydraulic::WAVEVARIABLE);
        double Zc = p1_ptr->getData(NodeHydraulic::CHARIMP);

        //Flow source equations
        double q = mFlow;
		double p = c + mFlow*Zc;

        //Write new values to nodes
        p1_ptr->setData(NodeHydraulic::MASSFLOW, q);
        p1_ptr->setData(NodeHydraulic::PRESSURE, p);
    }
};

#endif // FLOWSOURCEQ_HPP_INCLUDED
