//!
//! @file   HydraulicVolume.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2009-12-19
//!
//! @brief Contains a Hydraulic Volume Component
//!
//$Id$

#ifndef HYDRAULICVOLUME_HPP_INCLUDED
#define HYDRAULICVOLUME_HPP_INCLUDED

#include "HopsanCore.h"

class HydraulicVolume : public ComponentC
{

private:
    double mStartPressure;
    double mStartFlow;
    double mZc;
    double mAlpha;
    double mVolume;
    double mBulkmodulus;
    Port *mpP1, *mpP2;

public:
    static Component *Creator()
    {
        std::cout << "running volume creator" << std::endl;
        return new HydraulicVolume("DefaultVolumeName");
    }

    HydraulicVolume(const string name,
                    const double volume      = 1.0e-3,
                    const double bulkmudulus = 1.0e9,
                    const double alpha       = 0.0,
                    const double timestep    = 0.001)
    : ComponentC(name, timestep)
    {
        //Set member attributes
        mStartPressure = 0.0;
        mStartFlow     = 0.0;
        mBulkmodulus   = bulkmudulus;
        mVolume        = volume;
        mZc            = mBulkmodulus/mVolume*mTimestep;
		mAlpha         = alpha;

		//Add ports to the component
        mpP1 = addPowerPort("P1", "NodeHydraulic");
        mpP2 = addPowerPort("P2", "NodeHydraulic");

        //Register changable parameters to the HOPSAN++ core
        registerParameter("V", "Volume", "[m^3]",            mVolume);
        registerParameter("Be", "Bulkmodulus", "[Pa]", mBulkmodulus);
        registerParameter("a", "LowPass coeficient to dampen standing delayline waves", "[-]",  mAlpha);
        registerParameter("Zc", "Impedans", "[Ns/m^5]",   mZc);
    }


	void initialize()
    {
        mZc = mBulkmodulus/mVolume*mTimestep; //Need to be updated at simulation start since it is volume and bulk that are set.

        //Write to nodes
        mpP1->writeNode(NodeHydraulic::MASSFLOW,     mStartFlow);
        mpP1->writeNode(NodeHydraulic::PRESSURE,     mStartPressure);
        mpP1->writeNode(NodeHydraulic::WAVEVARIABLE, mStartPressure+mZc*mStartFlow);
        mpP1->writeNode(NodeHydraulic::CHARIMP,      mZc);
        mpP2->writeNode(NodeHydraulic::MASSFLOW,     mStartFlow);
        mpP2->writeNode(NodeHydraulic::PRESSURE,     mStartPressure);
        mpP2->writeNode(NodeHydraulic::WAVEVARIABLE, mStartPressure+mZc*mStartFlow);
        mpP2->writeNode(NodeHydraulic::CHARIMP,      mZc);
	}


    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double q1  = mpP1->readNode(NodeHydraulic::MASSFLOW);
        double c1  = mpP1->readNode(NodeHydraulic::WAVEVARIABLE);
        double q2  = mpP2->readNode(NodeHydraulic::MASSFLOW);
        double c2  = mpP2->readNode(NodeHydraulic::WAVEVARIABLE);

        //Orifice equations
        double c10 = c2 + 2.0*mZc * q2;
        double c20 = c1 + 2.0*mZc * q1;
        c1 = mAlpha*c1 + (1.0-mAlpha)*c10;
        c2 = mAlpha*c2 + (1.0-mAlpha)*c20;

        //Write new values to nodes
        mpP1->writeNode(NodeHydraulic::WAVEVARIABLE, c1);
        mpP2->writeNode(NodeHydraulic::WAVEVARIABLE, c2);
        mpP1->writeNode(NodeHydraulic::CHARIMP,      mZc);
        mpP2->writeNode(NodeHydraulic::CHARIMP,      mZc);
    }
};

#endif // HYDRAULICVOLUME_HPP_INCLUDED
