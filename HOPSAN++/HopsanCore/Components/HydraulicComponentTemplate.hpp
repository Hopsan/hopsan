//!
//! @file   HydraulicComponentTemplate.hpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2009-12-23
//!
//! @brief Contains a Hydraulic Component Template
//!
//$Id$

#ifndef HYDRAULICCOMPONENTTAMPLTE_HPP_INCLUDED
#define HYDRAULICCOMPONENTTAMPLTE_HPP_INCLUDED

#include "Component.h"
#include "CoreUtilities/Delay.h"
#include "CoreUtilities/SecondOrderFilter.h"

class HydraulicComponentTemplate : public ComponentC
{

private:
    double mUserVariable1;
    double mUserVariable2;
    double mUserVariable3;
	Delay mUserDelayedVariable1;
	Delay mUserDelayedVariable2;
	SecondOrderFilter mUserFilter1;
    Port *mpPORT1, *mpPORT1;

public:
    HydraulicComponentTemplate(const string ComponentName,
                               const double userVariable1 = 1.0,
                               const double userVariable2 = 2.0,
                               const double timestep  = 0.001)
	: ComponentC(ComponentName, timestep)
    {
        //Initialize the member attributes
        mUserVariable1 = userVariable1;
        mUserVariable2 = userVariable2;
        mUserVariable3 = 42.0;

		//Add ports to the component
        addPowerPort("PORT1", "NodeHydraulic", PORT1);
        addPowerPort("PORT2", "NodeHydraulic", PORT2);

        //Register changable parameters to the HOPSAN++ core
        registerParameter("Var1", "First parameter", "kg", mUserVariable1);
        registerParameter("Par3", "Third parameter", "1/kg", mUserVariable3);
    }


	void initialize()
    {
        //Write to nodes
        mpPORT1->writeNode(NodeHydraulic::MASSFLOW,     0.0);
        mpPORT1->writeNode(NodeHydraulic::PRESSURE,     mUserVariable3);
        mpPORT1->writeNode(NodeHydraulic::WAVEVARIABLE, 0.0);
        mpPORT1->writeNode(NodeHydraulic::CHARIMP,      0.0);
        mpPORT2->writeNode(NodeHydraulic::MASSFLOW,     0.0);
        mpPORT2->writeNode(NodeHydraulic::PRESSURE,     0.0);
        mpPORT2->writeNode(NodeHydraulic::WAVEVARIABLE, 0.0);
        mpPORT2->writeNode(NodeHydraulic::CHARIMP,      0.0);

		//Init delay
        mUserDelayedVariable1.initialize(mTime, 0.0);
        mUserDelayedVariable2.initialize(mTime, 13.0);

        //Init filter
        mUserFilter1.initialize(mTime, mTimestep, 0.0, 0.0);
        double num = {0.0, 0.0, 1.0};
        double den = {2.0, 1.0, 1.0};
        mUserFilter1.setNumDen(num, den);

		//Set external parameters
		mUserDelayedVariable1.setStepDelay(18);
		mUserDelayedVariable2.setTimeDelay(3.48, mTimestep);
	}


	void simulateOneTimestep()
    {
        //Get variable values from nodes
        double q1 = mpPORT1->readNode(NodeHydraulic::MASSFLOW);
        double p1 = mpPORT1->readNode(NodeHydraulic::PRESSURE);
        double q2 = mpPORT2->readNode(NodeHydraulic::MASSFLOW);
        double p2 = mpPORT2->readNode(NodeHydraulic::PRESSURE);

        //Equations
        double c1  = mUserVariable1*q1 + p1;
        double c2  = mUserVariable2*q2 + p2;
        double zc1 = mUserDelayedVariable1.value(mUserVariable1)*mUserDelayedVariable2.value(mUserVariable2);
        double zc2 = mUserFilter1.value(c1);

        //Write new values to nodes
        mpPORT1->writeNode(NodeHydraulic::WAVEVARIABLE, c1);
        mpPORT1->writeNode(NodeHydraulic::CHARIMP,      zc1);
        mpPORT2->writeNode(NodeHydraulic::WAVEVARIABLE, c2);
        mpPORT2->writeNode(NodeHydraulic::CHARIMP,      zc2);
   }
};

#endif // HYDRAULICCOMPONENTTAMPLTE_HPP_INCLUDED
