/*
 *  Source.hpp
 *  HOPSAN++
 *
 *  Created by Björn Eriksson on 2009-01-05.
 *  Copyright 2010 LiU. All rights reserved.
 *
 */


#ifndef SIGNALSOURCE_HPP_INCLUDED
#define SIGNALSOURCE_HPP_INCLUDED

#include "HopsanCore.h"

class SignalSource : public ComponentSignal
{

private:
    double mValue;
    enum {out};

public:
    static Component *Creator()
    {
        std::cout << "running Source creator" << std::endl;
        return new SignalSource("DefaultSourceName");
    }


    SignalSource(const string name,
                    const double value    = 1.0,
                    const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        mValue = value;

        addPort("out", "NodeSignal", out);

        registerParameter("Value", "Värde", "-", mValue);
    }


	void initialize()
	{
        //Nothing to initilize
	}


    void simulateOneTimestep()
    {
        //read fron nodes
   		Node* p1_ptr = mPorts[out].getNodePtr();

        //Write new values to nodes
        p1_ptr->setData(NodeSignal::VALUE, mValue);
    }
};

#endif // SIGNALSOURCE_HPP_INCLUDED
