/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   Component.cpp
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
#include "CoreUtilities/HmfLoader.h"
#include "Port.h"
#include "HopsanEssentials.h"
#include "CoreUtilities/StringUtilities.h"
#include "ComponentUtilities/num2string.hpp"
#include "Quantities.h"

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
    mIsDisabled = false;
    mTimestep = 0.001;

    mpSystemParent = 0;
    mModelHierarchyDepth = 0;

    mpParameters = new ParameterEvaluatorHandler(this);

    mSearchPaths.clear();
}


//! @brief Virtual Function, base version which gives you an error if you try to use it.
bool Component::initialize(const double startT, const double stopT)
{
    HOPSAN_UNUSED(stopT)
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

double Component::evaluateDoubleParameter(const HString &rName, bool &rEvalOK)
{
    HString val;
    bool rc = false;
    if(rName.startsWith("self#"))
    {
        HString name =rName.substr(5,rName.size()-5);
        rc = mpParameters->evaluateInLocalComponent(name, val, "double");
    }
    else
    {
        rc = mpParameters->evaluateInSystemParent(rName, val, "double");
    }
    double v = val.toDouble(&rEvalOK);
    rEvalOK = (rEvalOK && rc);
    return v;
}

//! @brief Returns a pointer directly to the parameter data variable
//! @warning Don't use this function unless YOU REALLY KNOW WHAT YOU ARE DOING
void* Component::getParameterDataPtr(const HString &rName)
{
    return mpParameters->getParameterDataPtr(rName);
}

bool Component::setParameterValue(const HString &rName, const HString &rValue, bool force)
{
    return mpParameters->setParameterValue(rName, rValue, force);
}


void Component::evaluateParameters()
{
    mpParameters->evaluateParameters();
}

bool Component::evaluateParameter(const HString &rName, HString &rEvaluatedParameterValue, const HString &rType)
{
    return mpParameters->evaluateInLocalComponent(rName, rEvaluatedParameterValue, rType);
}


bool Component::checkParameters(HString &errParName)
{
    return mpParameters->checkParameters(errParName);
}

const std::vector<VariameterDescription>* Component::getVariameters()
{
    //! @todo don't rebuild this every time, question is should this be in the nodes and or ports maybe, or should it only be in the components
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
                data.mNodeType = pPort->getNodeType();
                data.mUnit = pDesc->unit;
                data.mQuantity = pDesc->quantity;
                data.mUserModifiableQuantity = pDesc->userModifiableQuantity;
                data.mVariableId = pDesc->id;
                data.mVarType = pDesc->varType;
                data.mAlias = pPort->getVariableAlias(data.mVariableId);
                data.mDataType = "double"; //!< @todo not hardcoded

                if ( (pPort->getNodeType() == "NodeSignal") && (pPort->getPortType() == ReadPortType) )
                {
                    data.mVariameterType = InputVariable;
                    data.mDescription = pPort->getDescription();
                }
                else if ( (pPort->getNodeType() == "NodeSignal") && (pPort->getPortType() == WritePortType) )
                {
                    data.mVariameterType = OutputVariable;
                    data.mDescription = pPort->getDescription();
                }
                else
                {
                    data.mVariameterType = OtherVariable;
                }

                mVariameters.push_back(data);
                //! @todo some of these will never change after a component has been configured, (but some may, like alias description unit)
            }
        }

    }
    return &mVariameters;
}

std::list<HString> Component::getModelAssets() const
{
    std::list<HString> assets;
    const std::vector<ParameterEvaluator*>* pParameters = mpParameters->getParametersVectorPtr();
    std::vector<ParameterEvaluator*>::const_iterator it;
    for (it=pParameters->begin(); it!=pParameters->end(); ++it)
    {
        ParameterEvaluator* pPE = *it;
        if (pPE->getType() == "string" && !pPE->getValue().empty())
        {
            //! @todo add parameter type (file or asset)
            // Check if parameter value represents a file
            HString filePath = findFilePath(pPE->getValue());
            if (FILE *pFile = fopen(filePath.c_str(), "r"))
            {
                fclose(pFile);
                // Remember the identifier (relative file path) (it may be an absolute path as-well)
                assets.push_back(pPE->getValue());
            }
        }
    }
    return assets;
}

//! @brief Loads parameters from a file
//! @param[in] rFilePath The file to load from
//! @return Number of changed parameters
size_t hopsan::Component::loadParameterValues(const hopsan::HString &rFilePath)
{
    return loadHopsanParameterFile(rFilePath, getHopsanEssentials()->getCoreMessageHandler(), this);
}



///@{
//! @brief Set the value of a constant parameter
//! @note Don't use this function during simulation, it is slow
//! @todo check return value from setParameter check if Ok error message otherwise, also in the other functions
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

void Component::setConstantValue(const HString &rName, const char *value)
{
    setConstantValue(rName, HString(value));
}

void Component::setConstantValue(const HString &rName, const bool value)
{
    setParameterValue(rName, to_hstring(value), true);
}
///@}


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

    //DEBUG
//    while(mTime < stopT)
//    {
//        mTime += mTimestep;
//        simulateOneTimestep();
//    }
    //END DEBUG
}

void Component::setDisabled(bool value)
{
    mIsDisabled = value;
}

bool Component::isDisabled() const
{
    return mIsDisabled;
}

//! @brief The initialize function must be overloaded in each component, it is used to initialize the component just before simulation begins
//! @details In this function you should get node data ptrs and calculate initial values to write to the nodes
//! You are not allowed to reconnect internal connections in this function, as other components may already have initialized and fetch data pointers to ports/nodes in this component
//! @ingroup ComponentSimulationFunctions
void Component::initialize()
{
    addErrorMessage("You MUST! implement your own initialize() method");
    stopSimulation();
}


//! @brief Simulates one time step. This component must be overloaded en each component.
//! @details This is the function where all the component model equations should be written.
//! This function is called once for every time step
//! @ingroup ComponentSimulationFunctions
void Component::simulateOneTimestep()
{
    addErrorMessage("You MUST! implement your own simulateOneTimestep() method");
    stopSimulation();
}

//! @brief Optional function that is called after every simulation, can be used to clean up memory allocation made in initialize
//! @ingroup ComponentSimulationFunctions
void Component::finalize()
{
    //Default does nothing
}


//! @brief Set the desired component name
//! @param [in] name The desired component name
//!
//! Set the desired component name, if name is already taken in a subsystem the desired name will be modified with a suffix.
//! If you set doOnlyLocalRename to true, the smart rename will not be attempted, avoid doing this as the component storage map will not be updated on name change
//! This is a somewhat ugly fix for some special situations where we want to make sure that a smart rename is not attempted
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
            // Let parent handle names so that we do not get duplicate names
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
//! @param[in] rReason An optional HString describing the reason for the stop
//! @details Typically used inside components simulateOneTimestep method
//! @ingroup ComponentSimulationFunctions
void Component::stopSimulation(const HString &rReason)
{
    HString infoMsg;
    if (rReason.empty())
    {
        infoMsg = "Simulation was stopped at t="+to_hstring(mTime);
    }
    else
    {
        infoMsg = "Simulation was stopped at t="+to_hstring(mTime)+ " : "+rReason;
    }
    addInfoMessage(infoMsg);
    addCoreLogMessage(infoMsg);

    mpSystemParent->stopSimulation(""); // We use string version here to make sure sub system hierarchy is printed
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
    addWarningMessage("Component::addConstant(rName, rDescription, rUnit, rData) is DEPRECATED use Component::addConstant(rName, rDescription, rUnit, defaultValue, rData) instead!", "deprecatedAddConstant");
    registerParameter(rName, rDescription, "", rUnit, rData);
}

void Component::addConstant(const HString &rName, const HString &rDescription, const HString &rUnit, int &rData)
{
    addWarningMessage("Component::addConstant(rName, rDescription, rUnit, rData) is DEPRECATED use Component::addConstant(rName, rDescription, rUnit, defaultValue, rData) instead!", "deprecatedAddConstant");
    registerParameter(rName, rDescription, rUnit, rData);
}

void Component::addConstant(const HString &rName, const HString &rDescription, const HString &rUnit, HString &rData)
{
    addWarningMessage("Component::addConstant(rName, rDescription, rUnit, rData) is DEPRECATED use Component::addConstant(rName, rDescription, rUnit, defaultValue, rData) instead!", "deprecatedAddConstant");
    registerParameter(rName, rDescription, rUnit, rData);
}

void Component::addConstant(const HString &rName, const HString &rDescription, const HString &rUnit, bool &rData)
{
    addWarningMessage("Component::addConstant(rName, rDescription, rUnit, rData) is DEPRECATED use Component::addConstant(rName, rDescription, rUnit, defaultValue, rData) instead!", "deprecatedAddConstant");
    registerParameter(rName, rDescription, rUnit, rData);
}
///@}

//! @brief Add (register) a conditional constant parameter to the component
//! @param [in] rName The name of the constant
//! @param [in] rDescription The description of the constant
//! @param [in] rConditions The condition descriptions as a vector of text
//! @param [in] rData A reference to the condition data constant (it will automatically get default value 0)
//! @todo Using a reference is not that clear, we should use a ptr instead
//! @ingroup ComponentSetupFunctions
void Component::addConditionalConstant(const HString &rName, const HString &rDescription, std::vector<HString> &rConditions, int &rData)
{
    rData=0;    //Always initialize conditionals with first condition
    registerConditionalParameter(rName, rDescription, rConditions, rData);
}

//! @brief Add (register) a conditional constant parameter to the component
//! @param [in] rName The name of the constant
//! @param [in] rDescription The description of the constant
//! @param [in] rConditions The condition descriptions as a vector of text
//! @param [in] defaultValue The default value
//! @param [in] rData A reference to the condition data constant
//! @todo Using a reference is not that clear, we should use a ptr instead
//! @ingroup ComponentSetupFunctions
void Component::addConditionalConstant(const HString &rName, const HString &rDescription, std::vector<HString> &rConditions, const int defaultValue, int &rData)
{
    if (defaultValue < int(rConditions.size()))
    {
        rData=defaultValue;
    }
    else
    {
        addWarningMessage("Conditional parameter: "+rName+" defaultValue is out of bounds");
        rData=0;
    }
    registerConditionalParameter(rName, rDescription, rConditions, rData);
}

//! @brief Add (register) a constant parameter with a default value to the component
//! @param [in] rName The name of the constant
//! @param [in] rDescription The description of the constant
//! @param [in] rQuantityOrUnit The Quantity or Unit of the constant value
//! @param [in] defaultValue Default constant value
//! @param [in] rData A reference to the data variable
//! @todo Using a reference is not that clear, we should use a ptr instead
//! @ingroup ComponentSetupFunctions
void Component::addConstant(const HString &rName, const HString &rDescription, const HString &rQuantityOrUnit, const double defaultValue, double &rData)
{
    // If unit is actually a quantity, then register the quantity with the base unit
    HString quantity, baseunit;
    checkIfQuantityOrUnit(rQuantityOrUnit, quantity, baseunit);
    rData = defaultValue;
    registerParameter(rName, rDescription, quantity, baseunit, rData);
}

///@{
//! @brief Add (register) a constant parameter with a default value to the component
//! @param [in] rName The name of the constant
//! @param [in] rDescription The description of the constant
//! @param [in] rUnit The unit of the constant value
//! @param [in] defaultValue Default constant value
//! @param [in] rData A reference to the data variable
//! @todo Using a reference is not that clear, we should use a ptr instead
//! @ingroup ComponentSetupFunctions

void Component::addConstant(const HString &rName, const HString &rDescription, const HString &rUnit, const int defaultValue, int &rData)
{
    rData = defaultValue;
    registerParameter(rName, rDescription, rUnit, rData);
}

void Component::addConstant(const HString &rName, const HString &rDescription, const HString &rUnit, const HString &defaultValue, HString &rData)
{
    rData = defaultValue;
    registerParameter(rName, rDescription, rUnit, rData);
}

void Component::addConstant(const HString &rName, const HString &rDescription, const HString &rUnit, const bool defaultValue, bool &rData)
{
    rData = defaultValue;
    registerParameter(rName, rDescription, rUnit, rData);
}
///@}

//! @brief Add (register) a constant parameter with a default value to the component
//! @param [in] rName The name of the constant
//! @param [in] rDescription The description of the constant
//! @param [in] rQuantity The physical quantity type (if any)
//! @param [in] rUnit The unit of the constant value
//! @param [in] defaultValue Default constant value
//! @param [in] rData A reference to the data variable
//! @todo Using a reference is not that clear, we should use a ptr instead
//! @ingroup ComponentSetupFunctions
void Component::addConstant(const HString &rName, const HString &rDescription, const HString &rQuantity, const HString &rUnit, const double defaultValue, double &rData)
{
    rData = defaultValue;
    if (rUnit.empty())
    {
        HString bu = gpInternalCoreQuantityRegister->lookupBaseUnit(rQuantity);
        registerParameter(rName, rDescription, rQuantity, bu, rData);
    }
    else if (!rQuantity.empty())
    {
        // Make sure unit correct, since we do not yet support non base units in core
        HString bu = gpInternalCoreQuantityRegister->lookupBaseUnit(rQuantity);
        if (bu != rUnit)
        {
            addErrorMessage(HString("Using non base units together with a quantity is not yet supported: "+bu+" != "+rUnit+" ("+rQuantity+")"));
        }
        registerParameter(rName, rDescription, rQuantity, rUnit, rData);
    }
    else
    {
        registerParameter(rName, rDescription, rQuantity, rUnit, rData);
    }
}

void Component::addConstant(const HString &rName, const HString &rDescription, HTextBlock &rData)
{
    registerParameter(rName, rDescription, rData);
}

///@{
//! @brief Register a parameter value so that it can be accessed for read and write. Set a Name, Description and Unit.
//! @param [in] rName The name of the parameter
//! @param [in] rDescription A description of the parameter
//! @param [in] rUnit The unit of the parameter value
//! @param [in] rValue A reference to the double variable representing the value, its address will be registered
//! @details This function is used in the constructor of the Component modelling code to register member attributes as HOPSAN parameters
//! @todo Using a reference is not that clear, we should use a ptr instead
void Component::registerParameter(const HString &rName, const HString &rDescription, const HString &rQuantity, const HString &rUnit, double &rValue)
{
    // We allow the # exception for registering start value parameters
    if (!isNameValid(rName, "#"))
    {
        addErrorMessage("Will not register Invalid parameter name: "+rName);
        return;
    }

    if(mpParameters->hasParameter(rName))
        mpParameters->deleteParameter(rName);     //Remove parameter if it is already registered

    stringstream ss;
    ss << rValue;
    mpParameters->addParameter(rName, ss.str().c_str(), rDescription, rQuantity, rUnit, "double", &rValue);
}

void Component::registerParameter(const HString &rName, const HString &rDescription, const HString &rUnit, int &rValue)
{
    if (!isNameValid(rName))
    {
        addErrorMessage("Will not register Invalid parameter name: "+rName);
        return;
    }

    if(mpParameters->hasParameter(rName))
        mpParameters->deleteParameter(rName);     //Remove parameter if it is already registered

    mpParameters->addParameter(rName, to_hstring(rValue), rDescription, "", rUnit, "integer", &rValue);
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

    mpParameters->addParameter(rName, rValue, rDescription, "", rUnit, "string", &rValue);
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
        mpParameters->addParameter(rName, "true", rDescription, "", rUnit, "bool", &rValue);
    else
        mpParameters->addParameter(rName, "false", rDescription, "", rUnit, "bool", &rValue);
}

void Component::registerParameter(const HString &rName, const HString &rDescription, HTextBlock &rValue)
{
    if (!isNameValid(rName))
    {
        addErrorMessage("Will not register Invalid parameter name: "+rName);
        return;
    }

    if(mpParameters->hasParameter(rName))
        mpParameters->deleteParameter(rName);     //Remove parameter if it is already registered

    mpParameters->addParameter(rName, rValue, rDescription, "", "", "textblock", &rValue);
}
///@}

//! @brief Register a conditional parameter value so that it can be accessed for read and write.
//! @param [in] rName The name of the parameter
//! @param [in] rDescription A description of the parameter
//! @param [in] rConditions The condition descriptions as a vector of text
//! @param [in] rValue A reference to the double variable representing the value, its address will be registered
//! @details This function is used in the constructor of the Component modelling code to register member attributes as HOPSAN parameters
//! @todo Using a reference is not that clear, we should use a ptr instead
void Component::registerConditionalParameter(const HString &rName, const HString &rDescription, std::vector<HString> &rConditions, int &rValue)
{
    if (!isNameValid(rName))
    {
        addErrorMessage("Will not register Invalid parameter name: "+rName);
        return;
    }

    if(mpParameters->hasParameter(rName))
        mpParameters->deleteParameter(rName);     //Remove parameter if it is already registered

    mpParameters->addParameter(rName, to_hstring(rValue), rDescription, "", "", "conditional", &rValue, false, rConditions);
}



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

//! @brief Set whether the component should inherit timestep from its system parent
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

//! @brief Check if component is tagged as experimental
//! @returns true or false
bool Component::isExperimental() const
{
    return false;
}

//! @brief Check if component is tagged as obsolete
//! @returns true or false
bool Component::isObsolete() const
{
    return false;
}

//! @brief Returns a pointer to the simulation time variable in the component
//! @returns pointer to time variable
double *Component::getTimePtr()
{
    return &mTime;
}


//! @brief Adds a port to the component, do not call this function directly unless you have to
//! @param [in] rPortName The desired name of the port (may be automatically changed)
//! @param [in] portType The type of port
//! @param [in] rNodeType The type of node that must be connected to the port
//! @param [in] rDescription A description string describing the port
//! @param [in] reqConnection Specify if the port must be connected or if it is optional
//! @returns A pointer to the created port
Port *Component::addPort(const HString &rPortName, const PortTypesEnumT portType, const HString &rNodeType, const HString &rDescription, const Port::RequireConnectionEnumT reqConnection)
{
    addCoreLogMessage(getName()+"::addPort "+rPortName);

    //Make sure name is unique before insert
    HString newname = this->determineUniquePortName(rPortName);
    //! @todo for ordinary components give an error message, users rarely check debug messages

    Port* pNewPort = createPort(portType, rNodeType, newname, this);
    if (pNewPort)
    {
        //Set whether the port must be connected before simulation
        if (reqConnection == Port::NotRequired)
        {
            //! @todo maybe use a string for OPTIONAL instead, to reduce the number of compile time dependencies, will need to think about that a bit more
            pNewPort->mConnectionRequired = false;
        }

        // Store the port in the port map, for faster port by name lookup
        mPortPtrMap.insert(PortPtrPairT(newname, pNewPort));
        // Store the port in the vector, to remember the order of added ports (useful when retrieving variameters)
        mPortPtrVector.push_back(pNewPort);

        //Signal automatic name change
        if (newname != rPortName)
        {
            addDebugMessage("Automatically changed name of added port from: {" + rPortName + "} to {" + newname + "}");
        }

        pNewPort->setDescription(rDescription);
    }
    else
    {
        addErrorMessage("Could not create port of type: "+portTypeToString(portType));
    }
    return pNewPort;
}

void Component::removePort(const HString &rPortName)
{
    PortPtrMapT::iterator it = mPortPtrMap.find(rPortName);
    Port *pPort = it->second;    //!< @todo dangerous we don't check if we found something
    std::vector<Port *> connectedPorts = pPort->getConnectedPorts();
    for(size_t p=0; p<connectedPorts.size(); ++p)
    {
        mpSystemParent->disconnect(pPort, connectedPorts.at(p));
    }
    mPortPtrMap.erase(rPortName);
    mPortPtrVector.erase(std::remove(mPortPtrVector.begin(), mPortPtrVector.end(), pPort), mPortPtrVector.end());
    mAutoSignalNodeDataPtrPorts.erase(pPort);

    // Unregister all startvalue parameters connected to this port
    pPort->unRegisterStartValueParameters();
//    if(pPort->getStartNodePtr())
//    {
//        for(size_t i=0; i<pPort->getStartNodePtr()->getNumDataVariables(); ++i)
//        {
//            const NodeDataDescription* pDesc = pPort->getStartNodePtr()->getDataDescription(0);
//            const HString name = getName()+"#"+pDesc->name;
//            unRegisterParameter(name);
//        }
//    }

    //! @todo Deleting port like this seem to cause cowboy-writing. Figure out why...
    //! @todo memory leak??? !
    //delete pPort;
}

//! @brief Add a PowerPort with description to the component
//! @ingroup ComponentSetupFunctions
//! @param [in] rPortName The desired name of the port (may be automatically changed)
//! @param [in] rNodeType The type of node that must be connected to the port
//! @param [in] rDescription The port description
//! @param [in] reqConnect Specify if the port must be connected or if it is optional (Required or NotRequired)
//! @return A pointer to the created port
Port *Component::addPowerPort(const HString &rPortName, const HString &rNodeType, const HString &rDescription, const Port::RequireConnectionEnumT reqConnect)
{
    return addPort(rPortName, PowerPortType, rNodeType, rDescription, reqConnect);
}


//! @brief Add a PowerMultiPort with description  to the component
//! @ingroup ComponentSetupFunctions
//! @param [in] rPortName The desired name of the port (may be automatically changed)
//! @param [in] rNodeType The type of node that must be connected to the port
//! @param [in] rDescription The port description
//! @param [in] reqConnect Specify if the port must be connected or if it is optional (Required or NotRequired)
//! @return A pointer to the created port
Port *Component::addPowerMultiPort(const HString &rPortName, const HString &rNodeType, const HString &rDescription, const Port::RequireConnectionEnumT reqConnect)
{
    return addPort(rPortName, PowerMultiportType, rNodeType, rDescription, reqConnect);
}


//! @brief Add a ReadMultiPort with description to the component
//! @ingroup ComponentSetupFunctions
//! @param [in] rPortName The desired name of the port (may be automatically changed)
//! @param [in] rNodeType The type of node that must be connected to the port
//! @param [in] rDescription The port description
//! @param [in] reqConnect Specify if the port must be connected or if it is optional (Required or NotRequired)
//! @return A pointer to the created port
Port *Component::addReadMultiPort(const HString &rPortName, const HString &rNodeType, const HString &rDescription, const Port::RequireConnectionEnumT reqConnect)
{
    return addPort(rPortName, ReadMultiportType, rNodeType, rDescription, reqConnect);
}


//! @brief Add a ReadPort with description to the component
//! @note Usually you should use addInputVariable instead of this one
//! @ingroup ComponentSetupFunctions
//! @param [in] rPortName The desired name of the port (may be automatically changed)
//! @param [in] rNodeType The type of node that must be connected to the port
//! @param [in] rDescription The port description
//! @param [in] reqConnect Specify if the port must be connected or if it is optional (Required or NotRequired)
//! @return A pointer to the created port
Port *Component::addReadPort(const HString &rPortName, const HString &rNodeType, const HString &rDescription, const Port::RequireConnectionEnumT reqConnect)
{
    return addPort(rPortName, ReadPortType, rNodeType, rDescription, reqConnect);
}


//! @brief Add a WritePort with description to the component
//! @note Usually you should use addOutputVariable instead of this one unless you need a "sniffer port"
//! @ingroup ComponentSetupFunctions
//! @param [in] rPortName The desired name of the port (may be automatically changed)
//! @param [in] rNodeType The type of node that must be connected to the port
//! @param [in] rDescription The port description
//! @param [in] reqConnect Specify if the port must be connected or if it is optional (Required or NotRequired)
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

        // Unregister any start value parameters connected to this port
        pPort->unRegisterStartValueParameters();

        // delete the port
        delete pPort;
    }
    else
    {
        addWarningMessage("Trying to delete port {" + rName + "}, but not found");
    }
}


//! @brief Get a pointer to the node data variable, (Port pointer version)
//! @ingroup ComponentSimulationFunctions
//! @note This function is slow, you should not run it during simulation
//! @details The safe in this version means that a dummy pointer will be returned if the desired one was not found,
//! this prevents crash and the need to check that return pointer is ok, giving cleaner component code
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
        addErrorMessage("Data pointer could not be retrieved in getSafeNodeDataPtr(), Requested dataId: "+to_hstring(dataId));
        stopSimulation();
        // Return pointer to dummy data
        pData = &dummyDouble;
    }
    return pData;
}

//! @brief Get a pointer to the node data variable, (Port name version)
//! @ingroup ComponentSimulationFunctions
//! @note This function is slow, you should not run it during simulation
//! @details The safe in this version means that a dummy pointer will be returned if the desired one was not found,
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
    //addCoreLogMessage(getName()+"::getNodeDataPtr Id:"+to_hstring(dataId));
    //If this is one of the multiports then give an error message to the user so that they KNOW that they have made a mistake
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
    //If this is not a multiport then give an error message to the user so that they KNOW that they have made a mistake
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
//! @param[in] defaultValue The initial value to set for this node data
//! @returns A pointer to the specified NodeData or a pointer to dummy NodeData
double *Component::getSafeMultiPortNodeDataPtr(Port* pPort, const size_t portIdx, const int dataId, const double defaultValue)
{
    double* pData = getSafeMultiPortNodeDataPtr(pPort, portIdx, dataId);
    *pData = defaultValue; // Set desired initial value
    return pData;
}


//! @brief Read value based on the port and node data name
//! @note This functions is slow but safe, do not use it during simulation
//! @details It searches for data based on strings, this make it unsuitable for use during simulation but it is suitable during initialize when port pointers are not desired/available
//! This function will also check so that the desired data actually exist in the requested node, error message will be sent if it does not
//! @ingroup ComponentSimulationFunctions
//! @param [in] rPortName The port to write data to
//! @param [in] rDataName The data variable name for the data to be written
//! @param [in] subPortIdx The index of a multiport subport
//! @returns The value from the requested PortName#DataName or -1 if failure, Note! an error message will also be shown
double Component::readNodeSafe(const HString &rPortName, const HString &rDataName, const size_t subPortIdx)
{
    Port *pPort = getPort(rPortName);
    if (pPort)
    {
        int id = pPort->getNodeDataIdFromName(rDataName);
        if (id > 0)
        {
            return pPort->readNodeSafe(id, subPortIdx);
        }
        addErrorMessage("You are trying to get a dataName: "+rDataName+" that does not exist in port: "+rPortName);
        return -1;
    }
    addErrorMessage("You are trying to access port: "+rPortName+" that does not exist");
    return -1;
}

//! @brief Read value based on the port name and node data id
//! @note This functions is slow but safe, do not use it during simulation
//! @details It searches for data based on strings, this make it unsuitable for use during simulation but it is suitable during initialize when port pointers are not desired/available
//! This function will only check if node data id is within range in the node, not if the data actually exist
//! @ingroup ComponentSimulationFunctions
//! @param [in] rPortName The port to write data to
//! @param [in] dataId The data variable id for the data to be written
//! @param [in] subPortIdx The index of a multiport subport
//! @returns The value from the requested PortName#DataId or -1 if failure, Note! an error message will also be shown
double Component::readNodeSafe(const HString &rPortName, const size_t dataId, const size_t subPortIdx)
{
    Port *pPort = getPort(rPortName);
    if (pPort)
    {
        return pPort->readNodeSafe(dataId, subPortIdx);
    }
    addErrorMessage("You are trying to access port: "+rPortName+" that does not exist");
    return -1;
}


//! @brief Write node data based on port and data name
//! @note This functions is slow but safe, do not use it during simulation
//! @details It searches for data based on strings, this make it unsuitable for use during simulation but it is suitable during initialize when port pointers are not desired/available
//! This function will also check so that the desired data actually exist in the requested node, error message will be sent if it does not
//! @ingroup ComponentSimulationFunctions
//! @param [in] rPortName The port to write data to
//! @param [in] rDataName The data variable name for the data to be written
//! @param [in] value The value to write
//! @param [in] subPortIdx The index of a multiport subport
void Component::writeNodeSafe(const HString &rPortName, const HString &rDataName, const double value, const size_t subPortIdx)
{
    Port *pPort = getPort(rPortName);
    if (pPort)
    {
        int id = pPort->getNodeDataIdFromName(rDataName);
        if (id >= 0)
        {
            if (pPort->isMultiPort())
            {
                //! @todo We wont be able to overwrite read sub ports here, we "solved" that for some reason before in the code below
                pPort->writeNodeSafe(id, value, subPortIdx);
            }
            else
            {
                // We want to make sure we use the base class Port::writeNodeSafe function to force writing the value even if this is a ReadPort
                pPort->Port::writeNodeSafe(id, value, subPortIdx);
                return;
            }
        }
        addErrorMessage("You are trying to set value for dataName: "+rDataName+" that does not exist in port: "+rPortName);
        return;
    }
    addErrorMessage("You are trying to access port: "+rPortName+" that does not exist");
    return;
}

//! @brief Write node data based on port and data id
//! @note This functions is slow but safe, do not use it during simulation
//! @details It searches for data based on strings, this make it unsuitable for use during simulation but it is suitable during initialize when port pointers are not desired/available
//! This function will only check if node data id is within range in the node, not if the data actually exist
//! @ingroup ComponentSimulationFunctions
//! @param [in] rPortName The port to write data to
//! @param [in] dataId The data variable id for the data to be written
//! @param [in] value The value to write
//! @param [in] subPortIdx The index of a multiport subport
void Component::writeNodeSafe(const HString &rPortName, const size_t dataId, const double value, const size_t subPortIdx)
{
    Port *pPort = getPort(rPortName);
    if (pPort)
    {
        if (pPort->isMultiPort())
        {
            //! @todo We wont be able to overwrite read sub ports here, we "solved" that for some reason before in the code below
            pPort->writeNodeSafe(dataId, value, subPortIdx);
        }
        else
        {
            // We want to make sure we use the base class Port::writeNodeSafe function to force writing the value even if this is a ReadPort
            pPort->Port::writeNodeSafe(dataId, value, subPortIdx);
            return;
        }
    }
    addErrorMessage("You are trying to access port: "+rPortName+" that does not exist");
    return;
}


//! @brief a virtual function that determines a unique port name, needs to be overloaded in ComponentSystem to do this slightly different
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

//! @brief This is supposed to be used by hopsan essentials to set the typename to the same as the registered key value
void Component::setTypeName(const HString &rTypeName)
{
    mTypeName = rTypeName;
}


//! @todo Maybe not have this function, solve in some other nicer way
vector<Port*> Component::getPortPtrVector() const
{
    vector<Port*> vec;
    //Copy every port pointer
    PortPtrMapT::const_iterator ports_it;
    for (ports_it = mPortPtrMap.begin(); ports_it != mPortPtrMap.end(); ++ports_it)
    {
        vec.push_back(ports_it->second);
    }
    return vec;
}

//! @brief Returns a pointer to the port with the given name.
//! @param[in] rPortname The name of the port
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
//! @param [in] rQuantityOrUnit The quantity or unit of the variable value
//! @param [in] defaultValue The default variable value (if not connected)
//! @param [in,out] ppNodeData Optional pointer to pointer to data. The data pointer will be registered and automatically assigned before initialisation)
//! @returns A pointer to the port created.
//! @ingroup ComponentSetupFunctions
Port *Component::addInputVariable(const HString &rName, const HString &rDescription, const HString &rQuantityOrUnit, const double defaultValue, double **ppNodeData)
{
    //! @todo support more types
    Port *pPort = addReadPort(rName,"NodeSignal", rDescription, Port::NotRequired);
    if (rQuantityOrUnit.empty())
    {
        pPort->setSignalNodeQuantityModifyable(true);
    }
    else
    {
        pPort->setSignalNodeQuantityModifyable(true);
        pPort->setSignalNodeQuantityOrUnit(rQuantityOrUnit);
        pPort->setSignalNodeQuantityModifyable(false);
    }
    pPort->registerStartValueParameters(); // Reregister after unit has been changed
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
//! @param [in] rQuantityOrUnit The quantity or unit of the variable value
//! @param [in,out] ppNodeData Optional pointer to pointer to data. The data pointer will be registered and automatically assigned before initialisation)
//! @returns A pointer to the port created.
//! @ingroup ComponentSetupFunctions
Port *Component::addOutputVariable(const HString &rName, const HString &rDescription, const HString &rQuantityOrUnit, double **ppNodeData)
{
    Port *pPort = addWritePort(rName, "NodeSignal", rDescription, Port::NotRequired);
    if (rQuantityOrUnit.empty())
    {
        pPort->setSignalNodeQuantityModifyable(true);
    }
    else
    {
        pPort->setSignalNodeQuantityModifyable(true);
        pPort->setSignalNodeQuantityOrUnit(rQuantityOrUnit);
        pPort->setSignalNodeQuantityModifyable(false);
    }
    pPort->registerStartValueParameters(); // Reregister after unit has been changed
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
//! @param [in] rQuantityOrUnit The quantity or unit of the variable value
//! @param [in] defaultValue The default variable value (if not connected)
//! @param [in,out] ppNodeData Optional pointer to pointer to data. The data pointer will be registered and automatically assigned before initialisation)
//! @returns A pointer to the port created.
//! @ingroup ComponentSetupFunctions
Port *Component::addOutputVariable(const HString &rName, const HString &rDescription, const HString &rQuantityOrUnit, const double defaultValue, double **ppNodeData)
{
    Port *pPort = addWritePort(rName, "NodeSignal", rDescription, Port::NotRequired);
    if (rQuantityOrUnit.empty())
    {
        pPort->setSignalNodeQuantityModifyable(true);
    }
    else
    {
        pPort->setSignalNodeQuantityModifyable(true);
        pPort->setSignalNodeQuantityOrUnit(rQuantityOrUnit);
        pPort->setSignalNodeQuantityModifyable(false);
    }
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
    addCoreLogMessage("Fatal error: "+rMessage+" in component: "+getName());
    if (mpMessageHandler)
    {
        mpMessageHandler->addFatalMessage(getName()+"::"+rMessage, rTag);
    }
}


//! @brief Get the an actual start value of a port
//! @param[in] pPort is the port which should be read from
//! @param[in] idx is the index of the start value e.g. NodeHydraulic::Pressure
//! @param[in] portIdx The index of a subport in a multiport. If pPort is not a multiport this value will be ignored
//! @returns The default start value
//! @ingroup ComponentSetupFunctions
double Component::getDefaultStartValue(Port* pPort, const size_t idx, const size_t portIdx)
{
    return pPort->getStartValue(idx, portIdx);
}

//! @brief Get the an actual start value of a port
//! @param[in] rPortName is the port which should be read from
//! @param[in] rDataName The name of the data in the port to read from example: "Pressure"
//! @param[in] portIdx The index of a subport in a multiport. If pPort is not a multiport this value will be ignored
//! @returns The default start value
//! @ingroup ComponentSetupFunctions
//! @details This slower version uses string names for lookup, and will report errors if names are incorrect
double Component::getDefaultStartValue(const HString &rPortName, const HString &rDataName, const size_t portIdx)
{
    Port *pPort = getPort(rPortName);
    if (pPort)
    {
        if (pPort->getNodePtr())
        {
            int id = pPort->getNodePtr()->getDataIdFromName(rDataName);
            if (id >= 0)
            {
                return getDefaultStartValue(pPort, id, portIdx);
            }
            else
            {
                addErrorMessage("setDefaultStartValue(): Data named '"+rDataName+"' was not found in Port '"+rPortName+"'!");
            }
        }
    }
    else
    {
        addErrorMessage("setDefaultStartValue(): Port '"+rPortName+"' not found!");
    }
    return -1;
}


//! @brief Set the default startvalue in a port
//! @param [in] pPort is the port which should be written to
//! @param [in] idx is the index of the start value e.g. NodeHydraulic::Pressure
//! @param [in] value is the start value that should be written
//! @ingroup ComponentSetupFunctions
void Component::setDefaultStartValue(Port *pPort, const size_t idx, const double value)
{
    pPort->setDefaultStartValue(idx, value);
    // If a description exist, then refresh the value text
    if (pPort->getNodeDataDescription(idx))
    {
        mpParameters->refreshParameterValueText(pPort->getName()+"#"+pPort->getNodeDataDescription(idx)->name);
    }
}

//! @brief Set the default startvalue in a port
//! @param[in] rPortName The name of the port that should be written to
//! @param[in] rDataName The port variable to be written to, Example: "Pressure"
//! @param[in] value is the start value that should be written
//! @ingroup ComponentSetupFunctions
//! @details This slower version uses string names for lookup, and will report errors if names are incorrect
void Component::setDefaultStartValue(const HString &rPortName, const HString &rDataName, const double value)
{
    Port *pPort = getPort(rPortName);
    if (pPort)
    {
        if (pPort->getNodePtr())
        {
            int id = pPort->getNodePtr()->getDataIdFromName(rDataName);
            if (id >= 0)
            {
                setDefaultStartValue(pPort, id, value);
            }
            else
            {
                addErrorMessage("setDefaultStartValue(): Data named '"+rDataName+"' was not found in Port '"+rPortName+"'!");
            }
        }
    }
    else
    {
        addErrorMessage("setDefaultStartValue(): Port '"+rPortName+"' not found!");
    }
}


//! @brief Disable a start value to prevent the user from setting it, this is usefully if you calculate the value internally
//! @param [in] pPort Pointer to the port to disable start value on
//! @param [in] idx The node data index to disable start value for
void Component::disableStartValue(Port *pPort, const size_t idx)
{
    pPort->disableStartValue(idx);
}

//! @brief Disable a start value to prevent the user from setting it, this is usefully if you calculate the value internally
//! @param [in] rPortName The name of the port to disable start value on
//! @param [in] idx The node data index to disable start value for
//! @details If the given port name is incorrect the an error message will be added
void Component::disableStartValue(const HString &rPortName, const size_t idx)
{
    Port *pPort = getPort(rPortName);
    if (pPort)
    {
        disableStartValue(pPort, idx);
    }
    else
    {
        addErrorMessage("disableStartValue(): Port '"+rPortName+"' not found!");
    }
}

//! @brief Disable a start value to prevent the user from setting it, this is usefully if you calculate the value internally
//! @param [in] rPortName The name of the port to disable start value on
//! @param [in] rDataName The name of the node data to disable start value for
//! @details If the given port or node data name is incorrect the an error message will be added
void Component::disableStartValue(const HString &rPortName, const HString &rDataName)
{
    Port *pPort = getPort(rPortName);
    if (pPort && pPort->getNodePtr())
    {
        int id = pPort->getNodePtr()->getDataIdFromName(rDataName);
        if (id >= 0)
        {
            disableStartValue(pPort, id);
        }
        else
        {
            addErrorMessage("disableStartValue(): Data named '"+rDataName+"' was not found in Port '"+rPortName+"'!");
        }
    }
    else
    {
        addErrorMessage("disableStartValue(): Port '"+rPortName+"' not found!");
    }
}


ComponentSystem *Component::getSystemParent() const
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

size_t Component::getModelHierarchyDepth() const
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

//! @brief Configures a component by setting up ports, variables, constants and other resources
//! @details Every function must overload this function, in this function ports variables and constants will be added
//! The component author is however free to add any other desired code as well.
//! This function is called immediately after a component instance has been created
//! @ingroup ComponentSetup
void Component::configure()
{
    // This function ust be overloaded in every component
    addErrorMessage("You must overload the configure() function in Component: " + mTypeName);
}

//! @brief Deconfigure a component, use this to cleanup and memory/resource allocations you have made in configure
//! @details This function can be optionally overloaded if it is needed
//! You can use it to free memory or other resources that you have created in configure
//! This function is the last one called before a component instance is deleted (destructor called)
//! @ingroup ComponentSetup
void Component::deconfigure()
{
    // This function can be overloaded in every component
    // Does nothing by default
}

//! @brief This function can be used to automate things prior to component initialization, only use this if you know what you are doing
//! @details One example of what you can do, is reconnecting internal connections in programmed subsystems
//! @returns True or False to signal success or failure
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

//! @todo this function is strange, need to find a better name and make sure that this really works and is really the thing you want to do
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
//! This makes it possible to avoid absolute paths for external file resources
//! @param rFileName the name of the file to search for
//! @return full file name path, empty string if it does not exists
//! @ingroup ComponentSetupFunctions
HString Component::findFilePath(const HString &rFileName) const
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

std::vector<HString> Component::getSearchPaths() const
{
    return mSearchPaths;
}

void Component::reInitializeValuesFromNodes()
{
    addErrorMessage("reInitializeValuesFromNodes() is not implemented in component.");
    stopSimulation();

    return;
}

void Component::solveSystem()
{
    addErrorMessage("solveSystem() is not implemented in component.");
    stopSimulation();

    return;
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

