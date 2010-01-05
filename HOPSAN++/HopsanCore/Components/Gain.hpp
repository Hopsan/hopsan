/*
 *  Gain.hpp
 *  HOPSAN++
 *
 *  Created by Bjšrn Eriksson on 2009-01-05.
 *  Copyright 2010 LiU. All rights reserved.
 *
 */


#ifndef GAIN_HPP_INCLUDED
#define GAIN_HPP_INCLUDED

class ComponentGain : public ComponentSignal
{

private:
    double mGain;
    enum {IN, OUT};

public:
    ComponentGain(const string name,
                  const double gain     = 1.0,
                  const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        mGain = gain;

        addPort("IN", "NodeSignal", IN);
        addPort("OUT", "NodeSignal", OUT);

        registerParameter("Gain", "FšrstŠrkning", "-", mGain);
    }


	void initialize()
	{
        //Nothing to initilize
	}


    void simulateOneTimestep()
    {
        //read fron nodes
   		Node* p1_ptr = mPorts[IN].getNodePtr();
   		Node* p2_ptr = mPorts[OUT].getNodePtr();

        //Get variable values from nodes
        double u = p1_ptr->getData(NodeSignal::VALUE);

        //Pressure source equations
		double y = mGain*u;

        //Write new values to nodes
        p2_ptr->setData(NodeSignal::VALUE, y);
    }
};

#endif // GAIN_HPP_INCLUDED
