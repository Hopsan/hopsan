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

//! @class hopsan::Parameter
//! @brief The Parameter class implements the parameter used in the container class Parameters
//!
//! The parameter is implemented with a name, a value string and a data pointer that can be of various types.
//! Both Compontents and Systems use the same paramter implementation. In the case of a system parameter the
//! data pointer is 0.

//! @brief Constructor
//! @param [in] parameterName The desired parameter name, e.g. m
//! @param [in] parameterValue The value of the parameter, always a string
//! @param [in] description The description of the parameter e.g. Mass
//! @param [in] unit The physical unit of the parameter e.g. kg
//! @param [in] type The type of the parameter e.g. double
//! @param [in] pDataPtr Only used by Components, system parameters don't use this, default: 0
//! @param [in] pParentParameters A pointer to the Parameters object that contains the Parameter
Parameter::Parameter(std::string parameterName, std::string parameterValue, std::string description, std::string unit, std::string type, void* pDataPtr, Parameters* pParentParameters)
{
    mParameterName = parameterName;
    mParameterValue = parameterValue;
    mDescription = description;
    mUnit = unit;
    std::string tmp = type;
    if((type=="double") || (type=="integer") || (type=="bool") || (type=="string"))
    {
        mType = type;
    }
    else
    {
        assert(false);
    }
    mpData = pDataPtr;
    mpParentParameters = pParentParameters;
    evaluate();
}


//! @brief Read out the properties of the parameter
//! @param [out] rParameterName The parameter name, e.g. m
//! @param [out] rParameterValue The value of the parameter, e.g. 13
//! @param [out] rDescription The description of the parameter e.g. Mass
//! @param [out] rUnit The physical unit of the parameter e.g. kg
//! @param [out] rType The type of the parameter e.g. double
//!
//! This function is used by Parameters
void Parameter::getParameter(std::string &rParameterName, std::string &rParameterValue, std::string &rDescription, std::string &rUnit, std::string &rType)
{
    rParameterName = mParameterName;
    rParameterValue = mParameterValue;
    rDescription = mDescription;
    rUnit = mUnit;
    rType = mType;
}


bool Parameter::setParameter(std::string parameterValue, std::string description, std::string unit, std::string type, Parameter **pNeedEvaluation, bool force)
{
    bool success;
    std::string oldValue = mParameterValue;
    std::string oldDescription = mDescription;
    std::string oldUnit = mUnit;
    std::string oldType = mType;
    if(!description.empty())
    {
        mDescription = description;
    }
    if(!unit.empty())
    {
        mUnit = unit;
    }
    if(!type.empty())
    {
        mType = type;
    }
    success = setParameterValue(parameterValue, pNeedEvaluation);
    if((force) && !(success))
    {
        *pNeedEvaluation = this;
        mParameterValue = parameterValue;
    }
    else if(!success)
    {
        mParameterValue = oldValue;
        mDescription = oldDescription;
        mUnit = oldUnit;
        mType = oldType;
    }
    return success;
}

//! @brief Set the parameter value for an exsisting parameter
//! @param [in] value The new value for the parameter
//! @param [out] pNeedEvaluation Tell is the parameter needs evaluation, e.g. is a system parameter or an expression
//! @return true if success, otherwise false
//!
//! This function is used by Parameters
bool Parameter::setParameterValue(const std::string value, Parameter **pNeedEvaluation)
{
    bool success=false;
    if(!(mParameterName==value))
    {
        std::string oldValue = mParameterValue;
        mParameterValue = value;
        std::string evalResult = value;
        success = evaluate(evalResult);
        if(!success)
        {
            mParameterValue = oldValue;
        }
        if(value != evalResult)
        {
            *pNeedEvaluation = this;
        }
        else
        {
            *pNeedEvaluation = 0;
        }
    }
    return success;
}


//! @brief Returns the type of the parameter
//! @return The type of the parameter
std::string Parameter::getType()
{
    return mType;
}


//! @brief Evaluate the parameter
//! @return true if success, otherwise false
//!
//! This function is used by Parameters. The point with run this function is
//! to write the right value to the mData pointer.
//! @see evaluate(std::string &result)
bool Parameter::evaluate()
{
    std::string dummy;
    return evaluate(dummy);
}


//! @brief Evaluate the parameter
//! @param [out] rResult The result of the evaluation
//! @return true if success, otherwise false
//!
//! This function is used by Parameters
//! @see evaluate()
bool Parameter::evaluate(std::string &rResult)
{
    bool success = true;
    std::string evaluatedParameterValue;
    if(!(mpParentParameters->evaluateParameter(mParameterValue, evaluatedParameterValue, mType)))
    {
        evaluatedParameterValue = mParameterValue;
    }

    if(mType=="double")
    {
        double tmpParameterValue;
        istringstream is(evaluatedParameterValue);
        if(is >> tmpParameterValue)
        {
            if(mpData)
            {
                double* apa = static_cast<double*> (mpData);
                *apa = tmpParameterValue;
            }
        }
        else
        {
            success = false;
        }
    }
    else if(mType=="integer")
    {
        int tmpParameterValue;
        istringstream is(evaluatedParameterValue);
        if(is >> tmpParameterValue)
        {
            if(mpData)
            {
                int* apa = static_cast<int*> (mpData);
                *apa = tmpParameterValue;
            }
        }
        else
        {
            success = false;
        }
    }
    else if(mType=="bool")
    {
        bool tmpParameterValue;
        istringstream is(evaluatedParameterValue);
        if(is >> tmpParameterValue)
        {
            if(mpData)
            {
                bool* apa = static_cast<bool*> (mpData);
                *apa = tmpParameterValue;
            }
        }
        else if((evaluatedParameterValue == "false") || (evaluatedParameterValue == "0"))
        {
            if(mpData)
            {
                bool* apa = static_cast<bool*> (mpData);
                *apa = false;
            }
        }
        else if((evaluatedParameterValue == "true") || (evaluatedParameterValue == "1"))
        {
            if(mpData)
            {
                bool* apa = static_cast<bool*> (mpData);
                *apa = true;
            }
        }
        else
        {
            success = false;
        }
    }
    else if(mType=="string")
    {
        if(mpData)
        {
            string* apa = static_cast<string*> (mpData);
            *apa = evaluatedParameterValue;
        }
    }
    else
    {
        success = false;
    }

    rResult = evaluatedParameterValue;
    return success;
}

//! @class hopsan::Parameters
//! @brief The Parameters class implements the parameters used in both Componenets and ComponentSystems
//!


//! @brief Constructor
//!
//! @param pParentComponent A pointer to the Component that contains the Parameters
Parameters::Parameters(Component* pParentComponent)
{
    mParentComponent = pParentComponent;
}


//! @brief Add a new parameter
//! @param [in] parameterName The desired parameter name, e.g. m
//! @param [in] parameterValue The value of the parameter, always a string
//! @param [in] description The description of the parameter e.g. Mass, default: ""
//! @param [in] unit The physical unit of the parameter e.g. kg, default: "0""
//! @param [in] type The type of the parameter e.g. double, default: ""
//! @param [in] pDataPtr Only used by Components, system parameters don't use this, default: 0
//! @return true if success, otherwise false
bool Parameters::addParameter(std::string parameterName, std::string parameterValue, std::string description, std::string unit, std::string type, void* dataPtr)
{
    bool success = true;
    istringstream is(parameterValue);
    //Type not decided, to be done here
    if((type.empty()) || (0 == type.compare("double")))
    {
        double tmpDouble;
        if(is >> tmpDouble)
        {
            type = "double";
            success *= true;
        }
        else
        {
            success *= false;
        }
    }
    else if(0 == type.compare("bool"))
    {
        bool tmpBool;
        if((is >> tmpBool) || (parameterValue.compare("false")) == 0 || (parameterValue.compare("true") == 0)
                           || (parameterValue.compare("0"))     == 0 || (parameterValue.compare("1")    == 0))
        {
            success *= true;
        }
        else
        {
            success *= false;
        }
    }
    else if(0 == type.compare("integer"))
    {
        int tmpInt;
        if(is >> tmpInt)
        {
            success *= true;
        }
        else
        {
            success *= false;
        }
    }
    else if(0 == type.compare("string"))
    {
        std::string tmpStr;
        if(is >> tmpStr)
        {
            success *= true;
        }
        else
        {
            success *= false;
        }
    }
    else
    {
        success *= false;
    }

    if(success)
    {
        Parameter* newParameter = new Parameter(parameterName, parameterValue, description, unit, type, dataPtr, this);
        mParameters.push_back(newParameter);
    }

    return success;
}


//! @brief Deletes a parameter
//!
//! @param parameterName The name of the paramter to delete
void Parameters::deleteParameter(std::string parameterName)
{
    std::string name, value, description, unit, type;

    std::vector<Parameter*>::iterator parIt;
    for(parIt = mParameters.begin(); (parIt != mParameters.end()) && (!mParameters.empty()); ++parIt)
    {
        (*parIt)->getParameter(name, value, description, unit, type);
        if(parameterName == name)
        {
            mParameters.erase(parIt);
            //delete (*parIt); //Kolla på detta! FIXA!
            //++parIt;
            return;     //We can return now, since there should never be multiple parameters with same name
        }
    }
}


//! @brief Read out all parameters
//! @param [out] rParameterNames The parameter names
//! @param [out] rParameterValues The value of the parameters
//! @param [out] rDescriptions The description of the parameters
//! @param [out] rUnits The physical unit of the parameters
//! @param [out] rTypes The type of the parameters
void Parameters::getParameters(std::vector<std::string> &rParameterNames, std::vector<std::string> &rParameterValues, std::vector<std::string> &rDescriptions, std::vector<std::string> &rUnits, vector<std::string> &rTypes)
{
    rParameterNames.resize(mParameters.size());
    rParameterValues.resize(mParameters.size());
    rDescriptions.resize(mParameters.size());
    rUnits.resize(mParameters.size());
    rTypes.resize(mParameters.size());
    for(size_t i = 0; i < mParameters.size(); ++i)
    {
        mParameters[i]->getParameter(rParameterNames[i], rParameterValues[i], rDescriptions[i], rUnits[i], rTypes[i]);
    }
}


bool Parameters::setParameter(std::string name, std::string value, std::string description, std::string unit, std::string type, bool force)
{
    bool success = false;
    std::string parameterName, parameterValue, parameterDescription, parameterUnit, parameterType;
    for(size_t i=0; i<mParameters.size(); ++i) //Find the parameter among the excisting parameters
    {
        mParameters[i]->getParameter(parameterName, parameterValue, parameterDescription, parameterUnit, parameterType);
        if((name == parameterName) && (value != parameterName)) //Found! (It cannot find itself)
        {
            Parameter *needEvaluation=0;
            success = mParameters[i]->setParameter(value, description, unit, type, &needEvaluation, force); //Sets the new value, if the parameter is of the type to need evaluation e.g. if it is a system parameter needEvaluation points to the parameter
            if(needEvaluation)
            {
                if(mParametersNeedEvaluation.end() == find(mParametersNeedEvaluation.begin(), mParametersNeedEvaluation.end(), needEvaluation))
                {
                    mParametersNeedEvaluation.push_back(needEvaluation); //The parameter needs evaluation and is not already stored
                }
            }
            else //mParameters[i] don't need evaluation, this loop erases it from mParametersNeedEvaluation
            {
                std::vector<Parameter*>::iterator parIt = mParametersNeedEvaluation.begin();
                while( parIt != mParametersNeedEvaluation.end() )
                {
                    if(*parIt == mParameters[i])
                    {
                        parIt = mParametersNeedEvaluation.erase(parIt);
                    }
                    else
                    {
                        (*parIt)->getParameter(parameterName, parameterValue, parameterDescription, parameterUnit, parameterType);//debug
                        cout << parameterName << endl;//debug
                        ++parIt;
                    }
                }

            }
        }
    }
    return success;
}

//! @brief Set the parameter value for an exsisting parameter
//! @param [in] name The name of the parameter to be set
//! @param [in] value The new value for the parameter
//! @return true if success, otherwise false
bool Parameters::setParameterValue(const std::string name, const std::string value, bool force)
{
    return setParameter(name, value, "", "", "", force);
}


//! @brief Evaluate a specific parameter
//! @param [in] parameterName The name of the parameter to be evaluated
//! @param [out] rEvaluatedParameterValue The result of the evaluation
//! @param [in] type The type of how the parameter should be interpreted
//! @return true if success, otherwise false
bool Parameters::evaluateParameter(const std::string parameterName, std::string &rEvaluatedParameterValue, const std::string type)
{
    bool success = false;
    std::string parameterName2, parameterValue2, description2, unit2, type2;
    for(size_t i = 0; i < mParameters.size(); ++i)
    {
        mParameters[i]->getParameter(parameterName2, parameterValue2, description2, unit2, type2);
        if((parameterName == parameterName2) && (mParameters[i]->getType() == type))
        {
            success = mParameters[i]->evaluate(rEvaluatedParameterValue);
        }
    }
    if(!success)
    {
        if(mParentComponent)
        {
            if(mParentComponent->getSystemParent())
            {
                success = mParentComponent->getSystemParent()->getSystemParameters().evaluateParameter(parameterName, parameterValue2, type);
                rEvaluatedParameterValue = parameterValue2;
            }
        }
    }
    return success;
}


//! @brief Evaluate all parameters
//! @return true if success, otherwise false
bool Parameters::evaluateParameters()
{
    bool success = true;
    for(size_t i=0; i<mParameters.size(); ++i)
    {
        success *= mParameters[i]->evaluate();
    }
    return success;
}


//! @brief Check all parameters that need evaluation are able to be evaluated
//! @param [out] errParName Tell which parameter that can't be evaluated is not successful
//! @return true if success, otherwise false
//!
//! Check all parameters that need evaluation are able to be evaluated. The function will
//! stop as soon as one parameter turns out to be faulty. So in the case of many bad parameters
//! only the name of the first one is returned.
bool Parameters::checkParameters(std::string &errParName)
{
    bool success = true;
    std::vector<Parameter*>::iterator parIt;
    for(parIt = mParametersNeedEvaluation.begin(); (parIt != mParametersNeedEvaluation.end()) && (!mParametersNeedEvaluation.empty()); ++parIt)
    {
        success *= (*parIt)->evaluate();
        if(!(success))
        {
            std::string parameterName, parameterValue, description, unit, type;
            (*parIt)->getParameter(parameterName, parameterValue, description, unit, type);
            errParName = parameterName;
            break;
        }
    }
    return success;
}


//Constructor
Component::Component(string name)
{
    mName = name;
    mTimestep = 0.001;

    mIsComponentSystem = false;
    mTypeCQS = Component::UNDEFINEDCQSTYPE;

    mpSystemParent = 0;
    mModelHierarchyDepth = 0;

    mParameters = new Parameters(this);

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


void Component::getParameters(vector<string> &parameterNames, vector<string> &parameterValues, vector<string> &descriptions, vector<string> &units, vector<string> &types)
{
    mParameters->getParameters(parameterNames, parameterValues, descriptions, units, types);
}


bool Component::setParameterValue(const std::string name, const std::string value, bool force)
{
    return mParameters->setParameterValue(name, value, force);
}


void Component::updateParameters()
{
    mParameters->evaluateParameters();
}


bool Component::checkParameters(std::string &errParName)
{
    return mParameters->checkParameters(errParName);
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

////! @brief A finilize method that contains stuff that the user should not need to care about
////! @todo OK I admit, the name is kind of bad
//void Component::secretFinalize()
//{
//    //delete any created dummy node data variables created and then clear the pointer storage vector
//    for (size_t i=0; i<mDummyNDptrs.size(); ++i)
//    {
//        delete mDummyNDptrs[i];
//    }
//    mDummyNDptrs.clear();
//}


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
    return "";           //Needed for VC compilations
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
    #ifdef USETBB
    mpSystemParent->mpStopMutex->lock();
    #endif
    this->getSystemParent()->stop();
    #ifdef USETBB
    mpSystemParent->mpStopMutex->unlock();
    #endif
}


//! Register a parameter value so that it can be accessed for read and write. Set a Name, Description and Unit.
//! This function is used in the constructor of the Component modelling code to register member attributes as HOPSAN parameters
void Component::registerParameter(const string name, const string description, const string unit, double &rValue)
{
    stringstream ss;
    if(ss << rValue)
    {
        mParameters->addParameter(name, ss.str(), description, unit, "double", &rValue);
    }
    else
    {
        assert(false);
    }
}


//! Register a parameter value so that it can be accessed for read and write. Set a Name, Description and Unit.
//! This function is used in the constructor of the Component modelling code to register member attributes as HOPSAN parameters
void Component::registerParameter(const string name, const string description, const string unit, string &rValue)
{
    mParameters->addParameter(name, rValue, description, unit, "string", &rValue);
}


//! Register a parameter value so that it can be accessed for read and write. Set a Name, Description and Unit.
//! This function is used in the constructor of the Component modelling code to register member attributes as HOPSAN parameters
void Component::registerParameter(const string name, const string description, const string unit, bool &rValue)
{
    if(rValue)
        mParameters->addParameter(name, "true", description, unit, "bool", &rValue);
    else
        mParameters->addParameter(name, "false", description, unit, "bool", &rValue);
}


//! @brief Removes a parameter value from the component
void Component::unRegisterParameter(const string name)
{
    mParameters->deleteParameter(name);
}


double Component::getDefaultParameterValue(const string name)
{
    return mDefaultParameters.find(name)->second;
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


bool Component::doesInheritTimestep()
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


bool Component::isComponentC()
{
    return mTypeCQS == C;
}


bool Component::isComponentQ()
{
    return mTypeCQS == Q;
}


bool Component::isComponentSystem()
{
    return mIsComponentSystem;
}


bool Component::isComponentSignal()
{
    return mTypeCQS == S;
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

//    double *pND;
//    //! @todo Maybe we should somehow try to use the startvalue as default instead somehow, need to think about this, seems like double work right now
//    if(pPort->isConnected())
//    {
//        pND = pPort->getNodeDataPtr(dataId, portIdx);
//    }
//    else
//    {
//        pND = new double(defaultValue);
//        mDummyNDptrs.push_back(pND); //Store the pointer to dummy for automatic finilize removal
//    }
//    return pND;
    return pPort->getSafeNodeDataPtr(dataId, defaultValue, portIdx);
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


//constructor ComponentSignal
ComponentSignal::ComponentSignal(string name) : Component(name)
{
    mTypeCQS = Component::S;
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

    delete mParameters;
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
}


//Constructor ComponentQ
ComponentQ::ComponentQ(string name) : Component(name)
{
    mTypeCQS = Component::Q;
}
