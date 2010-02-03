//!
//! @file   HydraulicAckumulator.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-12
//!
//! @brief Contains a Hydraulic Ackumulator component with constant bulk modulus
//!
//$Id$

#ifndef HYDRAULICCHECKACKUMULATOR_HPP_INCLUDED
#define HYDRAULICCHECKACKUMULATOR_HPP_INCLUDED

#include <iostream>
#include "HopsanCore.h"
#include "math.h"

class HydraulicAckumulator : public ComponentQ
{
private:
    double mPmin, mVtot, mVoil, mVgas, mBetae, mKappa, mKce, mStartPressure, mStartFlow;
    Delay mDelayedP2, mDelayedC1, mDelayedZc1, mDelayedQ2;
    enum {P1, out};

public:
    static Component *Creator()
    {
        std::cout << "running ackumulator creator" << std::endl;
        return new HydraulicAckumulator("DefaultAckumulatorName");
    }

    HydraulicAckumulator(const string name,
                                const double pmin       = 1000000.0,
                                const double vtot       = 0.005,
                                const double voil       = 0.0,
                                const double betae      = 1000000000.0,
                                const double kappa      = 1.4,
                                const double kce        = 0.0000000001,
                                const double startpressure = 1000000.0,         //Start pressure, must beequal to or higher than pmin
                                const double startflow = 0.0,                   //Initial flow into the accumulator
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
        mStartPressure          = startpressure;
        mStartFlow              = startflow;

        addPowerPort("P1", "NodeHydraulic", P1);     //External port
        addWritePort("out", "NodeSignal", out);     //Internal pressure output

        registerParameter("Pmin", "Minimum Internal Pressure", "Pa", mPmin);
        registerParameter("Vtot", "Total Volume", "m^3", mVtot);
        registerParameter("Voil", "Oil Volume", "m^3", mVoil);
        registerParameter("Betae", "Effective Bulk Modulus", "Pa", mBetae);
        registerParameter("Kappa", "Polytropic Exponent", "-", mKappa);
        registerParameter("Kce", "Flow-Pressure Coefficient", "(m^3/s)/Pa", mKce);
        registerParameter("StartPressure", "Initial Internal Pressure", "Pa", mStartPressure);
        registerParameter("StartFlow", "Initial Flow Into Ackumulator", "m^3/s", mStartFlow);
    }


    void initialize()
    {
        double p1 = mPortPtrs[P1]->readNode(NodeHydraulic::PRESSURE);
        double c1 = mPortPtrs[P1]->readNode(NodeHydraulic::WAVEVARIABLE);
        double Zc1 = mPortPtrs[P1]->readNode(NodeHydraulic::CHARIMP);

        if (mStartPressure < mPmin)         //User has selected an initial pressure lower than the minimum pressure, so use minimum pressure instead
        {
            mStartPressure = mPmin;
            if (mStartPressure < 0)         //User has selected a minimum pressure lower than zero, put it to zero
            {
                mStartPressure = 0;
            }
            mVgas = mVtot;                  //Pressure is minimum, so ackumulator is empty
            mVoil = 0;
            mDelayedQ2.initialize(mTime, mStartFlow);        //"Previous" value for q2 first step
        }
        else
        {
            if (p1 < 0.0)                               //Neighbour component has provided a presssure lower than zero, so put it so zero
            {
                p1 = 0.0;
            }
            mDelayedQ2.initialize(mTime, mKce*(p1-mStartPressure));              //"Previous" value for q2 first step, calculated with orifice equation
            mVgas = pow(mPmin*pow(mVtot, mKappa)/mStartPressure, 1/mKappa);     //Initial gas volume, calculated from initial pressure
            mVoil = mVtot - mVgas;
        }
        mDelayedP2.initialize(mTime, mStartPressure);
        mDelayedC1.initialize(mTime, c1);
        mDelayedZc1.initialize(mTime, Zc1);

        mDelayedP2.setStepDelay(1);
        mDelayedC1.setStepDelay(1);
        mDelayedZc1.setStepDelay(1);
        mDelayedQ2.setStepDelay(1);

        cout << "Start Pressure: " << mStartPressure << endl;;
    }


    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double c1 = mPortPtrs[P1]->readNode(NodeHydraulic::WAVEVARIABLE);
        double Zc1 = mPortPtrs[P1]->readNode(NodeHydraulic::CHARIMP);

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

        if (mTime < 0.05)
        {
                cout << "p1 = " << p1 << ", p2 = " << p2 << ", q = " << q2 << ", ct = " << ct << endl;
        }


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
            cout << "Empty!" << endl;
            mVgas = mVtot;
            q2 = 0.0;
            p1 = c1;
            p2 = mPmin;
        }

        mVgas = pow(e0/p2, 1/mKappa);
        mVoil = mVtot - mVgas;
        q1 = -q2;

        if (mTime < 0.05)
        {
                cout << "Voil = " << mVoil << endl;
        }

        mDelayedP2.update(p2);
        mDelayedQ2.update(q2);
        mDelayedC1.update(c1);
        mDelayedZc1.update(Zc1);

        //Write new values to nodes
        mPortPtrs[P1]->writeNode(NodeHydraulic::PRESSURE, p1);
        mPortPtrs[P1]->writeNode(NodeHydraulic::MASSFLOW, q1);
        if (mPortPtrs[out]->isConnected())
        {
            mPortPtrs[out]->writeNode(NodeSignal::VALUE, mVoil);
        }

    }
};

#endif // HYDRAULICACKUMULATOR_HPP_INCLUDED
