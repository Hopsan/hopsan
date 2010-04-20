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
    double mX0, mPref, mCq, mW, mSpoolDiameter, mFrac, mPilotArea, mK, mC, mXhyst, mXmax, mFs;
    Delay mDelayedX0;
    TurbulentFlowFunction mTurb;
    ValveHysteresis mHyst;
    FirstOrderFilter mFilter;
    Port *mpP1, *mpP2;

public:
    static Component *Creator()
    {
        std::cout << "running AlternativePRV creator" << std::endl;
        return new HydraulicAlternativePRV("AlternativePRV");
    }

    HydraulicAlternativePRV(const string name,
                                 const double pref          = 20000000,
                                 const double cq            = 0.67,
                                 const double spooldiameter = 0.01,
                                 const double frac          = 1.0,
                                 const double pilotarea     = 0.001,
                                 const double k             = 1e6,
                                 const double c             = 1000,
                                 const double xhyst         = 0.0,
                                 const double xmax          = 0.001,
                                 const double timestep      = 0.001)
        : ComponentQ(name, timestep)
    {
        mTypeName = "HydraulicAlternativePRV";
        mPref = pref;
        mCq = cq;
        mSpoolDiameter = spooldiameter;
        mFrac = frac;
        mW = mSpoolDiameter*mFrac;
        mPilotArea = pilotarea;
        mK = k;
        mC = c;
        mXhyst = xhyst;
        mXmax = xmax;
        mFs = mPilotArea * mPref;   //Spring preload

        mpP1 = addPowerPort("P1", "NodeHydraulic");
        mpP2 = addPowerPort("P2", "NodeHydraulic");

        registerParameter("pref", "Reference Opening Pressure", "[Pa]", mPref);
        registerParameter("cq", "Flow Coefficient", "[-]", mCq);
        registerParameter("spooldiameter", "Spool Diameter", "[m]", mSpoolDiameter);
        registerParameter("frac", "Fraction of Spool Circumference that is Opening", "[-]", mFrac);
        registerParameter("pilotarea", "Working Area of Pilot Pressure", "[m^2]", mPilotArea);
        registerParameter("k", "Steady State Characheristics of Spring", "[N/m]", mK);
        registerParameter("c", "Steady State Damping Coefficient", "[Ns/m]", mC);
        registerParameter("xhyst", "Hysteresis of Spool Position", "[m]", mXhyst);
        registerParameter("xmax", "Maximum Spool Position", "[m]", mXmax);
    }


    void initialize()
    {
        mX0 = 0.00001;

        mDelayedX0.setStepDelay(1);
        mDelayedX0.initialize(mTime, 0.0);

        //double p1 = mpP1->readNode(NodeHydraulic::PRESSURE);

        double num [2] = {1.0, 0.0};
        double den [2] = {mK, mC};
        mFilter.initialize(mTime, mTimestep, num, den, 0.0, 0.0, 0.0, 1.0);
    }

    void simulateOneTimestep()
    {
        //Get variable values from nodes
        //double p1 = mpP1->readNode(NodeHydraulic::PRESSURE);
        double q1 = mpP1->readNode(NodeHydraulic::MASSFLOW);
        double c1  = mpP1->readNode(NodeHydraulic::WAVEVARIABLE);
        double Zc1 = mpP1->readNode(NodeHydraulic::CHARIMP);
        double p2 = mpP2->readNode(NodeHydraulic::PRESSURE);
        double q2 = mpP2->readNode(NodeHydraulic::MASSFLOW);
        double c2  = mpP2->readNode(NodeHydraulic::WAVEVARIABLE);
        double Zc2 = mpP2->readNode(NodeHydraulic::CHARIMP);

        //PRV Equations

            //Calculation of Spool Position
        mFs = mPilotArea*mPref;

        double p1 = c1 + q1*Zc1;
        double Ftot = p1*mPilotArea - mFs;      //Sum of forces in x direction
        double num [2] = {1.0, 0.0};
        double den [2] = {mK, mC};
        mFilter.setNumDen(num,den);
        double x0 = mFilter.value(Ftot);            //Filter function
        double x0h = mHyst.getValue(x0, mXhyst, mDelayedX0.value());            //Hysteresis

        //Turbulent flow equation
        mTurb.setFlowCoefficient(mCq*mW*x0);
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
        if (cav)        //Cavitation, redo calculations with new c and Zc
        {
                //Calculation of Spool Position
            double p1 = c1 + q1*Zc1;
            double Ftot = p1*mPilotArea - mFs;      //Sum of forces in x direction
            double num [2] = {1.0, 0.0};
            double den [2] = {mK, mC};
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

        //Write new values to nodes

        mpP1->writeNode(NodeHydraulic::PRESSURE, p1);
        mpP1->writeNode(NodeHydraulic::MASSFLOW, q1);
        mpP2->writeNode(NodeHydraulic::PRESSURE, p2);
        mpP2->writeNode(NodeHydraulic::MASSFLOW, q2);

        mDelayedX0.update(x0h);  //Ska vara x0h

    }
};

#endif // HYDRAULICALTERNATIVEPRV_HPP_INCLUDED
