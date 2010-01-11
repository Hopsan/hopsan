#include "Component.h"
#include <iostream>
#include <sstream>
#include <cassert>
#include <math.h>

//Constructor
CompParameter::CompParameter(const string name, const string description, const string unit, double &rValue)
{
    mName = name;
    mDescription = description;
    mUnit = unit;
    mpValue = &rValue;
};


string CompParameter::getName()
{
    return mName;
}

string CompParameter::getDesc()
{
    return mDescription;
}


string CompParameter::getUnit()
{
    return mUnit;
}


double CompParameter::getValue()
{
    return *mpValue;
}


void CompParameter::setValue(const double value)
{
    *mpValue = value;
}


//Constructor
Port::Port()
{
    mpNode  = 0;
    mpComponent = 0;
    mIsConnected = false;
}

Port::Port(string portname, string node_type)
{
    mPortName = portname;
    mNodeType = node_type;
    mpNode  = 0;
    mpComponent = 0;
    mIsConnected = false;
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
    mIsConnected = true;
}

bool Port::isConnected()
{
    return mIsConnected;
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

    registerParameter("Ts", "Sample time", "[s]",   mTimestep);
}

void Component::simulate(const double startT, const double Ts)
{
//TODO: adjust self.timestep or simulation depending on Ts from system above (self.timestep should be multipla of Ts)
    double stopT = startT+Ts;
    mTime = startT;
    while (mTime < stopT)
    {
        simulateOneTimestep();
        mTime += mTimestep;
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

void Component::setName(string name)
{
    mName = name;
}

string &Component::getName()
{
    return mName;
}

string &Component::getType()
{
    return mType;
}

void Component::registerParameter(const string name, const string description, const string unit, double &rValue)
{
    ///TODO: handle trying to add multiple comppar with same name or pos
    CompParameter new_comppar(name, description, unit, rValue);
    mParameters.push_back(new_comppar); //Copy parameters into storage
}

void Component::listParametersConsole()
{
    cout <<"-----------------------------------------------" << endl << getName() << ":" << endl;
    for (size_t i=0; i<mParameters.size(); ++i)
    {
        cout << "Parameter " << i << ": " << mParameters[i].getName() << " = " << mParameters[i].getValue() << " " << mParameters[i].getUnit() << " " << mParameters[i].getDesc() << endl;
    }
    cout <<"-----------------------------------------------" << endl;
}

double Component::getParameter(const string name)
{
    for (size_t i=0; i<mParameters.size(); ++i)
    {
        if (mParameters[i].getName() == name)
        {
            return mParameters[i].getValue();
        }
    }
    cout << "No such parameter" << endl;
    assert(false);
	return 0.0;
}

void Component::setParameter(const string name, const double value)
{
    bool notset = 1;
    for (size_t i=0; i<mParameters.size(); ++i)
    {
//        if (name.compare(mParameters[i].getName()) == 0)
        if (name == mParameters[i].getName())
        {
            mParameters[i].setValue(value);
            notset = 0;
        }
    }
    if (notset)
    {
        cout << "No such parameter" << endl;
        assert(false);
    }
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
    ///TODO: error handle if request outside of vector
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

bool Component::getPort(const string portname, Port &rPort)
{
    vector<Port>::iterator it;
    for (it=mPorts.begin(); it!=mPorts.end(); ++it)
    {
        if (it->mPortName == portname)
        {
            rPort = *it;
            return true;
        }
    }
    return false;
}

ComponentSystem &Component::getSystemparent()
{
    return *mpSystemparent;
}

//constructor ComponentSignal
ComponentSignal::ComponentSignal(string name, double timestep) : Component(name, timestep)
{
    mType = "ComponentSignal";
    mIsComponentSignal = true;
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
    mInnerPorts.clear();
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
        else if (comp_ptr->isComponentSignal())
        {
            mComponentSignalptrs.push_back(comp_ptr);
        }
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

void Component::addInnerPortSetNode(const string portname, Node &rNode)
{
    ///TODO: handle trying to add multiple ports with same name or pos
    Port new_port(portname, rNode.getNodeType());
    new_port.mpComponent = this;    //Set port owner
    mInnerPorts.push_back(new_port);     //Copy port into storage
}

void Component::addSubNode(Node* node_ptr)
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
}

void ComponentSystem::connect(Component &rComponent1, const string portname1, Component &rComponent2, const string portname2)
{
    Node* node_ptr;
    Port port1, port2;
    //First some error checking

    //Check if commponents have specified ports
    if (!rComponent1.getPort(portname1, port1))
    {
        //raise Exception('type of port does not exist')
        cout << "rComponent1: "<< rComponent1.getName() << " does not have a port with name " << portname1 << endl;
        assert(false);
    }

    if (!rComponent2.getPort(portname2, port2))
    {
        //raise Exception('type of port does not exist')
        cout << "rComponent2: "<< rComponent2.getName() << " does not have a port with name " << portname2 << endl;
        assert(false);
    }

    //check if both ports have the same node type specified
    if (rComponent1.getPort(portname1).getNodeType() != rComponent2.getPort(portname2).getNodeType())
    {
        cout << "You are trying to connect a " << rComponent1.getPort(portname1).getNodeType() << " to " << rComponent2.getPort(portname2).getNodeType() << endl;
        cout << "raise Exception('component port nodetypes mismatch') or similar should be here" << endl;
        assert(false);
    }

    //Check if component1 is a System component containing Component2
        if (&rComponent1 == &(rComponent2.getSystemparent()))
        {
            //Create an instance of the node specified in nodespecifications
            node_ptr = gCoreNodeFactory.CreateInstance(rComponent2.getPort(portname2).getNodeType());
            //add node to components and parent system
            rComponent1.addInnerPortSetNode(portname1, *node_ptr); //Add and set inner port
            rComponent1.addPort(portname1, rComponent2.getPort(portname2).getNodeType()); //Add outer port
            rComponent2.getPort(portname2).setNode(node_ptr);
            rComponent1.addSubNode(node_ptr);    //Component1 contains this node as subnode
        }
        //Check if component2 is a System component containing Component1
        else if (&rComponent2 == &(rComponent1.getSystemparent()))
        {
            //Create an instance of the node specified in nodespecifications
            node_ptr = gCoreNodeFactory.CreateInstance(rComponent1.getPort(portname1).getNodeType());
            //NodeHydraulic* node_ptr = new NodeHydraulic();///TODO:
            //add node to parentsystem
            rComponent2.addInnerPortSetNode(portname2, *node_ptr); //Add and set inner port
            rComponent2.addPort(portname2, rComponent1.getPort(portname1).getNodeType()); //Add outer port
            rComponent1.getPort(portname1).setNode(node_ptr);
            rComponent2.addSubNode(node_ptr);    //Component2 contains this node as subnode
        }
        else   //Both components are on the same level
        {
            ///TODO: this maybe should be checked every time not only if same level, with some modification as i can connect to myself aswell
            //Check so that both systems to connect have been added to this system
            if ((&rComponent1.getSystemparent() != (Component*)this) && ((&rComponent1.getSystemparent() != (Component*)this)) )
            {
                cout << "The two components to be connected are not contained within the connecting system" << endl;
                assert(false);
            }

            //Create an instance of the node specified in nodespecifications
            node_ptr = gCoreNodeFactory.CreateInstance(rComponent1.getPort(portname1).getNodeType());
            cout << "Created NodeType: " << node_ptr->getNodeType() << endl;
            //Set node in both components ports and add it to the parent system component
            rComponent1.getPort(portname1).setNode(node_ptr);
            rComponent2.getPort(portname2).setNode(node_ptr);
            //rComponent1.getSystemparent().addSubNode(node_ptr); //doesnt work getSystemparent returns Component , addSubNode is in ComponentSystem
            this->addSubNode(node_ptr);
        }
}

void ComponentSystem::simulate(const double startT, const double stopT)
{
    ///TODO: quick hack for now
    mTime = startT;

    ///TODO: problem with several subsystems
	//Init
	for (size_t i=0; i<1; ++i)
	{
		//Signal components
		for (size_t s=0; s < mComponentSignalptrs.size(); ++s)
		{
			mComponentSignalptrs[s]->initialize();
		}

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
    ///TODO: while (time < stopT) will not work sometimes the loop will run even if time == stopT probably due to numeric error
	while (mTime < stopT)
    {
        if (mTime > stopT-0.01)
        {
            //debug output for time in the last 0.01 second
            cout <<"time: " << mTime << " stopT: " << stopT << endl;
        }

        logAllNodes(mTime);

        ///TODO: maybe use iterators instead
        //Signal components
        for (size_t s=0; s < mComponentSignalptrs.size(); ++s)
        {
            mComponentSignalptrs[s]->simulate(mTime, mTimestep);
        }

        //C components
        for (size_t c=0; c < mComponentCptrs.size(); ++c)
        {
            mComponentCptrs[c]->simulate(mTime, mTimestep);
        }

        //Q components
        for (size_t q=0; q < mComponentQptrs.size(); ++q)
        {
            mComponentQptrs[q]->simulate(mTime, mTimestep);
        }

        mTime += mTimestep;
    }
}

ComponentFactory gCoreComponentFactory;
DLLIMPORTEXPORT ComponentFactory* getCoreComponentFactoryPtr()
{
    return &gCoreComponentFactory;
}
