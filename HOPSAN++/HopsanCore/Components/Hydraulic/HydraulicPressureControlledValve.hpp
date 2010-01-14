/*
 *  HydraulicPresssureControlledValve.hpp
 *  HOPSAN++
 *
 *  Created by Robert Braun on 2010-01-13.
 *  Copyright 2010 LiU. All rights reserved.
 *
 */

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
                                     const double tao        = 0.001,
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

        addPort("P1", "NodeHydraulic", P1);
        addPort("P2", "NodeHydraulic", P2);
        addPort("P_OPEN", "NodeHydraulic", P_OPEN);
        addPort("P_CLOSE", "NodeHydraulic", P_CLOSE);

        registerParameter("pref", "Reference Opening Pressure", "[Pa]", mPref);
        registerParameter("tao", "Time Constant of Spool", "[s]", mTao);
        registerParameter("kcs", "Steady State Characteristic due to Spring", "[(m^3/s)/Pa]", mKcs);
        registerParameter("kcf", "Steady State Characteristic due to Flow Forces", "[(m^3/s)/Pa]", mKcf);
        registerParameter("qnom", "Flow with Fully Open Valve and pressure drop Pnom", "[m^3/s]", mQnom);
        registerParameter("ph", "Hysteresis Width", "[Pa]", mPh);
    }


    void initialize()
    {
        mX0 = 0;

        mDelayedX0.setStepDelay(1);
        mDelayedX0.initilizeValues(0);

        //double wCutoff = mTimestep / mTao;
        double wCutoff = 1000000;
        double num [3] = {1.0, 0.0, 0.0};
        double den [3] = {1.0, 1.0/wCutoff, 0.0};
        mFilterLP.setCoefficients(num, den, mTimestep);
    }

    void simulateOneTimestep()
    {

        //Get the nodes
        Node* p1_ptr = mPorts[P1].getNodePtr();
        Node* p2_ptr = mPorts[P2].getNodePtr();
        Node* p3_ptr = mPorts[P_OPEN].getNodePtr();
        Node* p4_ptr = mPorts[P_CLOSE].getNodePtr();

        //Get variable values from nodes
        double p1 = p1_ptr->getData(NodeHydraulic::PRESSURE);
        double q1 = p1_ptr->getData(NodeHydraulic::MASSFLOW);
        double c1  = p1_ptr->getData(NodeHydraulic::WAVEVARIABLE);
        double Zc1 = p1_ptr->getData(NodeHydraulic::CHARIMP);
        double p2 = p2_ptr->getData(NodeHydraulic::PRESSURE);
        double q2 = p1_ptr->getData(NodeHydraulic::MASSFLOW);
        double c2  = p2_ptr->getData(NodeHydraulic::WAVEVARIABLE);
        double Zc2 = p2_ptr->getData(NodeHydraulic::CHARIMP);
        double p_open = p3_ptr->getData(NodeHydraulic::PRESSURE);
        double q_open = p3_ptr->getData(NodeHydraulic::MASSFLOW);
        double c_open = p3_ptr->getData(NodeHydraulic::WAVEVARIABLE);
        double p_close = p4_ptr->getData(NodeHydraulic::PRESSURE);
        double q_close = p4_ptr->getData(NodeHydraulic::MASSFLOW);
        double c_close = p4_ptr->getData(NodeHydraulic::WAVEVARIABLE);

        //Equations

        /* First timestep */

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
        double xsh = mHyst.getValue(xs, xh, mDelayedX0.value());
        mX0 = mFilterLP.filter(xsh);          //Filter disabled because it's not working!
        //mX0 = xsh;
        if (mX0 > mX0max)
        {
            if (mTime > 0.9 && mTime < 1.0) { cout << mX0 << endl; }
            if (mTime > 1.9 && mTime < 2.0) { cout << mX0 << endl; }
            mX0 = mX0max;
        }
        else if (mX0 < 0)
        {
            mX0 = 0;
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
            //mX0 = mFilterLP.filter(xsh);          //Filter is not working
            mX0 = xsh;
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

        if (mTime > 0.9 && mTime < 1.0) { cout << xsh << "   " << mX0 << "   " << cav << endl; }
        if (mTime > 1.9 && mTime < 2.0) { cout << xsh << "   " << mX0 << "   " << cav << endl; }

        //Write new values to nodes

        p1_ptr->setData(NodeHydraulic::PRESSURE, p1);
        p1_ptr->setData(NodeHydraulic::MASSFLOW, q1);
        p2_ptr->setData(NodeHydraulic::PRESSURE, p2);
        p2_ptr->setData(NodeHydraulic::MASSFLOW, q2);
        p3_ptr->setData(NodeHydraulic::PRESSURE, p_open);
        p4_ptr->setData(NodeHydraulic::PRESSURE, p_close);
    }
};

#endif // HYDRAULICPRESSURECONTROLLEDVALVE_HPP_INCLUDED
