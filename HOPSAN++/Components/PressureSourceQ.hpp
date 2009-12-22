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
    double mStartPressure;
    double mStartFlow;
    double mPressure;
    enum {P1};

public:
    ComponentPressureSourceQ(const string name,
                             const double pressure = 1.0e5,
                             const double timestep = 0.001)
	:ComponentQ(name, timestep)
    {
        mStartPressure = 0.0;
        mStartFlow     = 0.0;
        mPressure      = pressure;

        addPort("P1", "NodeHydraulic", P1);

        registerParameter("Tryck", "Pa", mPressure);
    }


	void initialize()
	{
        //Nothing to initialize
	}


    void simulateOneTimestep()
    {
        //Get the nodes
   		Node* p1_ptr = mPorts[P1].getNodePtr();

        //Get variable values from nodes
		double c  = p1_ptr->getData(NodeHydraulic::WAVEVARIABLE);
        double Zc = p1_ptr->getData(NodeHydraulic::CHARIMP);

        //Pressure source equations
        double q = (mPressure - c)/Zc;
		double p = mPressure;

        //Write new values to nodes
        p1_ptr->setData(NodeHydraulic::MASSFLOW, q);
        p1_ptr->setData(NodeHydraulic::PRESSURE, p);
    }
};

#endif // PRESSURESOURCEQ_HPP_INCLUDED
