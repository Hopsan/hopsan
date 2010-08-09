//!
//! @file   HydraulicFixedDisplacementPump.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-07
//!
//! @brief Contains a hydraulic pump with fixed displacement component
//!
//$Id$

#ifndef HYDRAULICFIXEDDISPLACEMENTPUMP_HPP_INCLUDED
#define HYDRAULICFIXEDDISPLACEMENTPUMP_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"

//!
//! @brief
//! @ingroup HydraulicComponents
//!
class HydraulicFixedDisplacementPump : public ComponentQ
{
private:
    double mSpeed;             // rad/s
    double mDp;
    double mKcp;

    Port *mpP1, *mpP2;

public:
    static Component *Creator()
    {
        return new HydraulicFixedDisplacementPump("FixedDisplacementPump");
    }

    HydraulicFixedDisplacementPump(const std::string name) : ComponentQ(name)
    {
        mTypeName = "HydraulicFixedDisplacementPump";
        mSpeed = 125.0;
        mDp = 0.00005;
        mKcp = 0.001;

        mpP1 = addPowerPort("P1", "NodeHydraulic");
        mpP2 = addPowerPort("P2", "NodeHydraulic");

        registerParameter("Speed", "Angular Velocity", "rad/s", mSpeed);
        registerParameter("Dp", "Displacement", "m^3/rev", mDp);
        registerParameter("Kcp", "Leakage Coefficient", "(m^3/s)/Pa", mKcp);
    }


    void initialize()
    {
        //Nothing to initilize
    }


    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double c1 = mpP1->readNode(NodeHydraulic::WAVEVARIABLE);
        double Zc1 = mpP1->readNode(NodeHydraulic::CHARIMP);
        double c2 = mpP2->readNode(NodeHydraulic::WAVEVARIABLE);
        double Zc2 = mpP2->readNode(NodeHydraulic::CHARIMP);

        //Fixed Displacement Pump equations

        double pi = 3.1415926536;

        double q2 = ( mDp*mSpeed/(2.0*pi) + mKcp*(c1-c2) ) / ( (Zc1+Zc2)*mKcp+1 );
        double q1 = -q2;
        double p2 = c2 + Zc2*q2;
        double p1 = c1 + Zc1*q1;

        /* Cavitation Check */

        bool cav = false;

        if (p1 < 0.0)
        {
            c1 = 0.0;
            Zc1 = 0.0;
            cav = true;
        }
        if (p2 < 0.0)
        {
            c2 = 0.0;
            Zc2 = 0.0;
            cav = true;
        }
        if (cav)
        {
            q2 = ( mDp*mSpeed/(2.0*pi) + mKcp*(c1-c2) ) / ( (Zc1+Zc2)*mKcp+1 );
            q1 = -q2;
            p1 = c1 + Zc1 * q1;
            p2 = c2 + Zc2 * q2;
            if (p1 < 0.0) { p1 = 0.0; }
            if (p2 < 0.0) { p2 = 0.0; }
        }

        //Write new values to nodes
        mpP1->writeNode(NodeHydraulic::PRESSURE, p1);
        mpP1->writeNode(NodeHydraulic::MASSFLOW, q1);
        mpP2->writeNode(NodeHydraulic::PRESSURE, p2);
        mpP2->writeNode(NodeHydraulic::MASSFLOW, q2);
    }
};

#endif // HYDRAULICFIXEDDISPLACEMENTPUMP_HPP_INCLUDED
