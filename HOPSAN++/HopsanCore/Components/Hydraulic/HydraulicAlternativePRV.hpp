//!
//! @file   HydraulicAlternativePRV.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-01-22
//!
//! @brief Alternative version of pressure relief valve with first order dynamics
//!
//$Id$

#ifndef HYDRAULICALTERNATIVEPRV_HPP_INCLUDED
#define HYDRAULICALTERNATIVEPRV_HPP_INCLUDED

#include <iostream>
#include "../../ComponentEssentials.h"
#include "../../ComponentUtilities.h"

//!
//! @brief
//! @ingroup HydraulicComponents
//!
class HydraulicAlternativePRV : public ComponentQ
{
private:
    double mX0, mPref, mCq, mW, mSpoolDiameter, mFrac, mPilotArea, mK, mC, mMass, mXhyst, mXmax, mFs;
    Delay mDelayedX0;
    TurbulentFlowFunction mTurb;
    ValveHysteresis mHyst;
    SecondOrderFilter mFilter;
    Port *mpP1, *mpP2, *mpX;

public:
    static Component *Creator()
    {
        return new HydraulicAlternativePRV("AlternativePRV");
    }

    HydraulicAlternativePRV(const string name) : ComponentQ(name)
    {
        mTypeName = "HydraulicAlternativePRV";
        mPref = 20000000;
        mCq = 0.67;
        mSpoolDiameter = 0.01;
        mFrac = 1.0;
        mW = mSpoolDiameter*mFrac;
        mPilotArea = 0.001;
        mK = 1e6;
        mC = 1000;
        mMass = 0.05;
        mXhyst = 0.0;
        mXmax = 0.001;
        mFs = mPilotArea * mPref;   //Spring preload

        mpP1 = addPowerPort("P1", "NodeHydraulic");
        mpP2 = addPowerPort("P2", "NodeHydraulic");
        mpX  = addWritePort("X", "NodeSignal");

        registerParameter("pref", "Reference Opening Pressure", "[Pa]", mPref);
        registerParameter("cq", "Flow Coefficient", "[-]", mCq);
        registerParameter("spooldiameter", "Spool Diameter", "[m]", mSpoolDiameter);
        registerParameter("frac", "Fraction of Spool Circumference that is Opening", "[-]", mFrac);
        registerParameter("pilotarea", "Working Area of Pilot Pressure", "[m^2]", mPilotArea);
        registerParameter("k", "Steady State Characheristics of Spring", "[N/m]", mK);
        registerParameter("c", "Steady State Damping Coefficient", "[Ns/m]", mC);
        registerParameter("m", "Ineretia of Spool", "kg", mMass);
        registerParameter("xhyst", "Hysteresis of Spool Position", "[m]", mXhyst);
        registerParameter("xmax", "Maximum Spool Position", "[m]", mXmax);
    }


    void initialize()
    {
        mX0 = 0.00001;

        mDelayedX0.setStepDelay(1);
        mDelayedX0.initialize(mTime, 0.0);

        double num[3];
        double den[3];
        num[0] = 0.0;
        num[1] = 0.0;
        num[2] = 1.0;
        den[0] = mMass;
        den[1] = mC;
        den[2] = mK;

        mFilter.initialize(mTime, mTimestep, num, den, 0.0, 0.0, 0.0, mXmax);
    }

    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double c1  = mpP1->readNode(NodeHydraulic::WAVEVARIABLE);
        double Zc1 = mpP1->readNode(NodeHydraulic::CHARIMP);
        double c2  = mpP2->readNode(NodeHydraulic::WAVEVARIABLE);
        double Zc2 = mpP2->readNode(NodeHydraulic::CHARIMP);

        //PRV Equations

        mFs = mPilotArea*mPref;

        double q1 = mpP1->readNode(NodeHydraulic::MASSFLOW); //Needed to calculate p1 for the force equilibrium
        double p1 = c1 + q1*Zc1;

        double Ftot = p1*mPilotArea - mFs;      //Sum of forces in x direction beside from spring coeff and viscous friction
        double x0 = mFilter.value(Ftot);        //Filter function G = 1/(mMass*s^2 + mC*s + mK)
        double x0h = mHyst.getValue(x0, mXhyst, mDelayedX0.value()); //Hysteresis

        //Turbulent flow equation
        mTurb.setFlowCoefficient(mCq*mW*x0);

        double q2 = mTurb.getFlow(c1,c2,Zc1,Zc2);
        q1 = -q2;

        p1 = c1 + Zc1*q1;
        double p2 = c2 + Zc2*q2;

/*Kör utan kaviataion nu i början när vi jämför med OpenModelica
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
        if (cav)        //Cavitation, redo calculations with new c and Zc
        {
                //Calculation of Spool Position
            double p1 = c1 + q1*Zc1;
            double Ftot = p1*mPilotArea - mFs;      //Sum of forces in x direction
            double num [3] = {1.0, 0.0, 0.0};
            double den [3] = {mK, mC, mMass};
            mFilter.setNumDen(num,den);
            double x0 = mFilter.value(Ftot);            //Filter function
            double x0h = mHyst.getValue(x0, mXhyst, mDelayedX0.value());            //Hysteresis

                //Turbulent flow equation
            mTurb.setFlowCoefficient(mCq*mW*x0h);
            q2 = mTurb.getFlow(c1,c2,Zc1,Zc2);
            q1 = -q2;
            p2 = c2+Zc2*q2;
            p1 = c1+Zc1*q1;

            if (p1 < 0.0) { p1 = 0.0; }
            if (p2 < 0.0) { p2 = 0.0; }
        }
*/
        //Write new values to nodes

        mpP1->writeNode(NodeHydraulic::PRESSURE, p1);
        mpP1->writeNode(NodeHydraulic::MASSFLOW, q1);
        mpP2->writeNode(NodeHydraulic::PRESSURE, p2);
        mpP2->writeNode(NodeHydraulic::MASSFLOW, q2);

        if(mpX->isConnected())
            mpX->writeNode(NodeSignal::VALUE, x0);

        mDelayedX0.update(x0h);  //Ska vara x0h
    }
};

#endif // HYDRAULICALTERNATIVEPRV_HPP_INCLUDED
