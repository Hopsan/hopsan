//!
//! @file   Component.cc
//! @author FluMeS
//! @date   2009-12-20
//!
//! @brief Contains Component base classes as well as Component Parameter class
//!
//$Id$

//! @defgroup Components Components

#include <iostream>
#include <sstream>
#include <cassert>
#include <math.h>
#include "Component.h"
#include "CoreUtilities/HopsanCoreMessageHandler.h"

//! @brief Helper function to create a unique name among names from a Map
template<typename MapType>
string modifyName(MapType &rMap, string name)
{
    //Make sure name is not empty
    if (name.empty())
    {
        name = "empty_name";
    }

    size_t ctr = 1; //The suffix number
    while(rMap.count(name) != 0)
    {
        //strip suffix
        size_t foundpos = name.rfind("_");
        if (foundpos != string::npos)
        {
            if (foundpos+1 < name.size())
            {
                unsigned char nr = name.at(foundpos+1);
                //cout << "nr after _: " << nr << endl;
                //Check the ascii code for the charachter
                if ((nr >= 48) && (nr <= 57))
                {
                    //Is number lets assume that the _ found is the beginning of a suffix
                    name.erase(foundpos, string::npos);
                }
            }
        }
        //cout << "ctr: " << ctr << " stripped tempname: " << name << endl;

        //add new suffix
        stringstream suffix;
        suffix << ctr;
        name.append("_");
        name.append(suffix.str());
        ++ctr;
        //cout << "ctr: " << ctr << " appended tempname: " << name << endl;
    }
    //cout << name << endl;

    return name;
}


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
    //mTypeCQS = "";
    mTypeCQS = Component::NOCQSTYPE;

    mpSystemParent = 0;

    //registerParameter("Ts", "Sample time", "[s]",   mTimestep);
}


//! Virtual Function, base version which gives you an error if you try to use it.
void Component::initialize(const double startT, const double stopT)
{
    cout << "Error! This function should only be used by system components, it should be overloded. For a component use initialize() instead" << endl;
    assert(false);
}


//! Virtual Function, base version which gives you an error if you try to use it.
void Component::finalize(const double startT, const double stopT)
{
    cout << "Error! This function should only be used by system components, it should be overloded. For a component use finalize() instead" << endl;
    assert(false);
}


//! @todo adjust self.timestep or simulation depending on Ts from system above (self.timestep should be multipla of Ts)
void Component::simulate(const double startT, const double stopT)
{
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


void Component::finalize()
{
    cout << "Warning! You should implement your own finalize() method" << endl;
    //assert(false);
}


//! @brief Set the desired component name
//! @param [in] name The desired component name
//! @param [in] doOnlyLocalRename Only use this if you know what you are doing, default: false
//!
//! Set the desired component name, if name is already taken in a subsystem the desired name will be modified with a suffix.
//! If you set doOnlyLocalRename to true, the smart rename will not be atempted, avoid doing this as the component storage map will not be updated on anme change
//! This is a somewhat ugly fix for some special situations where we want to make sure that a smart rename is not atempted
void Component::setName(string name, bool doOnlyLocalRename)
{
    //! @todo fix the trailing _ removal
    //First strip any lonely trailing _ from the name (and give a warning)
//    string::iterator lastchar = --(name.end());
//    while (*lastchar == "_")
//    {
//        cout << "Warning underscore is not alowed in the end of a name (to avoid ugly collsion with name suffix)" << endl;
//        name.erase(lastchar);
//        lastchar = --(name.end());
//    }

    //If name same as before do nothing
    if (name != mName)
    {
        //Do we have a system parent
        if (mpSystemParent != 0)
        {
            //Need this to prevent risk of loop between rename and this function (rename cals this one again)
            if (doOnlyLocalRename)
            {
                mName = name;
            }
            else
            {
                //Do smart rename (to prevent same names)
                mpSystemParent->renameSubComponent(mName, name);
            }
        }
        else
        {
            //Ok no systemparent is set yet so lets set our own name
            mName = name;
        }
    }
}


//! Get the component name
const string &Component::getName()
{
    return mName;
}


//! Get the C, Q or S type of the component as enum
Component::typeCQS Component::getTypeCQS()
{
    return mTypeCQS;
}


//! Convert the C, Q or S type from enum to string
//! @todo This function may not need to be meber in Component, (maybe enum should be free aswell), this function may be completely useless
string Component::convertTypeCQS2String(typeCQS type)
{
    switch (type)
    {
    case C :
        return "C";
        break;
    case Q :
        return "Q";
        break;
    case S :
        return "S";
        break;
    case NOCQSTYPE :
        return "NOCQSTYPE";
        break;
    default :
        return "Invalid CQS Type";
    }
}


//! Get the CQStype as string
string Component::getTypeCQSString()
{
    switch (mTypeCQS)
    {
    case C :
        return "C";
        break;
    case Q :
        return "Q";
        break;
    case S :
        return "S";
        break;
    case NOCQSTYPE :
        return "NOCQSTYPE";
        break;
    default :
        return "Invalid CQS Type";
    }
}


//! Get the type name of the component
const string &Component::getTypeName()
{
    return mTypeName;
}


//! Register a parameter value so that it can be accessed for read and write. Set a Name, Description and Unit.
void Component::registerParameter(const string name, const string description, const string unit, double &rValue)
{
    //! @todo handle trying to add multiple comppar with same name
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


double Component::getParameterValue(const string name)
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


const vector<string> Component::getParameterNames()
{
    vector<string> names;
    for (size_t i=0; i<mParameters.size(); ++i)
    {
        names.push_back(mParameters[i].getName());
    }
    return names;
}


const string Component::getParameterUnit(const string name)
{
    for (size_t i=0; i<mParameters.size(); ++i)
    {
        if (mParameters[i].getName() == name)
        {
            return mParameters[i].getUnit();
        }
    }
    cout << "No such parameter" << endl;
    assert(false);
}


const string Component::getParameterDescription(const string name)
{
    for (size_t i=0; i<mParameters.size(); ++i)
    {
        if (mParameters[i].getName() == name)
        {
            return mParameters[i].getDesc();
        }
    }
    cout << "No such parameter" << endl;
    assert(false);
}


vector<CompParameter> Component::getParameterVector()
{
    return mParameters;
}


map<string, double> Component::getParameterMap()
{
    map<string, double> parameterMap;
    for (size_t i=0; i<mParameters.size(); ++i)
    {
        parameterMap.insert(pair<string, double>(mParameters[i].getName(), mParameters[i].getValue()));
    }
    return parameterMap;
}


void Component::setParameterValue(const string name, const double value)
{
    bool notset = 1;
    for (size_t i=0; i<mParameters.size(); ++i)
    {
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


//! @todo Maby not have this function, solve in some other nicer way
vector<Port*> Component::getPortPtrVector()
{
    vector<Port*> vec;
    vec.clear();
    PortPtrMapT::iterator ports_it;

    //Copy every port pointer
    for (ports_it = mPortPtrMap.begin(); ports_it != mPortPtrMap.end(); ++ports_it)
    {
        vec.push_back(ports_it->second);
    }
    return vec;
}


void Component::setDesiredTimestep(const double timestep)
{
    cout << "Warning this function setDesiredTimestep is only available on subsystem components" << endl;
    assert(false);
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


double *Component::getTimePtr()
{
    return &mTime;
}


//! @brief Adds a port to the component
//! @param [in] portname The desired name of the port (may be automatically changed)
//! @param [in] porttype The type of port
//! @param [in] nodetype The type of node that must be connected to the port
Port* Component::addPort(const string portname, Port::PORTTYPE porttype, const NodeTypeT nodetype, Port::CONREQ connection_requirement)
{
    Port* new_port = CreatePort(porttype);
    new_port->mNodeType = nodetype;
    new_port->mpComponent = this;    //Set port owner
    //Set wheter the port must be connected before simulation
    if (connection_requirement == Port::OPTIONAL)
    {
        //! @todo maybe use a string for OPTIONAL instead, to reduce the number of compiletime dependencies, will need to think about that a bit more
        new_port->mConnectionRequired = false;
    }

    //Make sure name is unique before insert
    string newname = modifyName<PortPtrMapT>(mPortPtrMap, portname);
    new_port->mPortName = newname;

    mPortPtrMap.insert(PortPtrPairT(newname, new_port));

    //Signal autmatic name change
    if (newname != portname)
    {
        gCoreMessageHandler.addInfoMessage("Automatically changed name of added port from: {" + portname + "} to {" + newname + "}", 1);
    }

    return new_port;
}


//! @brief Convenience method to add a PowerPort
//! @param [in] porttype The type of port
//! @param [in] nodetype The type of node that must be connected to the port
Port* Component::addPowerPort(const string portname, const string nodetype)
{
    return addPort(portname, Port::POWERPORT, nodetype, Port::REQUIRED);
}


//! @brief Convenience method to add a ReadPort
//! @param [in] porttype The type of port
//! @param [in] nodetype The type of node that must be connected to the port
Port* Component::addReadPort(const string portname, const string nodetype, Port::CONREQ connection_requirement)
{
    return addPort(portname, Port::READPORT, nodetype, connection_requirement);
}


//! @brief Convenience method to add a WritePort
//! @param [in] porttype The type of port
//! @param [in] nodetype The type of node that must be connected to the port
Port* Component::addWritePort(const string portname, const string nodetype, Port::CONREQ connection_requirement)
{
    return addPort(portname, Port::WRITEPORT, nodetype, connection_requirement);
}


//! @todo this could be a template function to use with all rename in map
string Component::renamePort(const string oldname, const string newname)
{
    if (mPortPtrMap.count(oldname) != 0)
    {
        Port* temp_port_ptr;
        PortPtrMapT::iterator it;

        it = mPortPtrMap.find(oldname); //Find iterator to port
        temp_port_ptr = it->second;     //Backup copy of port ptr
        mPortPtrMap.erase(it);          //Erase old value
        string modnewname = modifyName<PortPtrMapT>(mPortPtrMap, newname); //Make sure new name is unique
        temp_port_ptr->mPortName = modnewname;  //Set new name in port
        mPortPtrMap.insert(PortPtrPairT(modnewname, temp_port_ptr)); //Re add to map
        return modnewname;
    }
    else
    {
        gCoreMessageHandler.addWarningMessage("Trying to rename port {" + oldname + "}, but not found", 1);
        return oldname;
    }
}


void Component::deletePort(const string name)
{
    PortPtrMapT::iterator it;
    it = mPortPtrMap.find(name);
    if (it != mPortPtrMap.end())
    {
        mPortPtrMap.erase(it);
    }
    else
    {
        gCoreMessageHandler.addWarningMessage("Trying to delete port {" + name + "}, but not found", 1);
    }
}


void Component::setSystemParent(ComponentSystem &rComponentSystem)
{
    mpSystemParent = &rComponentSystem;
}

//Port &Component::getPortById(const size_t port_idx)
//{
//    //! @todo error handle if request outside of vector
//    return *mPortPtrs[port_idx];
//}


Port *Component::getPort(const string portname)
{
    PortPtrMapT::iterator it;
    it = mPortPtrMap.find(portname);
    if (it != mPortPtrMap.end())
    {
        return it->second;
    }
    else
    {
        gCoreMessageHandler.addWarningMessage("Trying to get port {" + portname + "}, but not found, pointer invalid", 1);
        return 0;
    }
}


bool Component::getPort(const string portname, Port* &rpPort)
{
    rpPort = getPort(portname);
    if (rpPort != 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}


void Component::setTimestep(const double timestep)
{
    mTimestep = timestep;
}


//! The subcomponent storage, Makes it easier to add (with auto unique name), erase and get components
//! @todo quite ugly code for now
void ComponentSystem::SubComponentStorage::add(Component* pComponent)
{
    //First check if the name already exists, in that case change the suffix
    string modname = modifyName<SubComponentMapT>(mSubComponentMap, pComponent->getName());
    pComponent->setName(modname);

    //Add to the cqs component vectors
    switch (pComponent->getTypeCQS())
    {
    case Component::C :
        mComponentCptrs.push_back(pComponent);
        break;
    case Component::Q :
        mComponentQptrs.push_back(pComponent);
        break;
    case Component::S :
        mComponentSignalptrs.push_back(pComponent);
        break;
    case Component::NOCQSTYPE :
        mComponentUndefinedptrs.push_back(pComponent);
        break;
    default :
            gCoreMessageHandler.addErrorMessage("Trying to add module with unspecified CQS type: " + pComponent->getTypeCQSString()  + ", (Not added)");
        return;
    }

    mSubComponentMap.insert(pair<string, Component*>(modname, pComponent));
}


Component* ComponentSystem::SubComponentStorage::get(const string &rName)
{
    map<string, Component*>::iterator it;
    it = mSubComponentMap.find(rName);
    if (it != mSubComponentMap.end())
    {
        return it->second;
    }
    else
    {
        cout << "The component you requested: " << rName << " does not exist" << endl;
        return 0;
    }
}


void ComponentSystem::SubComponentStorage::remove(const string &rName)
{
    map<string, Component*>::iterator it;
    it = mSubComponentMap.find(rName);
    if (it != mSubComponentMap.end())
    {
        vector<Component*>::iterator cit; //Component iterator
        switch (it->second->getTypeCQS())
        {
        case Component::C :
            for (cit = mComponentCptrs.begin(); cit != mComponentCptrs.end(); ++cit)
            {
                if ( (*cit)->getName() == rName )
                {
                    mComponentCptrs.erase(cit);
                    break;
                }
            }
            break;
        case Component::Q :
            for (cit = mComponentQptrs.begin(); cit != mComponentQptrs.end(); ++cit)
            {
                if ( (*cit)->getName() == rName )
                {
                    mComponentQptrs.erase(cit);
                    break;
                }
            }
            break;
        case Component::S :
            for (cit = mComponentSignalptrs.begin(); cit != mComponentSignalptrs.end(); ++cit)
            {
                if ( (*cit)->getName() == rName )
                {
                    mComponentSignalptrs.erase(cit);
                    break;
                }
            }
            break;
        case Component::NOCQSTYPE :
            for (cit = mComponentUndefinedptrs.begin(); cit != mComponentUndefinedptrs.end(); ++cit)
            {
                if ( (*cit)->getName() == rName )
                {
                    mComponentUndefinedptrs.erase(cit);
                    break;
                }
            }
            break;
        default :
            cout << "This should not happen neither C Q or S type" << endl;
            assert(false);
        }

        mSubComponentMap.erase(it);
    }
    else
    {
        gCoreMessageHandler.addErrorMessage("The component you are trying to remove: " + rName + " does not exist (Does Nothing)");
    }
}


bool ComponentSystem::SubComponentStorage::have(const string &rName)
{
    if (mSubComponentMap.count(rName) > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}


void ComponentSystem::SubComponentStorage::rename(const string &rOldName, const string &rNewName)
{
    //cout << "Trying to rename: " << old_name << " to " << new_name << endl;
    //First find the post in the map where the old name resides, copy the data stored there
    map<string, Component*>::iterator it = mSubComponentMap.find(rOldName);
    Component* temp_c_ptr;
    if (it != mSubComponentMap.end())
    {
        //If found erase old record
        temp_c_ptr = it->second;
        mSubComponentMap.erase(it);

        //insert new (with new name)
        string mod_new_name = modifyName<SubComponentMapT>(mSubComponentMap, rNewName);

        //cout << "new name is: " << mod_name << endl;
        mSubComponentMap.insert(pair<string, Component*>(mod_new_name, temp_c_ptr));

        //Now change the actual component name, without trying to do rename (we are in rename now, would cause infinite loop)
        //! @todo it might be a good idea to rething all of this renaming stuff, right now its prety strange (but works), setname loop
        temp_c_ptr->setName(mod_new_name, true);
    }
    else
    {
        cout << "Error no component with old_name: " << rOldName << " found!" << endl;
        assert(false);
    }
}


//! @brief Change the cqs type of a stored subsystem component
bool ComponentSystem::SubComponentStorage::changeTypeCQS(const string &rName, const typeCQS newType)
{
    Component* tmpptr;
    //First get the component ptr and check if we are requesting new type
    tmpptr = get(rName);
    if (newType != tmpptr->getTypeCQS())
    {
        //check that it is a system component, in that case change the cqs type
        if ( tmpptr->isComponentSystem() )
        {
            //Cast to system ptr
            //! @todo should have a member function that return systemcomponent ptrs
            ComponentSystem* tmpsysptr = dynamic_cast<ComponentSystem*>(tmpptr);

            //Remove old version
            remove(rName);

            //Change cqsType localy in the subcomponent, make sure to set true to avoid looping back to this rename
            tmpsysptr->setTypeCQS(newType, true);

            //readd to system
            add(tmpsysptr);
        }
        else
        {
            return false;
        }
    }
    return true;
}


ComponentSystem *Component::getSystemParent()
{
    return mpSystemParent;
}


//constructor ComponentSignal
ComponentSignal::ComponentSignal(string name, double timestep) : Component(name, timestep)
{
    //mTypeCQS = "S";
    mTypeCQS = Component::S;
    mIsComponentSignal = true;
}


//constructor ComponentC
ComponentC::ComponentC(string name, double timestep) : Component(name, timestep)
{
    //mTypeCQS = "C";
    mTypeCQS = Component::C;
    mIsComponentC = true;
}


//Constructor ComponentQ
ComponentQ::ComponentQ(string name, double timestep) : Component(name, timestep)
{
    //mTypeCQS = "Q";
    mTypeCQS = Component::Q;
    mIsComponentQ = true;
}


//Constructor
ComponentSystem::ComponentSystem(string name, double timestep) : Component(name, timestep)
{
    mTypeName = "ComponentSystem";
    mIsComponentSystem = true;
    mDesiredTimestep = timestep;
}


double ComponentSystem::getDesiredTimeStep()
{
    return mDesiredTimestep;
}


//! Sets a bool which is looked at in initialization and simulation loops.
//! This method can be used by users e.g. GUIs to stop an started initializatiion/simulation process
void ComponentSystem::stop()
{
    mStop = true;
}


void ComponentSystem::addComponents(vector<Component*> components)
{
    //! @todo use iterator instead of idx loop (not really necessary)
    for (size_t idx=0; idx<components.size(); ++idx)
    {
        mSubComponentStorage.add(components[idx]);
        components[idx]->setSystemParent(*this);
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


//! Rename a sub component and automatically fix unique names
void ComponentSystem::renameSubComponent(string old_name, string new_name)
{
    mSubComponentStorage.rename(old_name, new_name);
}


//! Remove a dub component from a system, can also be used to actually delete the component
//! @param[in] name The name of the component to remove from the system
//! @param[in] doDelete Set this to true if the component should be deleted after removal
void ComponentSystem::removeSubComponent(string name, bool doDelete)
{
    Component* c_ptr = getSubComponent(name);
    removeSubComponent(c_ptr, doDelete);
}


//! Remove a sub component from a system, can also be used to actually delete the component
//! @param[in] c_ptr A pointer to the component to remove
//! @param[in] doDelete Set this to true if the component should be deleted after removal
void ComponentSystem::removeSubComponent(Component* c_ptr, bool doDelete)
{
    //Disconnect all ports before erase from system
    PortPtrMapT::iterator ports_it;
    vector<Port*>::iterator conn_ports_it;
    for (ports_it = c_ptr->mPortPtrMap.begin(); ports_it != c_ptr->mPortPtrMap.end(); ++ports_it)
    {
        vector<Port*> connected_ports = ports_it->second->getConnectedPorts(); //Get a copy of the connected ports ptr vector
        //We can not use an iterator directly connected to the vector inside the port as this will be changed by the disconnect calls
        for (conn_ports_it = connected_ports.begin(); conn_ports_it != connected_ports.end(); ++conn_ports_it)
        {
            disconnect(ports_it->second, *conn_ports_it);
        }
    }

    //Remove from storage
    mSubComponentStorage.remove(c_ptr->getName());

    //Shall we also delete the component completely
    if (doDelete)
    {
        delete c_ptr; //! @todo can I really delete here or do I need to use the factory for external components
    }
}


//! @brief Get a Component ptr the the component with supplied name, can also return a ptr to self if no subcomponent found
//! @todo for this to work we need to amke sure that the system and its sub components have unique names
Component* ComponentSystem::getComponent(string name)
{
    cout << "getComponent: " << name << " in: " << mName << endl;
    //First try to find among subcomponents
    Component *tmp = getSubComponent(name);
    if (tmp != 0)
    {
        return tmp;
    }
    else if (name == mName)
    {
        cout << "getComponent (name == mName): " << name << " in: " << mName << " returning this" << endl;
        return this;
    }
    else
    {
        cout << "return null" << endl;
        return 0;
    }
}


Component* ComponentSystem::getSubComponent(string name)
{
    return mSubComponentStorage.get(name);
}


ComponentSystem* ComponentSystem::getSubComponentSystem(string name)
{
    Component* temp_component_ptr = getSubComponent(name);
    ComponentSystem* temp_compsys_ptr = dynamic_cast<ComponentSystem*>(temp_component_ptr);

    if (temp_compsys_ptr == NULL)
    {
        cout << "dynamic cast failed, maybe " << name << " is not a component system" << endl;
        assert(false);
    }

    return temp_compsys_ptr;
}


vector<string> ComponentSystem::getSubComponentNames()
{
    //! @todo for now create a vector of the component names, later maybe we should return a pointer to the real internal map
    vector<string> names;
    map<string, Component*>::iterator it;
    for (it = mSubComponentStorage.mSubComponentMap.begin(); it != mSubComponentStorage.mSubComponentMap.end(); ++it)
    {
        names.push_back(it->first);
    }

    return names;
}


bool  ComponentSystem::haveSubComponent(string name)
{
    return mSubComponentStorage.have(name);
}


//! Adds a node as subnode in the system
void ComponentSystem::addSubNode(Node* node_ptr)
{
    mSubNodePtrs.push_back(node_ptr);
}


//! Removes a previously added node
void ComponentSystem::removeSubNode(Node* node_ptr)
{
    vector<Node*>::iterator it;
    for (it=mSubNodePtrs.begin(); it!=mSubNodePtrs.end(); ++it)
    {
        if (*it == node_ptr)
        {
            mSubNodePtrs.erase(it);
            break;
        }
    }
    //! @todo some notification if you try to remove something that does not exist (can not check it==mSubNodePtrs.end() ) this check can be OK after an successfull erase
}


//! preAllocates log space (to speed up later access for log writing)
void ComponentSystem::preAllocateLogSpace(const double startT, const double stopT)
{
    cout << "stopT = " << stopT << ", startT = " << startT << ", mTimestep = " << mTimestep << endl;

    //First allocate memory for own subnodes
    vector<Node*>::iterator it;
    for (it=mSubNodePtrs.begin(); it!=mSubNodePtrs.end(); ++it)
    {
        if (mStop)
            break;

        //! @todo this is an ugly quit hack test
        (*it)->setLogSettingsNSamples(2048, startT, stopT, mTimestep);
        //(*it)->setLogSettingsSkipFactor(1, startT, stopT, mTimestep);
        //(*it)->preAllocateLogSpace(needed_slots);
        (*it)->preAllocateLogSpace();
    }



    //! @todo Call allocate for subsubsystems

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
Port* ComponentSystem::addSystemPort(string portname)
{
    if (portname.empty())
    {
        //Force default portname p, if nothing else specified
        portname = "p";
    }

    return addPort(portname, Port::SYSTEMPORT, "undefined_nodetype", Port::REQUIRED);
}


//! Rename system port
string ComponentSystem::renameSystemPort(const string oldname, const string newname)
{
    return renamePort(oldname,newname);
}


//! delete System prot
void ComponentSystem::deleteSystemPort(const string name)
{
    deletePort(name);
}


//! Set the type C, Q, or S of the subsystem by using string
void ComponentSystem::setTypeCQS(const string cqs_type, bool doOnlyLocalSet)
{
    if (cqs_type == string("C"))
    {
        setTypeCQS(Component::C, doOnlyLocalSet);
    }
    else if (cqs_type == string("Q"))
    {
        setTypeCQS(Component::Q, doOnlyLocalSet);
    }
    else if (cqs_type == string("S"))
    {
        setTypeCQS(Component::S, doOnlyLocalSet);
    }
    else
    {
        cout << "Error: Specified type _" << cqs_type << "_ does not exist!" << endl;
        gCoreMessageHandler.addWarningMessage("Specified type: " + cqs_type + " does not exist!, System CQStype unchanged");
    }
}


//! Set the type C, Q, or S of the subsystem
void ComponentSystem::setTypeCQS(typeCQS cqs_type, bool doOnlyLocalSet)
{
    //! @todo should really try to figure out a better way to do this
    //! @todo need to do erro checking, and make sure that the specified type really is valid, first and last component should be of this type (i think)

    //If type same as before do nothing
    if (cqs_type !=  mTypeCQS)
    {
        //Do we have a system parent
        if ( (mpSystemParent != 0) && (!doOnlyLocalSet) )
        {
            //Request change by our parent (som parent cahnges are neeeded)
            mpSystemParent->mSubComponentStorage.changeTypeCQS(mName, cqs_type);
        }
        else
        {
            switch (cqs_type)
            {
            case Component::C :
                mTypeCQS = Component::C;
                mIsComponentC = true;
                mIsComponentQ = false;
                mIsComponentSignal = false;
                break;

            case Component::Q :
                mTypeCQS = Component::Q;
                mIsComponentC = false;
                mIsComponentQ = true;
                mIsComponentSignal = false;
                break;

            case Component::S :
                mTypeCQS = Component::S;
                mIsComponentC = false;
                mIsComponentQ = false;
                mIsComponentSignal = true;
                break;

            case Component::NOCQSTYPE :
                mTypeCQS = Component::NOCQSTYPE;
                mIsComponentC = false;
                mIsComponentQ = false;
                mIsComponentSignal = false;
                break;

            default :
                cout << "Error: Specified type _" << getTypeCQSString() << "_ does not exist!" << endl;
                gCoreMessageHandler.addWarningMessage("Specified type: " + getTypeCQSString() + " does not exist!, System CQStype unchanged");
            }
        }
    }
}


//! Connect two ports to each other ptr version
bool ComponentSystem::connect(Port *pPort1, Port *pPort2)
{
    return connect(pPort1->mpComponent, pPort1->mPortName, pPort2->mpComponent, pPort2->mPortName);
}


//! Connect two ports to each other ref version
bool ComponentSystem::connect(Port &rPort1, Port &rPort2)
{
    return connect(*rPort1.mpComponent, rPort1.mPortName, *rPort2.mpComponent, rPort2.mPortName);
}


//! Connect two components with specified ports to each other, pointer version
bool ComponentSystem::connect(Component *pComponent1, const string portname1, Component *pComponent2, const string portname2)
{
    return connect(*pComponent1, portname1, *pComponent2, portname2);
}

//! Connect two commponents string version
bool ComponentSystem::connect(string compname1, string portname1, string compname2, string portname2)
{
    return connect( *getComponent(compname1), portname1, *getComponent(compname2), portname2 );
}


//! Connect two components with specified ports to each other, reference version
//! @todo Cleanup this connect madness only 1 or two alternatives maybe string version default (or pointer version)
bool ComponentSystem::connect(Component &rComponent1, const string portname1, Component &rComponent2, const string portname2)
{
    Node* pNode;
    Port* pPort1;
    Port* pPort2;

    //First some error checking
    stringstream ss; //Error string stream

    //Check if commponents have specified ports
    if (!rComponent1.getPort(portname1, pPort1))
    {
        ss << "Component: "<< rComponent1.getName() << " does not have a port named " << portname1;
        cout << ss.str() << endl;
        gCoreMessageHandler.addErrorMessage(ss.str());
        return false;
    }

    if (!rComponent2.getPort(portname2, pPort2)) //Not else if because pPort2 has to be set in getPort
    {
        //raise Exception('type of port does not exist')
        ss << "Component: "<< rComponent2.getName() << " does not have a port named " << portname2;
        cout << ss.str() << endl;
        gCoreMessageHandler.addErrorMessage(ss.str());
        return false;
    }

    if (pPort1 == pPort2)
    {
        ss << "You can not connect a port to it self";
        gCoreMessageHandler.addErrorMessage(ss.str());
        return false;
    }

    //! @todo this will be a problem if we want to connect sensors and such
    if (pPort1->isConnected() && pPort2->isConnected())
        //Both already are connected to nodes
    {
        ss << "Both component ports are already connected: " << rComponent1.getName() << ": " << portname1 << " and " << rComponent2.getName() << ": " << portname2;
        cout << ss.str() << endl;
        gCoreMessageHandler.addWarningMessage(ss.str());
        return false;
    }
    else
    {
        //! @todo must make sure that ONLY ONE poerport can be internally connected to systemports (no sensors or anything else) to amke stuff simpler
        //! @todo No error handling nor checks are done here
        //Check if component1 is a System component containing Component2
        if (&rComponent1 == rComponent2.getSystemParent())
        {
            //! @todo check so that the parent system port is a system port
            //Create an instance of the node specified in nodespecifications
            pNode = gCoreNodeFactory.CreateInstance(pPort2->getNodeType());
            //Set nodetype in the systemport (should be empty by default)
            pPort1->mNodeType = pPort2->getNodeType();
            //add node to components and parent system
            pPort2->setNode(pNode);
            pPort1->setNode(pNode);
            pNode->setPort(pPort1);
            pNode->setPort(pPort2);
            rComponent2.getSystemParent()->addSubNode(pNode);    //Component1 will contain this node as subnode
            //let the ports know about each other
            pPort1->addConnectedPort(pPort2);
            pPort2->addConnectedPort(pPort1);
        }
        //Check if component2 is a System component containing Component1
        else if (&rComponent2 == rComponent1.getSystemParent())
        {
            //! @todo both these checks could be boken out into subfunction as the code is the same only swapped 1 with 2
            //Create an instance of the node specified in nodespecifications
            pNode = gCoreNodeFactory.CreateInstance(pPort1->getNodeType());
            //Set nodetype in the systemport (should be empty by default)
            pPort2->mNodeType = pPort1->getNodeType();
            //add node to components and parentsystem
            pPort1->setNode(pNode);
            pPort2->setNode(pNode);
            pNode->setPort(pPort2);
            pNode->setPort(pPort1);
            rComponent1.getSystemParent()->addSubNode(pNode);    //Component2 will contain this node as subnode
            //let the ports know about each other
            pPort1->addConnectedPort(pPort2);
            pPort2->addConnectedPort(pPort1);
        }
        else
        {
            //check if both ports have the same node type specified
            if (pPort1->getNodeType() != pPort2->getNodeType())
            {
                ss << "You can not connect a {" << rComponent1.getPort(portname1)->getNodeType() << "} port to a {" << rComponent2.getPort(portname2)->getNodeType()  << "} port." << std::endl << "When connecting: {" << rComponent1.getName() << "::" << portname1 << "} to {" << rComponent2.getName() << "::" << portname2 << "}";
                cout << ss.str() << endl;
                gCoreMessageHandler.addErrorMessage(ss.str());
                return false;
            }
            //Check so ...C-Q-C-Q-C... pattern is consistent
            else if ((pPort1->getPortType() == Port::POWERPORT) && (pPort2->getPortType() == Port::POWERPORT))
            {
                if ((pPort1->mpComponent->isComponentC()) && (pPort2->mpComponent->isComponentC()))
                {
                    ss << "Both components, {" << pPort1->mpComponent->getName() << "} and {" << pPort2->mpComponent->getName() << "}, are of C-type";
                    cout << ss.str() << endl;
                    gCoreMessageHandler.addErrorMessage(ss.str());
                    return false;
                }
                else if ((pPort1->mpComponent->isComponentQ()) && (pPort2->mpComponent->isComponentQ()))
                {
                    ss << "Both components, {" << pPort1->mpComponent->getName() << "} and {" << pPort2->mpComponent->getName() << "}, are of Q-type";
                    cout << ss.str() << endl;
                    gCoreMessageHandler.addErrorMessage(ss.str());
                    return false;
                }
            }

            //! @todo this maybe should be checked every time not only if same level, with some modification as i can connect to myself aswell
            //Check so that both systems to connect have been added to this system
            if ((rComponent1.getSystemParent() != (Component*)this) && ((rComponent1.getSystemParent() != (Component*)this)) )
            {
                ss << "The components, {"<< rComponent1.getName() << "} and {" << rComponent2.getName() << "}, "<< "must belong to the same subsystem";
                cout << ss.str() << endl;
                gCoreMessageHandler.addErrorMessage(ss.str());
                return false;
            }

            //Check if One of the ports already is connected to a node
            if (pPort1->isConnected() || pPort2->isConnected())
            {
                //If rComponent1 is connected to a node
                if (pPort1->isConnected())
                {
                    pNode = pPort1->getNodePtr();
                    // Check so the ports can be connected
                    if (!connectionOK(pNode, pPort1, pPort2))
                    {
                        ss << "Problem occured at connection" << rComponent1.getName() << " and " << rComponent2.getName();
                        cout << ss.str() << endl;
                        gCoreMessageHandler.addErrorMessage(ss.str());
                        return false;
                    }
                    else
                    {
                        //Set node in both components ports and add it to the parent system component
                        pPort2->setNode(pNode);

                        //Add port pointers to node
                        pNode->setPort(pPort2);

                        //let the ports know about each other
                        pPort1->addConnectedPort(pPort2);
                        pPort2->addConnectedPort(pPort1);

                    }
                }
                //else rComponent2 is connected to a node
                else
                {
                    pNode = pPort2->getNodePtr();
                    // Check so the ports can be connected
                    if (!connectionOK(pNode, pPort1, pPort2))
                    {
                        ss << "Problem occured at connection" << rComponent1.getName() << " and " << rComponent2.getName();
                        cout << ss.str() << endl;
                        gCoreMessageHandler.addErrorMessage(ss.str());
                        return false;
                    }
                    else
                    {
                        //Set node in both components ports and add it to the parent system component
                        pPort1->setNode(pNode);

                        //Add port pointers to node
                        pNode->setPort(pPort1);

                        //let the ports know about each other
                        pPort1->addConnectedPort(pPort2);
                        pPort2->addConnectedPort(pPort1);
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
                    ss << "Problem occured at connection" << rComponent1.getName() << " and " << rComponent2.getName();
                    cout << ss.str() << endl;
                    gCoreMessageHandler.addErrorMessage(ss.str());
                    return false;
                }
                //rComponent1.getSystemparent().addSubNode(pNode); //doesnt work getSystemparent returns Component , addSubNode is in ComponentSystem
                this->addSubNode(pNode);

                //Set node in both components ports and add it to the parent system component
                pPort1->setNode(pNode);
                pPort2->setNode(pNode);

                //Add port pointers to node
                pNode->setPort(pPort1);
                pNode->setPort(pPort2);

                //let the ports know about each other
                pPort1->addConnectedPort(pPort2);
                pPort2->addConnectedPort(pPort1);
            }
        }
    }
    ss << "Connected: {" << rComponent1.getName() << "::" << portname1 << "} and {" << rComponent2.getName() << "::" << portname2 << "}";
    cout << ss.str() << endl;
    gCoreMessageHandler.addInfoMessage(ss.str());
    return true;
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
        if ((*it)->getPortType() == Port::READPORT)
        {
            n_ReadPorts += 1;
        }
        if ((*it)->getPortType() == Port::WRITEPORT)
        {
            n_WritePorts += 1;
        }
        if ((*it)->getPortType() == Port::POWERPORT)
        {
            n_PowerPorts += 1;
        }
    }

    //Check the kind of ports in the components subjected for connection
    //                                 This checks that rPort1 is not already connected to pNode
    if ((pPort1->getPortType() == Port::READPORT) && !(pNode->isConnectedToPort(pPort1)))
    {
        n_ReadPorts += 1;
    }
    if ((pPort2->getPortType() == Port::READPORT) && !(pNode->isConnectedToPort(pPort2)))
    {
        n_ReadPorts += 1;
    }
    if ((pPort1->getPortType() == Port::WRITEPORT) && !(pNode->isConnectedToPort(pPort1)))
    {
        n_WritePorts += 1;
    }
    if ((pPort2->getPortType() == Port::WRITEPORT) && !(pNode->isConnectedToPort(pPort2)))
    {
        n_WritePorts += 1;
    }
    if ((pPort1->getPortType() == Port::POWERPORT) && !(pNode->isConnectedToPort(pPort1)))
    {
        n_PowerPorts += 1;
    }
    if ((pPort2->getPortType() == Port::POWERPORT) && !(pNode->isConnectedToPort(pPort2)))
    {
        n_PowerPorts += 1;
    }

    //Check if there are some problems with the connection
    if (n_PowerPorts > 2)
    {
        gCoreMessageHandler.addErrorMessage("Trying to connect more than two PowerPorts to same node");
        return false;
    }
    if (n_WritePorts > 1)
    {
        gCoreMessageHandler.addErrorMessage("Trying to connect more than one WritePort to same node");
        return false;
    }
    if ((n_PowerPorts > 0) && (n_WritePorts > 0))
    {
        gCoreMessageHandler.addErrorMessage("Trying to connect WritePort and PowerPort to same node");
        return false;
    }
    if ((n_PowerPorts == 0) && (n_WritePorts == 0))
    {
        cout << "Trying to connect only ReadPorts" << endl;
        gCoreMessageHandler.addErrorMessage("Trying to connect only ReadPorts");
        return false;
    }
    if ((pPort1->getPortType() == Port::READPORT) &&  (pPort2->getPortType() == Port::READPORT))
    {
        gCoreMessageHandler.addErrorMessage("Trying to connect ReadPort to ReadPort");
        return false;
    }
//    if( ((pPort1->getPortType() == Port::READPORT) && pPort2->getPortType() == Port::POWERPORT && n_PowerPorts > 1) or
//        ((pPort2->getPortType() == Port::READPORT) && pPort1->getPortType() == Port::POWERPORT && n_PowerPorts > 1) )
//    {
//        gCoreMessageHandler.addErrorMessage("Trying to connect one ReadPort to more than one PowerPort");
//        return false;
//    }
    //It seems to be OK!
    return true;
}


//! @brief Disconnect two ports, string version
//! @todo maybe clean up and have one (or maybe ok with two dissconnect functions)
bool ComponentSystem::disconnect(string compname1, string portname1, string compname2, string portname2)
{
    disconnect( getComponent(compname1)->getPort(portname1), getComponent(compname2)->getPort(portname2) );
}


//! Disconnects two ports and remove node if no one is using it any more
//! @todo whay about system ports they are somewaht speciall
void ComponentSystem::disconnect(Port *pPort1, Port *pPort2)
{
    stringstream ss;
    //! @todo some more advanced error handling (are the ports really connected to each other and such)
    if (pPort1->isConnected() && pPort2->isConnected())
    {
        cout << "disconnecting " << pPort1->mpComponent->getName() << " " << pPort1->getPortName() << "  and  " << pPort2->mpComponent->getName() << " " << pPort2->getPortName() << endl;

        Node* node_ptr = pPort1->getNodePtr();
        cout << "nPorts in node: " << node_ptr->mPortPtrs.size() << endl;

        //Remove the port pointer from the node and clear the port if it is not used by someone else

        //If we are only connected to one other port then clear the port and remove the pointer from the node
        //! @todo we assume that the other port is the one we are disconnecting (should be true but check may be a good idea)
        if (pPort1->mConnectedPorts.size() <= 1)
        {
            node_ptr->removePort(pPort1);
            pPort1->clearConnection();
        }
        else
        {
            pPort1->eraseConnectedPort(pPort2);
        }
        cout << "nPorts in node after remove 1: " << node_ptr->mPortPtrs.size() << endl;

        if (pPort2->mConnectedPorts.size() <= 1)
        {
            node_ptr->removePort(pPort2);
            pPort2->clearConnection();
        }
        else
        {
            pPort2->eraseConnectedPort(pPort1);
        }
        cout << "nPorts in node after remove 2: " << node_ptr->mPortPtrs.size() << endl;

        ss << "Disconnected: {"<< pPort1->mpComponent->getName() << "::" << pPort1->getPortName() << "} and {" << pPort2->mpComponent->getName() << "::" << pPort2->getPortName() << "}";
        cout << ss.str() << endl;
        gCoreMessageHandler.addInfoMessage(ss.str());


        //If no more connections exist, remove the entier node and free the memory
        if (node_ptr->mPortPtrs.size() == 0)
        {
            cout << "No more connections to the node exists, deleteing the node";
            removeSubNode(node_ptr);
            delete node_ptr;
            //! @todo maybe need to let the factory remove it insted of manually, in case of user supplied external nodes
        }
    }
    else
    {
        gCoreMessageHandler.addWarningMessage("In disconnect: At least one of the ports do not seem to be connected, (does nothing)", 1);
    }
}


void ComponentSystem::setDesiredTimestep(const double timestep)
{
    mDesiredTimestep = timestep;
    setTimestep(timestep);
}

//void ComponentSystem::setTimestep(const double timestep)
//{
//    mTimestep = timestep;
//
//    for (size_t s=0; s < mComponentSignalptrs.size(); ++s)
//    {
//        if (!(mComponentSignalptrs[s]->isComponentSystem()))
//        {
//            mComponentSignalptrs[s]->setTimestep(timestep);
//        }
//    }
//
//    //C components
//    for (size_t c=0; c < mComponentCptrs.size(); ++c)
//    {
//        if (!(mComponentCptrs[c]->isComponentSystem()))
//        {
//            mComponentCptrs[c]->setTimestep(timestep);
//        }
//    }
//
//    //Q components
//    for (size_t q=0; q < mComponentQptrs.size(); ++q)
//    {
//        if (!(mComponentQptrs[q]->isComponentSystem()))
//        {
//            mComponentQptrs[q]->setTimestep(timestep);
//        }
//    }
//}


void ComponentSystem::setTimestep(const double timestep)
{
    mTimestep = timestep;

    for (size_t s=0; s < mSubComponentStorage.mComponentSignalptrs.size(); ++s)
    {
        if (!(mSubComponentStorage.mComponentSignalptrs[s]->isComponentSystem()))
        {
            mSubComponentStorage.mComponentSignalptrs[s]->setTimestep(timestep);
        }
    }

    //C components
    for (size_t c=0; c < mSubComponentStorage.mComponentCptrs.size(); ++c)
    {
        if (!(mSubComponentStorage.mComponentCptrs[c]->isComponentSystem()))
        {
            mSubComponentStorage.mComponentCptrs[c]->setTimestep(timestep);
        }
    }

    //Q components
    for (size_t q=0; q < mSubComponentStorage.mComponentQptrs.size(); ++q)
    {
        if (!(mSubComponentStorage.mComponentQptrs[q]->isComponentSystem()))
        {
            mSubComponentStorage.mComponentQptrs[q]->setTimestep(timestep);
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


////! Initializes a system component and all its contained components, also allocates log data memory
//void ComponentSystem::initialize(const double startT, const double stopT)
//{
//    //preAllocate local logspace
//    preAllocateLogSpace(startT, stopT);
//
//    adjustTimestep(mTimestep, mComponentSignalptrs);
//    adjustTimestep(mTimestep, mComponentCptrs);
//    adjustTimestep(mTimestep, mComponentQptrs);
//
//    //Init
//    //Signal components
//    for (size_t s=0; s < mComponentSignalptrs.size(); ++s)
//    {
//        if (mComponentSignalptrs[s]->isComponentSystem())
//        {
//            mComponentSignalptrs[s]->initialize(startT, stopT);
//        }
//        else
//        {
//            mComponentSignalptrs[s]->initialize();
//        }
//    }
//
//    //C components
//    for (size_t c=0; c < mComponentCptrs.size(); ++c)
//    {
//        if (mComponentCptrs[c]->isComponentSystem())
//        {
//            mComponentCptrs[c]->initialize(startT, stopT);
//        }
//        else
//        {
//            mComponentCptrs[c]->initialize();
//        }
//    }
//
//    //Q components
//    for (size_t q=0; q < mComponentQptrs.size(); ++q)
//    {
//        if (mComponentQptrs[q]->isComponentSystem())
//        {
//            mComponentQptrs[q]->initialize(startT,stopT);
//        }
//        else
//        {
//            mComponentQptrs[q]->initialize();
//        }
//
//    }
//}


bool ComponentSystem::isSimulationOk()
{
    bool itLooksGood = true;

    //First check all subcomponents to make sure that all requirements for simulation are met
    //scmit = The subcomponent map iterator
    SubComponentStorage::SubComponentMapT::iterator scmit = mSubComponentStorage.mSubComponentMap.begin();
    for ( ; scmit!=mSubComponentStorage.mSubComponentMap.end(); ++scmit)
    {
        Component* pComp = scmit->second; //Component pointer
        //! @todo how to handle subsystems should recurse

        //Check that ALL ports that MUST be connected are connected
        vector<Port*> ports = pComp->getPortPtrVector();
        for (size_t i=0; i<ports.size(); ++i)
        {
            if ( ports[i]->isConnectionRequired() and !ports[i]->isConnected() )
            {
                gCoreMessageHandler.addErrorMessage("Port " + ports[i]->getPortName() + " on " + pComp->getName() + " is not connected!");
                itLooksGood = false;
            }
        }



        //! @todo check more stuff
    }

    //Check the this systems own SystemPorts, are they connected (they must be)
    vector<Port*> ports = getPortPtrVector();
    for (size_t i=0; i<ports.size(); ++i)
    {
        if ( ports[i]->isConnectionRequired() and !ports[i]->isConnected() )
        {
            gCoreMessageHandler.addErrorMessage("Port " + ports[i]->getPortName() + " in " + getName() + " is not connected!");
            itLooksGood = false;
        }
    }


//    for (size_t c=0; c < mSubComponentStorage.mComponentCptrs.size(); ++c)
//    {
//        for(size_t p=0; p < mSubComponentStorage.mComponentCptrs[c]->getPortPtrVector().size(); ++p)
//        {
//            if (!mSubComponentStorage.mComponentCptrs[c]->getPortPtrVector()[p]->isConnected() and
//                mSubComponentStorage.mComponentCptrs[c]->getPortPtrVector()[p]->getPortType() == Port::POWERPORT)
//            {
//                gCoreMessageHandler.addErrorMessage("Port " + mSubComponentStorage.mComponentCptrs[c]->getPortPtrVector()[p]->getPortName() + " on " +
//                                                    mSubComponentStorage.mComponentCptrs[c]->getName() + " is not connected!");
//                itLooksGood = false;
//            }
//        }
//    }

//    for (size_t q=0; q < mSubComponentStorage.mComponentQptrs.size(); ++q)
//    {
//        for(size_t p=0; p < mSubComponentStorage.mComponentQptrs[q]->getPortPtrVector().size(); ++p)
//        {
//            if (!mSubComponentStorage.mComponentQptrs[q]->getPortPtrVector()[p]->isConnected() and
//                mSubComponentStorage.mComponentQptrs[q]->getPortPtrVector()[p]->getPortType() == Port::POWERPORT)
//            {
//                gCoreMessageHandler.addErrorMessage("Port " + mSubComponentStorage.mComponentQptrs[q]->getPortPtrVector()[p]->getPortName() + " on " +
//                                                    mSubComponentStorage.mComponentQptrs[q]->getName() + " is not connected!");
//                itLooksGood = false;
//            }
//        }
//    }
    return itLooksGood;
}


//! Initializes a system component and all its contained components, also allocates log data memory
void ComponentSystem::initialize(const double startT, const double stopT)
{
    mStop = false; //This variable can not be written on below, then problem might occur with thread safety, it's a bit ugly to write on it on this row.

    //preAllocate local logspace
    preAllocateLogSpace(startT, stopT);

    adjustTimestep(mTimestep, mSubComponentStorage.mComponentSignalptrs);
    adjustTimestep(mTimestep, mSubComponentStorage.mComponentCptrs);
    adjustTimestep(mTimestep, mSubComponentStorage.mComponentQptrs);

    //Init
    //Signal components
    for (size_t s=0; s < mSubComponentStorage.mComponentSignalptrs.size(); ++s)
    {
        if (mStop)
            break;

        if (mSubComponentStorage.mComponentSignalptrs[s]->isComponentSystem())
        {
            mSubComponentStorage.mComponentSignalptrs[s]->initialize(startT, stopT);
        }
        else
        {
            mSubComponentStorage.mComponentSignalptrs[s]->initialize();
        }
    }

    //C components
    for (size_t c=0; c < mSubComponentStorage.mComponentCptrs.size(); ++c)
    {
        if (mStop)
            break;

        if (mSubComponentStorage.mComponentCptrs[c]->isComponentSystem())
        {
            mSubComponentStorage.mComponentCptrs[c]->initialize(startT, stopT);
        }
        else
        {
            mSubComponentStorage.mComponentCptrs[c]->initialize();
        }
    }

    //Q components
    for (size_t q=0; q < mSubComponentStorage.mComponentQptrs.size(); ++q)
    {
        if (mStop)
            break;

        if (mSubComponentStorage.mComponentQptrs[q]->isComponentSystem())
        {
            mSubComponentStorage.mComponentQptrs[q]->initialize(startT,stopT);
        }
        else
        {
            mSubComponentStorage.mComponentQptrs[q]->initialize();
        }

    }
}


//! The system component version of simulate
void ComponentSystem::simulate(const double startT, const double stopT)
{
    mStop = false; //This variable can not be written on below, then problem might occur with thread safety, it's a bit ugly to write on it on this row.

    mTime = startT;

    //Simulate
    double stopTsafe = stopT - mTimestep/2.0; //minus halv a timestep is here to ensure that no numerical issues occure

    while ((mTime < stopTsafe) && (!mStop))
    {
        //cout << this->getName() << ": starT: " << startT  << " stopT: " << stopT << " mTime: " << mTime << endl;
//        if (mTime > stopT-0.01)
//        {
//            //debug output for time in the last 0.01 second
//            cout << this->getName() << " time: " << mTime << " stopT: " << stopT << endl;
//        }

        logAllNodes(mTime);

        //! @todo maybe use iterators instead
        //Signal components
        for (size_t s=0; s < mSubComponentStorage.mComponentSignalptrs.size(); ++s)
        {
            mSubComponentStorage.mComponentSignalptrs[s]->simulate(mTime, mTime+mTimestep);
        }

        //C components
        for (size_t c=0; c < mSubComponentStorage.mComponentCptrs.size(); ++c)
        {
            mSubComponentStorage.mComponentCptrs[c]->simulate(mTime, mTime+mTimestep);
        }

        //Q components
        for (size_t q=0; q < mSubComponentStorage.mComponentQptrs.size(); ++q)
        {
            mSubComponentStorage.mComponentQptrs[q]->simulate(mTime, mTime+mTimestep);
        }

        mTime += mTimestep;
    }
}


//! Finalizes a system component and all its contained components
void ComponentSystem::finalize(const double startT, const double stopT)
{
    //! @todo take the final simulation step is suitable here

    //Finalize
    //Signal components
    for (size_t s=0; s < mSubComponentStorage.mComponentSignalptrs.size(); ++s)
    {
        if (mSubComponentStorage.mComponentSignalptrs[s]->isComponentSystem())
        {
            mSubComponentStorage.mComponentSignalptrs[s]->finalize(startT, stopT);
        }
        else
        {
            mSubComponentStorage.mComponentSignalptrs[s]->finalize();
        }
    }

    //C components
    for (size_t c=0; c < mSubComponentStorage.mComponentCptrs.size(); ++c)
    {
        if (mSubComponentStorage.mComponentCptrs[c]->isComponentSystem())
        {
            mSubComponentStorage.mComponentCptrs[c]->finalize(startT, stopT);
        }
        else
        {
            mSubComponentStorage.mComponentCptrs[c]->finalize();
        }
    }

    //Q components
    for (size_t q=0; q < mSubComponentStorage.mComponentQptrs.size(); ++q)
    {
        if (mSubComponentStorage.mComponentQptrs[q]->isComponentSystem())
        {
            mSubComponentStorage.mComponentQptrs[q]->finalize(startT,stopT);
        }
        else
        {
            mSubComponentStorage.mComponentQptrs[q]->finalize();
        }

    }
}


ComponentFactory gCoreComponentFactory;
DLLIMPORTEXPORT ComponentFactory* getCoreComponentFactoryPtr()
{
    return &gCoreComponentFactory;
}
