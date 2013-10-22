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
#include <fstream>
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

//! @defgroup ComponentAuthorFunctions ComponentAuthorFunctions

//! @defgroup ComponentSetupFunctions ComponentSetupFunctions
//! @ingroup ComponentAuthorFunctions

//! @defgroup ComponentSimulationFunctions ComponentSimulationFunctions
//! @ingroup ComponentAuthorFunctions

//! @defgroup ComponentPowerAuthorFunctions ComponentPowerAuthorFunctions
//! @ingroup ComponentAuthorFunctions

//! @defgroup ComponentMessageFunctions ComponentMessageFunctions
//! @ingroup ComponentAuthorFunctions

// This is a dummy variable
double dummyDouble=0;

//! @brief Component base class Constructor
Component::Component()
{
    // Set initial values, they will be overwritten soon, but good for debugging
    mpHopsanEssentials = 0;
    mpMessageHandler = 0;
    mTypeName = "NoTypeNameSetYet";
    mSubTypeName = "";
    mName = "NoNameSetYet";

    mInheritTimestep = true;
    mTimestep = 0.001;

    mpSystemParent = 0;
    mModelHierarchyDepth = 0;

    mpParameters = new ParameterEvaluatorHandler(this);

    mSearchPaths.clear();
}


//! @brief Virtual Function, base version which gives you an error if you try to use it.
bool Component::initialize(const double startT, const double /*stopT*/)
{
    mTime = startT;
    initialize();

    return true;        //Always return true, because we cannot know if it was successful or not (yet)
}

void Component::getParameterNames(std::vector<HString> &rParameterNames)
{
    mpParameters->getParameterNames(rParameterNames);
}

const ParameterEvaluator *Component::getParameter(const HString &rName)
{
    return mpParameters->getParameter(rName);
}

const std::vector<ParameterEvaluator*> *Component::getParametersVectorPtr() const
{
    return mpParameters->getParametersVectorPtr();
}

//! @brief Check if a component has a specific parameter
bool Component::hasParameter(const HString &rName) const
{
    return mpParameters->hasParameter(rName);
}

void Component::getParameterValue(const HString &rName, HString &rValue)
{
    mpParameters->getParameterValue(rName, rValue);
}

//! @brief Returns a pointer directly to the parameter data variable
//! @warning Dont use this function unless YOU REALLY KNOW WHAT YOU ARE DOING
//! @warning This function may be removed in the future
void* Component::getParameterDataPtr(const HString &rName)
{
    return mpParameters->getParameterDataPtr(rName);
}

bool Component::setParameterValue(const HString &rName, const HString &rValue, bool force)
{
    return mpParameters->setParameterValue(rName, rValue, force);
}


void Component::updateParameters()
{
    mpParameters->evaluateParameters();
}


bool Component::checkParameters(HString &errParName)
{
    return mpParameters->checkParameters(errParName);
}

const std::vector<VariameterDescription>* Component::getVariameters()
{
    //! @todo dont rebuild this every time, question is should this be in the nodes and or ports maybe, or should it only be in the components
    mVariameters.clear();

    std::vector<Port*>::iterator pit;
    for (pit=mPortPtrVector.begin(); pit!=mPortPtrVector.end(); ++pit)
    {
        Port *pPort = *pit;
        for (size_t id=0; id<pPort->getNumDataVariables(); ++id)
        {
            const NodeDataDescription *pDesc = pPort->getNodeDataDescription(id);
            if (pDesc)
            {
                VariameterDescription data;
                data.mName = pDesc->name;
                data.mShortName = pDesc->shortname;
                data.mPortName = pPort->getName();
                data.mUnit = pDesc->unit;
                data.mDescription = pDesc->description;
                data.mVariableId = pDesc->id;
                data.mVarType = pDesc->varType;
                data.mAlias = pPort->getVariableAlias(data.mVariableId);
                data.mDataType = "double"; //!< @todo not hardcoded

                if ( (pPort->getNodeType() == "NodeSignal") && (pPort->getPortType() == ReadPortType) )
                {
                    data.mVariameterType = InputVariable;
                }
                else if ( (pPort->getNodeType() == "NodeSignal") && (pPort->getPortType() == WritePortType) )
                {
                    data.mVariameterType = OutputVariable;
                }
                else
                {
                    data.mVariameterType = OtherVariable;
                }

                mVariameters.push_back(data);
                //! @todo some of these will never change after a component has been configured, (but som may, like alias description unit)
            }
        }

    }
    return &mVariameters;
}

//! @deprecated use addConstant() instead, used for backwards compatibility but gives error message if used
void Component::registerParameter(const HString &rName, const HString &rDescription, const HString &rUnit, double &rValue)
{
    addErrorMessage("registerParameter() is deprecated, use addConstant() instead");
    registerParameter(rName, rDescription, rUnit, rValue, 0);
}

//! @deprecated use addConstant() instead, used for backwards compatibility but gives error message if used
void Component::registerParameter(const HString &rName, const HString &rDescription, const HString &rUnit, int &rValue)
{
    addErrorMessage("registerParameter() is deprecated, use addConstant() instead");
    registerParameter(rName, rDescription, rUnit, rValue, 0);
}



///@{
//! @brief Set the value of a constant parameter
//! @note Dont use this function during simulation, it is slow
//! @todo check returnvalue from setParameter check if Ok error emssage otherwise, also in the other functions
//! @ingroup ComponentSetupFunctions
void Component::setConstantValue(const HString &rName, const double value)
{
    setParameterValue(rName, to_hstring(value), true);
}

void Component::setConstantValue(const HString &rName, const int value)
{
    setParameterValue(rName, to_hstring(value), true);
}

void Component::setConstantValue(const HString &rName, const HString &rValue)
{
    setParameterValue(rName, rValue, true);
}

void Component::setConstantValue(const HString &rName, const bool value)
{
    setParameterValue(rName, to_hstring(value), true);
}
///@}

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
    //updateDynamicParameterValues();
    const size_t nSteps = calcNumSimSteps(mTime, stopT); //Here mTime is the last time step since it is not updated yet
    for (size_t i=0; i<nSteps; ++i)
    {
        mTime += mTimestep; //mTime is updated here before the simulation,
                            //mTime is the current time during the simulateOneTimestep
        simulateOneTimestep();
    }
}

//! @brief The initialize function must be overloaded in each component, it is used to initialize the component just before simulation begins
//! @details In this function you should get node data ptrs and caluclate inital values to write to the nodes
//! You are not allowed to reconnect internal connections in this function, as other components may already have initialized and fetch data porinters to poorts/nodes in this component
//! @ingroup ComponentSimulationFunctions
void Component::initialize()
{
    addErrorMessage("You MUST! implement your own initialize method");
    stopSimulation();
}


//! @brief Simulates one time step. This component must be overloaded en each component.
//! @details This is the function where all the component model equitions should be written.
//! This function is called once for every time step
//! @ingroup ComponentSimulationFunctions
void Component::simulateOneTimestep()
{
    addErrorMessage("You MUST! implement your own simulateOneTimestep() method");
    stopSimulation();
}

//! @brief Optional function that is called after every simulation, can be used to clean up memmory allocation made in initialize
//! @ingroup ComponentSimulationFunctions
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
void Component::setName(HString name)
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
const HString &Component::getName() const
{
    return mName;
}


//! @brief Get the C, Q or S type of the component as enum
Component::CQSEnumT Component::getTypeCQS() const
{
    return UndefinedCQSType;
}


//! @brief Get the CQStype as string
HString Component::getTypeCQSString() const
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
const HString &Component::getTypeName() const
{
    return mTypeName;
}

//! @brief Get the SubType name of the component
const HString &Component::getSubTypeName() const
{
    return mSubTypeName;
}

//! @brief Set the SubType name of the component
void Component::setSubTypeName(const HString &rSubTypeName)
{
    mSubTypeName = rSubTypeName;
}


//! @brief Terminate/stop a running initialization or simulation
//! @details Typically used inside components simulateOneTimestep method
//! @ingroup ComponentSimulationFunctions
void Component::stopSimulation()
{
    mpSystemParent->stopSimulation();
}

HopsanEssentials *Component::getHopsanEssentials()
{
    return mpHopsanEssentials;
}

///@{
//! @brief Add (register) a constant parameter to the component
//! @param [in] rName The name of the constant
//! @param [in] rDescription The description of the constant
//! @param [in] rUnit The unit of the constant value
//! @param [in] rData A reference to the data constant
//! @todo Using a reference is not that clear, we should use a ptr instead
//! @ingroup ComponentSetupFunctions
void Component::addConstant(const HString &rName, const HString &rDescription, const HString &rUnit, double &rData)
{
    registerParameter(rName, rDescription, rUnit, rData, 0);
}

void Component::addConstant(const HString &rName, const HString &rDescription, const HString &rUnit, int &rData)
{
    registerParameter(rName, rDescription, rUnit, rData, 0);
}

void Component::addConstant(const HString &rName, const HString &rDescription, const HString &rUnit, HString &rData)
{
    registerParameter(rName, rDescription, rUnit, rData);
}

void Component::addConstant(const HString &rName, const HString &rDescription, const HString &rUnit, bool &rData)
{
    registerParameter(rName, rDescription, rUnit, rData);
}

void Component::addConditionalConstant(const HString &rName, const HString &rDescription, std::vector<HString> &rConditions, int &rData)
{
    rData=0;    //Always initialize conditionals with first condition
    registerConditionalParameter(rName, rDescription, rConditions, rData);
}

///@}

///@{
//! @brief Add (register) a constant parameter with a default value to the component
//! @param [in] rName The name of the constant
//! @param [in] rDescription The description of the constant
//! @param [in] rUnit The unit of the constant value
//! @param [in] defaultValue Default constant value
//! @param [in] rData A reference to the data variable
//! @todo Using a reference is not that clear, we should use a ptr instead
//! @ingroup ComponentSetupFunctions
void Component::addConstant(const HString &rName, const HString &rDescription, const HString &rUnit, const double defaultValue, double &rData)
{
    rData = defaultValue;
    addConstant(rName, rDescription, rUnit, rData);
}

void Component::addConstant(const HString &rName, const HString &rDescription, const HString &rUnit, const int defaultValue, int &rData)
{
    rData = defaultValue;
    addConstant(rName, rDescription, rUnit, rData);
}

void Component::addConstant(const HString &rName, const HString &rDescription, const HString &rUnit, const HString &defaultValue, HString &rData)
{
    rData = defaultValue;
    addConstant(rName, rDescription, rUnit, rData);
}

void Component::addConstant(const HString &rName, const HString &rDescription, const HString &rUnit, const bool defaultValue, bool &rData)
{
    rData = defaultValue;
    addConstant(rName, rDescription, rUnit, rData);
}
///@}

///@{
//! @brief Register a parameter value so that it can be accessed for read and write. Set a Name, Description and Unit.
//! @param [in] rName The name of the parameter
//! @param [in] rDescription A description of the parameter
//! @param [in] rUnit The unit of the parameter value
//! @param [in] rValue A reference to the double variable representing the value, its adress will be registered
//! @details This function is used in the constructor of the Component modelling code to register member attributes as HOPSAN parameters
//! @todo remove the dummy argument once the public deprecated version of this function is removed
//! @todo Using a reference is not that clear, we should use a ptr instead
void Component::registerParameter(const HString &rName, const HString &rDescription, const HString &rUnit, double &rValue, int /*dummy*/)
{
    // We allow the : exception for registring start value parameters
    if (!isNameValid(rName, "#"))
    {
        addErrorMessage("Will not register Invalid parameter name: "+rName);
        return;
    }

    if(mpParameters->hasParameter(rName))
        mpParameters->deleteParameter(rName);     //Remove parameter if it is already registered

    stringstream ss;
    ss << rValue;
    mpParameters->addParameter(rName, ss.str().c_str(), rDescription, rUnit, "double", &rValue);
}

void Component::registerParameter(const HString &rName, const HString &rDescription, const HString &rUnit, int &rValue, int /*dummy*/)
{
    if (!isNameValid(rName))
    {
        addErrorMessage("Will not register Invalid parameter name: "+rName);
        return;
    }

    if(mpParameters->hasParameter(rName))
        mpParameters->deleteParameter(rName);     //Remove parameter if it is already registered

    mpParameters->addParameter(rName, to_hstring(rValue), rDescription, rUnit, "integer", &rValue);
}

void Component::registerParameter(const HString &rName, const HString &rDescription, const HString &rUnit, HString &rValue)
{
    if (!isNameValid(rName))
    {
        addErrorMessage("Will not register Invalid parameter name: "+rName);
        return;
    }

    if(mpParameters->hasParameter(rName))
        mpParameters->deleteParameter(rName);     //Remove parameter if it is already registered

    mpParameters->addParameter(rName, rValue, rDescription, rUnit, "string", &rValue);
}

void Component::registerParameter(const HString &rName, const HString &rDescription, const HString &rUnit, bool &rValue)
{
    if (!isNameValid(rName))
    {
        addErrorMessage("Will not register Invalid parameter name: "+rName);
        return;
    }

    if(mpParameters->hasParameter(rName))
        mpParameters->deleteParameter(rName);     //Remove parameter if it is already registered

    if(rValue)
        mpParameters->addParameter(rName, "true", rDescription, rUnit, "bool", &rValue);
    else
        mpParameters->addParameter(rName, "false", rDescription, rUnit, "bool", &rValue);
}

void Component::registerConditionalParameter(const HString &rName, const HString &rDescription, std::vector<HString> &rConditions, int &rValue)
{
    if (!isNameValid(rName))
    {
        addErrorMessage("Will not register Invalid parameter name: "+rName);
        return;
    }

    if(mpParameters->hasParameter(rName))
        mpParameters->deleteParameter(rName);     //Remove parameter if it is already registered

    mpParameters->addParameter(rName, to_hstring(rValue), rDescription, "", "conditional", &rValue, false, rConditions);
}
///@}


//! @brief Removes a parameter from the component
void Component::unRegisterParameter(const HString &rName)
{
    mpParameters->deleteParameter(rName);
}


void Component::setDesiredTimestep(const double timestep)
{
    setInheritTimestep(false);
    mDesiredTimestep = timestep;
    //setTimestep(timestep);
    //addWarningMessage("Function setDesiredTimestep() is only available on subsystem components.");
}

//! @brief Set wheter teh component should inherit timestep from its system parent
//! @param [in] inherit True or False
void Component::setInheritTimestep(const bool inherit)
{
    mInheritTimestep = inherit;
    //addWarningMessage("Function setInheritTimestep() is only available on subsystem components.");
}

//! @brief Check if a component inherits timestep from its system parent
//! @returns True or False
bool Component::doesInheritTimestep() const
{
    return mInheritTimestep;
    //addWarningMessage("Function doesInheritTimestep() is only available on subsystem components.");
    //return true;       //Components always inherit timestep, so let's return true
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

//! @brief Returns a pointer to the simulation time variable in the component
//! @returns pointer to time variable
double *Component::getTimePtr()
{
    return &mTime;
}


//! @brief Adds a port to the component, do not call this function directlly unless you have to for some reason
//! @param [in] rPortName The desired name of the port (may be automatically changed)
//! @param [in] portType The type of port
//! @param [in] rNodeType The type of node that must be connected to the port
//! @param [in] reqConnection Specify if the port must be connecteed or if it is optional
//! @return A pointer to the created port
Port* Component::addPort(const HString &rPortName, const PortTypesEnumT portType, const HString &rNodeType, const Port::RequireConnectionEnumT reqConnection)
{
    addLogMess((getName()+"::addPort").c_str());

    //Make sure name is unique before insert
    HString newname = this->determineUniquePortName(rPortName);
    //! @todo for ordinary components give an error message, users rarely check debug messages

    Port* pNewPort = createPort(portType, rNodeType, newname, this);

    //Set wheter the port must be connected before simulation
    if (reqConnection == Port::NotRequired)
    {
        //! @todo maybe use a string for OPTIONAL instead, to reduce the number of compiletime dependencies, will need to think about that a bit more
        pNewPort->mConnectionRequired = false;
    }

    // Store the port in the port map, for faster port by name lookup
    mPortPtrMap.insert(PortPtrPairT(newname, pNewPort));
    // Store the port in the vector, to remeber the order of added ports (usefull when retreiving variameters)
    mPortPtrVector.push_back(pNewPort);

    //Signal autmatic name change
    if (newname != rPortName)
    {
        addDebugMessage("Automatically changed name of added port from: {" + rPortName + "} to {" + newname + "}");
    }
    return pNewPort;
}

//! @brief Adds a port to the component, do not call this function directly unless you have to
//! @param [in] rPortName The desired name of the port (may be automatically changed)
//! @param [in] portType The type of port
//! @param [in] rNodeType The type of node that must be connected to the port
//! @param [in] rDescription A description string describing the port
//! @param [in] reqConnection Specify if the port must be connecteed or if it is optional
//! @returns A pointer to the created port
Port *Component::addPort(const HString &rPortName, const PortTypesEnumT portType, const HString &rNodeType, const HString &rDescription, const Port::RequireConnectionEnumT reqConnection)
{
    Port *pPort = addPort(rPortName, portType, rNodeType, reqConnection);
    pPort->setDescription(rDescription);
    return pPort;
}

//! @brief Add a PowerPort to the component
//! @ingroup ComponentSetupFunctions
//! @param [in] rPortName The desired name of the port (may be automatically changed)
//! @param [in] rNodeType The type of node that must be connected to the port
//! @param [in] reqConnect Specify if the port must be connecteed or if it is optional (Required or NotRequired)
//! @return A pointer to the created port
Port* Component::addPowerPort(const HString &rPortName, const HString &rNodeType, const Port::RequireConnectionEnumT reqConnect)
{
    return addPort(rPortName, PowerPortType, rNodeType, reqConnect);
}

//! @brief Add a PowerPort with description to the component
//! @ingroup ComponentSetupFunctions
//! @param [in] rPortName The desired name of the port (may be automatically changed)
//! @param [in] rNodeType The type of node that must be connected to the port
//! @param [in] rDescription The port description
//! @param [in] reqConnect Specify if the port must be connecteed or if it is optional (Required or NotRequired)
//! @return A pointer to the created port
Port *Component::addPowerPort(const HString &rPortName, const HString &rNodeType, const HString &rDescription, const Port::RequireConnectionEnumT reqConnect)
{
    return addPort(rPortName, PowerPortType, rNodeType, rDescription, reqConnect);
}

//! @brief Add a PowerMultiPort to the component
//! @ingroup ComponentSetupFunctions
//! @param [in] rPortName The desired name of the port (may be automatically changed)
//! @param [in] rNodeType The type of node that must be connected to the port
//! @param [in] reqConnect Specify if the port must be connecteed or if it is optional (Required or NotRequired)
//! @return A pointer to the created port
Port* Component::addPowerMultiPort(const HString &rPortName, const HString &rNodeType, const Port::RequireConnectionEnumT reqConnect)
{
    return addPort(rPortName, PowerMultiportType, rNodeType, reqConnect);
}

//! @brief Add a PowerMultiPort with description  to the component
//! @ingroup ComponentSetupFunctions
//! @param [in] rPortName The desired name of the port (may be automatically changed)
//! @param [in] rNodeType The type of node that must be connected to the port
//! @param [in] rDescription The port description
//! @param [in] reqConnect Specify if the port must be connecteed or if it is optional (Required or NotRequired)
//! @return A pointer to the created port
Port *Component::addPowerMultiPort(const HString &rPortName, const HString &rNodeType, const HString &rDescription, const Port::RequireConnectionEnumT reqConnect)
{
    return addPort(rPortName, PowerMultiportType, rNodeType, rDescription, reqConnect);
}

//! @brief Add a ReadMultiPort to the component
//! @ingroup ComponentSetupFunctions
//! @param [in] rPortName The desired name of the port (may be automatically changed)
//! @param [in] rNodeType The type of node that must be connected to the port
//! @param [in] reqConnect Specify if the port must be connecteed or if it is optional (Required or NotRequired)
//! @return A pointer to the created port
Port* Component::addReadMultiPort(const HString &rPortName, const HString &rNodeType, const Port::RequireConnectionEnumT reqConnect)
{
    return addPort(rPortName, ReadMultiportType, rNodeType, reqConnect);
}

//! @brief Add a ReadMultiPort with description to the component
//! @ingroup ComponentSetupFunctions
//! @param [in] rPortName The desired name of the port (may be automatically changed)
//! @param [in] rNodeType The type of node that must be connected to the port
//! @param [in] rDescription The port description
//! @param [in] reqConnect Specify if the port must be connecteed or if it is optional (Required or NotRequired)
//! @return A pointer to the created port
Port *Component::addReadMultiPort(const HString &rPortName, const HString &rNodeType, const HString &rDescription, const Port::RequireConnectionEnumT reqConnect)
{
    return addPort(rPortName, ReadMultiportType, rNodeType, rDescription, reqConnect);
}

//! @brief Add a ReadPort to the component
//! @note Usually you should use addInputVariable instead of this one
//! @ingroup ComponentSetupFunctions
//! @param [in] rPortName The desired name of the port (may be automatically changed)
//! @param [in] rNodeType The type of node that must be connected to the port
//! @param [in] reqConnect Specify if the port must be connecteed or if it is optional (Required or NotRequired)
//! @return A pointer to the created port
Port* Component::addReadPort(const HString &rPortName, const HString &rNodeType, const Port::RequireConnectionEnumT reqConnect)
{
    return addPort(rPortName, ReadPortType, rNodeType, reqConnect);
}

//! @brief Add a ReadPort with description to the component
//! @note Usually you should use addInputVariable instead of this one
//! @ingroup ComponentSetupFunctions
//! @param [in] rPortName The desired name of the port (may be automatically changed)
//! @param [in] rNodeType The type of node that must be connected to the port
//! @param [in] rDescription The port description
//! @param [in] reqConnect Specify if the port must be connecteed or if it is optional (Required or NotRequired)
//! @return A pointer to the created port
Port *Component::addReadPort(const HString &rPortName, const HString &rNodeType, const HString &rDescription, const Port::RequireConnectionEnumT reqConnect)
{
    return addPort(rPortName, ReadPortType, rNodeType, rDescription, reqConnect);
}

//! @brief Add a WritePort with description to the component
//! @note Usually you should use addOutputVariable instead of this one unless you need a "sniffer port"
//! @param [in] rPortName The desired name of the port (may be automatically changed)
//! @param [in] rNodeType The type of node that must be connected to the port
//! @param [in] reqConnect Specify if the port must be connecteed or if it is optional (Required or NotRequired)
//! @return A pointer to the created port
Port *Component::addWritePort(const HString &rPortName, const HString &rNodeType, const Port::RequireConnectionEnumT reqConnect)
{
    return addPort(rPortName, WritePortType, rNodeType, reqConnect);
}

//! @brief Add a WritePort with description to the component
//! @note Usually you should use addOutputVariable instead of this one unless you need a "sniffer port"
//! @param [in] rPortName The desired name of the port (may be automatically changed)
//! @param [in] rNodeType The type of node that must be connected to the port
//! @param [in] rDescription The port description
//! @param [in] reqConnect Specify if the port must be connecteed or if it is optional (Required or NotRequired)
//! @return A pointer to the created port
Port *Component::addWritePort(const HString &rPortName, const HString &rNodeType, const HString &rDescription, const Port::RequireConnectionEnumT reqConnect)
{
    return addPort(rPortName, WritePortType, rNodeType, rDescription, reqConnect);
}


//! @brief Rename a port
//! @param [in] rOldname The name of the the port to rename
//! @param [in] rNewname The desired new name of the the port
//! @return The actual new name of the port or old name if not renamed
//! @todo this could be a template function to use with all rename in map
HString Component::renamePort(const HString &rOldname, const HString &rNewname)
{
    if (mPortPtrMap.count(rOldname) != 0)
    {
        Port *pTempPort;
        PortPtrMapT::iterator it;

        it = mPortPtrMap.find(rOldname);                            // Find iterator to port
        pTempPort = it->second;                                     // Backup copy of port ptr
        mPortPtrMap.erase(it);                                      // Erase old value
        HString modnewname = determineUniquePortName(rNewname);     // Make sure new name is unique
        pTempPort->mPortName = modnewname;                          // Set new name in port
        mPortPtrMap.insert(PortPtrPairT(modnewname, pTempPort));    // Re add to map
        return modnewname;
    }
    else
    {
        addWarningMessage("Trying to rename port {" + rOldname + "}, but not found");
        return rOldname;
    }
}

//! @brief Removes and deletes a port from a component
//! @param [in] rName The name of the port to delete
//! @note Only use this function to remove systemports, removing ordinary ports from components is a bad idea
void Component::deletePort(const HString &rName)
{
    PortPtrMapT::iterator it;
    it = mPortPtrMap.find(rName);
    if (it != mPortPtrMap.end())
    {
        Port *pPort = it->second;

        // Remove in port vector first
        std::vector<Port*>::iterator pvit;
        for (pvit=mPortPtrVector.begin(); pvit!=mPortPtrVector.end(); ++pvit)
        {
            if ( *pvit == pPort )
            {
                mPortPtrVector.erase(pvit);
                break;
            }
        }

        // Erase from map
        mPortPtrMap.erase(it);

        // delete the port
        delete pPort;
    }
    else
    {
        addWarningMessage("Trying to delete port {" + rName + "}, but not found");
    }
}

//! @todo this is a temporary function for backwards compatibility where default values are set thourh getSafeNodeDataPtr
//! @deprecated
double *Component::getSafeNodeDataPtr(Port* pPort, const int dataId, const double defaultValue)
{
    addErrorMessage("In: "+this->getName()+", getSafeNodeDataPtr(pPort, dataId, defaultValue), is no longer supported. Use setStartValue() instead");
    double *pData = getSafeNodeDataPtr(pPort, dataId);
    *pData = defaultValue;
    return pData;
}

//! @brief Get a pointer to the node data variable, (Port pointer version)
//! @ingroup ComponentSimulationFunctions
//! @note This function is slow, you should not run it during simulation
//! @details The safe in this version means that a dummy pointer will be returnd if the desired one was not found,
//! this prevents krash and the need to check that return pointer is ok, giving cleaner component code
//! An error message is given if the desired data was not found
//! @param[in] pPort A pointer to the port from which to fetch the node data pointer
//! @param[in] dataId The enum id for the node value to fetch pointer to, (Ex: ModeHydraulic::Pressure)
//! @returns A pointer to the specified NodeData or a pointer to dummy NodeData
double *Component::getSafeNodeDataPtr(Port *pPort, const int dataId)
{
    double *pData=0;
    if (pPort)
    {
        pData = getNodeDataPtr(pPort, dataId);
    }

    if (!pData)
    {
        addErrorMessage("Data pointer could not be retreived in getSafeNodeDataPtr(), Requested dataId: "+to_hstring(dataId));
        stopSimulation();
        // Return pointer to dummy data
        pData = &dummyDouble;
    }
    return pData;
}

//! @brief Get a pointer to the node data variable, (Port name version)
//! @ingroup ComponentSimulationFunctions
//! @note This function is slow, you should not run it during simulation
//! @details The safe in this version means that a dummy pointer will be returnd if the desired one was not found,
//! this prevents crash and the need to check that return pointer is ok, giving cleaner component code
//! An error message is given if the desired data was not found
//! @param[in] rPortName The name of the port from which to fetch the node data pointer
//! @param[in] dataId The enum id for the node value to fetch pointer to, (Ex: ModeHydraulic::Pressure)
//! @returns A pointer to the specified NodeData or a pointer to dummy NodeData
double *Component::getSafeNodeDataPtr(const HString &rPortName, const int dataId)
{
    Port *pPort = this->getPort(rPortName);
    if (!pPort)
    {
        addErrorMessage("Could not find Port: "+rPortName+" in getSafeNodeDataPtr()");
    }
    return getSafeNodeDataPtr(pPort, dataId);
}

//! @brief Returns node data pointer
//! @param[in] pPort A pointer to the port from which to fetch NodeData pointer
//! @param[in] dataId The enum id for the node value to fetch pointer to
//! @returns A pointer to the specified NodeData or a null pointer
double *Component::getNodeDataPtr(Port *pPort, const int dataId)
{
    addLogMess((getName()+"::getNodeDataPtr").c_str());
    //If this is one of the multiports then give an error message to the user so that they KNOW that they have made a misstake
    if (pPort->getPortType() >= MultiportType)
    {
        addErrorMessage("Port: "+pPort->getName()+" is a multiport. Use getSafeMultiPortNodeDataPtr() instead of getSafeNodeDataPtr()");
        return 0;
    }
    return pPort->getNodeDataPtr(dataId);
}



//! @brief Get a pointer to node data in a subport in a multiport
//! @ingroup ComponentSimulationFunctions
//! @param[in] pPort A pointer to the port from which to fetch NodeData pointer
//! @param[in] portIdx The index of the subport in a multiport
//! @param[in] dataId The enum id for the node value to fetch pointer to
//! @returns A pointer to the specified NodeData or a pointer to dummy NodeData
double *Component::getSafeMultiPortNodeDataPtr(Port *pPort, const size_t portIdx, const int dataId)
{
    addLogMess((getName()+"::getSafeMultiPortNodeDataPtr").c_str());
    //If this is not a multiport then give an error message to the user so that they KNOW that they have made a misstake
    if (pPort->getPortType() < MultiportType)
    {
        addErrorMessage("Port: "+pPort->getName()+" is NOT a multiport. Use getSafeNodeDataPtr() instead of getSafeMultiPortNodeDataPtr()");
    }
    return pPort->getNodeDataPtr(dataId, portIdx);
}

//! @brief Get a pointer to node data in a subport in a multiport (also setting initial value at the same time)
//! @ingroup ComponentSimulationFunctions
//! @param[in] pPort A pointer to the port from which to fetch NodeData pointer
//! @param[in] portIdx The index of the subport in a multiport
//! @param[in] dataId The enum id for the node value to fetch pointer to
//! @returns A pointer to the specified NodeData or a pointer to dummy NodeData
double *Component::getSafeMultiPortNodeDataPtr(Port* pPort, const size_t portIdx, const int dataId, const double defaultValue)
{
    double* pData = getSafeMultiPortNodeDataPtr(pPort, portIdx, dataId);
    *pData = defaultValue; // Set desired initial value
    return pData;
}

//! @brief Read node data based on port and data name, also checks so that correct data is returned
//! @note This functions is slow, do not use it during simulation
//! @details It searches for data based on strings, this make it unsiutable for use during simualtion but its excelent for use in initialize when port pointers are not desired/availible
//! This function will also check so that the desired data actually exist in the requested node
//! @ingroup ComponentSimulationFunctions
//! @param [in] rPortName The port to get data from
//! @param [in] rDataName The data variable name for the data to retreive
//! @returns The node data value or -1 if failed. (And error message will also be sent)
double Component::readNodeSafeSlow(const HString &rPortName, const HString &rDataName)
{
    Port *pPort = getPort(rPortName);
    if (pPort)
    {
        //! @todo what about multiports
        int id = pPort->getNodeDataIdFromName(rDataName);
        if (id > 0)
        {
            return pPort->readNode(id);
        }
        addErrorMessage("You are trying to get a dataName: "+rDataName+" that does not exist in port: "+rPortName);
        return -1;
    }
    addErrorMessage("You are trying to access port: "+rPortName+" that does not exist");
    return -1;
}

//! @brief Write node data based on port and data name, also checks so that correct data is written
//! @note This functions is slow, do not use it during simulation
//! @details It searches for data based on strings, this make it unsiutable for use during simualtion but its excelent for use in initialize when port pointers are not desired/availible
//! This function will also check so that the desired data actually exist in the requested node, error message will be sent if it does not
//! @ingroup ComponentSimulationFunctions
//! @param [in] rPortName The port to write data to
//! @param [in] rDataName The data variable name for the data to be written
//! @param [in] value The value to write
void Component::writeNodeSafeSlow(const HString &rPortName, const HString &rDataName, const double value)
{
    Port *pPort = getPort(rPortName);
    if (pPort)
    {
        //! @todo what about multiports
        int id = pPort->getNodeDataIdFromName(rDataName);
        if (id >= 0)
        {
            pPort->writeNode(id, value);
            return;
        }
        addErrorMessage("You are trying to set value for dataName: "+rDataName+" that does not exist in port: "+rPortName);
        return;
    }
    addErrorMessage("You are trying to access port: "+rPortName+" that does not exist");
    return;
}

//! @brief This function is an alias function for writeNodeSafeSlow, to make component code more readable
//! @note This functions is slow, do not use it during simulation
//! @details It just calls writeNodeSafeSlow but is meant to be used if you want to set an inital value during initialize, as setdefaultStartValues changes the default value for the next simulation
//! @see writeNodeSafeSlow setDefaultStartValue
//! @ingroup ComponentSimulationFunctions
//! @param [in] rPortName The port to write data to
//! @param [in] rDataName The data variable name for the data to be written
//! @param [in] value The value to write
void Component::setInitialValue(const HString &rPortName, const HString &rDataName, const double value)
{
    writeNodeSafeSlow(rPortName, rDataName, value);
}


//! @brief a virtual function that detmines a unique port name, needs to be overloaded in ComponentSystem to do this slightly different
HString Component::determineUniquePortName(const HString &rPortname)
{
    return findUniqueName<PortPtrMapT>(mPortPtrMap, rPortname);
}

//! @brief Set the component parent system (tell component who parent is)
//! @param [in] pComponentSystem Pointer to parent system component
void Component::setSystemParent(ComponentSystem *pComponentSystem)
{
    mpSystemParent = pComponentSystem;
}

//! @brief This is suposed to be used by hopsan essentials to set the typename to the same as the registered key value
void Component::setTypeName(const HString &rTypeName)
{
    mTypeName = rTypeName;
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
Port *Component::getPort(const HString &rPortname) const
{
    PortPtrMapT::const_iterator it = mPortPtrMap.find(rPortname);
    if (it != mPortPtrMap.end())
    {
        return it->second;
    }
    else
    {
        addDebugMessage("Trying to get port '" + rPortname + "' in component '" + this->getName() + "', but not found, pointer invalid");
        return 0;
    }
}

//! @brief Returns a string vector containing names of all ports in the component
//! @returns A vector with the port names
std::vector<HString> Component::getPortNames()
{
    vector<HString> names;
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
bool Component::getPort(const HString &rPortname, Port* &rpPort)
{
    rpPort = getPort(rPortname);
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

//! @brief Add an inputVariable (Scalar signal ReadPort)
//! @param [in] rName The name of the variable
//! @param [in] rDescription The description of the variable
//! @param [in] rUnit The unit of the variable value
//! @param [in] defaultValue The default variable value (if not connected)
//! @param [in,out] ppNodeData Optional pointer to pointer to data. The data pointer will be registered and automatically assigned before initialisation)
//! @returns A pointer to the port created.
//! @ingroup ComponentSetupFunctions
Port *Component::addInputVariable(const HString &rName, const HString &rDescription, const HString &rUnit, const double defaultValue, double **ppNodeData)
{
    //! @todo suport more types
    Port *pPort = addReadPort(rName,"NodeSignal", Port::NotRequired);
    pPort->setSignalNodeUnitAndDescription(rUnit, rDescription);
    setDefaultStartValue(pPort, 0, defaultValue);

    if (ppNodeData)
    {
        mAutoSignalNodeDataPtrPorts.insert(std::pair<Port*, double**>(pPort, ppNodeData));
    }

    return pPort;
}

//! @brief Add an outputVariable (Scalar signal WritePort) without default value
//! @param [in] rName The name of the variable
//! @param [in] rDescription The description of the variable
//! @param [in] rUnit The unit of the variable value
//! @param [in,out] ppNodeData Optional pointer to pointer to data. The data pointer will be registered and automatically assigned before initialisation)
//! @returns A pointer to the port created.
//! @ingroup ComponentSetupFunctions
Port *Component::addOutputVariable(const HString &rName, const HString &rDescription, const HString &rUnit, double **ppNodeData)
{
    Port *pPort = addWritePort(rName, "NodeSignal", rDescription, Port::NotRequired);
    pPort->setSignalNodeUnitAndDescription(rUnit, rDescription);
    disableStartValue(pPort,0);

    if (ppNodeData)
    {
        mAutoSignalNodeDataPtrPorts.insert(std::pair<Port*, double**>(pPort, ppNodeData));
    }

    return pPort;
}

//! @brief Add an outputVariable (Scalar signal WritePort) with default value
//! @param [in] rName The name of the variable
//! @param [in] rDescription The description of the variable
//! @param [in] rUnit The unit of the variable value
//! @param [in] defaultValue The default variable value (if not connected)
//! @param [in,out] ppNodeData Optional pointer to pointer to data. The data pointer will be registered and automatically assigned before initialisation)
//! @returns A pointer to the port created.
//! @ingroup ComponentSetupFunctions
Port *Component::addOutputVariable(const HString &rName, const HString &rDescription, const HString &rUnit, const double defaultValue, double **ppNodeData)
{
    Port *pPort = addWritePort(rName, "NodeSignal", rDescription, Port::NotRequired);
    pPort->setSignalNodeUnitAndDescription(rUnit, rDescription);
    setDefaultStartValue(pPort, 0, defaultValue);

    if (ppNodeData)
    {
        mAutoSignalNodeDataPtrPorts.insert(std::pair<Port*, double**>(pPort, ppNodeData));
    }

    return pPort;
}

//! @brief Automatically retrieve and assign node data pointers that have been registered
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
//! @ingroup ComponentMessageFunctions
//! @param [in] rMessage The message string
//! @param [in] rTag The message tag, used to group similar messages
void Component::addDebugMessage(const HString &rMessage, const HString &rTag) const
{
    if (mpMessageHandler)
    {
        mpMessageHandler->addDebugMessage("In "+getName()+ ";  " + rMessage, rTag);
    }
}


//! @brief Write an Warning message.
//! @ingroup ComponentMessageFunctions
//! @param [in] rMessage The message string
//! @param [in] rTag The message tag, used to group similar messages
void Component::addWarningMessage(const HString &rMessage, const HString &rTag) const
{
    if (mpMessageHandler)
    {
        mpMessageHandler->addWarningMessage("In "+getName()+ ";  " + rMessage, rTag);
    }
}


//! @brief Write an Error message.
//! @ingroup ComponentMessageFunctions
//! @param [in] rMessage The message string
//! @param [in] rTag The message tag, used to group similar messages
void Component::addErrorMessage(const HString &rMessage, const HString &rTag) const
{
    if (mpMessageHandler)
    {
        mpMessageHandler->addErrorMessage("In "+getName()+ ";  " + rMessage, rTag);
    }
}


//! @brief Write an Info message.
//! @ingroup ComponentMessageFunctions
//! @param [in] rMessage The message string
//! @param [in] rTag The message tag, used to group similar messages
void Component::addInfoMessage(const HString &rMessage, const HString &rTag) const
{
    if (mpMessageHandler)
    {
        mpMessageHandler->addInfoMessage("In "+getName()+ ";  " + rMessage, rTag);
    }
}

//! @brief Writes a Fatal message and tells the receiver of the message to close program in a controlled way. Also prints message to log file.
//! @ingroup ComponentMessageFunctions
//! @param [in] rMessage The message string
//! @param [in] rTag The message tag, used to group similar messages
void Component::addFatalMessage(const HString &rMessage, const HString &rTag) const
{
    addLogMess(rMessage.c_str());
    if (mpMessageHandler)
    {
        mpMessageHandler->addFatalMessage(getName()+"::"+rMessage, rTag);
    }
}


//! @deprecated Use getDefaultStartValue instead
double Component::getStartValue(Port* pPort, const size_t idx, const size_t portIdx)
{
    addErrorMessage("getStartValue() is deprecated, use getDefaultStartValue() instead. Note!, it will not return the initial value!");
    return getDefaultStartValue(pPort, idx, portIdx);
}

//! @brief Get the an actual start value of a port
//! @param[in] pPort is the port which should be read from
//! @param[in] idx is the index of the start value e.g. NodeHydraulic::Pressure
//! @returns The default start value
//! @ingroup ComponentSetupFunctions
double Component::getDefaultStartValue(Port* pPort, const size_t idx, const size_t portIdx)
{
    return pPort->getStartValue(idx, portIdx);
}


//! @deprecated Use setDefaultStartValue instead
void Component::setStartValue(Port* pPort, const size_t idx, const double value)
{
    addErrorMessage("setStartValue() is deprecated, use setDefaultStartValue() instead. Note!, it will not set the initial value!");
    setDefaultStartValue(pPort,idx,value);
}

//! @brief Set the default startvalue in a port
//! @param [in] pPort is the port which should be written to
//! @param [in] idx is the index of the start value e.g. NodeHydraulic::Pressure
//! @param [in] value is the start value that should be written
//! @ingroup ComponentSetupFunctions
void Component::setDefaultStartValue(Port *pPort, const size_t idx, const double value)
{
    addLogMess((getName()+"::setDefaultStartValue").c_str());
    pPort->setDefaultStartValue(idx, value);
    // If a description exist, then refresh the value text
    if (pPort->getNodeDataDescription(idx))
    {
        mpParameters->refreshParameterValueText(pPort->getName()+"#"+pPort->getNodeDataDescription(idx)->name);
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
    // Delete any ports that have been added to the component
    PortPtrMapT::iterator ppmit;
    for (ppmit=mPortPtrMap.begin(); ppmit!=mPortPtrMap.end(); ++ppmit)
    {
        delete (*ppmit).second;
    }

    // Delete any registered parameters (Constants and start values)
    delete mpParameters;
}

//! @brief Configures a component by setting up ports, variabled, constants and other resources
//! @details Every function must overload this function, in this function ports variablesd and constants will be added
//! The component author is howere free to add any other desired code as well.
//! This function is called immediately after a component instace has been created
//! @ingroup ComponentSetup
void Component::configure()
{
    // This function ust be overloaded in every component
    addErrorMessage("You must overload the configure() function in Component: " + mTypeName);
}

//! @brief Deconfigure a component, use this to cleanup and memory/resource allocations you have made in configure
//! @details This function can be optionally overloaded if it is needed
//! You can use it to free memmory or other resources that you have created in configure
//! This function is the last one called before a component instance is deleted (destructor called)
//! @ingroup ComponentSetup
void Component::deconfigure()
{
    // This function can be overloaded in every component
    // Does nothing by default
}

//! @brief This function can be used to automate things prior to component initialization, only use this if you know what you are doing
//! @details One example of what you can do, is reconnecting interanl connections in programed subsystems
//! @returns True or False to signal sucess or failure
//! @ingroup ComponentPowerAuthorFunctions
bool Component::preInitialize()
{
    // This function can be overloaded in components if needed
    // Does nothing by default
    return true;
}


//! @brief Loads the default start values to the connected node in each port on the component
void Component::loadStartValues()
{
    PortPtrMapT::iterator pit;
    for(pit=mPortPtrMap.begin(); pit!=mPortPtrMap.end(); ++pit)
    {
        pit->second->loadStartValues();
    }
}

//! @todo this function is strange, need to tfind a better name and make sure taht this really works in is really the thing you want to do
//! @details The function saves the last values as default startvalues for the next simulation
void Component::loadStartValuesFromSimulation()
{
    PortPtrMapT::iterator pit;
    for(pit=mPortPtrMap.begin(); pit!=mPortPtrMap.end(); ++pit)
    {
        pit->second->loadStartValuesFromSimulation();
    }
}


//! @brief Find and return the full file path name of fileName within the system search path, parent systems included (path to HMF file is always in here)
//! @details With this function you can find external files based on a path relative to the model file path
//! This makes it possible to avoid absolut paths for external file resources
//! @param fileName the name of the file to search for
//! @return full file name path, empty string if it does not exsits
//! @ingroup ComponentSetupFunctions
HString Component::findFilePath(const HString &rFileName)
{
    bool found = false;
    HString fullPath;
    HString replacer = "/";

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
            fullPath.append(replacer).append(rFileName);

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
            fullPath = rFileName;
        else
            fullPath = getSystemParent()->findFilePath(rFileName);
    }

    return fullPath;
}

double Component::getStateVariableDerivative(int)
{
    addErrorMessage("getStateVariableDerivative() is not implemented in component.");
    stopSimulation();

    return 0;
}

double Component::getStateVariableSecondDerivative(int)
{
    addErrorMessage("getStateVariableSecondDerivative() is not implemented in component.");
    stopSimulation();

    return 0;
}

