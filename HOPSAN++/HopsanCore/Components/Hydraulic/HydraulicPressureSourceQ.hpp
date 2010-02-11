//!
//! @file   HydraulicPressureSourceQ.hpp
//! @author <FluMeS>
//! @date   2009-12-21
//!
//! @brief Contains a Hydraulic Pressure Source Component of Q-type
//!
//$Id$

#ifndef HYDRAULICPRESSURESOURCEQ_HPP_INCLUDED
#define HYDRAULICPRESSURESOURCEQ_HPP_INCLUDED

#include <iostream>
#include "HopsanCore.h"

class HydraulicPressureSourceQ : public ComponentQ
{
private:
    double mStartPressure;
    double mStartFlow;
    double mPressure;
    enum {P1,in};

public:
    static Component *Creator()
    {
        std::cout << "running pressureSourceQ creator" << std::endl;
        return new HydraulicPressureSourceQ("DefaultPressureSourceQName");
    }

    HydraulicPressureSourceQ(const string name,
                             const double pressure = 1.0e5,
                             const double timestep = 0.001)
	: ComponentQ(name, timestep)
    {
        mTypeName = "HydraulicPressureSourceQ";
        mStartPressure = 0.0;
        mStartFlow     = 0.0;
        mPressure      = pressure;

        addPowerPort("P1", "NodeHydraulic", P1);
        addReadPort("in", "NodeSignal", in);

        registerParameter("P", "Tryck", "Pa", mPressure);
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
        double q,p;

        if (mPortPtrs[in]->isConnected())
        {
            q = (mPortPtrs[in]->readNode(NodeSignal::VALUE) - c)/Zc;
            p = mPortPtrs[in]->readNode(NodeSignal::VALUE);         //We have a signal!
        }
        else
        {
            q = (mPressure - c)/Zc;
            p = mPressure;                                  //No signal, use internal parameter
        }

        //Write new values to nodes
        mPortPtrs[P1]->writeNode(NodeHydraulic::MASSFLOW, q);
        mPortPtrs[P1]->writeNode(NodeHydraulic::PRESSURE, p);
    }
};

#endif // HYDRAULICPRESSURESOURCEQ_HPP_INCLUDED
