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
#include "CoreUtilities/StringUtilities.h"
#include "ComponentUtilities/num2string.hpp"

#ifdef USETBB
#include "mutex.h"
#endif

using namespace std;
using namespace hopsan;

//! @defgroup ConvenientComponentFunctions ConvenientComponentFunctions
//! @defgroup ConvenientPortFunctions ConvenientPortFunctions
//! @ingroup ConvenientComponentFunctions
//! @defgroup ConvenientParameterFunctions ConvenientParameterFunctions
//! @ingroup ConvenientComponentFunctions
//! @defgroup ConvenientMessageFunctions ConvenientMessageFunctions
//! @ingroup ConvenientComponentFunctions
//! @defgroup ConvenientSimulationFunctions ConvenientSimulationFunctions
//! @ingroup ConvenientComponentFunctions

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

    mSearchPaths.clear();
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

void Component::getParameterValue(const std::string name, char** pValue)
{
    mpParameters->getParameterValue(name, pValue);
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

const std::vector<VariameterDescription>* Component::getVariameters()
{
    //! @todo dont rebuild this every time, question is should this be in the nodes and or ports maybe, or should it only be in the components
    mVariameters.clear();

    PortPtrMapT::iterator pit;
    for (pit=mPortPtrMap.begin(); pit!=mPortPtrMap.end(); ++pit)
    {
        const vector<NodeDataDescription>* pDescs = pit->second->getNodeDataDescriptions();
        if (pDescs)
        {
            for (size_t i=0; i<pDescs->size(); ++i)
            {
                VariameterDescription data;
                data.mName = pDescs->at(i).name;
                data.mShortName = pDescs->at(i).shortname;
                data.mPortName = pit->second->getName();
                data.mUnit = pDescs->at(i).unit;
                data.mDescription = pDescs->at(i).description;
                data.mVariableId = pDescs->at(i).id;
                data.mVarType = pDescs->at(i).varType;
                data.mAlias = pit->second->getVariableAlias(data.mVariableId);
                data.mDataType = "double"; //!< @todo not hardcoded
                mVariameters.push_back(data);
                //! @todo some of these will never change after a component has been configured, (but som may, like alias description unit)
            }
        }
    }
    return &mVariameters;
}


//! @brief Virtual Function, base version which gives you an error if you try to use it.
void Component::finalize(const double /*startT*/, const double /*stopT*/)
{
    addErrorMessage("This function should only be used by ComponentSystem, it should be overloded. For a component, use finalize() instead");
    stopSimulation();
}


//! @brief Simulates the component from current simulation position to stopT using previously set timestep
//! @param [in] stopT Stop time
void Component::simulate(const double stopT)
{
    updateDynamicParameterValues();
    const size_t nSteps = calcNumSimSteps(mTime, stopT); //Here mTime is the last time step since it is not updated yet
    for (size_t i=0; i<nSteps; ++i)
    {
        mTime += mTimestep; //mTime is updated here before the simulation,
                            //mTime is the current time during the simulateOneTimestep
        simulateOneTimestep();
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
void Component::setName(string name)
{
    // Make sure name is clean
    santizeName(name);

    //If name same as before do nothing
    if (name != mName)
    {
        //Do we have a system parent
        if (mpSystemParent != 0)
        {
            // Let parent handle namees so that we do not get duplicate names
            mpSystemParent->renameSubComponent(mName, name);
        }
        else
        {
            // No systemparent is set yet so lets set our own desired name, also make sure that it is a clean name
            mName = name;
        }
    }
}


//! @brief Get the component name
const std::string &Component::getName() const
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
    case CType :
        return "C";
        break;
    case QType :
        return "Q";
        break;
    case SType :
        return "S";
        break;
    case UndefinedCQSType :
        return "UndefinedCQSType";
        break;
    default :
        addFatalMessage("Component::getTypeCQSString(): Invalid CQS Type.");
        return "Invalid CQS Type";
    }
}


//! @brief Get the TypeName of the component
const std::string &Component::getTypeName() const
{
    return mTypeName;
}

//! @brief Get the SubType name of the component
const std::string &Component::getSubTypeName() const
{
    return mSubTypeName;
}

//! @brief Set the SubType name of the component
void Component::setSubTypeName(const string subTypeName)
{
    mSubTypeName = subTypeName;
}


//! @brief Terminate/stop a running initialization or simulation
//! @details Typically used inside components simulateOneTimestep method
//! @ingroup ConvenientSimulationFunctions
void Component::stopSimulation()
{
    mpSystemParent->stopSimulation();
}

HopsanEssentials *Component::getHopsanEssentials()
{
    return mpHopsanEssentials;
}

//! @deprecated
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

//! @deprecated
void Component::updateDynamicParameterValues()
{
    for (size_t i=0; i<mDynamicParameterDataPtrs.size(); ++i)
    {
        *(mDynamicParameterDataPtrs[i].second) = *(mDynamicParameterDataPtrs[i].first);
    }
}

void Component::addConstant(const string name, const string description, const string unit, const double defaultValue, double &rData)
{
    rData = defaultValue;
    registerParameter(name, description, unit, rData, Constant);
}

void Component::addConstant(const string name, const string description, const string unit, const int defaultValue, int &rData)
{
    rData = defaultValue;
    registerParameter(name, description, unit, rData);
}

void Component::addConstant(const string name, const string description, const string unit, const char* defaultValue, std::string &rData)
{
    rData = defaultValue;
    registerParameter(name, description, unit, rData);
}

void Component::addConstant(const string name, const string description, const string unit, const bool defaultValue, bool &rData)
{
    rData = defaultValue;
    registerParameter(name, description, unit, rData);
}


//! @brief Register a double parameter value so that it can be accessed for read and write. Set a Name, Description and Unit.
//! @ingroup ConvenientParameterFunctions
//! @param [in] name The name of the parameter
//! @param [in] description A description of the parameter
//! @param [in] unit The unit of the parameter value
//! @param [in] rValue A reference to the double variable representing the value, its adress will be registered
//! @param [in] dynconst Choose if parameter is dynamic (default) or constant (one that can not be converted into a port)
//! @details This function is used in the constructor of the Component modelling code to register member attributes as HOPSAN parameters
void Component::registerParameter(const string name, const string description, const string unit, double &rValue, const ParamDynConstEnumT dynconst)
{
    // We allow the : exception for registring start value parameters
    if (!isNameValid(name, ":"))
    {
        addErrorMessage("Will not register Invalid parameter name: "+name);
        return;
    }

    if(mpParameters->exist(name))
        mpParameters->deleteParameter(name);     //Remove parameter if it is already registered

    //! @todo what if dynamic parameter should we not remove the port as well
    stringstream ss;
    ss << rValue;
    if (dynconst == Dynamic)
    {
        //! @deprecated
        //! @todo remove this later in 0.7
        // Make a port with same name so that parameter can be switch to dynamic parameter that can be changed during simulation
        this->addReadPort(name, "NodeSignal", Port::NotRequired);
        mpParameters->addParameter(name, ss.str(), description, unit, "double", true, &rValue);
        this->addErrorMessage("Dynamic parmeters are no longer supported!!! Use:   addInputVariable()   instead!");
    }
    else
    {
        mpParameters->addParameter(name, ss.str(), description, unit, "double", false, &rValue);
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
    if (!isNameValid(name))
    {
        addErrorMessage("Will not register Invalid parameter name: "+name);
        return;
    }

    if(mpParameters->exist(name))
        mpParameters->deleteParameter(name);     //Remove parameter if it is already registered

    stringstream ss;
    ss << rValue;
    mpParameters->addParameter(name, ss.str(), description, unit, "integer", false, &rValue);
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
    if (!isNameValid(name))
    {
        addErrorMessage("Will not register Invalid parameter name: "+name);
        return;
    }

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
    if (!isNameValid(name))
    {
        addErrorMessage("Will not register Invalid parameter name: "+name);
        return;
    }

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
    addWarningMessage("Function setDesiredTimestep() is only available on subsystem components.");
}


void Component::setInheritTimestep(const bool /*inherit*/)
{
    addWarningMessage("Function setInheritTimestep() is only available on subsystem components.");
}


bool Component::doesInheritTimestep() const
{
    addWarningMessage("Function doesInheritTimestep() is only available on subsystem components.");
    return true;       //Components always inherit timestep, so let's return true
}


bool Component::checkModelBeforeSimulation()
{
    addWarningMessage("Function checkModelBeforeSimulation() is only available on subsystem components.");
    return true;        //Assume component is ok
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
Port* Component::addPort(const string portName, const PortTypesEnumT portType, const std::string nodeType, const Port::RequireConnectionEnumT reqConnection)
{
    addLogMess(getName()+"::addPort");

    //Make sure name is unique before insert
    string newname = this->determineUniquePortName(portName);

    Port* new_port = createPort(portType, nodeType, newname, this);

    //Set wheter the port must be connected before simulation
    if (reqConnection == Port::NotRequired)
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

//! @brief Adds a port to the component
//! @param [in] portName The desired name of the port (may be automatically changed)
//! @param [in] porttype The type of port
//! @param [in] nodetype The type of node that must be connected to the port
//! @param [in] description A description string describing the port
//! @param [in] connection_requirement Specify if the port must be connecteed or if it is optional
//! @return A pointer to the created port
Port *Component::addPort(const string portName, const PortTypesEnumT portType, const string nodeType, const string description, const Port::RequireConnectionEnumT reqConnection)
{
    Port *pPort = addPort(portName, portType, nodeType, reqConnection);
    pPort->setDescription(description);
    return pPort;
}

Port *Component::addWritePort(const string portName, const string nodeType, const string description, const Port::RequireConnectionEnumT reqConnect)
{
    return addPort(portName, WritePortType, nodeType, description, reqConnect);
}


//! @brief Convenience method to add a PowerPort
//! @ingroup ConvenientPortFunctions
//! @param [in] portName The desired name of the port (may be automatically changed)
//! @param [in] nodeType The type of node that must be connected to the port
//! @param [in] reqConnect Specify if the port must be connecteed or if it is optional (Required or NotRequired)
//! @return A pointer to the created port
Port* Component::addPowerPort(const string portName, const string nodeType, const Port::RequireConnectionEnumT reqConnect)
{
    return addPort(portName, PowerPortType, nodeType, reqConnect);
}

//! @brief Convenience method to add a PowerMultiPort
//! @ingroup ConvenientPortFunctions
//! @param [in] portName The desired name of the port (may be automatically changed)
//! @param [in] nodeType The type of node that must be connected to the port
//! @param [in] reqConnect Specify if the port must be connecteed or if it is optional (Required or NotRequired)
//! @return A pointer to the created port
Port* Component::addPowerMultiPort(const string portName, const string nodeType, const Port::RequireConnectionEnumT reqConnect)
{
    return addPort(portName, PowerMultiportType, nodeType, reqConnect);
}

//! @brief Convenience method to add a ReadMultiPort
//! @ingroup ConvenientPortFunctions
//! @param [in] portName The desired name of the port (may be automatically changed)
//! @param [in] nodeType The type of node that must be connected to the port
//! @param [in] reqConnect Specify if the port must be connecteed or if it is optional (Required or NotRequired)
//! @return A pointer to the created port
Port* Component::addReadMultiPort(const string portName, const string nodeType, const Port::RequireConnectionEnumT reqConnect)
{
    return addPort(portName, ReadMultiportType, nodeType, reqConnect);
}

Port *Component::addPowerPort(const string portName, const string nodeType, const string description, const Port::RequireConnectionEnumT reqConnect)
{
    return addPort(portName, PowerPortType, nodeType, description, reqConnect);
}

Port *Component::addReadPort(const string portName, const string nodeType, const string description, const Port::RequireConnectionEnumT reqConnect)
{
    return addPort(portName, ReadPortType, nodeType, description, reqConnect);
}

Port *Component::addPowerMultiPort(const string portName, const string nodeType, const string description, const Port::RequireConnectionEnumT reqConnect)
{
    return addPort(portName, PowerMultiportType, nodeType, description, reqConnect);
}

Port *Component::addReadMultiPort(const string portName, const string nodeType, const string description, const Port::RequireConnectionEnumT reqConnect)
{
    return addPort(portName, ReadMultiportType, nodeType, description, reqConnect);
}

//! @brief Convenience method to add a ReadPort
//! @ingroup ConvenientPortFunctions
//! @param [in] portName The desired name of the port (may be automatically changed)
//! @param [in] nodeType The type of node that must be connected to the port
//! @param [in] reqConnect Specify if the port must be connecteed or if it is optional (Required or NotRequired)
//! @return A pointer to the created port
Port* Component::addReadPort(const string portName, const string nodeType, const Port::RequireConnectionEnumT reqConnect)
{
    return addPort(portName, ReadPortType, nodeType, reqConnect);
}


//! @brief Convenience method to add a WritePort
//! @ingroup ConvenientPortFunctions
//! @param [in] portName The desired name of the port (may be automatically changed)
//! @param [in] nodeType The type of node that must be connected to the port
//! @param [in] reqConnect Specify if the port must be connecteed or if it is optional (Required or NotRequired)
//! @return A pointer to the created port
Port* Component::addWritePort(const string portName, const string nodeType, const Port::RequireConnectionEnumT reqConnect)
{
    return addPort(portName, WritePortType, nodeType, reqConnect);
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

//! @todo this is a temporary function for backwards compatibility where default values are set thourh getSafeNodeDataPtr
//! @deprecated
double *Component::getSafeNodeDataPtr(Port* pPort, const int dataId, const double defaultValue)
{
    double *pData = getSafeNodeDataPtr(pPort, dataId);
    *pData = defaultValue;
    return pData;
}

//! @deprecated
//! @note Use getNodeDataPtr(Port *pPort, const int dataId) instead
double *Component::getSafeNodeDataPtr(Port *pPort, const int dataId)
{
    double *pData=0;
    if (pPort)
    {
        pData = getNodeDataPtr(pPort, dataId);
    }

    if (!pData)
    {
        addErrorMessage("Data pointer could not be retreived in getSafeNodeDataPtr(), Requested dataId: "+to_string(dataId));
        stopSimulation();
        // Create a dummy double, this will cause a small memory leak
        //! @todo maybe solve this somehow leak in the future, maybe keep a dumy variable somwhere to whcihc everyone will point
        pData = new double();
    }
    return pData;
}

//! @brief This is a help function that returns a pointer to desired NodeData, only for Advanced Use instead of read/write Node
//! @ingroup ConvenientPortFunctions
//! @param[in] pPort A pointer to the port from which to fetch NodeData pointer
//! @param[in] dataId The enum id for the node value to fetch pointer to
//! @param[in] defaultValue Optional default value if port should not be connected (optional), if ommitet it will be 0
//! @returns A pointer to the specified NodeData or a pointer to dummy NodeData
//! @details It is only ment to be used inside individual component code and automatically handles creation of dummy veriables in case optional ports are not connected
//! @todo Dont know if name really good, should indicate that you should only run this once in initialize (otherwise a lot of new doubls may be created)
double *Component::getNodeDataPtr(Port *pPort, const int dataId)
{
    addLogMess(getName() + string("::getNodeDataPtr"));
    //If this is one of the multiports then give an error message to the user so that they KNOW that they have made a misstake
    if (pPort->getPortType() >= MultiportType)
    {
        addErrorMessage("Port: "+pPort->getName()+" is a multiport. Use getSafeMultiPortNodeDataPtr() instead of getSafeNodeDataPtr()");
        return 0;
    }
    return pPort->getNodeDataPtr(dataId);
}

double *Component::getSafeNodeDataPtr(const string &rPortName, const int dataId)
{
    Port *pPort = this->getPort(rPortName);
    if (!pPort)
    {
        addErrorMessage("Could not find Port: "+rPortName+" in getNodeDataPtr()");
    }
    return getSafeNodeDataPtr(pPort, dataId);
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
    if (pPort->getPortType() < MultiportType)
    {
        addErrorMessage(string("Port: ")+pPort->getName()+string(" is NOT a multiport. Use getSafeNodeDataPtr() instead of getSafeMultiPortNodeDataPtr()"));
    }
    *pPort->getNodeDataPtr(dataId, portIdx) = defaultValue;
    return pPort->getNodeDataPtr(dataId, portIdx);
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
    //Copy every port pointer
    PortPtrMapT::iterator ports_it;
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
    PortPtrMapT::iterator it = mPortPtrMap.find(portname);
    if (it != mPortPtrMap.end())
    {
        return it->second;
    }
    else
    {
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

Port *Component::addInputVariable(const string name, const string description, const string unit, const double defaultValue, double **ppNodeData)
{
    //! @todo suport more types
    Port *pPort = addReadPort(name,"NodeSignal", Port::NotRequired);
    pPort->setSignalNodeUnitAndDescription(unit, description);
    setStartValue(pPort, 0, defaultValue);

    if (ppNodeData)
    {
        mAutoSignalNodeDataPtrPorts.insert(std::pair<Port*, double**>(pPort, ppNodeData));
    }

    return pPort;
}

Port *Component::addOutputVariable(const string name, const string description, const string unit, double **ppNodeData)
{
    Port *pPort = addWritePort(name, "NodeSignal", Port::NotRequired);
    pPort->setSignalNodeUnitAndDescription(unit, description);
    disableStartValue(pPort,0);

    if (ppNodeData)
    {
        mAutoSignalNodeDataPtrPorts.insert(std::pair<Port*, double**>(pPort, ppNodeData));
    }

    return pPort;
}

Port *Component::addOutputVariable(const string name, const string description, const string unit, const double defaultValue, double **ppNodeData)
{
    Port *pPort = addWritePort(name, "NodeSignal", Port::NotRequired);
    pPort->setSignalNodeUnitAndDescription(unit, description);
    setStartValue(pPort, 0, defaultValue);

    if (ppNodeData)
    {
        mAutoSignalNodeDataPtrPorts.insert(std::pair<Port*, double**>(pPort, ppNodeData));
    }

    return pPort;
}

void Component::initializeAutoSignalNodeDataPtrs()
{
    map<Port*,double**>::iterator it;
    for (it=mAutoSignalNodeDataPtrPorts.begin(); it!=mAutoSignalNodeDataPtrPorts.end(); ++it)
    {
        // We run the component locale getSafeNodeData ptr as it does more error checking then calling port->getNodeDataPtr directly
        (*(it->second)) = getSafeNodeDataPtr(it->first, 0); // 0 = NodeSignal::Value
    }
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
void Component::addDebugMessage(const string message, const string tag) const
{
    if (mpMessageHandler)
    {
        mpMessageHandler->addDebugMessage("In "+getName()+ ";  " + message, tag);
    }
}


//! @brief Write an Warning message.
//! @ingroup ConvenientMessageFunctions
//! @param [in] message The message string
void Component::addWarningMessage(const string message, const string tag) const
{
    if (mpMessageHandler)
    {
        mpMessageHandler->addWarningMessage("In "+getName()+ ";  " + message, tag);
    }
}


//! @brief Write an Error message.
//! @ingroup ConvenientMessageFunctions
//! @param [in] message The message string
void Component::addErrorMessage(const string message, const string tag) const
{
    if (mpMessageHandler)
    {
        mpMessageHandler->addErrorMessage("In "+getName()+ ";  " + message, tag);
    }
}


//! @brief Write an Info message.
//! @ingroup ConvenientMessageFunctions
//! @param [in] message The message string
void Component::addInfoMessage(const string message, const string tag) const
{
    if (mpMessageHandler)
    {
        mpMessageHandler->addInfoMessage("In "+getName()+ ";  " + message, tag);
    }
}

//! @brief Writes a Fatal message and tells the receiver of the message to close program in a controlled way. Also prints message to log file.
//! @ingroup ConvenientMessageFunctions
//! @param [in] message The message string
void Component::addFatalMessage(const string message, const string tag) const
{
    addLogMess(message);
    if (mpMessageHandler)
    {
        mpMessageHandler->addFatalMessage(getName()+ "::" + message, tag);
    }
}


//! @brief Get the an actual start value of a port
//! @param[in] pPort is the port which should be read from
//! @param[in] idx is the index of the start value e.g. NodeHydraulic::Pressure
//! @returns the start value
double Component::getStartValue(Port* pPort, const size_t idx, const size_t portIdx)
{
    return pPort->getStartValue(idx, portIdx);
}


//! @brief Set the an actual start value of a port
//! @param[in] pPort is the port which should be written to
//! @param[in] idx is the index of the start value e.g. NodeHydraulic::Pressure
//! @param[in] value is the start value that should be written
void Component::setStartValue(Port* pPort, const size_t idx, const double value)
{
    addLogMess(getName()+"::setStartValue");
    pPort->setStartValue(idx, value);
    // If a description exist, then refresh the value text
    if (pPort->getNodeDataDescription(idx))
    {
        mpParameters->refreshParameterValueText(pPort->getName()+"::"+pPort->getNodeDataDescription(idx)->name);
    }
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
    PortPtrMapT::iterator pit;
    for(pit=mPortPtrMap.begin(); pit!=mPortPtrMap.end(); ++pit)
    {
        pit->second->loadStartValues();
    }
}


void Component::loadStartValuesFromSimulation()
{
    PortPtrMapT::iterator pit;
    for(pit=mPortPtrMap.begin(); pit!=mPortPtrMap.end(); ++pit)
    {
        pit->second->loadStartValuesFromSimulation();
    }
}


//! @brief Find and return the full file path name of fileName within the system search path, parent systems included (path to HMF file is always in here)
//! @param fileName the name of the searched file
//! @return full file name path, empty string if it does not exsits
std::string Component::findFilePath(const std::string fileName)
{
    bool found = false;
    std::string fullPath;
    std::string replacer = "/";

    if(!(mSearchPaths.empty()))
    {
        for(size_t i = 0; i<mSearchPaths.size(); ++i)
        {
            size_t pSlash = mSearchPaths[i].find_first_of('/');
            size_t pBackSlash = mSearchPaths[i].find_first_of('\\');
            if(pBackSlash < pSlash)
                replacer = "\\";

            fullPath.clear();
            fullPath = mSearchPaths[i];
            fullPath.append(replacer).append(fileName);

            FILE *fp = fopen(fullPath.c_str(),"r");
            if( fp ) {
                fclose(fp);
                found = true;
                break;
            }
            else
                fullPath.clear();
        }
    }
    if(!found)
    {
        if(!getSystemParent())
            fullPath = fileName;
        else
            fullPath = getSystemParent()->findFilePath(fileName);
    }

    return fullPath;
}
