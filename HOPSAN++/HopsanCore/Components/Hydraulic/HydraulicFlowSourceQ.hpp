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

//!
//! @brief
//! @ingroup HydraulicComponents
//!
class HydraulicFlowSourceQ : public ComponentQ
{
private:
    double mFlow;
    Port *mpIn, *mpP1;

public:
    static Component *Creator()
    {
        return new HydraulicFlowSourceQ("FlowSourceQ");
    }

    HydraulicFlowSourceQ(const string name) : ComponentQ(name)
    {
        mTypeName = "HydraulicFlowSourceQ";
        mFlow = 1.0e-3;

        mpIn = addReadPort("in", "NodeSignal");
        mpP1 = addPowerPort("P1", "NodeHydraulic");

        registerParameter("Flow", "Flow", "m^3/s", mFlow);
    }


    void initialize()
    {
        //Nothing to initilize
    }


    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double c  = mpP1->readNode(NodeHydraulic::WAVEVARIABLE);
        double Zc = mpP1->readNode(NodeHydraulic::CHARIMP);

        //Flow source equations
        double q;
        if (mpIn->isConnected())
        {
            q = mpIn->readNode(NodeSignal::VALUE);         //Control signal exist!
        }
        else
        {
            q = mFlow;              //No control signal, use parameter...
        }
        double p = c + q*Zc;

        //Write new values to nodes
        mpP1->writeNode(NodeHydraulic::MASSFLOW, q);
        mpP1->writeNode(NodeHydraulic::PRESSURE, p);
    }

    void finalize()
    {

    }
};

#endif // HYDRAULICFLOWSOURCEQ_HPP_INCLUDED
