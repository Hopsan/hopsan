//!
//! @file   HydraulicTLMRLineR.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2009-12-21
//!
//! @brief Contains a Hydraulic Transmission Line Component with Resistors in the ends
//!
//$Id$

#ifndef HYDRAULICTLMRLINER_HPP_INCLUDED
#define HYDRAULICTLMRLINER_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

//!
//! @brief
//! @ingroup HydraulicComponents
//!
class HydraulicTLMRlineR : public ComponentC
{

private:
    double mStartPressure;
    double mStartFlow;
    double mTimeDelay;
    double mAlpha;
    double mZc;
    double mR1;
    double mR2;
    Delay mDelayedC1;
    Delay mDelayedC2;
    Port *mpP1, *mpP2;

public:
    static Component *Creator()
    {
        return new HydraulicTLMRlineR("TLMRlineR");
    }

    HydraulicTLMRlineR(const std::string name) : ComponentC(name)
    {
        //Set member attributes
        mTypeName = "HydraulicTLMRlineR";
        mStartPressure = 1.0;
        mStartFlow     = 0.0;
        mTimeDelay     = 0.1;
        mZc            = 1.0e9;
        mAlpha         = 0.0;
        mR1            = 0.5;
        mR2            = 0.5;

        //Add ports to the component
        mpP1 = addPowerPort("P1", "NodeHydraulic");
        mpP2 = addPowerPort("P2", "NodeHydraulic");

        //Register changable parameters to the HOPSAN++ core
        registerParameter("TD", "Time delay", "s",   mTimeDelay);
        registerParameter("a", "Low pass coefficient", "-", mAlpha);
        registerParameter("Zc", "Impedance", "Ns/m^5",  mZc);
        registerParameter("R1", "Resistance 1", "Ns/m^5",  mR1);
        registerParameter("R2", "Resistance 2", "Ns/m^5",  mR2);
    }


    void initialize()
    {
        //Write to nodes
        mpP1->writeNode(NodeHydraulic::MASSFLOW,     mStartFlow);
        mpP1->writeNode(NodeHydraulic::PRESSURE,     mStartPressure);
        mpP1->writeNode(NodeHydraulic::WAVEVARIABLE, mStartPressure+(mZc+mR1)*mStartFlow);
        mpP1->writeNode(NodeHydraulic::CHARIMP,      mZc+mR1);
        mpP2->writeNode(NodeHydraulic::MASSFLOW,     mStartFlow);
        mpP2->writeNode(NodeHydraulic::PRESSURE,     mStartPressure);
        mpP2->writeNode(NodeHydraulic::WAVEVARIABLE, mStartPressure+(mZc+mR2)*mStartFlow);
        mpP2->writeNode(NodeHydraulic::CHARIMP,      mZc+mR2);

        //Init delay
        mDelayedC1.initialize(mTime, mStartPressure+(mZc+mR1)*mStartFlow);
        mDelayedC2.initialize(mTime, mStartPressure+(mZc+mR2)*mStartFlow);

        //Set external parameters
        mDelayedC1.setTimeDelay(mTimeDelay-mTimestep, mTimestep); //-mTimestep sue to calc time
        mDelayedC2.setTimeDelay(mTimeDelay-mTimestep, mTimestep);
    }


    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double q1 = mpP1->readNode(NodeHydraulic::MASSFLOW);
        double p1 = mpP1->readNode(NodeHydraulic::PRESSURE);
        double q2 = mpP2->readNode(NodeHydraulic::MASSFLOW);
        double p2 = mpP2->readNode(NodeHydraulic::PRESSURE);
        double c1 = mpP1->readNode(NodeHydraulic::WAVEVARIABLE);
        double c2 = mpP2->readNode(NodeHydraulic::WAVEVARIABLE);

        //Delay Line equations
        double c10 = p2 + (mZc+mR2) * q2;
        double c20 = p1 + (mZc+mR1) * q1;
        c1  = mAlpha*c1 + (1.0-mAlpha)*c10;
        c2  = mAlpha*c2 + (1.0-mAlpha)*c20;

        //Write new values to nodes
        mpP1->writeNode(NodeHydraulic::WAVEVARIABLE, mDelayedC1.value(c1));
        mpP1->writeNode(NodeHydraulic::CHARIMP,      mZc+mR1);
        mpP2->writeNode(NodeHydraulic::WAVEVARIABLE, mDelayedC2.value(c2));
        mpP2->writeNode(NodeHydraulic::CHARIMP,      mZc+mR2);

        //Update the delayed variabels
        mDelayedC1.update(c1);
        mDelayedC2.update(c2);
    }
};

#endif // HYDRAULICTLMRLINER_HPP_INCLUDED
