//!
//! @file   HydraulicVolume.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-07-01
//!
//! @brief Contains a Hydraulic Volume Component With Three Ports (temporary solution)
//!
//$Id$

#ifndef HYDRAULICVOLUME3_HPP_INCLUDED
#define HYDRAULICVOLUME3_HPP_INCLUDED

#include "../../ComponentEssentials.h"

//!
//! @brief A hydraulic volume component
//! @ingroup HydraulicComponents
//!
class HydraulicVolume3 : public ComponentC
{

private:
    double mStartPressure;
    double mStartFlow;
    double mZc;
    double mAlpha;
    double mVolume;
    double mBulkmodulus;
    Port *mpP1, *mpP2, *mpP3;

public:
    static Component *Creator()
    {
        return new HydraulicVolume3("Volume3");
    }

    HydraulicVolume3(const std::string name) : ComponentC(name)
    {
        //Set member attributes
        mTypeName = "HydraulicVolume3";
        mStartPressure = 0.0;
        mStartFlow     = 0.0;
        mBulkmodulus   = 1.0e9;
        mVolume        = 1.0e-3;
        mAlpha         = 0.0;

        //Add ports to the component
        mpP1 = addPowerPort("P1", "NodeHydraulic");
        mpP2 = addPowerPort("P2", "NodeHydraulic");
        mpP3 = addPowerPort("P3", "NodeHydraulic");

        //Register changable parameters to the HOPSAN++ core
        registerParameter("V", "Volume", "[m^3]",            mVolume);
        registerParameter("Be", "Bulkmodulus", "[Pa]", mBulkmodulus);
        registerParameter("a", "Low pass coeficient to dampen standing delayline waves", "[-]",  mAlpha);
    }


    void initialize()
    {

        mZc = 3 / 2 * mBulkmodulus/mVolume*mTimestep/(1-mAlpha); //Need to be updated at simulation start since it is volume and bulk that are set.

        //Write to nodes
        mpP1->writeNode(NodeHydraulic::MASSFLOW,     mStartFlow);
        mpP1->writeNode(NodeHydraulic::PRESSURE,     mStartPressure);
        mpP1->writeNode(NodeHydraulic::WAVEVARIABLE, mStartPressure+mZc*mStartFlow);
        mpP1->writeNode(NodeHydraulic::CHARIMP,      mZc);
        mpP2->writeNode(NodeHydraulic::MASSFLOW,     mStartFlow);
        mpP2->writeNode(NodeHydraulic::PRESSURE,     mStartPressure);
        mpP2->writeNode(NodeHydraulic::WAVEVARIABLE, mStartPressure+mZc*mStartFlow);
        mpP2->writeNode(NodeHydraulic::CHARIMP,      mZc);
        mpP3->writeNode(NodeHydraulic::MASSFLOW,     mStartFlow);
        mpP3->writeNode(NodeHydraulic::PRESSURE,     mStartPressure);
        mpP3->writeNode(NodeHydraulic::WAVEVARIABLE, mStartPressure+mZc*mStartFlow);
        mpP3->writeNode(NodeHydraulic::CHARIMP,      mZc);
    }


    void simulateOneTimestep()
    {
        //Get variable values from nodes
        //double p1  = mpP1->readNode(NodeHydraulic::PRESSURE);
        double q1  = mpP1->readNode(NodeHydraulic::MASSFLOW);
        double c1  = mpP1->readNode(NodeHydraulic::WAVEVARIABLE);
        double Zc1 = mpP1->readNode(NodeHydraulic::CHARIMP);
        //double p2  = mpP2->readNode(NodeHydraulic::PRESSURE);
        double q2  = mpP2->readNode(NodeHydraulic::MASSFLOW);
        double c2  = mpP2->readNode(NodeHydraulic::WAVEVARIABLE);
        double Zc2 = mpP2->readNode(NodeHydraulic::CHARIMP);
        //double p3  = mpP3->readNode(NodeHydraulic::PRESSURE);
        double q3  = mpP3->readNode(NodeHydraulic::MASSFLOW);
        double c3  = mpP3->readNode(NodeHydraulic::WAVEVARIABLE);
        double Zc3 = mpP3->readNode(NodeHydraulic::CHARIMP);

        //Volume equations

        double pMean = ((c1 + Zc1 * 2 * q1) + (c2 + Zc1 * 2 * q2) + (c3 + Zc1 * 2 * q3)) / 3;

        double c10 = pMean * 2 - c1 - 2 * Zc1 * q1;
        c1 = mAlpha * c1 + (1.0 - mAlpha)*c10 + (Zc1 - mZc)*q1;

        double c20 = pMean * 2 - c2 - 2 * Zc2 * q2;
        c2 = mAlpha * c2 + (1.0 - mAlpha)*c20 + (Zc2 - mZc)*q2;

        double c30 = pMean * 2 - c3 - 2 * Zc3 * q3;
        c3 = mAlpha * c3 + (1.0 - mAlpha)*c30 + (Zc3 - mZc)*q3;

        //Write new values to nodes
        mpP1->writeNode(NodeHydraulic::WAVEVARIABLE, c1);
        mpP2->writeNode(NodeHydraulic::WAVEVARIABLE, c2);
        mpP3->writeNode(NodeHydraulic::WAVEVARIABLE, c3);
        mpP1->writeNode(NodeHydraulic::CHARIMP,      mZc);
        mpP2->writeNode(NodeHydraulic::CHARIMP,      mZc);
        mpP3->writeNode(NodeHydraulic::CHARIMP,      mZc);
    }

    void finalize()
    {

    }
};

#endif // HYDRAULICVOLUME3_HPP_INCLUDED
