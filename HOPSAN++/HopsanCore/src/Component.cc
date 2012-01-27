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
//! @brief Contains Component base classes
//!
//$Id$

#include <iostream>
#include <sstream>
#include <cassert>
#include <algorithm>
#include "Component.h"
#include "ComponentSystem.h"
#include "CoreUtilities/HopsanCoreMessageHandler.h"
#include "Port.h"
#include "HopsanEssentials.h"
#include "CoreUtilities/FindUniqueName.h"

#ifdef USETBB
#include "mutex.h"
#endif

using namespace std;
using namespace hopsan;

//! @defgroup ConvenientFunctions ConvenientFunctions
//! @defgroup ConvenientPortFunctions ConvenientPortFunctions
//! @ingroup ConvenientFunctions
//! @defgroup ConvenientParameterFunctions ConvenientParameterFunctions
//! @ingroup ConvenientFunctions
//! @defgroup ConvenientMessageFunctions ConvenientMessageFunctions
//! @ingroup ConvenientFunctions

//! @brief Component base class Constructor
Component::Component()
{
    // Set initial values, they will be overwritten soon, but good for debugging
    mTypeName = "NoTypeNameSetYet";
    mName = "NoNameSetYet";

    mTimestep = 0.001;

    mIsComponentSystem = false;
    mTypeCQS = Component::UndefinedCQSType;

    mpSystemParent = 0;
    mModelHierarchyDepth = 0;

    mpParameters = new Parameters(this);

    //registerParameter("Ts", "Sample time", "[s]",   mTimestep);
}


//! Virtual Function, base version which gives you an error if you try to use it.
bool Component::initialize(const double /*startT*/, const double /*stopT*/, const size_t /*nSamples*/)
{
    cout << "Error! This function should only be used by system components, it should be overloded. For a component use initialize() instead" << endl;
    assert(false);
    return false;
}


void Component::getParameters(vector<string> &parameterNames, vector<string> &parameterValues, vector<string> &descriptions, vector<string> &units, vector<string> &types)
{
    mpParameters->getParameters(parameterNames, parameterValues, descriptions, units, types);
}

void Component::getParameterValue(const std::string name, std::string &rValue)
{
    mpParameters->getParameterValue(name, rValue);
}


bool Component::setParameterValue(const std::string name, const std::string value, bool force)
{
    return mpParameters->setParameterValue(name, value, force);
}


void Component::updateParameters()
{
    mpParameters->evaluateParameters();
}


bool Component::checkParameters(std::string &errParName)
{
    return mpParameters->checkParameters(errParName);
}


//! @brief Virtual Function, base version which gives you an error if you try to use it.
void Component::finalize(const double /*startT*/, const double /*stopT*/)
{
    cout << "Error! This function should only be used by system components, it should be overloded. For a component use finalize() instead" << endl;
    assert(false);
}


//! @brief Simulates the component from startT to stopT using previously set timestep
//! @param [in] startT Start time
//! @param [in] stopT Stop time
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
    assert("You MUST! implement your own initialize method"==0);
}


void Component::simulateOneTimestep()
{
    assert("You MUST! implement your own simulateOneTimestep() method"==0);
}


void Component::finalize()
{
    //Default does nothing
}


//! @brief Set the desired component name
//! @param [in] name The desired component name
//! @param [in] doOnlyLocalRename Only use this if you know what you are doing, default: false
//!
//! Set the desired component name, if name is already taken in a subsystem the desired name will be modified with a suffix.
//! If you set doOnlyLocalRename to true, the smart rename will not be atempted, avoid doing this as the component storage map will not be updated on anme change
//! This is a somewhat ugly fix for some special situations where we want to make sure that a smart rename is not atempted
//!
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


//! @brief Get the component name
const string Component::getName() const
{
    return mName;
}


//! @brief Get the C, Q or S type of the component as enum
Component::CQSEnumT Component::getTypeCQS() const
{
    return mTypeCQS;
}


//! @brief Get the CQStype as string
string Component::getTypeCQSString() const
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
    case UndefinedCQSType :
        return "UNDEFINEDCQSTYPE";
        break;
    default :
        assert("Invalid CQS Type" == 0);
    }
    return "";           //Needed for VC compilations
}


//! @brief Get the type name of the component
const string Component::getTypeName() const
{
    return mTypeName;
}


//! @brief Terminate/stop a running simulation
//!
//! Typically used inside components simulateOneTimestep method
void Component::stopSimulation()
{
    #ifdef USETBB
    mpSystemParent->mpStopMutex->lock();
    #endif
    this->getSystemParent()->stopSimulation();
    #ifdef USETBB
    mpSystemParent->mpStopMutex->unlock();
    #endif
}


//! @brief Register a double parameter value so that it can be accessed for read and write. Set a Name, Description and Unit.
//! @ingroup ConvenientParameterFunctions
//! @param [in] name The name of the parameter
//! @param [in] description A description of the parameter
//! @param [in] unit The unit of the parameter value
//! @param [in] rValue A reference to the double variable representing the value, its adress will be registered
//! @details This function is used in the constructor of the Component modelling code to register member attributes as HOPSAN parameters
void Component::registerParameter(const string name, const string description, const string unit, double &rValue)
{
    if(mpParameters->exist(name))
        mpParameters->deleteParameter(name);     //Remove parameter if it is already registered

    stringstream ss;
    if(ss << rValue)
    {
        mpParameters->addParameter(name, ss.str(), description, unit, "double", &rValue);
    }
    else
    {
        assert(false);
    }
}


//! @brief Register a string parameter value so that it can be accessed for read and write. Set a Name, Description and Unit.
//! @ingroup ConvenientParameterFunctions
//! @param [in] name The name of the parameter
//! @param [in] description A description of the parameter
//! @param [in] unit The unit of the parameter value
//! @param [in] rValue A reference to the string variable representing the value, its adress will be registered
//! @details This function is used in the constructor of the Component modelling code to register member attributes as HOPSAN parameters
void Component::registerParameter(const string name, const string description, const string unit, string &rValue)
{
    if(mpParameters->exist(name))
        mpParameters->deleteParameter(name);     //Remove parameter if it is already registered

    mpParameters->addParameter(name, rValue, description, unit, "string", &rValue);
}


//! @brief Register a bool parameter value so that it can be accessed for read and write. Set a Name, Description and Unit.
//! @ingroup ConvenientParameterFunctions
//! @param [in] name The name of the parameter
//! @param [in] description A description of the parameter
//! @param [in] unit The unit of the parameter value
//! @param [in] rValue A reference to the bool variable representing the value, its adress will be registered
//! @details This function is used in the constructor of the Component modelling code to register member attributes as HOPSAN parameters
void Component::registerParameter(const string name, const string description, const string unit, bool &rValue)
{
    if(mpParameters->exist(name))
        mpParameters->deleteParameter(name);     //Remove parameter if it is already registered

    if(rValue)
        mpParameters->addParameter(name, "true", description, unit, "bool", &rValue);
    else
        mpParameters->addParameter(name, "false", description, unit, "bool", &rValue);
}


//! @brief Removes a parameter value from the component
void Component::unRegisterParameter(const string name)
{
    mpParameters->deleteParameter(name);
}


void Component::setDesiredTimestep(const double /*timestep*/)
{
    cout << "Warning this function setDesiredTimestep is only available on subsystem components" << endl;
    assert(false);
}


void Component::setInheritTimestep(const bool /*inherit*/)
{
    cout << "Warning this function setInheritTimestep is only available on subsystem components" << endl;
    assert(false);
}


bool Component::doesInheritTimestep() const
{
    cout << "Warning this function doesInheritTimestep is only available on subsystem components" << endl;
    assert(false);
    return false;       //Needed for VC compilations
}


bool Component::isSimulationOk()
{
    cout << "Warning this function isSimulationOk() is only available on subsystem components" << endl;
    assert(false);
	return false;
}

//! @brief Check if a component is a C-Component
//! @returns true or false
//! @see getTypeCQS() getTypeCQSString()
bool Component::isComponentC()
{
    return mTypeCQS == C;
}

//! @brief Check if a component is a Q-Component
//! @returns true or false
bool Component::isComponentQ()
{
    return mTypeCQS == Q;
}

//! @brief Check if a component is a System-Component
//! @returns true or false
bool Component::isComponentSystem()
{
    return mIsComponentSystem;
}

//! @brief Check if a component is a Signal-Component
//! @returns true or false
bool Component::isComponentSignal()
{
    return mTypeCQS == S;
}


double *Component::getTimePtr()
{
    return &mTime;
}


//! @brief Adds a port to the component
//! @param [in] portName The desired name of the port (may be automatically changed)
//! @param [in] porttype The type of port
//! @param [in] nodetype The type of node that must be connected to the port
//! @param [in] connection_requirement Specify if the port must be connecteed or if it is optional
//! @return A pointer to the created port
Port* Component::addPort(const string portName, const PortTypesEnumT portType, const NodeTypeT nodeType, const Port::ReqConnEnumT reqConnection)
{
    std::stringstream ss;
    ss << getName() << "::addPort";
    addLogMess(ss.str());

    //Make sure name is unique before insert
    string newname = this->determineUniquePortName(portName);

    Port* new_port = createPort(portType, nodeType, newname, this);

    //Set wheter the port must be connected before simulation
    if (reqConnection == Port::NOTREQUIRED)
    {
        //! @todo maybe use a string for OPTIONAL instead, to reduce the number of compiletime dependencies, will need to think about that a bit more
        new_port->mConnectionRequired = false;
    }

    mPortPtrMap.insert(PortPtrPairT(newname, new_port));

    //Signal autmatic name change
    if (newname != portName)
    {
        gCoreMessageHandler.addDebugMessage("Automatically changed name of added port from: {" + portName + "} to {" + newname + "}");
    }
    return new_port;
}


//! @brief Convenience method to add a PowerPort
//! @ingroup ConvenientPortFunctions
//! @param [in] portName The desired name of the port (may be automatically changed)
//! @param [in] nodeType The type of node that must be connected to the port
//! @param [in] reqConnect Specify if the port must be connecteed or if it is optional (REQUIRED or NOTREQUIRED)
//! @return A pointer to the created port
Port* Component::addPowerPort(const string portName, const string nodeType, const Port::ReqConnEnumT reqConnect)
{
    return addPort(portName, POWERPORT, nodeType, reqConnect);
}

//! @brief Convenience method to add a PowerMultiPort
//! @ingroup ConvenientPortFunctions
//! @param [in] portName The desired name of the port (may be automatically changed)
//! @param [in] nodeType The type of node that must be connected to the port
//! @param [in] reqConnect Specify if the port must be connecteed or if it is optional (REQUIRED or NOTREQUIRED)
//! @return A pointer to the created port
Port* Component::addPowerMultiPort(const string portName, const string nodeType, const Port::ReqConnEnumT reqConnect)
{
    return addPort(portName, POWERMULTIPORT, nodeType, reqConnect);
}

//! @brief Convenience method to add a ReadMultiPort
//! @ingroup ConvenientPortFunctions
//! @param [in] portName The desired name of the port (may be automatically changed)
//! @param [in] nodeType The type of node that must be connected to the port
//! @param [in] reqConnect Specify if the port must be connecteed or if it is optional (REQUIRED or NOTREQUIRED)
//! @return A pointer to the created port
Port* Component::addReadMultiPort(const string portName, const string nodeType, const Port::ReqConnEnumT reqConnect)
{
    return addPort(portName, READMULTIPORT, nodeType, reqConnect);
}

//! @brief Convenience method to add a ReadPort
//! @ingroup ConvenientPortFunctions
//! @param [in] portName The desired name of the port (may be automatically changed)
//! @param [in] nodeType The type of node that must be connected to the port
//! @param [in] reqConnect Specify if the port must be connecteed or if it is optional (REQUIRED or NOTREQUIRED)
//! @return A pointer to the created port
Port* Component::addReadPort(const string portName, const string nodeType, const Port::ReqConnEnumT reqConnect)
{
    return addPort(portName, READPORT, nodeType, reqConnect);
}


//! @brief Convenience method to add a WritePort
//! @ingroup ConvenientPortFunctions
//! @param [in] portName The desired name of the port (may be automatically changed)
//! @param [in] nodeType The type of node that must be connected to the port
//! @param [in] reqConnect Specify if the port must be connecteed or if it is optional (REQUIRED or NOTREQUIRED)
//! @return A pointer to the created port
Port* Component::addWritePort(const string portName, const string nodeType, const Port::ReqConnEnumT reqConnect)
{
    return addPort(portName, WRITEPORT, nodeType, reqConnect);
}


//! @brief Rename a port
//! @param [in] oldname The name of the the port to rename
//! @param [in] newname The desired new name of the the port
//! @return The actual new name of the port or old name if not renamed
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

//! @brief Removes and deletes a port from a component
//! @param [in] name The name of the port to delete
//! @note Only use this function to remove systemports, removing ordinary ports from components is a bad idea
void Component::deletePort(const string name)
{
    PortPtrMapT::iterator it;
    it = mPortPtrMap.find(name);
    if (it != mPortPtrMap.end())
    {
        delete it->second;
        mPortPtrMap.erase(it);
    }
    else
    {
        gCoreMessageHandler.addWarningMessage("Trying to delete port {" + name + "}, but not found");
    }
}

//! @brief This is a help function that returns a pointer to desired NodeData, only for Advanced Use instead of read/write Node
//! @ingroup ConvenientPortFunctions
//! @param[in] pPort A pointer to the port from which to fetch NodeData pointer
//! @param[in] dataId The enum id for the node value to fetch pointer to
//! @param[in] defaultValue Optional default value if port should not be connected (optional), if ommitet it will be 0
//! @param[in] portIdx The index of the subport in a multiport
//! @returns A pointer to the specified NodeData or a pointer to dummy NodeData
//! @details It is only ment to be used inside individual component code and automatically handles creation of dummy veriables in case optional ports are not connected
//! @todo Dont know if name really good, should indicate that you should only run this once in initialize (otherwise a lot of new doubls may be created)
double *Component::getSafeNodeDataPtr(Port* pPort, const int dataId, const double defaultValue, int portIdx)
{
    std::stringstream ss;
    ss << getName() << "::getSafeNodeDataPtr";
    addLogMess(ss.str());

    //If this is one of the multiports and we have NOT given a subport idx to use then give an error message to the user so that they KNOW that they have made a mistake
    //! @todo it would be nice to solve this in some other way to avoid unecessary code, duoble implemntation in the function bellow is one way but that is even worse, this check would still be needed
    if ((pPort->getPortType() >= MULTIPORT) && (portIdx<0))
    {
        gCoreMessageHandler.addErrorMessage(string("Port: ")+pPort->getPortName()+string(" is a multiport. Use getSafeMultiPortNodeDataPtr() instead of getSafeNodeDataPtr()"));
    }
    portIdx = max(portIdx,0); //Avoid underflow in size_t conversion in getNodeDataPtr()

    return pPort->getSafeNodeDataPtr(dataId, defaultValue, portIdx);
}

//! @brief This is a help function that returns a pointer to desired NodeData, only for Advanced Use instead of read/write Node
//! @ingroup ConvenientPortFunctions
//! @param[in] pPort A pointer to the port from which to fetch NodeData pointer
//! @param[in] dataId The enum id for the node value to fetch pointer to
//! @param[in] defaultValue Optional default value if port should not be connected (optional), if ommitet it will be 0
//! @param[in] portIdx The index of the subport in a multiport
//! @returns A pointer to the specified NodeData or a pointer to dummy NodeData
//! @details It is only ment to be used inside individual component code and automatically handles creation of dummy veriables in case optional ports are not connected
//! @todo Dont know if name really good, should indicate that you should only run this once in initialize (otherwise a lot of new doubls may be created)
double *Component::getSafeMultiPortNodeDataPtr(Port* pPort, const int portIdx, const int dataId, const double defaultValue)
{
    return getSafeNodeDataPtr(pPort, dataId, defaultValue, portIdx);
}


//! @brief a virtual function that detmines a unique port name, needs to be overloaded in ComponentSystem to do this slightly different
std::string Component::determineUniquePortName(std::string portname)
{
    return findUniqueName<PortPtrMapT>(mPortPtrMap, portname);
}

//! @brief Set the component parent system (tell component who parent is)
//! @param [in] pComponentSystem Pointer to parent system component
void Component::setSystemParent(ComponentSystem *pComponentSystem)
{
    mpSystemParent = pComponentSystem;
}

void Component::setTypeName(const string typeName)
{
    mTypeName = typeName;
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

//! @brief Returns a pointer to the port with the given name.
//! @param[in] portname The name of the port
//! @returns A pointer to the port, or 0 if port not found
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

//! @brief Get a port as reference to pointer
//! @todo do we really need this function
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


//! @brief Write an Debug message, i.e. for debugging purposes.
//! @ingroup ConvenientMessageFunctions
//! @param [in] message The message string
void Component::addDebugMessage(const string message)
{
    gCoreMessageHandler.addDebugMessage(getName()+ "::" + message);
}


//! @brief Write an Warning message.
//! @ingroup ConvenientMessageFunctions
//! @param [in] message The message string
void Component::addWarningMessage(const string message)
{
    gCoreMessageHandler.addWarningMessage(getName()+ "::" + message);
}


//! @brief Write an Error message.
//! @ingroup ConvenientMessageFunctions
//! @param [in] message The message string
void Component::addErrorMessage(const string message)
{
    gCoreMessageHandler.addErrorMessage(getName()+ "::" + message);
}


//! @brief Write an Info message.
//! @ingroup ConvenientMessageFunctions
//! @param [in] message The message string
void Component::addInfoMessage(const string message)
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
void Component::setStartValue(Port* pPort, const size_t idx, const double value)
{
    std::stringstream ss;
    ss << getName() << "::setStartValue";
    addLogMess(ss.str());
    pPort->setStartValue(idx, value);
}


void Component::disableStartValue(Port *pPort, const size_t idx)
{
    pPort->disableStartValue(idx);
}


ComponentSystem *Component::getSystemParent()
{
    if(mpSystemParent)
    {
        return mpSystemParent;
    }
    else
    {
        return 0;
    }
}

size_t Component::getModelHierarchyDepth()
{
    return mModelHierarchyDepth;
}


Component::~Component()
{
    //! Remove the mapping to eventual system parameters to avoid cowboy-writing in memory after deleted component.
//    for(size_t i = 0; i < mParameters.size(); ++i)
//    {
//        mpSystemParent->getSystemParameters().unMapParameter(mParameters[i].getValuePtr());
//    }

    //Delete any ports that have been added to the component
    PortPtrMapT::iterator ppmit;
    for (ppmit=mPortPtrMap.begin(); ppmit!=mPortPtrMap.end(); ++ppmit)
    {
        delete (*ppmit).second;
    }

    delete mpParameters;
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
ComponentC::ComponentC() : Component()
{
    mTypeCQS = Component::C;
}


//Constructor ComponentQ
ComponentQ::ComponentQ() : Component()
{
    mTypeCQS = Component::Q;
}


//constructor ComponentSignal
ComponentSignal::ComponentSignal() : Component()
{
    mTypeCQS = Component::S;
}
