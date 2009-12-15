#include "Components.h"
#include <iostream>
#include <cassert>

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

void Component::addPort(const size_t port_idx, Port port)
{
    if (port_idx+1 > mPorts.size())
    {
        mPorts.resize(port_idx+1);
    }

    mPorts[port_idx] = port;
}

void Component::setSystemparent(Component &rComponent)
{
    mpSystemparent = &rComponent;
}

Port &Component::getPort(const size_t port_idx)
{
    return mPorts[port_idx];
}

Component &Component::getSystemparent()
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
            mpComponentsC.push_back(comp_ptr);
        }
        //else if (comp_ptr->getType() == (string)"ComponentQ")
        else if (comp_ptr->isComponentQ())
        {
            mpComponentsQ.push_back(comp_ptr);
        }
//        else if (comp_ptr->isComponentSignal())
//        {
//            mpComponentsQ.push_back(comp_ptr);
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
    mpSubNodes.push_back(node_ptr);
}

void ComponentSystem::logAllNodes(const double time)
{
    vector<Node*>::iterator it;
    for (it=mpSubNodes.begin(); it!=mpSubNodes.end(); ++it)
    {
        (*it)->logData(time);
    }
    ///TODO: this should do something else for now print
    cout << "flow: " << mpSubNodes[0]->getData(0) << endl;
}

void ComponentSystem::connect(Component &rComponent1, size_t portname1, Component &rComponent2, size_t portname2)
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

void ComponentSystem::simulate(const double startT, const double Ts)
{
    ///TODO: quick hack for now
    double stopT = startT + Ts;
    double time = startT;

    while (time < stopT)
    {
        logAllNodes(time);

        //cout << "time: " << time << endl;
        ///TODO: signal components

        ///TODO: maybe use iterators instead
        //C components
        for (size_t c=0; c < mpComponentsC.size(); ++c)
        {
            mpComponentsC[c]->simulate(time, mTimestep);
        }

        //Q components
        for (size_t q=0; q < mpComponentsQ.size(); ++q)
        {
            mpComponentsQ[q]->simulate(time, mTimestep);
        }

        time += mTimestep;
    }
}
