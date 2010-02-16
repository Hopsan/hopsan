//!
//! @file   HydraulicFlowSourceQ.hpp
//! @author FluMeS
//! @date   2009-12-21
//!
//! @brief Contains a Hydraulic Flow Source Component of Q-type
//!
//$Id$

#ifndef HYDRAULICFLOWSOURCEQ_HPP_INCLUDED
#define HYDRAULICFLOWSOURCEQ_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"

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
        mTypeName = "HydraulicFlowSourceQ";
        mFlow = flow;

        addPowerPort("P1", "NodeHydraulic", P1);
        addReadPort("in", "NodeSignal", in);

        registerParameter("Flow", "Flöde", "m^3/s", mFlow);
    }


	void initialize()
	{
        //Nothing to initilize
	}


    void simulateOneTimestep()
    {
        //Get variable values from nodes
		double c  = mPortPtrs[P1]->readNode(NodeHydraulic::WAVEVARIABLE);
        double Zc = mPortPtrs[P1]->readNode(NodeHydraulic::CHARIMP);

        //Flow source equations
        double q;
        if (mPortPtrs[in]->isConnected())
        {
            q = mPortPtrs[in]->readNode(NodeSignal::VALUE);         //Control signal exist!
        }
        else
        {
            q = mFlow;              //No control signal, use parameter...
        }
        double p = c + mFlow*Zc;

        //Write new values to nodes
        mPortPtrs[P1]->writeNode(NodeHydraulic::MASSFLOW, q);
        mPortPtrs[P1]->writeNode(NodeHydraulic::PRESSURE, p);
    }
};

#endif // HYDRAULICFLOWSOURCEQ_HPP_INCLUDED
