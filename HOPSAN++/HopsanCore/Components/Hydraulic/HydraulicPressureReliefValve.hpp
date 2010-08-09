//!
//! @file   HydraulicPressureReliefValve.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-22
//!
//! @brief Contains a hydraulic pressure relief valve with first order dynamics
//!
//$Id$

#ifndef HYDRAULICPRESSURERELIEFVALVE_HPP_INCLUDED
#define HYDRAULICPRESSURERELIEFVALVE_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

//!
//! @brief
//! @ingroup HydraulicComponents
//!
class HydraulicPressureReliefValve : public ComponentQ
{
private:
    double mX0, mPref, mTao, mKcs, mKcf, mCs, mCf, mQnom, mPnom, mPh, mX0max;
    Delay mDelayedX0;
    TurbulentFlowFunction mTurb;
    ValveHysteresis mHyst;
    FirstOrderFilter mFilterLP;
    Port *mpP1, *mpP2;

public:
    static Component *Creator()
    {
        return new HydraulicPressureReliefValve("PressureReliefValve");
    }

    HydraulicPressureReliefValve(const std::string name) : ComponentQ(name)
    {
        mTypeName = "HydraulicPressureReliefValve";
        mPref = 2000000;
        mTao = 0.01;
        mKcs = 0.00000001;
        mKcf = 0.00000001;
        mQnom = 0.001;
        mPh = 500000;
        mPnom = 7e6f;
        mX0max = mQnom / sqrt(mPnom);
        mCs = sqrt(mPnom) / mKcs;
        mCf = 1.0 / (mKcf*sqrt(mPnom));

        mpP1 = addPowerPort("P1", "NodeHydraulic");
        mpP2 = addPowerPort("P2", "NodeHydraulic");

        registerParameter("pref", "Reference Opening Pressure", "[Pa]", mPref);
        registerParameter("tao", "Time Constant of Spool", "[s]", mTao);
        registerParameter("kcs", "Steady State Characteristic due to Spring", "[(m^3/s)/Pa]", mKcs);
        registerParameter("kcf", "Steady State Characteristic due to Flow Forces", "[(m^3/s)/Pa]", mKcf);
        registerParameter("qnom", "Flow with Fully Open Valve and pressure drop Pnom", "[m^3/s]", mQnom);
        registerParameter("ph", "Hysteresis Width", "[Pa]", mPh);
    }


    void initialize()
    {
        mX0 = 0.00001;

        mDelayedX0.setStepDelay(1);
        mDelayedX0.initialize(mTime, 0.0);

        double wCutoff = 1 / mTao;
        double num [2] = {0.0, 1.0};
        double den [2] = {1.0/wCutoff, 1.0};
        mFilterLP.initialize(mTime, mTimestep, num, den, 0.0, 0.0, 0.0, mX0max);
    }


    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double p1 = mpP1->readNode(NodeHydraulic::PRESSURE);
        double q1 = mpP1->readNode(NodeHydraulic::MASSFLOW);
        double c1  = mpP1->readNode(NodeHydraulic::WAVEVARIABLE);
        double Zc1 = mpP1->readNode(NodeHydraulic::CHARIMP);
        double p2 = mpP2->readNode(NodeHydraulic::PRESSURE);
        double q2 = mpP2->readNode(NodeHydraulic::MASSFLOW);
        double c2  = mpP2->readNode(NodeHydraulic::WAVEVARIABLE);
        double Zc2 = mpP2->readNode(NodeHydraulic::CHARIMP);

        //PRV Equations

        //Help variable b1
        double b1 = mCs + (p1-p2)*mCf;
        if ( (p1-p2)*mCf < 0.0 )
        {
            b1 = mCs;
        }

        //Help variable gamma
        double gamma;
        if (p1>p2)
        {
            if ( (sqrt(p1-p2)*2.0 + (Zc1+Zc2)*mX0) != 0.0 )
            {
                gamma = sqrt(p1-p2)*2.0 / (sqrt(p1-p2)*2.0 + (Zc1+Zc2)*mX0);
            }
            else
            {
                gamma = 1.0;
            }
        }
        else
        {
            if ( (sqrt(p2-p1)*2.0 + (Zc1+Zc2)*mX0) != 0.0 )
            {
                gamma = sqrt(p2-p1)*2.0 / (sqrt(p2-p1)*2.0 + (Zc1+Zc2)*mX0);
            }
            else
            {
                gamma = 1.0;
            }
        }

        //Help variable b2
        double b2;
        if (p1 > p2)
        {
            b2 = gamma*(Zc1+Zc2)*sqrt(p1-p2);
        }
        else
        {
            b2 = gamma*(Zc1+Zc2)*sqrt(p2-p1);
        }
        if (b2 < 0.0)
        {
            b2 = 0.0;
        }

        // Calculation of spool position
        double xs = (gamma*(c1-c2) + b2*mX0/2.0 - mPref) / (b1+b2);

        //Hysteresis
        double xh = mPh / (b1+b2);                                  //Hysteresis width [m]
        double xsh = mHyst.getValue(xs, xh, mDelayedX0.value());

        //Filter
        double wCutoff = (1.0 + b2/b1) * 1.0/mTao;                //Cutoff frequency
        double num [2] = {0.0, 1.0};
        double den [2] = {1.0/wCutoff, 1.0};
        mFilterLP.setNumDen(num,den);
        mX0 = mFilterLP.value(xsh);

        //Turbulent flow equation
        mTurb.setFlowCoefficient(mX0);
        q2 = mTurb.getFlow(c1,c2,Zc1,Zc2);
        q1 = -q2;
        p2 = c2+Zc2*q2;
        p1 = c1+Zc1*q1;

        // Cavitation
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
            xs = (c1-c2 + b2*mX0/2.0 - mPref) / (b1+b2);
            xsh = mHyst.getValue(xs, xh, mDelayedX0.value());
            mX0 = mFilterLP.value(xsh);

            mTurb.setFlowCoefficient(mX0);
            q2 = mTurb.getFlow(c1,c2,Zc1,Zc2);
            q1 = -q2;
            p2 = c2+Zc2*q2;
            p1 = c1+Zc1*q1;

            if (p1 < 0.0) { p1 = 0.0; }
            if (p2 < 0.0) { p2 = 0.0; }
        }

        //Write new values to nodes

        mpP1->writeNode(NodeHydraulic::PRESSURE, p1);
        mpP1->writeNode(NodeHydraulic::MASSFLOW, q1);
        mpP2->writeNode(NodeHydraulic::PRESSURE, p2);
        mpP2->writeNode(NodeHydraulic::MASSFLOW, q2);

        mDelayedX0.update(mX0);
    }
};

#endif // HYDRAULICPRESSURERELIEFVALVE_HPP_INCLUDED
