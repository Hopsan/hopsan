//!
//! @file   HydraulicPressureSource.hpp
//! @author Robert Braun
//! @date   2009-12-21
//!
//! @brief Contains a Hydraulic Tank Component of C-type
//!
//$Id$

#ifndef HYDRAULICTANKC_HPP_INCLUDED
#define HYDRAULICTANKC_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"

//!
//! @brief
//! @ingroup HydraulicComponents
//!
class HydraulicTankC : public ComponentC
{
private:
    //double mStartPressure;
    double mStartFlow;
    double mZc;
    double mPressure;
    Port *mpP1;

public:
    static Component *Creator()
    {
        return new HydraulicTankC("PressureSource");
    }

    HydraulicTankC(const string name,
                            const double pressure       = 1.0e5)
        : ComponentC(name)
    {
        mTypeName = "HydraulicTankC";
        mStartFlow      = 0.0;
        mPressure       = pressure;
        mZc             = 0.0;

        mpP1 = addPowerPort("P1", "NodeHydraulic");

        registerParameter("P", "Default pressure", "Pa", mPressure);
    }

    void initialize()
    {
        //write to nodes
        mpP1->writeNode(NodeHydraulic::PRESSURE, mPressure);
        mpP1->writeNode(NodeHydraulic::MASSFLOW, mStartFlow);
    }

    void simulateOneTimestep()
    {

        //Write new values to nodes
        mpP1->writeNode(NodeHydraulic::WAVEVARIABLE, mPressure);
        mpP1->writeNode(NodeHydraulic::CHARIMP, mZc);
    }
};

#endif // HYDRAULICTANKC_HPP_INCLUDED
