//!
//! @file   Component.cc
//! @author <FluMeS>
//! @date   2009-12-20
//!
//! @brief Contains Component base classes as well as Component Parameter class
//!
//$Id$

#include "Component.h"
#include <iostream>
#include <sstream>
#include <cassert>
#include <math.h>
#include <Port.h>

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
//    if (port_idx+1 > mPortPtrs.size())
//    {
//        mPortPtrs.resize(port_idx+1);
//    }
//
//    port.mpComponent = this;    //Set port owner
//    mPortPtrs[port_idx] = port;
//}

void Component::addPowerPort(const string portname, const string nodetype, const int id)
{
    ///TODO: handle trying to add multiple ports with same name or pos
    Port* new_port = new PowerPort(portname, nodetype);
    new_port->mpComponent = this;    //Set port owner

    if (id >= 0)
    {
        //Instead of allways push_back, make it possible to add ports out of order
        if ((size_t)id+1 > mPortPtrs.size())
        {
            mPortPtrs.resize(id+1);
        }
        mPortPtrs[id] = new_port;
    }
    else
    {
        //If no id specified push back
        mPortPtrs.push_back(new_port);     //Copy port into storage
    }
}

void Component::addPort(const string portname, const string nodetype, const int id)
{
    ///TODO: handle trying to add multiple ports with same name or pos
    Port* new_port = new Port(portname, nodetype);
    new_port->mpComponent = this;    //Set port owner

    if (id >= 0)
    {
        //Instead of allways push_back, make it possible to add ports out of order
        if ((size_t)id+1 > mPortPtrs.size())
        {
            mPortPtrs.resize(id+1);
        }
        mPortPtrs[id] = new_port;
    }
    else
    {
        //If no id specified push back
        mPortPtrs.push_back(new_port);     //Copy port into storage
    }
}

void Component::addReadPort(const string portname, const string nodetype, const int id)
{
    ///TODO: handle trying to add multiple ports with same name or pos
    Port* new_port = new ReadPort(portname, nodetype);
    new_port->mpComponent = this;    //Set port owner

    if (id >= 0)
    {
        //Instead of allways push_back, make it possible to add ports out of order
        if ((size_t)id+1 > mPortPtrs.size())
        {
            mPortPtrs.resize(id+1);
        }
        mPortPtrs[id] = new_port;
    }
    else
    {
        //If no id specified push back
        mPortPtrs.push_back(new_port);     //Copy port into storage
    }
}

void Component::addWritePort(const string portname, const string nodetype, const int id)
{
    ///TODO: handle trying to add multiple ports with same name or pos
    Port* new_port = new WritePort(portname, nodetype);
    new_port->mpComponent = this;    //Set port owner

    if (id >= 0)
    {
        //Instead of allways push_back, make it possible to add ports out of order
        if ((size_t)id+1 > mPortPtrs.size())
        {
            mPortPtrs.resize(id+1);
        }
        mPortPtrs[id] = new_port;
    }
    else
    {
        //If no id specified push back
        mPortPtrs.push_back(new_port);     //Copy port into storage
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
    return *mPortPtrs[port_idx];
}

Port &Component::getPort(const string portname)
{
    vector<Port*>::iterator it;
    for (it=mPortPtrs.begin(); it!=mPortPtrs.end(); ++it)
    {
        if ((*it)->mPortName == portname)
        {
            return *(*it);
        }
    }
    ///TODO: cast not found exception
    cout << "specified port: " << portname << " not found" << endl;
    assert(false);
}

bool Component::getPort(const string portname, Port* &prPort)
{
    vector<Port*>::iterator it;
    for (it=mPortPtrs.begin(); it!=mPortPtrs.end(); ++it)
    {
        if ((*it)->mPortName == portname)
        {
            prPort = (*it);
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
    mInnerPortPtrs.clear();
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
    ///TODO: reimplement with the new different port types
    ///TODO: handle trying to add multiple ports with same name or pos
    Port* new_port = new Port(portname, rNode.getNodeType());
    new_port->mpComponent = this;    //Set port owner
    mInnerPortPtrs.push_back(new_port);     //Copy port into storage
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

void ComponentSystem::connect(Port &rPort1, Port &rPort2)
{
    connect(*rPort1.mpComponent, rPort1.mPortName, *rPort2.mpComponent, rPort2.mPortName);
}


void ComponentSystem::connect(Component &rComponent1, const string portname1, Component &rComponent2, const string portname2)
{
    Node* node_ptr;
    Port* pPort1;
    Port* pPort2;

    //First some error checking

    //Check if commponents have specified ports
    if (!rComponent1.getPort(portname1, pPort1))
    {
        //raise Exception('type of port does not exist')
        cout << "rComponent1: "<< rComponent1.getName() << " does not have a port with name " << portname1 << endl;
        assert(false);
    }
    if (!rComponent2.getPort(portname2, pPort2)) //Not else if because pPort2 has to be set in getPort
    {
        //raise Exception('type of port does not exist')
        cout << "rComponent2: "<< rComponent2.getName() << " does not have a port with name " << portname2 << endl;
        assert(false);
    }
    else if (pPort1->isConnected() && pPort2->isConnected())
    //Both already are connected to nodes
    {
        //Do nothing, maybe raise exception?
        cout << "Both component ports are already connected: " << rComponent1.getName() << ": " << portname1 << " and " << rComponent2.getName() << ": " << portname2 << endl;
    }
    else
    {
        //check if both ports have the same node type specified
        if (pPort1->getNodeType() != pPort2->getNodeType())
        {
            cout << "You are trying to connect a " << rComponent1.getPort(portname1).getNodeType() << " to " << rComponent2.getPort(portname2).getNodeType()  << " when connect: " << rComponent1.getName() << ": " << portname1 << " and " << rComponent2.getName() << ": " << portname2 << endl;
            cout << "raise Exception('component port nodetypes mismatch') or similar should be here" << endl;
            assert(false);
        }
        ///TODO: No error handling nor cecks are done here
        //Check if component1 is a System component containing Component2
        else if (&rComponent1 == &(rComponent2.getSystemparent()))
        {
            //Create an instance of the node specified in nodespecifications
            node_ptr = gCoreNodeFactory.CreateInstance(pPort2->getNodeType());
            //add node to components and parent system
            rComponent1.addInnerPortSetNode(portname1, *node_ptr); //Add and set inner port
            rComponent1.addPort(portname1, pPort2->getNodeType()); //Add outer port
            pPort2->setNode(node_ptr);
            rComponent1.addSubNode(node_ptr);    //Component1 contains this node as subnode
        }
        //Check if component2 is a System component containing Component1
        else if (&rComponent2 == &(rComponent1.getSystemparent()))
        {
            //Create an instance of the node specified in nodespecifications
            node_ptr = gCoreNodeFactory.CreateInstance(pPort1->getNodeType());
            //NodeHydraulic* node_ptr = new NodeHydraulic();///TODO:
            //add node to parentsystem
            rComponent2.addInnerPortSetNode(portname2, *node_ptr); //Add and set inner port
            rComponent2.addPort(portname2, pPort1->getNodeType()); //Add outer port
            pPort1->setNode(node_ptr);
            rComponent2.addSubNode(node_ptr);    //Component2 contains this node as subnode
        }
        ///TODO: this maybe should be checked every time not only if same level, with some modification as i can connect to myself aswell
        //Check so that both systems to connect have been added to this system
        else if ((&rComponent1.getSystemparent() != (Component*)this) && ((&rComponent1.getSystemparent() != (Component*)this)) )
        {
            cout << "The two components, "<< rComponent1.getName() << " and " << rComponent2.getName() << ", "<< " to be connected are not contained within the connecting system" << endl;
            assert(false);
        }
        else if (pPort1->isConnected() || pPort2->isConnected())
        //One of them is connected to a node
        {
            if (pPort1->isConnected())
            //rComponent1 is connected to a node
            {
                node_ptr = pPort1->getNodePtr();
                // Check so the ports can be connected
                if (!connectionOK(node_ptr, pPort1, pPort2))
                {
                    cout << "Problem occured at connection" << rComponent1.getName() << " and " << rComponent2.getName() << endl;
                    assert(false);
                }
                else
                {
                    //Set node in both components ports and add it to the parent system component
                    pPort2->setNode(node_ptr);

                    //Add port pointers to node
                    node_ptr->setPort(pPort2);
                }
            }
            else
            //rComponent2 is connected to a node
            {
                node_ptr = pPort2->getNodePtr();
                // Check so the ports can be connected
                if (!connectionOK(node_ptr, pPort1, pPort2))
                {
                    cout << "Problem occured at connection" << rComponent1.getName() << " and " << rComponent2.getName() << endl;
                    assert(false);
                }
                else
                {
                    //Set node in both components ports and add it to the parent system component
                    pPort1->setNode(node_ptr);

                    //Add port pointers to node
                    node_ptr->setPort(pPort1);
                }
            }
        }
        else
        //None of the components is connected
        {
            //Create an instance of the node specified in nodespecifications
            node_ptr = gCoreNodeFactory.CreateInstance(pPort1->getNodeType());
            cout << "Created NodeType: " << node_ptr->getNodeType() << endl;
            // Check so the ports can be connected
            if (!connectionOK(node_ptr, pPort1, pPort2))
            {
                cout << "Problem occured at connection" << rComponent1.getName() << " and " << rComponent2.getName() << endl;
                assert(false);
            }
            //rComponent1.getSystemparent().addSubNode(node_ptr); //doesnt work getSystemparent returns Component , addSubNode is in ComponentSystem
            this->addSubNode(node_ptr);

            //Set node in both components ports and add it to the parent system component
            pPort1->setNode(node_ptr);
            pPort2->setNode(node_ptr);

            //Add port pointers to node
            node_ptr->setPort(pPort1);
            node_ptr->setPort(pPort2);
        }
        cout << "Connected " << rComponent1.getName() << ": " << portname1 << " with " << rComponent2.getName() << ": " << portname2 << " sucessfully" << endl;
    }
}


bool ComponentSystem::connectionOK(Node *pNode, Port *pPort1, Port *pPort2)
{
    size_t n_ReadPorts = 0;
    size_t n_WritePorts = 0;
    size_t n_PowerPorts = 0;

    vector<Port*>::iterator it;

    //Count the different kind of ports in the node
    for (it=(*pNode).mPortPtrs.begin(); it!=(*pNode).mPortPtrs.end(); ++it)
    {
        if ((*it)->getPortType() == "ReadPort")
        {
            n_ReadPorts += 1;
        }
        if ((*it)->getPortType() == "WritePort")
        {
            n_WritePorts += 1;
        }
        if ((*it)->getPortType() == "PowerPort")
        {
            n_PowerPorts += 1;
        }
    }
    //Check the kind of ports in the components subjected for connection
    //                                 This checks that rPort1 is not already connected to pNode
    if (((pPort1->getPortType() == "ReadPort") && !(pNode->connectedToPort(pPort1))) || ((pPort2->getPortType() == "ReadPort") && !(pNode->connectedToPort(pPort2))))
    {
        n_ReadPorts += 1;
    }
    if (((pPort1->getPortType() == "WritePort") && !(pNode->connectedToPort(pPort1))) || ((pPort2->getPortType() == "WritePort") && !(pNode->connectedToPort(pPort2))))
    {
        n_WritePorts += 1;
    }
    if (((pPort1->getPortType() == "PowerPort") && !(pNode->connectedToPort(pPort1))) || ((pPort2->getPortType() == "PowerPort") && !(pNode->connectedToPort(pPort2))))
    {
        n_PowerPorts += 1;
    }
    //Check if there are some problems with the connection
    if (n_PowerPorts > 2)
    {
        cout << "Trying to connect more than two PowerPorts to same node" << endl;
        assert(false);
    }
    if (n_WritePorts > 1)
    {
        cout << "Trying to connect more than one WritePort to same node" << endl;
        assert(false);
    }
    if ((n_PowerPorts > 0) && (n_WritePorts > 0))
    {
        cout << "Trying to connect WritePort and PowerPort to same node" << endl;
        assert(false);
    }
    if ((n_PowerPorts == 0) && (n_WritePorts == 0))
    {
        cout << "Trying to connect only ReadPorts" << endl;
        assert(false);
    }
    //It seems to be OK!
    return true;
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
	while (mTime < stopT - this->getTimestep()/2.0) //minus halv a timestep is here to ensure that no numerical issues occure
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
