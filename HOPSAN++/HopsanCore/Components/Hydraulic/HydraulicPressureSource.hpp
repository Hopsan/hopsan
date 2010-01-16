//!
//! @file   HydraulicPressureSource.hpp
//! @author <FluMeS>
//! @date   2009-12-21
//!
//! @brief Contains a Hydraulic Pressure Source Component of C-type
//!
//$Id$

#ifndef HYDRAULICPRESSURESOURCE_HPP_INCLUDED
#define HYDRAULICPRESSURESOURCE_HPP_INCLUDED

#include <iostream>
#include "HopsanCore.h"

class HydraulicPressureSource : public ComponentC
{
private:
    double mStartPressure;
    double mStartFlow;
    double mZc;
    double mPressure;
    enum {P1,in};

public:
    static Component *Creator()
    {
        std::cout << "running pressuresource creator" << std::endl;
        return new HydraulicPressureSource("DefaultPressureSourceName");
    }

    HydraulicPressureSource(const string name,
                                    const double pressure       = 1.0e5,
                                    const double timestep       = 0.001)
        : ComponentC(name, timestep)
    {
        mStartPressure  = 0.0;
        mStartFlow      = 0.0;
        mPressure       = pressure;
        mZc             = 0.0;

        addPowerPort("P1", "NodeHydraulic", P1);
        addReadPort("in", "NodeSignal", in);

        registerParameter("P", "Pressure", "Pa", mPressure);
    }


    void initialize()
    {
        //write to nodes
        mPortPtrs[P1]->WriteNode(NodeHydraulic::PRESSURE, mStartPressure);
        mPortPtrs[P1]->WriteNode(NodeHydraulic::MASSFLOW, mStartFlow);

    }


    void simulateOneTimestep()
    {
        //Pressure source equation
        double p;
        if (mPortPtrs[in]->isConnected())
        {
            p = mPortPtrs[in]->ReadNode(NodeSignal::VALUE);         //We have a signal!
        }
        else
        {
            p = mPressure;                                  //No signal, use internal parameter
        }

        //Write new values to nodes
        mPortPtrs[P1]->WriteNode(NodeHydraulic::WAVEVARIABLE, p);
        mPortPtrs[P1]->WriteNode(NodeHydraulic::CHARIMP, mZc);
    }
};

#endif // HYDRAULICPRESSURESOURCE_HPP_INCLUDED
