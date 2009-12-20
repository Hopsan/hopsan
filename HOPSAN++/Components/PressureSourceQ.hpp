/*
 *  PressureSourceQ.hpp
 *  HOPSAN++
 *
 *  Created by Björn Eriksson on 2009-12-20.
 *  Copyright 2009 LiU. All rights reserved.
 *
 */


#ifndef PRESSURESOURCEQ_HPP_INCLUDED
#define PRESSURESOURCEQ_HPP_INCLUDED

class ComponentPressureSourceQ : public ComponentQ
{
private:
    double mPressure;
    enum {P1};

public:
    ComponentPressureSourceQ(const string name, const double pressure=1.0e5, const double timestep=0.001)
	:ComponentQ(name, timestep)
    {
        mPressure = pressure;

        addPort("P1", "NodeHydraulic", P1);
    }

    void simulateOneTimestep()
    {
        //read fron node
   		Node* p1_ptr = mPorts[P1].getNodePtr();

		double c  = p1_ptr->getData(NodeHydraulic::WAVEVARIABLE);
        double Zc = p1_ptr->getData(NodeHydraulic::CHARIMP);

        //delayed line
        double q = (mPressure - c)/Zc;
		double p = mPressure;

        //write to node
        p1_ptr->setData(NodeHydraulic::MASSFLOW, q);
        p1_ptr->setData(NodeHydraulic::PRESSURE, p);
    }
};

#endif // PRESSURESOURCEQ_HPP_INCLUDED
