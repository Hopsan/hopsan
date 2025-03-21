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
//! @file   Parameters.cpp
//! @author FluMeS
//! @date   2012-01-05
//! @brief Contains the parameter and parameters classes
//!
//$Id$

#include "Parameters.h"
#include "HopsanCoreMacros.h"
#include "Component.h"
#include "ComponentSystem.h"
#include "CoreUtilities/StringUtilities.h"
#include "CoreUtilities/NumHopHelper.h"
#include "ComponentUtilities/num2string.hpp"
//#include "Quantities.h"
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
//! Both Components and Systems use the same parameter implementation. In the case of a system parameter the
//! data pointer is 0.

//! @brief Constructor
//! @param [in] rName The desired parameter name, e.g. m
//! @param [in] rValue The value of the parameter, always a string
//! @param [in] rDescription The description of the parameter e.g. Mass
//! @param [in] rQuantity The physical quantity of the parameter e.g. Mass
//! @param [in] rUnit The physical unit of the parameter e.g. kg
//! @param [in] rType The type of the parameter e.g. double
//! @param [in] pDataPtr Only used by Components, system parameters don't use this, default: 0
//! @param [in] pParentParameters A pointer to the Parameters object that contains the Parameter
ParameterEvaluator::ParameterEvaluator(const HString &rName, const HString &rValue, const HString &rDescription, const HString &rQuantity, const HString &rUnit,
                                       const HString &rType, const bool internal, void* pDataPtr, ParameterEvaluatorHandler* pParameterEvalHandler)
{
    mDepthCounter=0;
    mParameterName = rName;
    mParameterValue = rValue;
    mDescription = rDescription;
    mType = rType;
    mQuantity = rQuantity;
    mUnit = rUnit;
    mTriggersReconfiguration = false;
    mInternal = internal;

    mpData = pDataPtr;
    mpParameterEvaluatorHandler = pParameterEvalHandler;
    evaluate();
}


//! @brief Returns a pointer directly to the parameter data variable
//! @warning Don't use this function unless YOU REALLY KNOW WHAT YOU ARE DOING
void* ParameterEvaluator::getDataPtr()
{
    return mpData;
}

bool ParameterEvaluator::setParameter(const HString &rValue, const HString &rDescription, const HString &rQuantity, const HString &rUnit, const HString &rType, ParameterEvaluator **pNeedEvaluation, bool internal, bool force)
{
    bool success;
    HString oldValue = mParameterValue;
    HString oldDescription = mDescription;
    HString oldUnit = mUnit;
    HString oldType = mType;
    HString oldQuantity = mQuantity;
    bool oldInternal = mInternal;
    if(!rDescription.empty())
    {
        mDescription = rDescription;
    }
    if (!rQuantity.empty())
    {
        mQuantity = rQuantity;
    }
    if(!rUnit.empty())
    {
        mUnit = rUnit;
    }
    if(!rType.empty())
    {
        mType = rType;
    }
    mInternal = internal;
    success = setParameterValue(rValue, pNeedEvaluation);
    if((force) && !(success))
    {
        *pNeedEvaluation = this;
        mParameterValue = rValue;
    }
    else if(!success)
    {
        mParameterValue = oldValue;
        mDescription = oldDescription;
        mQuantity = oldQuantity;
        mUnit = oldUnit;
        mType = oldType;
        mInternal = oldInternal;
    }
    return success;
}

//! @brief Set the parameter value for an existing parameter
//! @param [in] rValue The new value for the parameter
//! @param [out] ppNeedEvaluation Tell if the parameter needs evaluation, e.g. is a system parameter or an expression
//! @return true if success, otherwise false
//!
//! This function is used by Parameters
bool ParameterEvaluator::setParameterValue(const HString &rValue, ParameterEvaluator **ppNeedEvaluation, bool force)
{
    bool success=false;

    HString oldValue = mParameterValue;
    mParameterValue = rValue;
    HString evalResult = rValue;
    success = evaluate(evalResult);
    if(!success && !force)
    {
        mParameterValue = oldValue;
    }

    if (ppNeedEvaluation) {
        if(rValue != evalResult) {
            *ppNeedEvaluation = this;
        }
        else {
            *ppNeedEvaluation = 0;
        }
    }

    return success;
}


//! @brief Returns the type of the parameter
//! @return The type of the parameter
const HString &ParameterEvaluator::getType() const
{
    return mType;
}


//! @brief Evaluate the parameter
//! @return true if success, otherwise false
//!
//! This function is used by Parameters. The point with run this function is
//! to write the right value to the mData pointer.
//! @see evaluate(HString &result)
bool ParameterEvaluator::evaluate()
{
    HString dummy;
    return evaluate(dummy);
}

bool ParameterEvaluator::refreshParameterValueText()
{
    if (mpData)
    {
        stringstream ss;
        if(mType=="double")
        {
            ss << *static_cast<double*>(mpData);
        }
        else if(mType=="integer")
        {
            ss << *static_cast<int*>(mpData);
        }
        else if(mType=="conditional")
        {
            ss << *static_cast<int*>(mpData);//mConditions[*static_cast<int*>(mpData)];
        }
        else if(mType=="bool")
        {
            if (*static_cast<bool*>(mpData))
            {
                ss << "true";
            }
            else
            {
                ss << "false";
            }
        }
        else if(mType=="string" || mType=="textblock" || mType=="filepath")
        {
            ss << *static_cast<string*>(mpData);
        }
        else
        {
            return false;
        }
        mParameterValue = ss.str().c_str();
        return true;
    }
    return false;
}


//! @brief Evaluate the parameter
//! @param [out] rResult The result of the evaluation
//! @return true if success, otherwise false
//!
//! This function is used by Parameters
//! @see evaluate()
bool ParameterEvaluator::evaluate(HString &rResult)
{

// These values are arejust a guess, there is no easy way of kowing how long it will take until the stack overflow
// MSVC is lower them MinGW, GCC, Clang
// Possible a value could be chose based on what would be a resonable max depth, but this is aslo impossible to know
// This recursion happens when a parameter depdens on an other which depends on another and so on
// The max_depth will usually trigger when someone tries to referr to is self or self assign
#if defined(_MSC_VER)
    #define max_depth 50
#else
    #define max_depth 250
#endif

    ++mDepthCounter;
    if (mDepthCounter > max_depth)
    {
        mpParameterEvaluatorHandler->getComponent()->addErrorMessage("You seem to be stuck in a parameter evaluation loop (aborting): " +
                                                                     mParameterName + " = " + mParameterValue);
        --mDepthCounter;
        return false;
    }

    if(!((mType=="double") || (mType=="integer") || (mType=="bool") || (mType=="string") || (mType=="textblock") || (mType=="filepath") || (mType=="conditional")))
    {
        mpParameterEvaluatorHandler->getComponent()->addErrorMessage("Parameter could not be evaluated, unknown type: " + mType);
    }

    bool success = true;
    HString evaluatedParameterValue;

    // Determine if we should look for parameter among other parameters and system parameters
    bool doCheckOthers=false;
    //! @todo handle conditional also
    if (mType=="double" || mType=="integer")
    {
        doCheckOthers = !mParameterValue.isNummeric();
    }
    else if (mType=="bool")
    {
        doCheckOthers = !mParameterValue.isBool();
    }
    else if ((mType=="string") || (mType=="textblock") || (mType=="filepath"))
    {
        doCheckOthers = true;
    }

    // Check parent system parameters
    //! @todo Use numhop to evaluate integer and bool expressions, possibly by converting to double and back
    if (doCheckOthers && (mType=="integer"))
    {
        // Strip + or - from name in case we want to take a negative value of a system parameter
        HString signPrefix, parameterValueWithoutSign;
        splitSignPrefix(mParameterValue, signPrefix, parameterValueWithoutSign);
        const HString& possibleNameOfOtherParameter = parameterValueWithoutSign;
        const bool isSelfParameter = possibleNameOfOtherParameter.startsWith("self.");

        if (isSelfParameter && mpParameterEvaluatorHandler->evaluateInComponent(possibleNameOfOtherParameter.substr(5), evaluatedParameterValue, mType)) {
            resolveSignPrefix(signPrefix);
            evaluatedParameterValue = signPrefix + evaluatedParameterValue;
        }
        else if(mpParameterEvaluatorHandler->evaluateRecursivelyInSystemParents(possibleNameOfOtherParameter,  evaluatedParameterValue, mType)) {
            resolveSignPrefix(signPrefix);
            evaluatedParameterValue = signPrefix + evaluatedParameterValue;
        }
        else {
            evaluatedParameterValue = mParameterValue;
        }
    }
    else if (doCheckOthers && (mType=="string" || mType=="textblock" || mType=="filepath" || mType=="bool")) {

        const HString& possibleNameOfOtherParameter = mParameterValue;
        const bool isSelfParameter = possibleNameOfOtherParameter.startsWith("self.");

        if (isSelfParameter && mpParameterEvaluatorHandler->evaluateInComponent(possibleNameOfOtherParameter.substr(5), evaluatedParameterValue, mType)) {
            //evaluatedParameterValue = evaluatedParameterValue; No point is self assignment,  but comment left here for clarity
        }
        else if(mpParameterEvaluatorHandler->evaluateRecursivelyInSystemParents(possibleNameOfOtherParameter,  evaluatedParameterValue, mType)) {
            //evaluatedParameterValue = evaluatedParameterValue;  No point is self assignment, but comment left here for clarity
        }
        else {
            evaluatedParameterValue = mParameterValue;
        }
    }
    // Use numhop expression evaluation for doubles
    else if (doCheckOthers)
    {
        if (mpParameterEvaluatorHandler->evaluateParameterExpression(mParameterValue, evaluatedParameterValue)) {
            //evaluatedParameterValue = evaluatedParameterValue;  No point is self assignment, but comment left here for clarity
        }
        else {
            evaluatedParameterValue = mParameterValue;
        }
    }
    else
    {
        evaluatedParameterValue = mParameterValue;
    }

    // Now try to evaluate the actual parameter value based on type
    if(mType=="double")
    {
        bool isOK;
        double v = evaluatedParameterValue.toDouble(&isOK);
        if(isOK)
        {
            // If a data pointer has been set, then write evaluated value to data variable
            if(mpData)
            {
                *static_cast<double*>(mpData) = v;
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
        istringstream is(evaluatedParameterValue.c_str());
        if(is >> tmpParameterValue)
        {
            // If a data pointer has been set, then write evaluated value to data variable
            if(mpData)
            {
                *static_cast<int*>(mpData) = tmpParameterValue;
            }
        }
        else
        {
            success = false;
        }
    }
    else if(mType=="conditional")
    {
        int tmpParameterValue;
        istringstream is(evaluatedParameterValue.c_str());
        if((is >> tmpParameterValue) && (tmpParameterValue >= 0) && (tmpParameterValue < int(this->mConditions.size())))
        {
            // If a data pointer has been set, then write evaluated value to data variable
            if(mpData)
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
        istringstream is(evaluatedParameterValue.c_str());
        if(is >> tmpParameterValue)
        {
            // If a data pointer has been set, then write evaluated value to data variable
            if(mpData)
            {
                *static_cast<bool*>(mpData) = tmpParameterValue;
            }
        }
        else if((evaluatedParameterValue == "false") || (evaluatedParameterValue == "0"))
        {
            // If a data pointer has been set, then write evaluated value to data variable
            if(mpData)
            {
                *static_cast<bool*>(mpData) = false;
            }
        }
        else if((evaluatedParameterValue == "true") || (evaluatedParameterValue == "1"))
        {
            // If a data pointer has been set, then write evaluated value to data variable
            if(mpData)
            {
                *static_cast<bool*>(mpData) = true;
            }
        }
        else
        {
            success = false;
        }
    }
    else if(mType=="string" || mType=="textblock" || mType=="filepath")
    {
        // If a data pointer has been set, then write evaluated value to data variable
        if(mpData)
        {
            static_cast<HString*>(mpData)->setString(evaluatedParameterValue.c_str());
        }
    }
    else
    {
        success = false;
    }

    rResult = evaluatedParameterValue;
    --mDepthCounter;
    return success;
}

const HString &ParameterEvaluator::getName() const
{
    return mParameterName;
}

const HString &ParameterEvaluator::getValue() const
{
    return mParameterValue;
}

const HString &ParameterEvaluator::getUnit() const
{
    return mUnit;
}

const HString &ParameterEvaluator::getDescription() const
{
    return mDescription;
}

const HString &ParameterEvaluator::getQuantity() const
{
    return mQuantity;
}

const std::vector<HString> &ParameterEvaluator::getConditions() const
{
    return mConditions;
}

bool ParameterEvaluator::isInternal() const
{
    return mInternal;
}

void ParameterEvaluator::setTriggersReconfiguration()
{
    mTriggersReconfiguration = true;
}

bool ParameterEvaluator::triggersReconfiguration()
{
    return mTriggersReconfiguration;
}

void ParameterEvaluator::resolveSignPrefix(HString &rSignPrefix) const
{
    // Resolve prefix, check num -, ignore +
    size_t nMinus=0;
    for (size_t n=0; n<rSignPrefix.size(); ++n)
    {
        if (rSignPrefix[n] == '-')
        {
            ++nMinus;
        }
    }

    // Check if odd, then minus else they cancel each other out
    if ((nMinus % 2) != 0)
    {
        rSignPrefix = '-';
    }
    else
    {
        rSignPrefix.clear();
    }
}

void ParameterEvaluator::splitSignPrefix(const HString &rString, HString &rPrefix, HString &rValue)
{
    rPrefix.clear();
    size_t n=0;
    for ( ; n<rString.size(); ++n)
    {
        if ( (rString[n] == '-') || (rString[n] == '+') )
        {
            rPrefix = rPrefix + rString[n];
        }
        else
        {
            // Break will prevent n from becoming n+1, (and break the loop)
            break;
        }
    }
    // Copy all but sign prefix
    rValue = rString.substr(n);
}

//! @class hopsan::Parameters
//! @brief The Parameters class implements the parameters used in both Components and ComponentSystems
//!


//! @brief Constructor
//! @param [in] pParentComponent A pointer to the Component that contains the Parameters
ParameterEvaluatorHandler::ParameterEvaluatorHandler(Component* pComponent)
{
    mComponent = pComponent;
}

//! @brief Destructor
ParameterEvaluatorHandler::~ParameterEvaluatorHandler()
{
    //Deletes all parameters stored in vector
    for (size_t i=0; i<mParameters.size(); ++i)
    {
        delete mParameters[i];
    }
}


//! @brief Add a new parameter
//! @param [in] rName The desired parameter name, e.g. m
//! @param [in] rValue The value of the parameter, always a string
//! @param [in] rDescription The description of the parameter e.g. Mass
//! @param [in] rQuantity The physical quantity of the parameter e.g. mass
//! @param [in] rUnit The physical unit of the parameter e.g. kg
//! @param [in] rType The type of the parameter e.g. double
//! @param [in] pData Only used by Components, system parameters don't use this, default: 0
//! @param [in] force Should we force to add parameter even if it fails to evaluate
//! @param [in] conditions Conditions for a conditional constant parameter
//! @return true if success, otherwise false
bool ParameterEvaluatorHandler::addParameter(const HString &rName, const HString &rValue, const HString &rDescription, const HString &rQuantity, const HString &rUnit, const HString &rType, void* pData, bool internal, bool force, std::vector<HString> conditions)
{
    bool success = false;
    if (!rName.empty())
    {
        if(!hasParameter(rName))
        {
            //! @todo should make sure that parameter names do not have + - * / . or similar as first character
            ParameterEvaluator* newParameter = new ParameterEvaluator(rName, rValue, rDescription, rQuantity, rUnit, rType, internal, pData, this);
            if(rType == "conditional")
            {
                newParameter->mConditions = conditions;
            }
            success = newParameter && newParameter->evaluate(); //! @todo here we evaluate again (why?)
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
//! @param[in] rName The name of the parameter to delete
void ParameterEvaluatorHandler::deleteParameter(const HString &rName)
{
    std::vector<ParameterEvaluator*>::iterator parIt, needevalIt;
    for(parIt=mParameters.begin(); parIt!=mParameters.end(); ++parIt)
    {
        if( rName == (*parIt)->getName() )
        {
            // First remove pointer from needs evaluation list, if it exist there (to avoid dangling pointer)
            for (needevalIt = mParametersNeedEvaluation.begin(); needevalIt != mParametersNeedEvaluation.end();)
            {
                if ((*needevalIt) == (*parIt))
                {
                    needevalIt = mParametersNeedEvaluation.erase(needevalIt);
                }
                else
                {
                    needevalIt++;
                }
            }

            delete *parIt;
            mParameters.erase(parIt);

            // We can return now, since there should never be multiple parameters with same name
            return;
        }
    }
}

//! @brief Rename a parameter (only useful for system parameters)
//! @todo do I need to call some needs evaluation here or ?
bool ParameterEvaluatorHandler::renameParameter(const HString &rOldName, const HString &rNewName)
{
    if (!hasParameter(rNewName))
    {
        std::vector<ParameterEvaluator*>::iterator parIt;
        for(parIt=mParameters.begin(); parIt!=mParameters.end(); ++parIt)
        {
            if( rOldName == (*parIt)->getName() )
            {
                (*parIt)->mParameterName = rNewName;
                return true;
            }
        }
    }
    return false;
}


const ParameterEvaluator* ParameterEvaluatorHandler::getParameter(const HString &rName) const
{
    for (size_t i=0; i<mParameters.size(); ++i)
    {
        if (mParameters[i]->getName() == rName)
        {
            return mParameters[i];
        }
    }

    // If parameter not found return 0
    return 0;
}

void ParameterEvaluatorHandler::getParameterNames(std::vector<HString> &rParameterNames)
{
    rParameterNames.resize(mParameters.size());
    for(size_t i=0; i<mParameters.size(); ++i)
    {
        rParameterNames[i] = mParameters[i]->getName();
    }
}

//! @brief Get the value of specified parameter
//! @param [in] rName The parameter name to get value of
//! @param [out] rValue Reference to the string variable that will contain the parameter value. The variable will be "" if parameter not found
void ParameterEvaluatorHandler::getParameterValue(const HString &rName, HString &rValue)
{
    for(size_t i=0; i<mParameters.size(); ++i)
    {
        if (mParameters[i]->getName() == rName)
        {
            rValue = mParameters[i]->getValue();
            return; //Abort function as value has been set
        }
    }
    rValue = "";
}

//! @brief Returns a pointer directly to the parameter data variable
//! @warning Don't use this function unless YOU REALLY KNOW WHAT YOU ARE DOING
void* ParameterEvaluatorHandler::getParameterDataPtr(const HString &rName)
{
    for(size_t i=0; i<mParameters.size(); ++i)
    {
        if (mParameters[i]->getName() == rName)
        {
            return mParameters[i]->getDataPtr();
        }
    }
    return 0;
}

const std::vector<ParameterEvaluator*> *ParameterEvaluatorHandler::getParametersVectorPtr() const
{
    return &mParameters;
}


bool ParameterEvaluatorHandler::setParameter(const HString &rName, const HString &rValue, const HString &rDescription,
                                             const HString &rQuantity, const HString &rUnit, const HString &rType, const bool internal, const bool force)
{
    bool success = false;

    // Try to find the parameter among the existing parameters
    for(size_t i=0; i<mParameters.size(); ++i)
    {
        // If Found (It cannot find itself)
        if( (rName == mParameters[i]->getName()) )//&& (value != mParameters[i]->getName()) ) //By commenting this a parameter can be set to a systems parameter with same name as component parameter e.g. mass m = m (system parameter) related to issue #783
        {
            ParameterEvaluator *needEvaluation=0;
            success = mParameters[i]->setParameter(rValue, rDescription, rQuantity, rUnit, rType, &needEvaluation, internal, force); //Sets the new value, if the parameter is of the type to need evaluation e.g. if it is a system parameter needEvaluation points to the parameter
            if(needEvaluation)
            {
                if(mParametersNeedEvaluation.end() == find(mParametersNeedEvaluation.begin(), mParametersNeedEvaluation.end(), needEvaluation))
                {
                    mParametersNeedEvaluation.push_back(needEvaluation); //The parameter needs evaluation and is not already stored
                }
            }
            else //mParameters[i] don't need evaluation, this loop erases it from mParametersNeedEvaluation
            {
                std::vector<ParameterEvaluator*>::iterator parIt = mParametersNeedEvaluation.begin();
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

//! @brief Set the parameter value for an existing parameter
//! @param [in] rName The name of the parameter to be set
//! @param [in] rValue The new value for the parameter
//! @param [in] force Should we force the value to be set
//! @return true if success, otherwise false
bool ParameterEvaluatorHandler::setParameterValue(const HString &rName, const HString &rValue, bool force)
{
    return setParameter(rName, rValue, "", "", "", "", false, force);
}


//! @brief Evaluate a specific parameter
//! @param [in] rName The name of the parameter to be evaluated
//! @param [out] rEvaluatedParameterValue The result of the evaluation
//! @param [in] rType The type of how the parameter should be interpreted
//! @return true if success, otherwise false
bool ParameterEvaluatorHandler::evaluateInComponent(const HString &rName, HString &rEvaluatedParameterValue, const HString &rType)
{
    bool success = false;
    for(size_t i = 0; i < mParameters.size(); ++i)
    {
        if ( (mParameters[i]->getName() == rName) &&
             (mParameters[i]->getType() == rType) )
        {
            success = mParameters[i]->evaluate(rEvaluatedParameterValue);
        }
    }
    return success;
}


//! @brief Evaluate all parameters
//! @return true if success, otherwise false
bool ParameterEvaluatorHandler::evaluateParameters()
{
    bool success = true;
    for(size_t i=0; i<mParameters.size(); ++i)
    {
        success = (success && mParameters[i]->evaluate());
    }
    return success;
}

bool ParameterEvaluatorHandler::refreshParameterValueText(const HString &rParameterName)
{
    for(size_t i=0; i<mParameters.size(); ++i)
    {
        if(mParameters[i]->getName() == rParameterName)
        {
            return mParameters[i]->refreshParameterValueText();
        }
    }
    return false;
}

bool ParameterEvaluatorHandler::evaluateRecursivelyInSystemParents(const HString &rName, HString &rEvaluatedParameterValue, const HString &rType)
{
    bool evalOK = false;
    ComponentSystem* pParentSystem = mComponent ? mComponent->getSystemParent() : 0;
    while (pParentSystem) {
        evalOK = pParentSystem->evaluateParameter(rName, rEvaluatedParameterValue , rType);
        if (evalOK) {
            break;
        }
        pParentSystem = pParentSystem->getSystemParent();
    }
    return evalOK;
}

bool ParameterEvaluatorHandler::evaluateParameterExpression(const HString &rExpression, HString &rEvaluatedParameterValue)
{
    //! @todo waste of time recreating the helper every time, should reuse one that always exists
    NumHopHelper nh;
    nh.setComponent(mComponent);
    HString dummy;
    double value;
    bool evalOK = nh.evalNumHopScript(rExpression.c_str(), value, false,dummy);
    if (evalOK)
    {
        rEvaluatedParameterValue = to_hstring(value);
    }
    return evalOK;
}

//! @brief Check if a parameter with given name exist among the parameters
//! @param [in] rName The name of the parameter to check for
//! @returns true if found else false
bool ParameterEvaluatorHandler::hasParameter(const HString &rName) const
{
    for(size_t i=0; i<mParameters.size(); ++i)
    {
        if(mParameters[i]->getName() == rName)
        {
            return true;
        }
    }
    return false;
}


//! @brief Check all parameters that need evaluation are able to be evaluated
//! @param [out] rErrParName The name of the parameter that could not be evaluated
//! @return true if success, otherwise false
//!
//! Check all parameters that need evaluation are able to be evaluated. The function will
//! stop as soon as one parameter turns out to be faulty. So in the case of many bad parameters
//! only the name of the first one is returned.
bool ParameterEvaluatorHandler::checkParameters(HString &rErrParName)
{
    bool success = true;
    std::vector<ParameterEvaluator*>::iterator parIt;
    for(parIt = mParametersNeedEvaluation.begin(); (parIt != mParametersNeedEvaluation.end()) && (!mParametersNeedEvaluation.empty()); ++parIt)
    {
        success = (success && (*parIt)->evaluate());
        if(!success)
        {
            rErrParName = (*parIt)->getName();
            break;
        }
    }
    return success;
}

void ParameterEvaluatorHandler::setParameterTriggersReconfiguration(const HString &rParameterName)
{
    for(size_t i=0; i<mParameters.size(); ++i)
    {
        if(mParameters[i]->getName() == rParameterName)
        {
            mParameters[i]->setTriggersReconfiguration();
        }
    }
}

bool ParameterEvaluatorHandler::parameterTriggersReconfiguration(const HString &rParameterName)
{
    for(size_t i=0; i<mParameters.size(); ++i)
    {
        if(mParameters[i]->getName() == rParameterName)
        {
            return mParameters[i]->triggersReconfiguration();
        }
    }
    return false;
}


Component *ParameterEvaluatorHandler::getComponent() const
{
    return mComponent;
}

