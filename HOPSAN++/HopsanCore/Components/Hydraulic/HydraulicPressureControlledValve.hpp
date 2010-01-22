//!
//! @file   HydraulicPressureControlledValve.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-13
//!
//! @brief Contains a hydraulic pressure controlled valve with first order dynamics
//!
//$Id$

#ifndef HYDRAULICPRESSURECONTROLLEDVALVE_HPP_INCLUDED
#define HYDRAULICPRESSURECONTROLLEDVALVE_HPP_INCLUDED

#include <iostream>
#include "HopsanCore.h"
#include "CoreUtilities/TransferFunction.h"
#include "CoreUtilities/TurbulentFlowFunction.h"
#include "CoreUtilities/ValveHysteresis.h"
#include "CoreUtilities/Delay.h"

class HydraulicPressureControlledValve : public ComponentQ
{
private:
    double mX0, mX0max, mPref, mTao, mKcs, mKcf, mCs, mCf, mPnom, mQnom, mPh;
    Delay mDelayedX0;
    TurbulentFlowFunction mTurb;
    ValveHysteresis mHyst;
    TransferFunction mFilterLP;
    enum {P1, P2, P_OPEN, P_CLOSE};

public:
    static Component *Creator()
    {
        std::cout << "running PressureControlledValve creator" << std::endl;
        return new HydraulicPressureControlledValve("DefaultPressureControlledValveName");
    }

    HydraulicPressureControlledValve(const string name,
                                     const double pref       = 2000000,
                                     const double tao        = 0.01,
                                     const double kcs        = 0.00000001,
                                     const double kcf        = 0.00000001,
                                     const double qnom       = 0.001,
                                     const double ph         = 500000,
                                     const double timestep   = 0.001)
        : ComponentQ(name, timestep)
    {
        mPref = pref;
        mTao = tao;
        mKcs = kcs;
        mKcf = kcf;
        mQnom = qnom;
        mPh = ph;
        mPnom = 7e6f;
        mCs = sqrt(mPnom)/mKcs;
        mCf = 1/(mKcf * sqrt(mPnom));
        mX0max = mQnom/sqrt(mPnom);

        addPowerPort("P1", "NodeHydraulic", P1);
        addPowerPort("P2", "NodeHydraulic", P2);
        addPowerPort("P_OPEN", "NodeHydraulic", P_OPEN);
        addPowerPort("P_CLOSE", "NodeHydraulic", P_CLOSE);

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

        mDelayedX0.initialize(mTime, 0.0);
        mDelayedX0.setStepDelay(1);

        double wCutoff = 1 / mTao;      //Ska det vara Timestep/Tao?
        //double wCutoff = 100;     DEBUG
        double num [3] = {1.0, 0.0, 0.0};
        double den [3] = {1.0, 1.0/wCutoff, 0.0};
        mFilterLP.initialize(0.0,0.0, mTime);
        mFilterLP.setCoefficients(num, den, mTimestep);
    }

    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double p1 = mPortPtrs[P1]->readNode(NodeHydraulic::PRESSURE);
        double q1 = mPortPtrs[P1]->readNode(NodeHydraulic::MASSFLOW);
        double c1  = mPortPtrs[P1]->readNode(NodeHydraulic::WAVEVARIABLE);
        double Zc1 = mPortPtrs[P1]->readNode(NodeHydraulic::CHARIMP);
        double p2 = mPortPtrs[P2]->readNode(NodeHydraulic::PRESSURE);
        double q2 = mPortPtrs[P2]->readNode(NodeHydraulic::MASSFLOW);
        double c2  = mPortPtrs[P2]->readNode(NodeHydraulic::WAVEVARIABLE);
        double Zc2 = mPortPtrs[P2]->readNode(NodeHydraulic::CHARIMP);
        double p_open = mPortPtrs[P_OPEN]->readNode(NodeHydraulic::PRESSURE);
        double q_open = mPortPtrs[P_OPEN]->readNode(NodeHydraulic::MASSFLOW);
        double c_open = mPortPtrs[P_OPEN]->readNode(NodeHydraulic::WAVEVARIABLE);
        double p_close = mPortPtrs[P_CLOSE]->readNode(NodeHydraulic::PRESSURE);
        double q_close = mPortPtrs[P_CLOSE]->readNode(NodeHydraulic::MASSFLOW);
        double c_close = mPortPtrs[P_CLOSE]->readNode(NodeHydraulic::WAVEVARIABLE);

        //Equations

        /* First timestep */        //This was used in gammelhopsan, but can not yet be implemented in this environment since the nodes are unavailable during initialization

        // Start values
        //    if (*zc1 == 0.0) { *c1 = *p1; }
        //    if (*zc2 == 0.0) { *c2 = *p2; }
        //    *c_open = *p_open;
        //    *c_close = *p_close;
        //

        /* Equations */

        double b1 = mCs+mCf*(p1-p2);        //Help Variable, equals sqrt(p1-p2)/Kctot

        // Spool position calculation
        double xs = (p_open - mPref - p_close) / b1;

        double xh = mPh/b1;
        //cout << "xs = " << xs << endl;
        double xsh = mHyst.getValue(xs, xh, mDelayedX0.value(1));
        //cout << "xsh = " << xsh << endl;
        mX0 = mFilterLP.getValue(xsh);          //Filter disabled because it's not working!
        if (mTime < 0.1) { cout << "p1 = " << p1 << ", xs = " << xs << ", xsh = " << xsh << ", mDelayedX0 = " << mDelayedX0.value(1) << ", mX0 = " << mX0 << endl; }
        //cout << "mX0 = " << mX0 << endl;
        //mX0 = xsh;      //Debug, ta bort sen
        if (xsh > mX0max)
        {

            xsh = mX0max;
        }
        else if (xsh < 0)
        {
            xsh = 0;
        }


        // Turbulent Flow Calculation
        mTurb.setFlowCoefficient(mX0);
        q2 = mTurb.getFlow(c1, c2, Zc1, Zc2);
        q1 = -q2;
        q_open = 0.0;
        q_close = 0.0;

        // Pressure Calulation
        p1 = c1 + Zc1 * q1;
        p2 = c2 + Zc2 * q2;
        p_open = c_open;
        p_close = c_close;

        // Check for cavitation
        bool cav = false;
        if (p1 < 0.0)
        {
            c1 = 0.f;
            Zc1 = 0.0;
            cav = true;
        }
        if (p2 < 0.0)
        {
            c2 = 0.0;
            Zc2 = 0.0;
            cav = true;
        }
        if (cav)        //Cavitatiaon, redo calculations with new c and Zc
        {
            xs = (p_open-mPref-p_close)/b1;
            xh = mPh / b1;
            //if (mTime == 0) { xs = mX0; }
            xsh = mHyst.getValue(xs, xh, mDelayedX0.value());
            mX0 = mFilterLP.getValue(xsh);          //Filter is not working
            //mX0 = xsh;
            if (mX0 > mX0max)
            {
                mX0 = mX0max;
            }
            else if (mX0 < 0)
            {
                mX0 = 0;
            }
            mTurb.setFlowCoefficient(mX0);
            q2 = mTurb.getFlow(c1, c2, Zc1, Zc2);
            q1 = -q2;
            p1 = c1 + Zc1 * q1;
            p2 = c2 + Zc2 * q2;
            if (p1 < 0.0) { p1 = 0.0; }
            if (p2 < 0.0) { p2 = 0.0; }
        }

        mDelayedX0.update(mX0);

        //Write new values to nodes

        mPortPtrs[P1]->writeNode(NodeHydraulic::PRESSURE, p1);
        mPortPtrs[P1]->writeNode(NodeHydraulic::MASSFLOW, q1);
        mPortPtrs[P2]->writeNode(NodeHydraulic::PRESSURE, p2);
        mPortPtrs[P2]->writeNode(NodeHydraulic::MASSFLOW, q2);
        mPortPtrs[P_OPEN]->writeNode(NodeHydraulic::PRESSURE, p_open);
        mPortPtrs[P_CLOSE]->writeNode(NodeHydraulic::PRESSURE, p_close);

        //mFilterLP.update(xsh);
    }
};

#endif // HYDRAULICPRESSURECONTROLLEDVALVE_HPP_INCLUDED
