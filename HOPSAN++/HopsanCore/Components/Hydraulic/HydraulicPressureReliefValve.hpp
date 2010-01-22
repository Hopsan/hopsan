//!
//! @file   HydraulicPressureReliefValve.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-22
//!
//! @brief Contains a hydraulic pressure relief valve with first order dynamics
//!

#ifndef HYDRAULICPRESSURERELIEFVALVE_HPP_INCLUDED
#define HYDRAULICPRESSURERELIEFVALVE_HPP_INCLUDED

#include <iostream>
#include "HopsanCore.h"
#include "CoreUtilities/TransferFunction.h"
#include "CoreUtilities/TurbulentFlowFunction.h"
#include "CoreUtilities/ValveHysteresis.h"
#include "CoreUtilities/Delay.h"

class HydraulicPressureReliefValve : public ComponentQ
{
private:
    double mX0, mPref, mTao, mKcs, mKcf, mCs, mCf, mQnom, mPnom, mPh, mX0max;
    Delay mDelayedX0;
    TurbulentFlowFunction mTurb;
    ValveHysteresis mHyst;
    TransferFunction mFilterLP;
    enum {P1, P2};

public:
    static Component *Creator()
    {
        std::cout << "running PressureReliefValve creator" << std::endl;
        return new HydraulicPressureReliefValve("DefaultPressureReliefValveName");
    }

    HydraulicPressureReliefValve(const string name,
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
        mX0max = mQnom / sqrt(mQnom);
        mCs = sqrt(mPnom) / mKcs;
        mCf = 1 / (mKcf*sqrt(mPnom));

        addPowerPort("P1", "NodeHydraulic", P1);
        addPowerPort("P2", "NodeHydraulic", P2);

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
        mDelayedX0.initializeValues(0.0, mTime);

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

        //Equations

            //Help variable b1
        double b1 = mCs + (p1-p2)*mCf;
        if ( (p1-p2)*mCf < 0.0 )
        {
            b1 = mCs;
        }

            //Help variable gamma
        double gamma;
        if (mTime > 1.99 && mTime < 2.01) { cout << "Zc1 = " << Zc1 << ", Zc2 = " << Zc2 << ", "; }
        if (p1>p2)
        {
            if (sqrt(p1-p2)*2 + (Zc1+Zc2)*mX0 != 0.f)
            {
                gamma = sqrt(p1-p2)*2 / (sqrt(p1-p2)*2 + (Zc1+Zc2)*mX0);
            }
            else
            {
                gamma = 1.0;
            }
        }
        else
        {
            if (sqrt(p2-p1)*2 + (Zc1+Zc2)*mX0 != 0.f)
            {
                gamma = sqrt(p2-p1)*2 / (sqrt(p2-p1)*2 + (Zc1+Zc2)*mX0);
            }
            else
            {
                gamma = 1.0;
            }
        }

            //Help variable b2
        double b2;
        if (p1 > p2)    { b2 = gamma*(Zc1+Zc2)*sqrt(p1-p2); }
        else            { b2 = gamma*(Zc1+Zc2)*sqrt(p2-p1); }
        if (b2 < 0.0)   { b2 = 0.0; }


            // Calculation of spool position
        double xs = (gamma*(c1-c2) + b2*mX0/2 - mPref) / (b1+b2);

            //Hysteresis
        double xh = mPh / (b1+b2);                                  //Hysteresis width [m]
        double xsh = mHyst.getValue(xs, xh, mDelayedX0.value());

            //Filter
        //double wCutoff = (b2 / b1 + 1) * mTimestep/mTao;                        //Cutoff frequency
        double wCutoff = (b2 / b1 + 1) * 1/mTao;                        //Cutoff frequency
        double num [3] = {1.0, 0.0, 0.0};
        double den [3] = {1.0, 1.0/wCutoff, 0.0};
        mFilterLP.setCoefficients(num,den,mTimestep);
        mX0 = mFilterLP.getValue(xsh);
        if (mX0 > mX0max)
        {
            mX0 = mX0max;
        }
        else if (mX0 < 0.0)
        {
            mX0 = 0.0;
        }

            //Turbulent flow equation
        mTurb.setFlowCoefficient(mX0);
        q2 = mTurb.getFlow(c1,c2,Zc1,Zc2);
        q1 = -q2;
        p2 = c2+Zc2*q2;
        p1 = c1+Zc1*q1;

        if (mTime > 1.99 && mTime < 2.01) { cout << endl; }

        if (mTime > 1.9 && mTime < 2.1)
        {
            cout << "p1-p2 = " << (p1-p2) << ", xs = " << xs << ", xsh = " << xsh << ", wCutoff = " << wCutoff << ", x0 = " << mX0 << endl;
        }


            // Cavitation
        //    cav = 0;
        //    if (*p1 < 0.0)
        //    {
        //        *c1 = 0.0;
        //        *zc1 = 0.0;
        //        cav = 1;
        //    }
        //    if (*p2 < 0.0)
        //    {
        //        *c2 = 0.0;
        //        *zc2 = 0.0;
        //        cav = 1;
        //    }
        //    if (cav)
        //    {
        //        xs = (*c1 - *c2 + b2 * *x0 / 2 - *pref) / (b1 + b2);
        //        xsh = hyst(&xs, &xh, &x0d);
        //        r1 = w01 * *time_step;
        //        *x0 = lp1(&xsh, &r1, &c_b93, &x0max, &*time_step, &*time);
        //        *q2 = qturb(&*x0, &*c1, &*c2, &*zc1, &*zc2);
        //        *q1 = -*q2;
        //        *p1 = *c1 + *zc1 * *q1;
        //        *p2 = *c2 + *zc2 *  *q2;
        //        if (*p1 < 0.0) { *p1 = 0.0; }
        //        if (*p2 < 0.0) { *p2 = 0.0; }
        //    }

        //Write new values to nodes

        mPortPtrs[P1]->writeNode(NodeHydraulic::PRESSURE, p1);
        mPortPtrs[P1]->writeNode(NodeHydraulic::MASSFLOW, q1);
        mPortPtrs[P2]->writeNode(NodeHydraulic::PRESSURE, p2);
        mPortPtrs[P2]->writeNode(NodeHydraulic::MASSFLOW, q2);

        //mFilterLP.update(xsh);
    }
};

#endif // HYDRAULICPRESSURERELIEFVALVE_HPP_INCLUDED
