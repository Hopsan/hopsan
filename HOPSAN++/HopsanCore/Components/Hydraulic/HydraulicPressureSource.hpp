//!
//! @file   HydraulicPressureSource.hpp
//! @author FluMeS
//! @date   2009-12-21
//!
//! @brief Contains a Hydraulic Pressure Source Component of C-type
//!
//$Id$

#ifndef HYDRAULICPRESSURESOURCE_HPP_INCLUDED
#define HYDRAULICPRESSURESOURCE_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"

//!
//! @brief
//! @ingroup HydraulicComponents
//!
class HydraulicPressureSource : public ComponentC
{
private:
    double mStartPressure;
    double mStartFlow;
    double mZc;
    double mPressure;
    Port *mpIn, *mpP1;

public:
    static Component *Creator()
    {
        return new HydraulicPressureSource("PressureSource");
    }

    HydraulicPressureSource(const string name,
                                    const double pressure       = 1.0e5,
                                    const double timestep       = 0.001)
        : ComponentC(name, timestep)
    {
        mTypeName = "HydraulicPressureSource";
        mStartPressure  = 0.0;
        mStartFlow      = 0.0;
        mPressure       = pressure;
        mZc             = 0.0;

        mpIn = addReadPort("In", "NodeSignal");
        mpP1 = addPowerPort("P1", "NodeHydraulic");


        registerParameter("P", "Pressure", "Pa", mPressure);
    }


    void initialize()
    {
        //write to nodes
        mpP1->writeNode(NodeHydraulic::PRESSURE, mStartPressure);
        mpP1->writeNode(NodeHydraulic::MASSFLOW, mStartFlow);

    }


    void simulateOneTimestep()
    {
        //Pressure source equation
        double c;
        if (mpIn->isConnected())
        {
            c = mpIn->readNode(NodeSignal::VALUE);         //We have a signal!
        }
        else
        {
            c = mPressure;                                  //No signal, use internal parameter
        }

        //Write new values to nodes
        mpP1->writeNode(NodeHydraulic::WAVEVARIABLE, c);
        mpP1->writeNode(NodeHydraulic::CHARIMP, mZc);
    }
};

#endif // HYDRAULICPRESSURESOURCE_HPP_INCLUDED
