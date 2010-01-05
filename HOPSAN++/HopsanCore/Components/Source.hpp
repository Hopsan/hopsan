/*
 *  Source.hpp
 *  HOPSAN++
 *
 *  Created by Bjšrn Eriksson on 2009-01-05.
 *  Copyright 2010 LiU. All rights reserved.
 *
 */


#ifndef SOURCE_HPP_INCLUDED
#define SOURCE_HPP_INCLUDED

class ComponentSource : public ComponentSignal
{

private:
    double mValue;
    enum {OUT};

public:
    ComponentSource(const string name,
                    const double value    = 1.0,
                    const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        mValue = value;

        addPort("OUT", "NodeSignal", OUT);

        registerParameter("Value", "VŠrde", "-", mValue);
    }


	void initialize()
	{
        //Nothing to initilize
	}


    void simulateOneTimestep()
    {
        //read fron nodes
   		Node* p1_ptr = mPorts[OUT].getNodePtr();

        //Write new values to nodes
        p1_ptr->setData(NodeSignal::VALUE, mValue);
    }
};

#endif // SOURCE_HPP_INCLUDED
