#include "Components.h"

Port::Port()
{
    //mNodeType should already be empty
    mpNode = 0;
}

//Constructor
Port::Port(string node_type)
{
    mNodeType = node_type;
    mpNode  = 0;
}

string &Port::getNodeType()
{
    return mNodeType;
}

Node &Port::getNode()
{
    ///TODO: error handle if 0
    return *mpNode;
}

Node* Port::getNodePtr()
{
    return mpNode;
}

//Constructor
Component::Component(string name, double timestep)
{
    mName = name;
    //this->mName = name;
    mTimestep = timestep;
}

void Component::simulate(const double startT, const double Ts)
{
//TODO: adjust self.timestep or simulation depending on Ts from system above (self.timestep should be multipla of Ts)
    double stopT = startT+Ts;
    double time = startT;
    while (time < stopT)
    {
        simulateOneTimestep();
        time += mTimestep;
    }
}

void Component::setName(string &rName)
{
    mName = rName;
}

string &Component::getName()
{
    return mName;
}

void Component::setTimestep(const double timestep)
{
    mTimestep = timestep;
}

double Component::getTimestep()
{
    return mTimestep;
}

void Component::addPort(const size_t port_idx, Port port)
{
    if (port_idx+1 > mPorts.size())
    {
        mPorts.resize(port_idx+1);
    }

    mPorts[port_idx] = port;
}

//constructor ComponentC
ComponentC::ComponentC(string name, double timestep) : Component(name, timestep)
{
    mType = "ComponentC";
}

//Constructor ComponentQ
ComponentQ::ComponentQ(string name, double timestep) : Component(name, timestep)
{
    mType = "ComponentQ";
}
