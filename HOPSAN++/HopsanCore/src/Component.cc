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
    mpHopsanEssentials = 0;
    mpMessageHandler = 0;
    mTypeName = "NoTypeNameSetYet";
    mSubTypeName = "";
    mName = "NoNameSetYet";

    mTimestep = 0.001;

    mpSystemParent = 0;
    mModelHierarchyDepth = 0;

    mpParameters = new Parameters(this);
}


//! @brief Virtual Function, base version which gives you an error if you try to use it.
bool Component::initialize(const double startT, const double /*stopT*/)
{
    mTime = startT;
    initialize();

    return true;        //Always return true, because we cannot know if it was successful or not (yet)
}

void Component::getParameterNames(std::vector<std::string> &rParameterNames)
{
    mpParameters->getParameterNames(rParameterNames);
}

const Parameter *Component::getParameter(const std::string name)
{
    return mpParameters->getParameter(name);
}

const std::vector<Parameter*> *Component::getParametersVectorPtr() const
{
    return mpParameters->getParametersVectorPtr();
}

//! @brief Check if a component has a specific parameter
bool Component::hasParameter(const std::string name) const
{
    return mpParameters->exist(name);
}

void Component::getParameterValue(const std::string name, std::string &rValue)
{
    mpParameters->getParameterValue(name, rValue);
}

//! @brief Returns a pointer directly to the parameter data variable
//! @warning Dont use this function unless YOU REALLY KNOW WHAT YOU ARE DOING
//! @warning This function may be removed in the future
void* Component::getParameterDataPtr(const std::string name)
{
    return mpParameters->getParameterDataPtr(name);
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
    addErrorMessage("This function should only be used by ComponentSystem, it should be overloded. For a component, use finalize() instead");
    stopSimulation();
}


//! @brief Simulates the component from startT to stopT using previously set timestep
//! @param [in] startT Start time
//! @param [in] stopT Stop time
//! @todo adjust self.timestep or simulation depending on Ts from system above (self.timestep should be multipla of Ts)
//void Component::simulate(const double /*startT*/, const double stopT)
//{
//    updateDynamicParameterValues();
//    double stopTsafe = stopT - mTimestep/2.0;
//    mTime = startT;
//    while (mTime < stopTsafe)
//    {
//        simulateOneTimestep();
//        mTime += mTimestep;
//    }
//    //cout << "simulate in: " << this->getName() << endl;
//}

void Component::simulate(const double /*startT*/, const double stopT)
{
    updateDynamicParameterValues();
    const size_t nSteps = calcNumSimSteps(mTime, stopT);
    for (size_t i=0; i<nSteps; ++i)
    {
        simulateOneTimestep();
        mTime += mTimestep;
    }
}


void Component::initialize()
{
    addErrorMessage("You MUST! implement your own initialize method");
    stopSimulation();
}


//! @brief Simulates one time step
void Component::simulateOneTimestep()
{
    addErrorMessage("You MUST! implement your own simulateOneTimestep() method");
    stopSimulation();
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
    return UndefinedCQSType;
}


//! @brief Get the CQStype as string
string Component::getTypeCQSString() const
{
    switch (getTypeCQS())
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


//! @brief Get the TypeName of the component
const string Component::getTypeName() const
{
    return mTypeName;
}

//! @brief Get the SubType name of the component
const string Component::getSubTypeName() const
{
    return mSubTypeName;
}

//! @brief Set the SubType name of the component
void Component::setSubTypeName(const string subTypeName)
{
    mSubTypeName = subTypeName;
}


//! @brief Terminate/stop a running simulation
//! @details Typically used inside components simulateOneTimestep method
void Component::stopSimulation()
{
    mpSystemParent->stopSimulation();
}

HopsanEssentials *Component::getHopsanEssentials()
{
    return mpHopsanEssentials;
}


//void Component::registerDynamicParameter(const std::string name, const std::string description, const std::string unit, double &rValue)
//{
//    if(mpParameters->exist(name))
//        mpParameters->deleteParameter(name);     //Remove parameter if it is already registered

//    this->addReadPort(name, "NodeSignal", Port::NOTREQUIRED);

//    stringstream ss;
//    if(ss << rValue)
//    {
//        mpParameters->addParameter(name, ss.str(), description, unit, "double", true, &rValue);
//    }
//    else
//    {
//        assert(false);
//    }
//}

void Component::initializeDynamicParameters()
{
    mDynamicParameterDataPtrs.clear();
    vector<string> parNames;
    mpParameters->getParameterNames(parNames);

    for (size_t i=0; i<parNames.size(); ++i)
    {
        // For now make sure enabled by default
        //! @todo maybe this should be set when enabling disabling not every time
        mpParameters->enableParameter(parNames[i], true);

        // Check if dynamic parameter, Port with same name exist
        //! @todo must make sure that other ports with this name do not exist, of other types then signal 1d readport
        if (mPortPtrMap.count(parNames[i]) > 0)
        {
            Port* pPort = mPortPtrMap.find(parNames[i])->second;
            if (pPort->isConnected())
            {
                mpParameters->enableParameter(parNames[i], false);
                //! @todo Not getNodeData(0) not hardcoded 0
                //! @todo this assumes signal node and double data ptr in parameter
                mDynamicParameterDataPtrs.push_back(std::pair<double*, double*>(pPort->getNodeDataPtr(0),
                                                              static_cast<double*>(mpParameters->getParameterDataPtr(parNames[i]))));

            }
        }
    }
}

void Component::updateDynamicParameterValues()
{
    for (size_t i=0; i<mDynamicParameterDataPtrs.size(); ++i)
    {
//        std::cout << &(mDynamicParameterDataPtrs[i].first) << std::endl;
//        std::cout << &(mDynamicParameterDataPtrs[i].second) << std::endl << std::endl;
//        std::cout << *(mDynamicParameterDataPtrs[i].first) << std::endl;
//        std::cout << *(mDynamicParameterDataPtrs[i].second) << std::endl << std::endl;
        *(mDynamicParameterDataPtrs[i].second) = *(mDynamicParameterDataPtrs[i].first);
    }
}


//! @brief Register a double parameter value so that it can be accessed for read and write. Set a Name, Description and Unit.
//! @ingroup ConvenientParameterFunctions
//! @param [in] name The name of the parameter
//! @param [in] description A description of the parameter
//! @param [in] unit The unit of the parameter value
//! @param [in] rValue A reference to the double variable representing the value, its adress will be registered
//! @param [in] dynconst Choose if parameter is dynamic (default) or constant (one that can not be converted into a port)
//! @details This function is used in the constructor of the Component modelling code to register member attributes as HOPSAN parameters
void Component::registerParameter(const string name, const string description, const string unit, double &rValue, const ParamDynConstT dynconst)
{
    if(mpParameters->exist(name))
        mpParameters->deleteParameter(name);     //Remove parameter if it is already registered

    //! @todo what if dynamic parameter should we not remove the port as well

    stringstream ss;
    if(ss << rValue)
    {
        if (dynconst == Dynamic)
        {
            // Make a port with same name so that paramter can be switch to dynamic parameter that can be changed during simulation
            this->addReadPort(name, "NodeSignal", Port::NOTREQUIRED);
            mpParameters->addParameter(name, ss.str(), description, unit, "double", true, &rValue);
        }
        else
        {
            mpParameters->addParameter(name, ss.str(), description, unit, "double", false, &rValue);
        }
    }
    else
    {
        assert(false);
    }
}

//! @brief Register a double parameter value so that it can be accessed for read and write. Set a Name, Description and Unit.
//! @ingroup ConvenientParameterFunctions
//! @param [in] name The name of the parameter
//! @param [in] description A description of the parameter
//! @param [in] unit The unit of the parameter value
//! @param [in] rValue A reference to the double variable representing the value, its adress will be registered
//! @details This function is used in the constructor of the Component modelling code to register member attributes as HOPSAN parameters
void Component::registerParameter(const string name, const string description, const string unit, int &rValue)
{
    if(mpParameters->exist(name))
        mpParameters->deleteParameter(name);     //Remove parameter if it is already registered

    stringstream ss;
    if(ss << rValue)
    {
        mpParameters->addParameter(name, ss.str(), description, unit, "integer", false, &rValue);
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

    mpParameters->addParameter(name, rValue, description, unit, "string", false, &rValue);
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
        mpParameters->addParameter(name, "true", description, unit, "bool", false, &rValue);
    else
        mpParameters->addParameter(name, "false", description, unit, "bool", false, &rValue);
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


bool Component::checkModelBeforeSimulation()
{
    cout << "Warning this function isSimulationOk() is only available on subsystem components" << endl;
    assert(false);
	return false;
}

//! @brief Check if a component is a C-Component
//! @returns true or false
//! @see getTypeCQS() getTypeCQSString()
bool Component::isComponentC() const
{
    return false;
}

//! @brief Check if a component is a Q-Component
//! @returns true or false
bool Component::isComponentQ() const
{
    return false;
}

//! @brief Check if a component is a System-Component
//! @returns true or false
bool Component::isComponentSystem() const
{
    return false;
}

//! @brief Check if a component is a Signal-Component
//! @returns true or false
bool Component::isComponentSignal() const
{
    return false;
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
        addDebugMessage("Automatically changed name of added port from: {" + portName + "} to {" + newname + "}");
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
        addWarningMessage("Trying to rename port {" + oldname + "}, but not found");
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
        addWarningMessage("Trying to delete port {" + name + "}, but not found");
    }
}

//! @brief This is a help function that returns a pointer to desired NodeData, only for Advanced Use instead of read/write Node
//! @ingroup ConvenientPortFunctions
//! @param[in] pPort A pointer to the port from which to fetch NodeData pointer
//! @param[in] dataId The enum id for the node value to fetch pointer to
//! @param[in] defaultValue Optional default value if port should not be connected (optional), if ommitet it will be 0
//! @returns A pointer to the specified NodeData or a pointer to dummy NodeData
//! @details It is only ment to be used inside individual component code and automatically handles creation of dummy veriables in case optional ports are not connected
//! @todo Dont know if name really good, should indicate that you should only run this once in initialize (otherwise a lot of new doubls may be created)
double *Component::getSafeNodeDataPtr(Port* pPort, const int dataId, const double defaultValue)
{
    addLogMess(getName() + string("::getSafeNodeDataPtr"));
    //If this is one of the multiports then give an error message to the user so that they KNOW that they have made a misstake
    if (pPort->getPortType() >= MULTIPORT)
    {
        addErrorMessage(string("Port: ")+pPort->getPortName()+string(" is a multiport. Use getSafeMultiPortNodeDataPtr() instead of getSafeNodeDataPtr()"));
    }
    return pPort->getSafeNodeDataPtr(dataId, defaultValue);
}

//! @brief This is a help function that returns a pointer to desired NodeData, only for Advanced Use instead of read/write Node
//! @ingroup ConvenientPortFunctions
//! @param[in] pPort A pointer to the port from which to fetch NodeData pointer
//! @param[in] portIdx The index of the subport in a multiport
//! @param[in] dataId The enum id for the node value to fetch pointer to
//! @param[in] defaultValue Optional default value if port should not be connected (optional), if ommitet it will be 0
//! @returns A pointer to the specified NodeData or a pointer to dummy NodeData
//! @details It is only ment to be used inside individual component code and automatically handles creation of dummy veriables in case optional ports are not connected
//! @todo Dont know if name really good, should indicate that you should only run this once in initialize (otherwise a lot of new doubls may be created)
double *Component::getSafeMultiPortNodeDataPtr(Port* pPort, const size_t portIdx, const int dataId, const double defaultValue)
{
    addLogMess(getName() + string("::getSafeMultiPortNodeDataPtr"));
    //If this is not a multiport then give an error message to the user so that they KNOW that they have made a misstake
    if (pPort->getPortType() < MULTIPORT)
    {
        addErrorMessage(string("Port: ")+pPort->getPortName()+string(" is NOT a multiport. Use getSafeNodeDataPtr() instead of getSafeMultiPortNodeDataPtr()"));
    }
    return pPort->getSafeNodeDataPtr(dataId, defaultValue, portIdx);
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

//! @brief This is suposed to be used by hopsan essentials to set the typename to the same as the registered key value
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
        addDebugMessage("Trying to get port '" + portname + "' in component '" + this->getName() + "', but not found, pointer invalid");
        return 0;
    }
}

//! @brief Returns a string vector containing names of all ports in the component
//! @returns A vector with the port names
std::vector<std::string> Component::getPortNames()
{
    vector<string> names;
    PortPtrMapT::iterator ports_it;

    //Copy every port name
    for( ports_it = mPortPtrMap.begin(); ports_it != mPortPtrMap.end(); ++ports_it)
    {
        names.push_back( ports_it->first);
    }

    return names;
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
void Component::setMeasuredTime(const double time)
{
    mMeasuredTime = time;
}


//! Returns the measured time variable for the component. This is used to measure time requirements when sorting components for multicore purposes.
double Component::getMeasuredTime() const
{
    return mMeasuredTime;
}


//! @brief Write an Debug message, i.e. for debugging purposes.
//! @ingroup ConvenientMessageFunctions
//! @param [in] message The message string
void Component::addDebugMessage(const string message, const string tag)
{
    mpMessageHandler->addDebugMessage(getName()+ "::" + message, tag);
}


//! @brief Write an Warning message.
//! @ingroup ConvenientMessageFunctions
//! @param [in] message The message string
void Component::addWarningMessage(const string message, const string tag)
{
    mpMessageHandler->addWarningMessage(getName()+ "::" + message, tag);
}


//! @brief Write an Error message.
//! @ingroup ConvenientMessageFunctions
//! @param [in] message The message string
void Component::addErrorMessage(const string message, const string tag)
{
    mpMessageHandler->addErrorMessage(getName()+ "::" + message, tag);
}


//! @brief Write an Info message.
//! @ingroup ConvenientMessageFunctions
//! @param [in] message The message string
void Component::addInfoMessage(const string message, const string tag)
{
    mpMessageHandler->addInfoMessage(getName()+ "::" + message, tag);
}


//! @brief Get the an actual start value of a port
//! @param[in] pPort is the port which should be read from
//! @param[in] idx is the index of the start value e.g. NodeHydraulic::PRESSURE
//! @returns the start value
double Component::getStartValue(Port* pPort, const size_t idx, const size_t portIdx)
{
    return pPort->getStartValue(idx, portIdx);
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

//! @brief Returns the component simulation time step
double Component::getTimestep() const
{
    return mTimestep;
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

void Component::configure()
{
    // This function ust be overloaded in every component
    addErrorMessage("You must overload the configure() function in Component: " + mTypeName);
}

void Component::deconfigure()
{
    // This function should be overloaded in every component
    // Does nothing by default
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


////constructor ComponentC
//ComponentC::ComponentC() : Component()
//{
//    mTypeCQS = Component::C;
//}


////Constructor ComponentQ
//ComponentQ::ComponentQ() : Component()
//{
//    mTypeCQS = Component::Q;
//}


////constructor ComponentSignal
//ComponentSignal::ComponentSignal() : Component()
//{
//    mTypeCQS = Component::S;
//}
