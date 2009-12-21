#include "Components.h"
#include <iostream>
#include <sstream>
#include <cassert>
#include <math.h>

//Constructor
Port::Port()
{
    mpNode  = 0;
    mpComponent = 0;
}

Port::Port(string portname, string node_type)
{
    mPortName = portname;
    mNodeType = node_type;
    mpNode  = 0;
    mpComponent = 0;
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

void Port::setNode(Node* node_ptr)
{
    mpNode = node_ptr;
}

//Constructor
Component::Component(string name, double timestep)
{
    mName = name;
    mTimestep = timestep;

    mIsComponentC = false;
    mIsComponentQ = false;
    mIsComponentSystem = false;
    mIsComponentSignal = false;
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

void Component::initialize()
{
    cout << "Warning! You should implement your own method" << endl;
    assert(false);
}

void Component::simulateOneTimestep()
{
    cout << "Warning! You should implement your own method" << endl;
    assert(false);
}

void Component::setName(string &rName)
{
    mName = rName;
}

string &Component::getName()
{
    return mName;
}

string &Component::getType()
{
    return mType;
}

void Component::setTimestep(const double timestep)
{
    mTimestep = timestep;
}

double Component::getTimestep()
{
    return mTimestep;
}

bool Component::isComponentC()
{
    return mIsComponentC;
}

bool Component::isComponentQ()
{
    return mIsComponentQ;
}

bool Component::isComponentSystem()
{
    return mIsComponentSystem;
}

bool Component::isComponentSignal()
{
    return mIsComponentSignal;
}

//void Component::addPort(const size_t port_idx, Port port)
//{
//    //Instead of push_back, make it possible to add ports out of order
//    if (port_idx+1 > mPorts.size())
//    {
//        mPorts.resize(port_idx+1);
//    }
//
//    port.mpComponent = this;    //Set port owner
//    mPorts[port_idx] = port;
//}

void Component::addPort(const string portname, const string nodetype, const int id)
{
    ///TODO: handle trying to add multiple ports with same name or pos
    Port new_port(portname, nodetype);
    new_port.mpComponent = this;    //Set port owner

    if (id >= 0)
    {
        //Instead of allways push_back, make it possible to add ports out of order
        if ((size_t)id+1 > mPorts.size())
        {
            mPorts.resize(id+1);
        }
        mPorts[id] = new_port;
    }
    else
    {
        //If no id specified push back
        mPorts.push_back(new_port);     //Copy port into storage
    }
}

//void Component::addMultiPort(const string portname, const string nodetype, const size_t nports, const size_t startctr)
//{
//    for (size_t idx=startctr; idx < nports+startctr; ++idx)
//    {
//        sstream ss;
//        ss << portname << idx;
//        addPort(ss.str(), nodetype);
//    }
//}

void Component::setSystemparent(ComponentSystem &rComponentSystem)
{
    mpSystemparent = &rComponentSystem;
}

Port &Component::getPortById(const size_t port_idx)
{
    return mPorts[port_idx];
}

Port &Component::getPort(const string portname)
{
    vector<Port>::iterator it;
    for (it=mPorts.begin(); it!=mPorts.end(); ++it)
    {
        if (it->mPortName == portname)
        {
            return *it;
        }
    }
    ///TODO: cast not found exception
    cout << "specified port: " << portname << " not found" << endl;
    assert(false);
}

ComponentSystem &Component::getSystemparent()
{
    return *mpSystemparent;
}

//constructor ComponentC
ComponentC::ComponentC(string name, double timestep) : Component(name, timestep)
{
    mType = "ComponentC";
    mIsComponentC = true;
}

//Constructor ComponentQ
ComponentQ::ComponentQ(string name, double timestep) : Component(name, timestep)
{
    mType = "ComponentQ";
    mIsComponentQ = true;
}

//Constructor
ComponentSystem::ComponentSystem(string name, double timestep) : Component(name, timestep)
{
    mType = "ComponentSystem";
    mIsComponentSystem = true;
}

void ComponentSystem::addComponents(vector<Component*> components)
{
    ///TODO: use iterator instead of idx loop
    for (size_t idx=0; idx<components.size(); ++idx)
    {
        Component* comp_ptr = components[idx];
        ///TODO: add subcomponent
        //if (comp_ptr->getType() == (string)"ComponentC")
        if (comp_ptr->isComponentC())
        {
            mComponentCptrs.push_back(comp_ptr);
        }
        //else if (comp_ptr->getType() == (string)"ComponentQ")
        else if (comp_ptr->isComponentQ())
        {
            mComponentQptrs.push_back(comp_ptr);
        }
//        else if (comp_ptr->isComponentSignal())
//        {
//            mComponentQptrs.push_back(comp_ptr);
//        }
        else
        {
            ///TODO: use exception instead
            cout << "Trying to add module of other type than c, q or signal" << endl;
            assert(false);
        }
        comp_ptr->setSystemparent(*this);
    }
}


void ComponentSystem::addComponent(Component &rComponent)
{
    vector<Component*> components;
    components.push_back(&rComponent);
    addComponents(components);
}

void ComponentSystem::addSubNode(Node* node_ptr)
{
    mSubNodePtrs.push_back(node_ptr);
}

void ComponentSystem::preAllocateLogSpace(const double startT, const double stopT)
{
    ///TODO: make sure this calculation is EXACTLY correct
    double dslots = ((double)(stopT-startT))/mTimestep;
    //std::cout << "dslots: " << dslots << std::endl;
    size_t needed_slots = (size_t)(dslots+0.5); //Round to nearest
    //size_t needed_slots = ((double)(stopT-startT))/mTimestep;

    //First allocate memory for own subnodes
    vector<Node*>::iterator it;
    for (it=mSubNodePtrs.begin(); it!=mSubNodePtrs.end(); ++it)
    {
        (*it)->preAllocateLogSpace(needed_slots);
    }

    ///TODO: Call allocate for subsubsystems

}

void ComponentSystem::logAllNodes(const double time)
{
    vector<Node*>::iterator it;
    for (it=mSubNodePtrs.begin(); it!=mSubNodePtrs.end(); ++it)
    {
        (*it)->logData(time);
    }
    ///TODO: this should do something else for now print
    //cout << "flow: " << mSubNodePtrs[0]->getData(0) << endl;
}

void ComponentSystem::connect(Component &rComponent1, const string portname1, Component &rComponent2, const string portname2)
{
    ///TODO: do it correct, for now quickhack

    //Create Node
    NodeHydraulic* node_ptr = new NodeHydraulic();

    //Set node in component ports and add it to the parent node
    rComponent1.getPort(portname1).setNode(node_ptr);
    rComponent2.getPort(portname2).setNode(node_ptr);
    //rComponent1.getSystemparent().addSubNode(node_ptr); //doesnt work getSystemparent returns Component , addSubNode is in ComponentSystem
    this->addSubNode(node_ptr);
}

void ComponentSystem::simulate(const double startT, const double stopT)
{
    ///TODO: quick hack for now
    double time = startT;

    ///TODO: problem with several subsystems
	//Init
	for (size_t i=0; i<1; ++i) 
	{
		//C components
		for (size_t c=0; c < mComponentCptrs.size(); ++c)
		{
			mComponentCptrs[c]->initialize();
		}
		
		//Q components
		for (size_t q=0; q < mComponentQptrs.size(); ++q)
		{
			mComponentQptrs[q]->initialize();
		}
    }
	
    //Simulate
	while (time < stopT)
    {
        logAllNodes(time);

        //cout << "time: " << time << endl;
        ///TODO: signal components

        ///TODO: maybe use iterators instead
        //C components
        for (size_t c=0; c < mComponentCptrs.size(); ++c)
        {
            mComponentCptrs[c]->simulate(time, mTimestep);
        }

        //Q components
        for (size_t q=0; q < mComponentQptrs.size(); ++q)
        {
            mComponentQptrs[q]->simulate(time, mTimestep);
        }

        time += mTimestep;
    }
}
