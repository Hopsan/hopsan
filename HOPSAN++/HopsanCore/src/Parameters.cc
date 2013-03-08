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
//! @file   Parameters.cc
//! @author FluMeS
//! @date   2012-01-05
//! @brief Contains the parameter and parameters classes
//!
//$Id$

#include "Parameters.h"
#include "Component.h"
#include "ComponentSystem.h"
#include <cassert>
#include <sstream>
#include <algorithm>
#include <iostream>

using namespace hopsan;
using namespace std;

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
Parameter::Parameter(std::string parameterName, std::string parameterValue, std::string description, std::string unit, std::string type, bool isDynamic, void* pDataPtr, Parameters* pParentParameters)
{
    mEnabled = true;
    mParameterName = parameterName;
    mParameterValue = parameterValue;
    mDescription = description;
    mUnit = unit;
    mType = type;

    mIsDynamic = isDynamic;
    mpData = pDataPtr;
    mpParentParameters = pParentParameters;
    evaluate();
}


//! @brief Returns a pointer directly to the parameter data variable
//! @warning Dont use this function unless YOU REALLY KNOW WHAT YOU ARE DOING
//! @warning This function may be removed in teh future
void* Parameter::getDataPtr()
{
    return mpData;
}

void Parameter::setEnabled(const bool enabled)
{
    mEnabled = enabled;
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
 //   if(!(mParameterName==value))
    {
        std::string oldValue = mParameterValue;
        mParameterValue = value;
        std::string evalResult = value;
        success = evaluate(evalResult, this);
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
std::string Parameter::getType() const
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
bool Parameter::evaluate(std::string &rResult, Parameter *ignoreMe)
{
    if(!((mType=="double") || (mType=="integer") || (mType=="bool") || (mType=="string")))
    {
        mpParentParameters->getParentComponent()->addErrorMessage("Parameter could not be evaluated, unknown type: " + mType);
    }

    bool success = true;
    std::string evaluatedParameterValue, prefix, valueName;

    // Strip + or - from name incase we want to take a negative value of a system parameter
    if (!mParameterValue.empty())
    {
        if ( (mParameterValue[0] == '-') || (mParameterValue[0] == '+') )
        {
            prefix = mParameterValue[0];
            valueName = mParameterValue.substr(1); //1 to end
        }
        else
        {
            valueName = mParameterValue;
        }
    }

    // First check if this parameter value is in fact the name of one of the other parameters or system parameter
    if( mpParentParameters->evaluateParameter(valueName, evaluatedParameterValue, mType, this) )
//        if( mpParentParameters->evaluateParameter(valueName, evaluatedParameterValue, mType, ignoreMe) ) //To allow a parameter to use a systemsparameter with same name the component parameter itself has to be excluded in this check by ignore it here, issue #783
    {
        evaluatedParameterValue = prefix + evaluatedParameterValue;
    }
    else
    {
        // If not then the value is actually the value
        evaluatedParameterValue = mParameterValue;
    }

    if(mType=="double")
    {
        double tmpParameterValue;
        istringstream is(evaluatedParameterValue);
        if(is >> tmpParameterValue)
        {
            // If a data pointer has been set, then write evaluated value to data variable
            if( (mpData!=0) && mEnabled )
            {
                *static_cast<double*>(mpData) = tmpParameterValue;
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
            // If a data pointer has been set, then write evaluated value to data variable
            if( (mpData!=0) && mEnabled )
            {
                *static_cast<int*>(mpData) = tmpParameterValue;
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
            // If a data pointer has been set, then write evaluated value to data variable
            if( (mpData!=0) && mEnabled )
            {
                *static_cast<bool*>(mpData) = tmpParameterValue;
            }
        }
        else if((evaluatedParameterValue == "false") || (evaluatedParameterValue == "0"))
        {
            // If a data pointer has been set, then write evaluated value to data variable
            if( (mpData!=0) && mEnabled )
            {
                *static_cast<bool*>(mpData) = false;
            }
        }
        else if((evaluatedParameterValue == "true") || (evaluatedParameterValue == "1"))
        {
            // If a data pointer has been set, then write evaluated value to data variable
            if( (mpData!=0) && mEnabled )
            {
                *static_cast<bool*>(mpData) = true;
            }
        }
        else
        {
            success = false;
        }
    }
    else if(mType=="string")
    {
        // If a data pointer has been set, then write evaluated value to data variable
        if( (mpData!=0) && mEnabled )
        {
            *static_cast<string*>(mpData) = evaluatedParameterValue;
        }
    }
    else
    {
        success = false;
    }

    rResult = evaluatedParameterValue;
    return success;
}

std::string Parameter::getName() const
{
    return mParameterName;
}

std::string Parameter::getValue() const
{
    return mParameterValue;
}

std::string Parameter::getUnit() const
{
    return mUnit;
}

std::string Parameter::getDescription() const
{
    return mDescription;
}

bool Parameter::isEnabled() const
{
    return mEnabled;
}

bool Parameter::isDynamic() const
{
    return mIsDynamic;
}

//! @class hopsan::Parameters
//! @brief The Parameters class implements the parameters used in both Componenets and ComponentSystems
//!


//! @brief Constructor
//! @param [in] pParentComponent A pointer to the Component that contains the Parameters
Parameters::Parameters(Component* pParentComponent)
{
    mParentComponent = pParentComponent;
}

//! @brief Destructor
Parameters::~Parameters()
{
    //Deleates all parameters stored in vector
    for (size_t i=0; i<mParameters.size(); ++i)
    {
        delete mParameters[i];
    }
}


//! @brief Add a new parameter
//! @param [in] parameterName The desired parameter name, e.g. m
//! @param [in] parameterValue The value of the parameter, always a string
//! @param [in] description The description of the parameter e.g. Mass, default: ""
//! @param [in] unit The physical unit of the parameter e.g. kg, default: "0""
//! @param [in] type The type of the parameter e.g. double, default: ""
//! @param [in] pDataPtr Only used by Components, system parameters don't use this, default: 0
//! @return true if success, otherwise false
bool Parameters::addParameter(std::string parameterName, std::string parameterValue, std::string description, std::string unit, std::string type, bool isDynamic, void* dataPtr, bool force)
{
    bool success = false;
    if (!parameterName.empty())
    {
        if(!exist(parameterName))
        {
            //! @todo should make sure that parameter names do not have + - * / . or similar as first charater
            Parameter* newParameter = new Parameter(parameterName, parameterValue, description, unit, type, isDynamic, dataPtr, this);
            success = newParameter && newParameter->evaluate();
            if(success || force)
            {
                mParameters.push_back(newParameter);
                success = true;
            }
            else
            {
                delete newParameter;
            }
        }
    }
    return success;
}


//! @brief Deletes a parameter
//! @param parameterName The name of the paramter to delete
void Parameters::deleteParameter(const std::string parameterName)
{
    std::vector<Parameter*>::iterator parIt;
    for(parIt=mParameters.begin(); parIt!=mParameters.end(); ++parIt)
    {
        if( parameterName == (*parIt)->getName() )
        {
            delete *parIt;
            mParameters.erase(parIt);

            //We can return now, since there should never be multiple parameters with same name
            return;
        }
    }
}

//! @brief Rename a parameter (only useful for system paramters)
//! @todo do I need to call some needs evaluation here or ?
bool Parameters::renameParameter(const std::string oldName, const std::string newName)
{
    if (!exist(newName))
    {
        std::vector<Parameter*>::iterator parIt;
        for(parIt=mParameters.begin(); parIt!=mParameters.end(); ++parIt)
        {
            if( oldName == (*parIt)->getName() )
            {
                (*parIt)->mParameterName = newName;
                return true;
            }
        }
    }
    return false;
}


void Parameters::enableParameter(std::string parameterName, const bool enable)
{
    std::vector<Parameter*>::iterator parIt;
    for(parIt=mParameters.begin(); parIt!=mParameters.end(); ++parIt)
    {
        if( parameterName == (*parIt)->getName() )
        {
            (*parIt)->setEnabled(enable);

            //We can return now, since there should never be multiple parameters with same name
            return;
        }
    }
}


const Parameter* Parameters::getParameter(const std::string parameterName) const
{
    for (size_t i=0; i<mParameters.size(); ++i)
    {
        if (mParameters[i]->getName() == parameterName)
        {
            return mParameters[i];
        }
    }

    // If paramter not found return 0
    return 0;
}

void Parameters::getParameterNames(std::vector<std::string> &rParameterNames)
{
    rParameterNames.resize(mParameters.size());
    for(size_t i=0; i<mParameters.size(); ++i)
    {
        rParameterNames[i] = mParameters[i]->getName();
    }
}

//! @brief Get the value of specified parameter
//! @param [in] name The parameter name to get value of
//! @param [out] rValue Reference to the string variable that will contain the parameter value. The variable will be "" if parameter not found
void Parameters::getParameterValue(const std::string name, std::string &rValue)
{
    for(size_t i=0; i<mParameters.size(); ++i)
    {
        if (mParameters[i]->getName() == name)
        {
            rValue = mParameters[i]->getValue();
            return; //Abort function as value has been set
        }
    }
    rValue = "";
}

//! @brief Returns a pointer directly to the parameter data variable
//! @warning Dont use this function unless YOU REALLY KNOW WHAT YOU ARE DOING
//! @warning This function may be removed in the future
void* Parameters::getParameterDataPtr(const std::string name)
{
    for(size_t i=0; i<mParameters.size(); ++i)
    {
        if (mParameters[i]->getName() == name)
        {
            return mParameters[i]->getDataPtr();
        }
    }
    return 0;
}

const std::vector<Parameter*> *Parameters::getParametersVectorPtr() const
{
    return &mParameters;
}


bool Parameters::setParameter(const std::string name, const std::string value, const std::string description,
                              const std::string unit, const std::string type,  const bool force)
{
    bool success = false;

    // Try to find the parameter among the excisting parameters
    for(size_t i=0; i<mParameters.size(); ++i)
    {
        // If Found (It cannot find itself)
        if( (name == mParameters[i]->getName()) )//&& (value != mParameters[i]->getName()) ) //By commenting this a parameter can be set to a systems parameter with same name as component parameter e.g. mass m = m (system parameter) related to issue #783
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
                        //cout << (*parIt)->getName() << endl;//debug
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
bool Parameters::evaluateParameter(const std::string parameterName, std::string &rEvaluatedParameterValue, const std::string type, Parameter *ignoreMe)
{
    bool success = false;
    //Try our own parameters
    for(size_t i = 0; i < mParameters.size(); ++i)
    {
        if ( (mParameters[i]->getName() == parameterName) &&
             (mParameters[i]->getType() == type) &&
             (mParameters[i] != ignoreMe) )
        {
            success = mParameters[i]->evaluate(rEvaluatedParameterValue, ignoreMe);
        }
    }
    if(!success)
    {
        //Try one of our components systemparents parameters
        if(mParentComponent)
        {
            if(mParentComponent->getSystemParent())
            {
                success = mParentComponent->getSystemParent()->getSystemParameters().evaluateParameter(parameterName, rEvaluatedParameterValue , type);
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
        success = (success && mParameters[i]->evaluate());
    }
    return success;
}

//! @brief Check if a parameter with given name exist among the parameters
//! @param [in] parameterName The name of the parameter to check for
//! @returns true if found else false
bool Parameters::exist(const std::string parameterName)
{
    for(size_t i=0; i<mParameters.size(); ++i)
    {
        if(mParameters[i]->getName() == parameterName)
        {
            return true;
        }
    }
    return false;
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
        success = (success && (*parIt)->evaluate());
        if(!success)
        {
            errParName = (*parIt)->getName();
            break;
        }
    }
    return success;
}


Component *Parameters::getParentComponent() const
{
    return this->mParentComponent;
}

