/*
 *  TLMRlineR.hpp
 *  HOPSAN++
 *
 *  Created by Björn Eriksson on 2009-12-21.
 *  Copyright 2009 LiU. All rights reserved.
 *
 */

#ifndef TLMRLINER_HPP_INCLUDED
#define TLMRLINER_HPP_INCLUDED

#include <iostream>

#include "Component.h"
#include "Nodes.h"
#include "Delay.h"

class ComponentTLMRlineR : public ComponentC
{

private:
    double mStartPressure;
    double mStartFlow;
    double mTimeDelay;
    double mAlpha;
    double mZc;
    double mR1;
    double mR2;
	Delay mDelayedC1;
	Delay mDelayedC2;
    enum {P1, P2};

public:
    ComponentTLMRlineR(const string name,
                     const double zc        = 1.0e9,
                     const double timeDelay = 0.1,
                     const double r1        = 0.5,
                     const double r2        = 0.5,
                     const double alpha     = 0.0,
                     const double timestep  = 0.001)
	: ComponentC(name, timestep)
    {
        //Set member attributes
        mStartPressure = 1.0;
        mStartFlow     = 0.0;
        mTimeDelay     = timeDelay;
        mZc            = zc;
		mAlpha         = alpha;
		mR1            = r1;
		mR2            = r2;

		//Add ports to the component
        addPort("P1", "NodeHydraulic", P1);
        addPort("P2", "NodeHydraulic", P2);

        //Register changable parameters to the HOPSAN++ core
        registerParameter("TD", "Tidsfördröjning", "s",   mTimeDelay);
        registerParameter("a", "Lågpasskoeficient", "-", mAlpha);
        registerParameter("Zc", "Kappasitans", "Ns/m^5",  mZc);
        registerParameter("R1", "Resistans 1", "Ns/m^5",  mR1);
        registerParameter("R2", "Resistans 2", "Ns/m^5",  mR2);
    }


	void initialize()
    {
        //Read from nodes
		Node* p1_ptr = mPorts[P1].getNodePtr();
		Node* p2_ptr = mPorts[P2].getNodePtr();

        //Write to nodes
        p1_ptr->setData(NodeHydraulic::MASSFLOW,     mStartFlow);
        p1_ptr->setData(NodeHydraulic::PRESSURE,     mStartPressure);
        p1_ptr->setData(NodeHydraulic::WAVEVARIABLE, mStartPressure+(mZc+mR1)*mStartFlow);
        p1_ptr->setData(NodeHydraulic::CHARIMP,      mZc+mR1);
        p2_ptr->setData(NodeHydraulic::MASSFLOW,     mStartFlow);
        p2_ptr->setData(NodeHydraulic::PRESSURE,     mStartPressure);
        p2_ptr->setData(NodeHydraulic::WAVEVARIABLE, mStartPressure+(mZc+mR2)*mStartFlow);
        p2_ptr->setData(NodeHydraulic::CHARIMP,      mZc+mR2);

		//Set external parameters
		mDelayedC1.setTimeDelay(mTimeDelay-mTimestep, mTimestep); //-mTimestep sue to calc time
		mDelayedC2.setTimeDelay(mTimeDelay-mTimestep, mTimestep);

		//Init delay
        mDelayedC1.initilizeValues(mStartPressure+(mZc+mR1)*mStartFlow);
		mDelayedC2.initilizeValues(mStartPressure+(mZc+mR2)*mStartFlow);
	}


	void simulateOneTimestep()
    {
        //Get the nodes
		Node* p1_ptr = mPorts[P1].getNodePtr();
		Node* p2_ptr = mPorts[P2].getNodePtr();

        //Get variable values from nodes
        double q1 = p1_ptr->getData(NodeHydraulic::MASSFLOW);
        double p1 = p1_ptr->getData(NodeHydraulic::PRESSURE);
        double q2 = p2_ptr->getData(NodeHydraulic::MASSFLOW);
        double p2 = p2_ptr->getData(NodeHydraulic::PRESSURE);
        double c1 = p1_ptr->getData(NodeHydraulic::WAVEVARIABLE);
        double c2 = p2_ptr->getData(NodeHydraulic::WAVEVARIABLE);

        //Delay Line equations
        double c10 = p2 + (mZc+mR2) * q2;
        double c20 = p1 + (mZc+mR1) * q1;
        c1  = mAlpha*c1 + (1.0-mAlpha)*c10;
        c2  = mAlpha*c2 + (1.0-mAlpha)*c20;

        //Write new values to nodes
        p1_ptr->setData(NodeHydraulic::WAVEVARIABLE, mDelayedC1.value());
        p1_ptr->setData(NodeHydraulic::CHARIMP,      mZc+mR1);
        p2_ptr->setData(NodeHydraulic::WAVEVARIABLE, mDelayedC2.value());
        p2_ptr->setData(NodeHydraulic::CHARIMP,      mZc+mR2);

        //Update the delayed variabels
		mDelayedC1.update(c1);
		mDelayedC2.update(c2);
    }
};

#endif // TLMRLINER_HPP_INCLUDED
