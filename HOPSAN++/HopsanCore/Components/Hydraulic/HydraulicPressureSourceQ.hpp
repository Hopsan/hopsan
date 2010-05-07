//!
//! @file   HydraulicPressureSourceQ.hpp
//! @author FluMeS
//! @date   2009-12-21
//!
//! @brief Contains a Hydraulic Pressure Source Component of Q-type
//!
//$Id$

#ifndef HYDRAULICPRESSURESOURCEQ_HPP_INCLUDED
#define HYDRAULICPRESSURESOURCEQ_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"

//!
//! @brief
//! @ingroup HydraulicComponents
//!
class HydraulicPressureSourceQ : public ComponentQ
{
private:
    double mStartPressure;
    double mStartFlow;
    double mPressure;
    Port *mpIn, *mpP1;

public:
    static Component *Creator()
    {
        return new HydraulicPressureSourceQ("PressureSourceQ");
    }

    HydraulicPressureSourceQ(const string name) : ComponentQ(name)
    {
        mTypeName = "HydraulicPressureSourceQ";
        mStartPressure = 0.0;
        mStartFlow     = 0.0;
        mPressure      = 1.0e5;

        mpIn = addReadPort("in", "NodeSignal");
        mpP1 = addPowerPort("P1", "NodeHydraulic");

        registerParameter("P", "Default pressure", "Pa", mPressure);
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
        double q,p;

        if (mpIn->isConnected())
        {
            q = (mpIn->readNode(NodeSignal::VALUE) - c)/Zc;
            p = mpIn->readNode(NodeSignal::VALUE);         //We have a signal!
        }
        else
        {
            q = (mPressure - c)/Zc;
            p = mPressure;                                  //No signal, use internal parameter
        }

        //Write new values to nodes
        mpP1->writeNode(NodeHydraulic::MASSFLOW, q);
        mpP1->writeNode(NodeHydraulic::PRESSURE, p);
    }
};

#endif // HYDRAULICPRESSURESOURCEQ_HPP_INCLUDED
