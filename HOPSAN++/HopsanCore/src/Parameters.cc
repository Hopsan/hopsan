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
//! @brief Contains all built in node types
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
        //! @todo we should not assert false here, need to handle in some nicer way
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
    bool success = true;
    std::string evaluatedParameterValue;
    if(!(mpParentParameters->evaluateParameter(mParameterValue, evaluatedParameterValue, mType, ignoreMe)))
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

std::string Parameter::getName() const
{
    return mParameterName;
}

std::string Parameter::getValue() const
{
    return mParameterValue;
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
bool Parameters::addParameter(std::string parameterName, std::string parameterValue, std::string description, std::string unit, std::string type, void* dataPtr, bool force)
{
    bool success = false;
//    istringstream is(parameterValue);
//    //Type not decided, to be done here
//    if((type.empty()) || (0 == type.compare("double")))
//    {
//        double tmpDouble;
//        if(is >> tmpDouble)
//        {
//            type = "double";
//            success *= true;
//        }
//        else
//        {
//            success *= false;
//        }
//    }
//    else if(0 == type.compare("bool"))
//    {
//        bool tmpBool;
//        if((is >> tmpBool) || (parameterValue.compare("false")) == 0 || (parameterValue.compare("true") == 0)
//                           || (parameterValue.compare("0"))     == 0 || (parameterValue.compare("1")    == 0))
//        {
//            success *= true;
//        }
//        else
//        {
//            success *= false;
//        }
//    }
//    else if(0 == type.compare("integer"))
//    {
//        int tmpInt;
//        if(is >> tmpInt)
//        {
//            success *= true;
//        }
//        else
//        {
//            success *= false;
//        }
//    }
//    else if(0 == type.compare("string"))
//    {
//        std::string tmpStr;
//        if(is >> tmpStr)
//        {
//            success *= true;
//        }
//        else
//        {
//            success *= false;
//        }
//    }
//    else
//    {
//        success *= false;
//    }

    if(!exist(parameterName))
    {
        Parameter* newParameter = new Parameter(parameterName, parameterValue, description, unit, type, dataPtr, this);
        success = newParameter->evaluate();
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

    return success;
}


//! @brief Deletes a parameter
//! @param parameterName The name of the paramter to delete
void Parameters::deleteParameter(std::string parameterName)
{
    std::string name, value, description, unit, type;

    //! @todo We should use find instead to find iterator to object to delete, while find result is != .end() we delete and erase
    std::vector<Parameter*>::iterator parIt;
    for(parIt = mParameters.begin(); (parIt != mParameters.end()) && (!mParameters.empty()); ++parIt)
    {
        (*parIt)->getParameter(name, value, description, unit, type);
        if(parameterName == name)
        {
            mParameters.erase(parIt);
            //! @todo Probably memmory leek here as no delete is performed
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
bool Parameters::evaluateParameter(const std::string parameterName, std::string &rEvaluatedParameterValue, const std::string type, Parameter *ignoreMe)
{
    bool success = false;
    std::string parameterName2, parameterValue2, description2, unit2, type2;
    for(size_t i = 0; i < mParameters.size(); ++i)
    {
        mParameters[i]->getParameter(parameterName2, parameterValue2, description2, unit2, type2);
        if((parameterName == parameterName2) && (mParameters[i]->getType() == type) && (mParameters[i] != ignoreMe))
        {
            success = mParameters[i]->evaluate(rEvaluatedParameterValue, ignoreMe);
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


bool Parameters::exist(std::string parameterName)
{
    std::string parameterName2, parameterValue, description, unit, type;
    for(size_t i=0; i<mParameters.size(); ++i)
    {
        mParameters[i]->getParameter(parameterName2, parameterValue, description, unit, type);
        if(parameterName2 == parameterName)
            return true;
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
