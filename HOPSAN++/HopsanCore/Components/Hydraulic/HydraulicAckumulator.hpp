/*
 *  HydraulicAckumulator.hpp
 *  HOPSAN++
 *
 *  Created by Robert Braun on 2010-01-12.
 *  Copyright 2010 LiU. All rights reserved.
 *
 */

#ifndef HYDRAULICCHECKACKUMULATOR_HPP_INCLUDED
#define HYDRAULICCHECKACKUMULATOR_HPP_INCLUDED

#include <iostream>
#include "HopsanCore.h"
#include "math.h"

class HydraulicAckumulator : public ComponentQ
{
private:
    double mPmin, mVtot, mVoil, mVgas, mBetae, mKappa, mKce, mStartPressureInternal, mStartPressureExternal, mStartFlowExternal;
    Delay mDelayedP2, mDelayedC1, mDelayedZc1, mDelayedQ2, ;
    enum {P1, P2};

public:
    static Component *Creator()
    {
        std::cout << "running checkvalve creator" << std::endl;
        return new HydraulicAckumulator("DefaultCheckValveName");
    }

    HydraulicAckumulator(const string name,
                                const double pmin       = 1000000.0,
                                const double vtot       = 0.005,
                                const double voil       = 0.0,
                                const double betae      = 1000000000.0,
                                const double kappa      = 1.4,
                                const double kce        = 0.0000000001,
                                const double startpressure = 1000000.0,         //Start pressure, must beequal to or higher than pmin
                                const double timestep   = 0.001)
        : ComponentQ(name, timestep)
    {
        mPmin                   = pmin;
        mVtot                   = vtot;
        mVoil                   = voil;
        mVgas                   = vtot-voil;
        mBetae                  = betae;
        mKappa                  = kappa;
        mKce                    = kce;
        mStartPressureInternal  = startpressure;
        mStartPressureExternal  = 1.0;
        mStartFlowExternal      = 0.0;


        addPort("P1", "NodeHydraulic", P1);     //External port
        addPort("P2", "NodeHydraulic", P2);     //Internal "port"

        registerParameter("Pmin", "Minimum Internal Pressure", "Pa", mPmin);
        registerParameter("Vtot", "Total Volume", "m^3", mVtot);
        registerParameter("Voil", "Oil Volume", "m^3", mVoil);
        registerParameter("Betae", "Effective Bulk Modulus", "Pa", mBetae);
        registerParameter("Kappa", "Polytropic Exponent", "-", mKappa);
        registerParameter("Kce", "Flow-Pressure Coefficient", "(m^3/s)/Pa", mKce);
        registerParameter("StartPressure", "Initial Internal Pressure", "Pa", mStartPressureInternal);
    }


    void initialize()
    {
        if (mStartPressureInternal < mPmin)
        {
            mStartPressureInternal = mPmin;
            mVgas = mVtot;
            mVoil = 0;
            mDelayedP2.initilizeValues(mStartPressureInternal);
            mDelayedC1.initilizeValues(mStartPressureExternal);
            mDelayedZc1.initilizeValues(0);                                     //This is a problem! Zc is not zero at the beginning, but we cannot know what it is...
            mDelayedQ2.initilizeValues(0);
        }
        else
        {
            mDelayedC1.initilizeValues(mStartPressureExternal);
            mDelayedZc1.initilizeValues(0);                                     //Problem here too...
            mDelayedQ2.initilizeValues(mKce*(mStartPressureExternal-mStartPressureInternal));
            mVgas = pow(mPmin*pow(mVtot, mKappa)/mStartPressureInternal, 1/mKappa);
            mVoil = mVtot - mVgas;
        }
    }


    void simulateOneTimestep()
    {
        //Get the nodes
        Node* p1_ptr = mPorts[P1].getNodePtr();
        Node* p2_ptr = mPorts[P2].getNodePtr();

        //Get variable values from nodes
        double c1  = p1_ptr->getData(NodeHydraulic::WAVEVARIABLE);
        double Zc1 = p1_ptr->getData(NodeHydraulic::CHARIMP);
        double c2  = p2_ptr->getData(NodeHydraulic::WAVEVARIABLE);
        double Zc2 = p2_ptr->getData(NodeHydraulic::CHARIMP);

        //Ackumulator equations

        double e0, ct;
        double p1,q1,q2;
        double p2 = mDelayedP2.value();

        e0 = mPmin * pow(mVtot, mKappa);
        ct = mVgas / (p2*mKappa) + (mVtot-mVgas) / mBetae;
        q2 = (mDelayedQ2.value() * (2*ct*(mKce*mDelayedZc1.value()+1) - mKce*mTimestep) +
              2*ct*mKce*(c1-mDelayedC1.value())) / (2*ct*(mKce*Zc1+1) + mKce*mTimestep);
        p1 = c1 - Zc1*q2;
        p2 = p1 - q2/mKce;

        if (p1 < 0.0)       //Cavitation!
        {
            c1 = 0.0;
            Zc1 = 0.0;
            q2 = (mDelayedQ2.value() * (2*ct*(mKce*mDelayedZc1.value()+1) - mKce*mTimestep) +
                  2*ct*mKce*(c1-mDelayedC1.value())) / (2*ct*(mKce*Zc1+1) + mKce*mTimestep);
            p1 = 0.0;
            p2 = -q2/mKce;
        }

        if (p2 < mPmin)     //Too low pressure (ack cannot be less than empty)
        {
            mVgas = mVtot;
            q2 = 0.0;
            p1 = c1;
            p2 = mPmin;
        }

        mVgas = pow(e0/p2, 1/mKappa);
        mVoil = mVtot - mVgas;
        q1 = -q2;

        mDelayedQ2.update(q2);
        mDelayedC1.update(c1);
        mDelayedZc1.update(Zc1);

        //Write new values to nodes
        p1_ptr->setData(NodeHydraulic::PRESSURE, p1);
        p1_ptr->setData(NodeHydraulic::MASSFLOW, q1);
        p2_ptr->setData(NodeHydraulic::PRESSURE, p2);
        p2_ptr->setData(NodeHydraulic::MASSFLOW, q2);
    }
};

#endif // HYDRAULICACKUMULATOR_HPP_INCLUDED
