/*
 *  untitled.hpp
 *  HOPSAN++
 *
 *  Created by Björn Eriksson on 2009-12-19.
 *  Copyright 2009 LiU. All rights reserved.
 *
 */

#ifndef TLMLOSSLESS_HPP_INCLUDED
#define TLMLOSSLESS_HPP_INCLUDED

#include "Components.h"
#include "Nodes.h"
#include "Delay.h"

class ComponentTLMlossless : public ComponentC
{
private:
    double mAlpha;
	Delay mDelayedC1;
	Delay mDelayedC2;
    double mZc; ///TODO: Should be only in node.
    enum {P1, P2};

public:
    ComponentTLMlossless(const string name, const double Zc=1.0e9,
						 const double timeDelay=0.1, const double alpha=0,
						 const double timestep=0.001)
	: ComponentC(name, timestep)
    {
        mZc = Zc;
		mAlpha = alpha;
        addPort("P1", "NodeHydraulic", P1);
        addPort("P2", "NodeHydraulic", P2);
		mDelayedC1.setTimeDelay(timeDelay, timestep);
		mDelayedC2.setTimeDelay(timeDelay, timestep);
    }

    void simulateOneTimestep()
    {
		//read from nodes
		Node* p1_ptr = mPorts[P1].getNodePtr();
		Node* p2_ptr = mPorts[P2].getNodePtr();

        double q1 = p1_ptr->getData(NodeHydraulic::MASSFLOW);
        double c1 = p1_ptr->getData(NodeHydraulic::WAVEVARIABLE);
        double q2 = p2_ptr->getData(NodeHydraulic::MASSFLOW);
        double c2 = p2_ptr->getData(NodeHydraulic::WAVEVARIABLE);

        //Delay Line
        double c10 = c2 + 2*mZc * q2;
        double c20 = c1 + 2*mZc * q1;
        c1 = mAlpha*c1 + (1-mAlpha)*c10;
        c2 = mAlpha*c2 + (1-mAlpha)*c20;

        //Write to nodes
        p1_ptr->setData(NodeHydraulic::WAVEVARIABLE, mDelayedC1.value());
        p2_ptr->setData(NodeHydraulic::WAVEVARIABLE, mDelayedC2.value());
        p1_ptr->setData(NodeHydraulic::CHARIMP, mZc);
        p2_ptr->setData(NodeHydraulic::CHARIMP, mZc);

		mDelayedC1.update(c1);
		mDelayedC2.update(c2);
	}
};


#endif // TLMLOSSLESS_HPP_INCLUDED
