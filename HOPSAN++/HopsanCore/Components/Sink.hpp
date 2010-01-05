/*
 *  Sink.hpp
 *  HOPSAN++
 *
 *  Created by Bjšrn Eriksson on 2009-01-05.
 *  Copyright 2010 LiU. All rights reserved.
 *
 */


#ifndef SINK_HPP_INCLUDED
#define SINK_HPP_INCLUDED

class ComponentSink : public ComponentSignal
{

private:
    enum {IN};

public:
    ComponentSink(const string name,
                  const double timestep = 0.001)
	: ComponentSignal(name, timestep)
    {
        addPort("IN", "NodeSignal", IN);
    }


	void initialize()
	{
        //Nothing to initilize
	}


    void simulateOneTimestep()
    {
        //Nothing to do
    }
};

#endif // SINK_HPP_INCLUDED
