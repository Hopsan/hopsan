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

//! Virtual Function, base version which gives you an error if you try to use it.
void Component::initialize(const double startT, const double stopT)
{
    cout << "Error! This function should only be used by system components, it should be overloded. For a component use initialize() instead" << endl;
    assert(false);
}

void Component::simulate(const double startT, const double stopT)
{
//TODO: adjust self.timestep or simulation depending on Ts from system above (self.timestep should be multipla of Ts)
    //double dT = stopT-startT;
    double stopTsafe = stopT - mTimestep/2.0;
    mTime = startT;
    while (mTime < stopTsafe)
    {
        simulateOneTimestep();
        mTime += mTimestep;
    }
    //cout << "simulate in: " << this->getName() << endl;
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

const string &Component::getName()
{
    return mName;
}

const string &Component::getType()
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

//void Component::setTimestep(const double timestep)
//{
//    mTimestep = timestep;
//}

//double Component::getTimestep()
//{
//    return mTimestep;
//}

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

Port* Component::addPort(const string portname, const string porttype, const NodeTypeT nodetype, const int id)
{
    ///TODO: handle trying to add multiple ports with same name or pos
    Port* new_port = CreatePort(porttype);
    new_port->mPortName = portname;
    new_port->mNodeType = nodetype;
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

    return new_port;
}

//void Component::addPort(const string portname, const string nodetype, const int id)
//{
//    ///TODO: handle trying to add multiple ports with same name or pos
//    Port* new_port = new Port(portname, nodetype);
//    new_port->mpComponent = this;    //Set port owner
//
//    if (id >= 0)
//    {
//        //Instead of allways push_back, make it possible to add ports out of order
//        if ((size_t)id+1 > mPortPtrs.size())
//        {
//            mPortPtrs.resize(id+1);
//        }
//        mPortPtrs[id] = new_port;
//    }
//    else
//    {
//        //If no id specified push back
//        mPortPtrs.push_back(new_port);     //Copy port into storage
//    }
//}

Port* Component::addPowerPort(const string portname, const string nodetype, const int id)
{
    return addPort(portname, "PowerPort", nodetype, id);
}

Port* Component::addReadPort(const string portname, const string nodetype, const int id)
{
    return addPort(portname, "ReadPort", nodetype, id);
}

Port* Component::addWritePort(const string portname, const string nodetype, const int id)
{
    return addPort(portname, "WritePort", nodetype, id);
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

void Component::setTimestep(const double timestep)
{
    mTimestep = timestep;
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
    //mInnerPortPtrs.clear();
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

void ComponentSystem::addComponent(Component *pComponent)
{
    vector<Component*> components;
    components.push_back(pComponent);
    addComponents(components);
}

////!TODO this should be in component system only, but its difficult to compile then
//Port* Component::addInnerPortSetNode(const string portname, const string porttype, Node* pNode)
//{
//    ///TODO: handle trying to add multiple ports with same name or pos
//    Port* new_port = CreatePort(porttype);
//    new_port->mPortName = portname;
//    new_port->mNodeType = pNode->getNodeType();
//    new_port->mpComponent = this;    //Set port owner
//
//    mInnerPortPtrs.push_back(new_port);     //Copy port into storage
//
//    return new_port;
//}

//! Adds a node as subnode to specified component
void Component::addSubNode(Node* node_ptr)
{
    mSubNodePtrs.push_back(node_ptr);
}

//! preAllocates log space (to speed up later access for log writing)
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

//! Tells all subnodes contained within a system to store current data in log
void ComponentSystem::logAllNodes(const double time)
{
    vector<Node*>::iterator it;
    for (it=mSubNodePtrs.begin(); it!=mSubNodePtrs.end(); ++it)
    {
        (*it)->logData(time);
    }
}

//! Adds a transparent SubSystemPort
Port* ComponentSystem::addSystemPort(const string portname)
{
    NodeTypeT undefined_nodetype;
    return addPort(portname, "SystemPort", undefined_nodetype);
}

//! Set the type C, Q, or S of the subsystem
void ComponentSystem::setTypeCQS(const string cqs_type)
{
    ///TODO: should really try to figure out a better way to do this
    ///TODO: need to do erro checking, and make sure that the specified type really is valid, first and last component should be of this type (i think)
    if (cqs_type == string("C"))
    {
        mIsComponentC = true;
        mIsComponentQ = false;
        mIsComponentSignal = false;
    }
    else if (cqs_type == string("Q"))
    {
        mIsComponentC = false;
        mIsComponentQ = true;
        mIsComponentSignal = false;
    }
    else if (cqs_type == string("S"))
    {
        mIsComponentC = false;
        mIsComponentQ = false;
        mIsComponentSignal = true;
    }
    else
    {
        cout << "Error: Specified type _" << cqs_type << "_ does not exist!" << endl;
    }
}

//! Connect two ports to each other
void ComponentSystem::connect(Port &rPort1, Port &rPort2)
{
    connect(*rPort1.mpComponent, rPort1.mPortName, *rPort2.mpComponent, rPort2.mPortName);
}

//! Connect two components with specified ports to each other, pointer version
void ComponentSystem::connect(Component *pComponent1, const string portname1, Component *pComponent2, const string portname2)
{
    connect(*pComponent1, portname1, *pComponent2, portname2);
}

//! Connect two components with specified ports to each other, reference version
void ComponentSystem::connect(Component &rComponent1, const string portname1, Component &rComponent2, const string portname2)
{
    Node* pNode;
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

    if (pPort1->isConnected() && pPort2->isConnected())
        //Both already are connected to nodes
    {
        //Do nothing, maybe raise exception?
        cout << "Both component ports are already connected: " << rComponent1.getName() << ": " << portname1 << " and " << rComponent2.getName() << ": " << portname2 << endl;
    }
    else
    {
        ///TODO: No error handling nor checks are done here
        //Check if component1 is a System component containing Component2
        if (&rComponent1 == &(rComponent2.getSystemparent()))
        {
            ///TODO: check so that the parent system port is a system port
            //Create an instance of the node specified in nodespecifications
            pNode = gCoreNodeFactory.CreateInstance(pPort2->getNodeType());
            //Set nodetype in the systemport (should be empty by default)
            pPort1->mNodeType = pPort2->getNodeType();
            //add node to components and parent system
            pPort2->setNode(pNode);
            pPort1->setNode(pNode);
            pNode->setPort(pPort1);
            rComponent1.addSubNode(pNode);    //Component1 contains this node as subnode
        }
        //Check if component2 is a System component containing Component1
        else if (&rComponent2 == &(rComponent1.getSystemparent()))
        {
            //! @TODO: both these checks could be boken out into subfunction as the code is the same only swapped 1 with 2
            //Create an instance of the node specified in nodespecifications
            pNode = gCoreNodeFactory.CreateInstance(pPort1->getNodeType());
            //Set nodetype in the systemport (should be empty by default)
            pPort2->mNodeType = pPort1->getNodeType();
            //add node to components and parentsystem
            pPort1->setNode(pNode);
            pPort2->setNode(pNode);
            pNode->setPort(pPort2);
            rComponent2.addSubNode(pNode);    //Component2 contains this node as subnode
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
            //Check so ...C-Q-C-Q-C... pattern is consistent
            else if ((pPort1->getPortType() == "PowerPort") && (pPort2->getPortType() == "PowerPort"))
            {
                if ((pPort1->mpComponent->isComponentC()) && (pPort2->mpComponent->isComponentC()))
                {
                    cout << "Both components, " << pPort1->mpComponent->getName() << " and " << pPort2->mpComponent->getName() << ", are of C-type" << endl;
                    assert(false);
                }
                else if ((pPort1->mpComponent->isComponentQ()) && (pPort2->mpComponent->isComponentQ()))
                {
                    cout << "Both components, " << pPort1->mpComponent->getName() << " and " << pPort2->mpComponent->getName() << ", are of Q-type" << endl;
                    assert(false);
                }
            }

            ///TODO: this maybe should be checked every time not only if same level, with some modification as i can connect to myself aswell
            //Check so that both systems to connect have been added to this system
            if ((&rComponent1.getSystemparent() != (Component*)this) && ((&rComponent1.getSystemparent() != (Component*)this)) )
            {
                cout << "The two components, "<< rComponent1.getName() << " and " << rComponent2.getName() << ", "<< " to be connected are not contained within the connecting system" << endl;
                assert(false);
            }

            //Check if One of them is connected to a node
            if (pPort1->isConnected() || pPort2->isConnected())
            {
                //If rComponent1 is connected to a node
                if (pPort1->isConnected())
                {
                    pNode = pPort1->getNodePtr();
                    // Check so the ports can be connected
                    if (!connectionOK(pNode, pPort1, pPort2))
                    {
                        cout << "Problem occured at connection" << rComponent1.getName() << " and " << rComponent2.getName() << endl;
                        assert(false);
                    }
                    else
                    {
                        //Set node in both components ports and add it to the parent system component
                        pPort2->setNode(pNode);

                        //Add port pointers to node
                        pNode->setPort(pPort2);
                    }
                }
                //else rComponent2 is connected to a node
                else
                {
                    pNode = pPort2->getNodePtr();
                    // Check so the ports can be connected
                    if (!connectionOK(pNode, pPort1, pPort2))
                    {
                        cout << "Problem occured at connection" << rComponent1.getName() << " and " << rComponent2.getName() << endl;
                        assert(false);
                    }
                    else
                    {
                        //Set node in both components ports and add it to the parent system component
                        pPort1->setNode(pNode);

                        //Add port pointers to node
                        pNode->setPort(pPort1);
                    }
                }
            }
            //else None of the components are connected
            else
            {
                //Create an instance of the node specified in nodespecifications
                pNode = gCoreNodeFactory.CreateInstance(pPort1->getNodeType());
                cout << "Created NodeType: " << pNode->getNodeType() << endl;
                // Check so the ports can be connected
                if (!connectionOK(pNode, pPort1, pPort2))
                {
                    cout << "Problem occured at connection" << rComponent1.getName() << " and " << rComponent2.getName() << endl;
                    assert(false);
                }
                //rComponent1.getSystemparent().addSubNode(pNode); //doesnt work getSystemparent returns Component , addSubNode is in ComponentSystem
                this->addSubNode(pNode);

                //Set node in both components ports and add it to the parent system component
                pPort1->setNode(pNode);
                pPort2->setNode(pNode);

                //Add port pointers to node
                pNode->setPort(pPort1);
                pNode->setPort(pPort2);
            }
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

void ComponentSystem::setDesiredTimestep(const double timestep) // FIPPLAR MED NU, be
{
    mDesiredTimestep = timestep;
    setTimestep(timestep);
}

void ComponentSystem::setTimestep(const double timestep) // FIPPLAR MED NU, be
{
    mTimestep = timestep;

    for (size_t s=0; s < mComponentSignalptrs.size(); ++s)
    {
        if (!(mComponentSignalptrs[s]->isComponentSystem()))
        {
            mComponentSignalptrs[s]->setTimestep(timestep);
        }
    }

    //C components
    for (size_t c=0; c < mComponentCptrs.size(); ++c)
    {
        if (!(mComponentCptrs[c]->isComponentSystem()))
        {
            mComponentCptrs[c]->setTimestep(timestep);
        }
    }

    //Q components
    for (size_t q=0; q < mComponentQptrs.size(); ++q)
    {
        if (!(mComponentQptrs[q]->isComponentSystem()))
        {
            mComponentQptrs[q]->setTimestep(timestep);
        }
    }
}

void ComponentSystem::adjustTimestep(double timestep, vector<Component*> componentPtrs)
{
    mTimestep = timestep;

    for (size_t c=0; c < componentPtrs.size(); ++c)
    {
        if (componentPtrs[c]->isComponentSystem())
        {
            double subTs = componentPtrs[c]->mDesiredTimestep;
//cout << componentPtrs[c]->mName << ", mTimestep: "<< componentPtrs[c]->mTimestep << endl;

            //If a subsystem's timestep is larger than this sytem's
            //timestep change it to this system's timestep
            if ((subTs > timestep) || (subTs < -0.0))
            {
                subTs = timestep;
            }
            //Check that subRs is a multiple of timestep
            else// if ((timestep/subTs - floor(timestep/subTs)) > 0.00001*subTs)
            {
                //subTs should get the nearest multiple of timestep as possible,
                subTs = timestep/floor(timestep/subTs+0.5);
            }
            componentPtrs[c]->setTimestep(subTs);
//cout << componentPtrs[c]->mName << ", subTs: "<< subTs << endl;
        }
        else
        {
            componentPtrs[c]->setTimestep(timestep);
//cout << componentPtrs[c]->mName << ", timestep: "<< timestep << endl;
        }
    }
}

//! Initializes a system component and all its contained components, also allocates log data memory
void ComponentSystem::initialize(const double startT, const double stopT)
{
    //preAllocate local logspace
    preAllocateLogSpace(startT, stopT);

    adjustTimestep(mTimestep, mComponentSignalptrs);
    adjustTimestep(mTimestep, mComponentCptrs);
    adjustTimestep(mTimestep, mComponentQptrs);

    //Init
    //Signal components
    for (size_t s=0; s < mComponentSignalptrs.size(); ++s)
    {
        if (mComponentSignalptrs[s]->isComponentSystem())
        {
            mComponentSignalptrs[s]->initialize(startT, stopT);
        }
        else
        {
            mComponentSignalptrs[s]->initialize();
        }
    }

    //C components
    for (size_t c=0; c < mComponentCptrs.size(); ++c)
    {
        if (mComponentCptrs[c]->isComponentSystem())
        {
            mComponentCptrs[c]->initialize(startT, stopT);
        }
        else
        {
            mComponentCptrs[c]->initialize();
        }
    }

    //Q components
    for (size_t q=0; q < mComponentQptrs.size(); ++q)
    {
        if (mComponentQptrs[q]->isComponentSystem())
        {
            mComponentQptrs[q]->initialize(startT,stopT);
        }
        else
        {
            mComponentQptrs[q]->initialize();
        }

    }
}

//! The system component version of simulate
void ComponentSystem::simulate(const double startT, const double stopT)
{
    mTime = startT;

    //Simulate
    double stopTsafe = stopT - this->mTimestep/2.0; //minus halv a timestep is here to ensure that no numerical issues occure

    while (mTime < stopTsafe)
    {
        //cout << this->getName() << ": starT: " << startT  << " stopT: " << stopT << " mTime: " << mTime << endl;
//        if (mTime > stopT-0.01)
//        {
//            //debug output for time in the last 0.01 second
//            cout << this->getName() << " time: " << mTime << " stopT: " << stopT << endl;
//        }

        logAllNodes(mTime);

        ///TODO: maybe use iterators instead
        //Signal components
        for (size_t s=0; s < mComponentSignalptrs.size(); ++s)
        {
            mComponentSignalptrs[s]->simulate(mTime, mTime+mTimestep);
        }

        //C components
        for (size_t c=0; c < mComponentCptrs.size(); ++c)
        {
            mComponentCptrs[c]->simulate(mTime, mTime+mTimestep);
        }

        //Q components
        for (size_t q=0; q < mComponentQptrs.size(); ++q)
        {
            mComponentQptrs[q]->simulate(mTime, mTime+mTimestep);
        }

        mTime += mTimestep;
    }
}

ComponentFactory gCoreComponentFactory;
DLLIMPORTEXPORT ComponentFactory* getCoreComponentFactoryPtr()
{
    return &gCoreComponentFactory;
}
