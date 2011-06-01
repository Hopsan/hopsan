/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   Component.cc
//! @author FluMeS
//! @date   2009-12-20
//!
//! @brief Contains Component base classes as well as Component Parameter class
//!
//$Id$

#include <iostream>
#include <sstream>
#include <cassert>
#include "Component.h"
#include "ComponentSystem.h"
#include "CoreUtilities/HopsanCoreMessageHandler.h"
#include "Port.h"
#include "HopsanEssentials.h"
#include "CoreUtilities/FindUniqueName.h"

using namespace std;
using namespace hopsan;

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


double *CompParameter::getValuePtr()
{
    return mpValue;
}


void CompParameter::setValue(const double value)
{
    *mpValue = value;
}





//Constructor
Component::Component(string name)
{
    mName = name;
    mTimestep = 0.001;

    mIsComponentC = false;
    mIsComponentQ = false;
    mIsComponentSystem = false;
    mIsComponentSignal = false;
    //mTypeCQS = "";
    mTypeCQS = Component::UNDEFINEDCQSTYPE;

    mpSystemParent = 0;
    mModelHierarchyDepth = 0;

    //registerParameter("Ts", "Sample time", "[s]",   mTimestep);
}


//! Virtual Function, base version which gives you an error if you try to use it.
void Component::initialize(const double /*startT*/, const double /*stopT*/, const size_t /*nSamples*/)
{
    cout << "Error! This function should only be used by system components, it should be overloded. For a component use initialize() instead" << endl;
    assert(false);
}


void Component::initializeComponentsOnly()
{
    assert(false);
}


//! Virtual Function, base version which gives you an error if you try to use it.
void Component::finalize(const double /*startT*/, const double /*stopT*/)
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
    cout << "Warning! You should implement your own method: " << mName << endl;
    assert(false);
}


void Component::finalize()
{
    //cout << "Warning! You should implement your own finalize() method" << endl;
    //assert(false);
}

//! @brief A finilize method that contains stuff that the user should not need to care about
//! @todo OK I admit, the name is kind of bad
void Component::secretFinalize()
{
    //delete any created dummy node data variables created and then clear the pointer storage vector
    for (size_t i=0; i<mDummyNDptrs.size(); ++i)
    {
        delete mDummyNDptrs[i];
    }
    mDummyNDptrs.clear();
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


////! Convert the C, Q or S type from enum to string
////! @todo This function may not need to be meber in Component, (maybe enum should be free aswell), this function may be completely useless
//string Component::convertTypeCQS2String(typeCQS type)
//{
//    switch (type)
//    {
//    case C :
//        return "C";
//        break;
//    case Q :
//        return "Q";
//        break;
//    case S :
//        return "S";
//        break;
//    case UNDEFINEDCQSTYPE :
//        return "NOCQSTYPE";
//        break;
//    default :
//        return "Invalid CQS Type";
//    }
//}


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
    case UNDEFINEDCQSTYPE :
        return "UNDEFINEDCQSTYPE";
        break;
    default :
        assert("Invalid CQS Type" == 0);
    }
}


//! Get the type name of the component
const string &Component::getTypeName()
{
    return mTypeName;
}


//! @brief Terminate/stop a running simulation
//!
//! Typically used inside components simulateOneTimestep method
void Component::stopSimulation()
{
    this->getSystemParent()->stop();
}


//! Register a parameter value so that it can be accessed for read and write. Set a Name, Description and Unit.
void Component::registerParameter(const string name, const string description, const string unit, double &rValue)
{
    //! @todo handle trying to add multiple comppar with same name

    std::stringstream ss;
    ss << getName() << "::registerParameter";
    addLogMess(ss.str());

    CompParameter new_comppar(name, description, unit, rValue);
    mParameters.push_back(new_comppar); //Copy parameters into storage
    mDefaultParameters.insert(std::pair<std::string, double>(description+name, rValue));
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


//! @brief Get the value of a parameter
//! @param[in] name is the name of the wanted parameter
//! @returns the value of the parameter
double Component::getParameterValue(const string name)
{
    for (size_t i=0; i<mParameters.size(); ++i)
    {
        if (mParameters[i].getName() == name)
        {
            return mParameters[i].getValue();
        }
    }
    cout << "No such parameter (return 0): " << name << endl;
    //! @todo We should create a debug warning to user if this happens (not only in this function)
    //! @todo maybe break out find parameter function (maybe even use something else then vector for storage)
    return 0.0;
}


double Component::getDefaultParameterValue(const string name)
{
    return mDefaultParameters.find(name)->second;
}


//! @brief Get a pointer to a parameter
//!
//! This method is useful with mapping and unmapping to System parameters
//!
//! @param[in] name is the name of the wanted parameter
//! @returns a pointer to the parameter, a null ponter if not present
double *Component::getParameterValuePtr(const string name)
{
    for (size_t i=0; i<mParameters.size(); ++i)
    {
        if (mParameters[i].getName() == name)
        {
            return mParameters[i].getValuePtr();
        }
    }
    cout << "No such parameter (return 0): " << name << endl;
    return 0;
}


//! @brief Get the value of a parameter in string format
//!
//! If the parameter is mapped by a System parameter the name of the System parameter is given instead of the value
//!
//! @param[in] name is the name of the wanted parameter
//! @returns the value of the parameter in string format, an empty string if name is not present as a parameter
std::string Component::getParameterValueTxt(const string name)
{
    std::string paramTxt="";
    for (size_t i=0; i<mParameters.size(); ++i)
    {
        //The parameter is present
        if (mParameters[i].getName() == name)
        {
            //Check if the parameter is mapped ba a System parameter, then set the system parameter name to paramTxt
            paramTxt = mpSystemParent->getSystemParameters().findOccurrence(mParameters[i].getValuePtr());
            //The parameter is not mapped to a system parameter
            if(paramTxt.empty())
            {
                //Read out the parameter value to the string
                double value = getParameterValue(name);
                std::ostringstream oss;
                oss << value;
                paramTxt = oss.str();
            }
        }
    }
//    cout << "No such parameter (return 0): " << name << endl;
    //! @todo We should create a debug warning to user if this happens (not only in this function)
    //! @todo maybe break out find parameter function (maybe even use something else then vector for storage)
    return paramTxt;
}


//! @brief Get the parameters of the component, typically "k" in the case of a spring coeff.
//! @returns a vector of the parameters
const vector<string> Component::getParameterNames()
{
    vector<string> names;
    for (size_t i=0; i<mParameters.size(); ++i)
    {
        names.push_back(mParameters[i].getName());
    }
    return names;
}


//! @brief Get the unit of the parameter, typically "N/m" in the case of a spring coeff.
//! @param[in] name is the name of the wanted parameter
//! @returns the unit of the parameter
const string Component::getParameterUnit(const string name)
{
    for (size_t i=0; i<mParameters.size(); ++i)
    {
        if (mParameters[i].getName() == name)
        {
            return mParameters[i].getUnit();
        }
    }
    cout << "No such parameter (return empty): " << name << endl;
    return string();
}


//! @brief Get the description of the parameter, typically "Spring coeff." in the case of a spring coeff.
//! @param[in] name is the name of the wanted parameter
//! @returns the description of the parameter
const string Component::getParameterDescription(const string name)
{
    for (size_t i=0; i<mParameters.size(); ++i)
    {
        if (mParameters[i].getName() == name)
        {
            return mParameters[i].getDesc();
        }
    }
    cout << "No such parameter (return empty): " << name << endl;
    return string();
}


//! @brief Access method for the parameter vector
//! @returns the parameter vector
vector<CompParameter> Component::getParameterVector()
{
    return mParameters;
}


//! @brief Access method for the parameters
//! @returns a map with parameter names and values
map<string, double> Component::getParameterMap()
{
    map<string, double> parameterMap;
    for (size_t i=0; i<mParameters.size(); ++i)
    {
        parameterMap.insert(pair<string, double>(mParameters[i].getName(), mParameters[i].getValue()));
    }
    return parameterMap;
}


//! @brief Sets a parameter to a value
//! @param name Name of the parameter
//! @param value Value to asign the parameter with
//! @return true if it went OK, false otherwise
bool Component::setParameterValue(const string name, const double value)
{
    bool success = false;
    for (size_t i=0; i<mParameters.size(); ++i)
    {
        if (name == mParameters[i].getName())
        {
            mParameters[i].setValue(value);
            //Unmap the parameter if it is pointed from the System parameters
            mpSystemParent->getSystemParameters().unMapParameter(mParameters.at(i).mpValue);
            success = true;
        }
    }
    if (!success)
    {
        //! @todo Maybe some error handling
        cout << "No such parameter (does nothing): " << name << endl;
    }
    return success;
}


//! @brief Sets a parameter value using a key to a system parameter
//! @param parName Name of the parameter
//! @param sysParName Name name of the system parameter
//! @return true if it went OK, false otherwise
bool Component::setParameterValue(const std::string parName, const std::string sysParName)
{
    bool success = false;
    //Map it to the system parameter
    success = getSystemParent()->getSystemParameters().mapParameter(sysParName, getParameterValuePtr(parName));
    return success;
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


void Component::setDesiredTimestep(const double /*timestep*/)
{
    cout << "Warning this function setDesiredTimestep is only available on subsystem components" << endl;
    assert(false);
}

bool Component::isSimulationOk()
{
    cout << "Warning this function isSimulationOk() is only available on subsystem components" << endl;
    assert(false);
	return false;
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
Port* Component::addPort(const string portname, PORTTYPE porttype, const NodeTypeT nodetype, Port::CONREQ connection_requirement)
{
    std::stringstream ss;
    ss << getName() << "::addPort";
    addLogMess(ss.str());

    //Make sure name is unique before insert
    string newname = this->determineUniquePortName(portname);

    Port* new_port = createPort(porttype, nodetype, newname, this);

    //Set wheter the port must be connected before simulation
    if (connection_requirement == Port::NOTREQUIRED)
    {
        //! @todo maybe use a string for OPTIONAL instead, to reduce the number of compiletime dependencies, will need to think about that a bit more
        new_port->mConnectionRequired = false;
    }

    mPortPtrMap.insert(PortPtrPairT(newname, new_port));

    //Signal autmatic name change
    if (newname != portname)
    {
        gCoreMessageHandler.addDebugMessage("Automatically changed name of added port from: {" + portname + "} to {" + newname + "}");
    }

    return new_port;
}


//! @brief Convenience method to add a PowerPort
//! @param [in] porttype The type of port
//! @param [in] nodetype The type of node that must be connected to the port
Port* Component::addPowerPort(const string portname, const string nodetype, Port::CONREQ connection_requirement)
{
    return addPort(portname, POWERPORT, nodetype, connection_requirement);
}

//! @brief Convenience method to add a PowerMultiPort
//! @param [in] porttype The type of port
//! @param [in] nodetype The type of node that must be connected to the port
Port* Component::addPowerMultiPort(const string portname, const string nodetype, Port::CONREQ connection_requirement)
{
    return addPort(portname, POWERMULTIPORT, nodetype, connection_requirement);
}

//! @brief Convenience method to add a ReadMultiPort
//! @param [in] porttype The type of port
//! @param [in] nodetype The type of node that must be connected to the port
Port* Component::addReadMultiPort(const string portname, const string nodetype, Port::CONREQ connection_requirement)
{
    return addPort(portname, READMULTIPORT, nodetype, connection_requirement);
}

//! @brief Convenience method to add a ReadPort
//! @param [in] porttype The type of port
//! @param [in] nodetype The type of node that must be connected to the port
Port* Component::addReadPort(const string portname, const string nodetype, Port::CONREQ connection_requirement)
{
    return addPort(portname, READPORT, nodetype, connection_requirement);
}


//! @brief Convenience method to add a WritePort
//! @param [in] porttype The type of port
//! @param [in] nodetype The type of node that must be connected to the port
Port* Component::addWritePort(const string portname, const string nodetype, Port::CONREQ connection_requirement)
{
    return addPort(portname, WRITEPORT, nodetype, connection_requirement);
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
        string modnewname = determineUniquePortName(newname); //Make sure new name is unique
        temp_port_ptr->mPortName = modnewname;  //Set new name in port
        mPortPtrMap.insert(PortPtrPairT(modnewname, temp_port_ptr)); //Re add to map
        return modnewname;
    }
    else
    {
        gCoreMessageHandler.addWarningMessage("Trying to rename port {" + oldname + "}, but not found");
        return oldname;
    }
}

//! @todo Do we ever actually DELETE the ports?, dosnt seem so
void Component::deletePort(const string name)
{
    PortPtrMapT::iterator it;
    it = mPortPtrMap.find(name);
    if (it != mPortPtrMap.end())
    {
        mPortPtrMap.erase(it);
        cout << "===================Erasing Port: " << name << endl;
    }
    else
    {
        gCoreMessageHandler.addWarningMessage("Trying to delete port {" + name + "}, but not found");
    }
}

//! @brief This is a help function that returns a pointer to desired NodeData
//! @param[in] pPort A pointer to the port from which to fetch NodeData pointer
//! @param[in] dataId The enum id for the node value to fetch pointer to
//! @param[in] defaultvalue Optional default value if port should not be connected (optional), if ommitet it will be 0
//! @returns A pointer to the specified NodeData or a pointer to dummy NodeData
//! It is only ment to be used insed individual component code and automatically handles creation of dummy veriables
//! in case optional ports are not connected
//! @todo Dont know if name really good, should indicate that you should only run this once in initialize (otherwise a lot of new doubls may be created)
double *Component::getSafeNodeDataPtr(Port* pPort, const int dataId, const double defaultValue, int portIdx)
{
    std::stringstream ss;
    ss << getName() << "::getSafeNodeDataPtr";
    addLogMess(ss.str());

    //If this is one of the multiports and we have NOT given a subport idx to use then give an error message to the user sothat they KNOW that they have made a mistake
    //! @todo it would be nice to solve this in some other way to avoid unecessary code, duoble implemntation in the function bellow is one way but that is even worse, this check would still be needed
    if ((pPort->getPortType() >= MULTIPORT) && (portIdx<0))
    {
        gCoreMessageHandler.addErrorMessage(string("Port: ")+pPort->getPortName()+string(" is a multiport. Use getSafeMultiPortNodeDataPtr() instead of getSafeNodeDataPtr()"));
    }
    portIdx = max(portIdx,0); //Avoid underflow in size_t conversion in getNodeDataPtr()

    double *pND;
    //! @todo Maybe we should somehow try to use the startvalue as default instead somehow, need to think about this, seems like double work right now
    if(pPort->isConnected())
    {
        pND = pPort->getNodeDataPtr(dataId, portIdx);
    }
    else
    {
        pND = new double(defaultValue);
        mDummyNDptrs.push_back(pND); //Store the pointer to dummy for automatic finilize removal
    }
    return pND;
}

double *Component::getSafeMultiPortNodeDataPtr(Port* pPort, const int portIdx, const int dataId, const double defaultValue)
{
    return getSafeNodeDataPtr(pPort, dataId, defaultValue, portIdx);
}


//! @brief a virtual function that detmines a unique port name, needs to be overloaded in ComponentSystem to do this slightly different
std::string Component::determineUniquePortName(std::string portname)
{
    return findUniqueName<PortPtrMapT>(mPortPtrMap, portname);
}


void Component::setSystemParent(ComponentSystem *pComponentSystem)
{
    mpSystemParent = pComponentSystem;
}

void Component::setTypeName(const string typeName)
{
    mTypeName = typeName;
}

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
        //cout << "failed to find port: " << portname << " in component: " << this->mName << endl;
        gCoreMessageHandler.addDebugMessage("Trying to get port '" + portname + "' in component '" + this->getName() + "', but not found, pointer invalid");
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


//! Sets the measured time variable for the component. This is used to measure time requirements when sorting components for multicore purposes.
//! @see getMeasuredTime()
void Component::setMeasuredTime(double time)
{
        mMeasuredTime = time;
}


//! Returns the measured time variable for the component. This is used to measure time requirements when sorting components for multicore purposes.
double Component::getMeasuredTime()
{
        return mMeasuredTime;
}


//! Write an Debug message, i.e. for debugging purposes.
void Component::addDebugMessage(string message)
{
    gCoreMessageHandler.addDebugMessage(getName()+ "::" + message);
}


//! Write an Warning message.
void Component::addWarningMessage(string message)
{
    gCoreMessageHandler.addWarningMessage(getName()+ "::" + message);
}


//! Write an Error message.
void Component::addErrorMessage(string message)
{
    gCoreMessageHandler.addErrorMessage(getName()+ "::" + message);
}


//! Write an Info message.
void Component::addInfoMessage(string message)
{
    gCoreMessageHandler.addInfoMessage(getName()+ "::" + message);
}


//! @brief Get the an actual start value of a port
//! @param[in] pPort is the port which should be read from
//! @param[in] idx is the index of the start value e.g. NodeHydraulic::PRESSURE
//! @returns the start value
double Component::getStartValue(Port* pPort, const size_t idx)
{
    return pPort->getStartValue(idx);
}


//! @brief Set the an actual start value of a port
//! @param[in] pPort is the port which should be written to
//! @param[in] idx is the index of the start value e.g. NodeHydraulic::PRESSURE
//! @param[in] value is the start value that should be written
void Component::setStartValue(Port* pPort, const size_t &idx, const double &value)
{
    std::stringstream ss;
    ss << getName() << "::setStartValue";
    addLogMess(ss.str());
    pPort->setStartValue(idx, value);
}


void Component::disableStartValue(Port *pPort, const size_t &idx)
{
    pPort->disableStartValue(idx);
}


//! @brief Change the cqs type of a stored subsystem component
bool ComponentSystem::changeTypeCQS(const string name, const typeCQS newType)
{
    //First get the component ptr and check if we are requesting new type
    Component* tmpptr = getSubComponent(name);
    if (newType != tmpptr->getTypeCQS())
    {
        //check that it is a system component, in that case change the cqs type
        if ( tmpptr->isComponentSystem() )
        {
            //Cast to system ptr
            //! @todo should have a member function that return systemcomponent ptrs
            ComponentSystem* tmpsysptr = dynamic_cast<ComponentSystem*>(tmpptr);

            //Remove old version
            this->removeSubComponentPtrFromStorage(tmpsysptr);

            //Change cqsType localy in the subcomponent, make sure to set true to avoid looping back to this rename
            tmpsysptr->setTypeCQS(newType, true);

            //readd to system
            this->addSubComponentPtrToStorage(tmpsysptr);
        }
        else
        {
            return false;
        }
    }
    return true;
}

//! @brief This function automatically determines the CQS type depending on the what has been connected to the systemports
//! @todo This function will go through all conected ports every time it is run, maybe a quicker version would only be run on the port beeing connected or disconnectd, in the connect and disconnect function
void ComponentSystem::determineCQSType()
{
    PortPtrMapT::iterator ppmit;

    size_t c_ctr=0;
    size_t q_ctr=0;
    size_t s_ctr=0;

    for (ppmit=mPortPtrMap.begin(); ppmit!=mPortPtrMap.end(); ++ppmit)
    {
        //all ports should be system ports in a subsystem
        assert((*ppmit).second->getPortType() == SYSTEMPORT);

        //! @todo I dont think that I really need to ask for ALL connected subports here, as it is actually only the component that is directly connected to the system port that is interesting
        //! @todo this means that I will be able to UNDO the Port getConnectedPorts madness, maybe, if wedont want ot in some other place
        vector<Port*> connectedPorts = (*ppmit).second->getConnectedPorts(-1); //Make a copy of connected ports
        vector<Port*>::iterator cpit;
        for (cpit=connectedPorts.begin(); cpit!=connectedPorts.end(); ++cpit)
        {
            if ( (*cpit)->getComponent()->getSystemParent() == this )
            {
                switch ((*cpit)->getComponent()->getTypeCQS())
                {
                case C :
                    ++c_ctr;
                    break;
                case Q :
                    ++q_ctr;
                    break;
                case S :
                    ++s_ctr;
                    break;
                default :
                    assert("This should not happen" == 0);
                }
            }
        }
    }

    //Ok now lets determine i we have a valid CQS type or not
    if ( (c_ctr > 0) && (q_ctr == 0) )
    {
        this->setTypeCQS(C);
    }
    else if ( (q_ctr > 0) && (c_ctr == 0) )
    {
        this->setTypeCQS(Q);
    }
    else if ( (s_ctr > 0) && (c_ctr==0) && (q_ctr==0) )
    {
        this->setTypeCQS(S);
    }
    else
    {
        //If we swap from valid type then give warning
        if (this->getTypeCQS() != UNDEFINEDCQSTYPE)
        {
            gCoreMessageHandler.addWarningMessage(string("Your action has caused the CQS type to become invalid in system: ")+this->getName());
        }
        //! @todo maybe we should let the GUI display ??? in port overlays instead of sending warning messages

        this->setTypeCQS(UNDEFINEDCQSTYPE);
    }
}


ComponentSystem *Component::getSystemParent()
{
    return mpSystemParent;
}

size_t Component::getModelHierarchyDepth()
{
    return mModelHierarchyDepth;
}


//constructor ComponentSignal
ComponentSignal::ComponentSignal(string name) : Component(name)
{
    mTypeCQS = Component::S;
    mIsComponentSignal = true;
}

Component::~Component()
{
    //! Remove the mapping to eventual system parameters to avoid cowboy-writing in memory after deleted component.
    for(size_t i = 0; i < mParameters.size(); ++i)
    {
        mpSystemParent->getSystemParameters().unMapParameter(mParameters[i].getValuePtr());
    }

    //Delete any ports that have been added to the component
    PortPtrMapT::iterator ppmit;
    for (ppmit=mPortPtrMap.begin(); ppmit!=mPortPtrMap.end(); ++ppmit)
    {
        delete (*ppmit).second;
    }
}


//! @brief Loads the start values to the connected Node from the "start value node" at each Port of the component
void Component::loadStartValues()
{
    std::vector<Port*> pPortPtrs = getPortPtrVector();
    std::vector<Port*>::iterator portIt;
    for(portIt = pPortPtrs.begin(); portIt != pPortPtrs.end(); ++portIt)
    {
        (*portIt)->loadStartValues();
    }
}


void Component::loadStartValuesFromSimulation()
{
    std::vector<Port*> pPortPtrs = getPortPtrVector();
    std::vector<Port*>::iterator portIt;
    for(portIt = pPortPtrs.begin(); portIt != pPortPtrs.end(); ++portIt)
    {
        (*portIt)->loadStartValuesFromSimulation();
    }
}


//constructor ComponentC
ComponentC::ComponentC(string name) : Component(name)
{
    mTypeCQS = Component::C;
    mIsComponentC = true;
}


//Constructor ComponentQ
ComponentQ::ComponentQ(string name) : Component(name)
{
    mTypeCQS = Component::Q;
    mIsComponentQ = true;
}


//Constructor
ComponentSystem::ComponentSystem(string name) : Component(name)
{
    mTypeName = "ComponentSystem";
    mIsComponentSystem = true;
    mDesiredTimestep = 0.001;
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
