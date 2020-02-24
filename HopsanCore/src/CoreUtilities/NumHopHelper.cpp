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

//$Id$

#define _USE_MATH_DEFINES
#include <cmath>
#include "CoreUtilities/NumHopHelper.h"
#include "ComponentSystem.h"
#include "CoreUtilities/StringUtilities.h"
#include "ComponentUtilities/num2string.hpp"
#include "numhop.h"

using namespace std;
using namespace hopsan;

namespace {

double evaluateDoubleParameter(Component* pComponent, const HString &rName, bool &rEvalOK)
{
    HString val;
    bool rc = pComponent->evaluateParameter(rName, val, "double");
    double v = val.toDouble(&rEvalOK);
    rEvalOK = (rEvalOK && rc);
    return v;
}

}

class HopsanParameterAccess :  public numhop::ExternalVariableStorage
{
public:
    bool setRegistered(const HString &name, double value)
    {
        std::map<HString, double*>::iterator it = mRegisteredDataPtrs.find(name.c_str());
        if (it != mRegisteredDataPtrs.end())
        {
            *it->second = value;
            return true;
        }
        return false;
    }

    double getRegistered(const HString &name, bool &rFound) const
    {
        std::map<HString, double*>::const_iterator it = mRegisteredDataPtrs.find(name.c_str());
        if (it != mRegisteredDataPtrs.end())
        {
            rFound = true;
            return *it->second;
        }
        rFound=false;
        return -1;
    }

    void registerDataPointer(const HString &name, double *pData)
    {
        mRegisteredDataPtrs.insert(std::pair<HString,double*>(name, pData));
    }

protected:
    std::map<HString, double*> mRegisteredDataPtrs;
};

class HopsanSystemAccess : public HopsanParameterAccess
{
public:
    HopsanSystemAccess(ComponentSystem *pSystem)
    {
        mpSystem = pSystem;
    }

    double externalValue(string name, bool &rFound) const
    {
        HString hname = name.c_str();
        vector<HString> parts;
        splitString(hname, '.', parts);

        bool isSelfValue = false;
        if (!parts.empty() && parts.front() == "self") {
            isSelfValue = true;
        }

        // First check if the name represents a system parameter in this system
        if (isSelfValue) {
            if ( (parts.size()==2) && mpSystem->hasParameter(parts[1]) ) {
                return evaluateDoubleParameter(mpSystem, parts[1], rFound);
            }
        }
        else
        {
            double value;
            if (mpSystem->getAliasHandler().hasAlias(hname))
            {
                parts.resize(3);
                mpSystem->getAliasHandler().getVariableFromAlias(hname,parts[0],parts[1],parts[2]);
            }

            // Now try to find the component/port/variable
            //! @todo handle pointing into subsystems or parent system
            if (parts.size() >= 2)
            {
                Component* pC = mpSystem->getSubComponent(parts[0]);
                if (pC)
                {
                    if (parts.size() == 2)
                    {
                        value = evaluateDoubleParameter(pC, parts[1], rFound);
                        // If not found, then try to add "Value", in case user is lazy and have not specified it for input or output start values
                        if (!rFound)
                        {
                            value = evaluateDoubleParameter(pC, parts[1]+"#Value", rFound);
                        }
                    }
                    else if (parts.size() == 3)
                    {
                        value = evaluateDoubleParameter(pC, parts[1]+"#"+parts[2], rFound);
                    }
                    else
                    {
                        value = -1;
                        rFound = false;
                    }
                    return value;
                }
            }
            // This seems to be a system parameter, recurse upwards in the model hierarcy until we find the parameter (or not)
            else if(parts.size() == 1) {
                ComponentSystem* pSystemParent = mpSystem->getSystemParent();
                while (pSystemParent) {
                    value = evaluateDoubleParameter(pSystemParent, parts[0], rFound);
                    if (rFound) {
                        break;
                    }
                    pSystemParent = pSystemParent->getSystemParent();
                }
            }
        }
        rFound = false;
        return -1;
    }

    bool setExternalValue(string name, double value)
    {
        HString hname = name.c_str();
        vector<HString> parts;
        splitString(hname, '.', parts);

        bool isSelfValue = false;
        if (!parts.empty() && parts.front() == "self") {
            isSelfValue = true;
        }

        // First check if the name represents a system parameter in this system
        if (isSelfValue) {
            if (mpSystem->hasParameter(parts[1]))
            {
                if (parts.size() == 2) {
                    return mpSystem->setParameterValue(parts[1], to_hstring(value));
                }
            }
        }
        else
        {
            vector<HString> parts;
            if (mpSystem->getAliasHandler().hasAlias(hname))
            {
                parts.resize(3);
                mpSystem->getAliasHandler().getVariableFromAlias(hname,parts[0],parts[1],parts[2]);
            }
            else
            {
                splitString(hname, '.', parts);
            }

            // Now try to find the component/port/variable
            //! @todo handle pointing into subsystems or parent system
            if (parts.size() >= 2)
            {
                Component* pC = mpSystem->getSubComponent(parts[0]);
                if (pC)
                {
                    if (parts.size() == 2)
                    {
                        bool rc = pC->setParameterValue(parts[1], to_hstring(value));
                        // If not found, then try to add "Value", in case user is lazy and have not specified it for input or output start values
                        if (!rc)
                        {
                            rc = pC->setParameterValue(parts[1]+"#Value", to_hstring(value));
                        }
                        return rc;
                    }
                    else if (parts.size() == 3)
                    {
                        return pC->setParameterValue(parts[1]+"#"+parts[2], to_hstring(value));
                    }
                }
            }
        }
        return false;
    }

private:
    ComponentSystem *mpSystem;
};

class HopsanComponentAccess : public HopsanParameterAccess
{
public:
    HopsanComponentAccess(Component *pComponent)
    {
        mpComponent = pComponent;
    }

    double externalValue(string name, bool &rFound) const
    {
        HString hname = name.c_str();

        // First try registered data pointer
        double vr = getRegistered(hname, rFound);
        if (rFound)
        {
            return vr;
        }

        double value=-1;
        HString valstring;
        vector<HString> parts;
        splitString(hname, '.', parts);

        bool isSelfValue = false;
        if (!parts.empty() && parts.front() == "self") {
            isSelfValue = true;
        }

        bool evalOK=false;
        // Check if this is a local constant
        if (isSelfValue && (parts.size() == 2))
        {
            mpComponent->getParameterValue(parts[1], valstring);
            // The value!=parts[1] is a hack to avoid infinite recursion when the value of the parameter is the same as the parameter name
            if (!valstring.empty() && valstring!=parts[1]) {
                value = evaluateDoubleParameter(mpComponent, parts[1], evalOK);
            }
        }
        // Check if this is a local port.value pair
        else if (isSelfValue && (parts.size() == 3))
        {
            value = evaluateDoubleParameter(mpComponent, parts[1]+"#"+parts[2], evalOK);
        }
        // This seems to be a system parameter, recurse upwards in the model hierarcy until we find the parameter (or not)
        else if(parts.size() == 1) {
            mpComponent->getParameterValue(parts[0], valstring);
            ComponentSystem* pSystemParent = mpComponent->getSystemParent();
            while (pSystemParent) {
                value = evaluateDoubleParameter(pSystemParent, parts[0], evalOK);
                if (evalOK) {
                    break;
                }
                pSystemParent = pSystemParent->getSystemParent();
            }
        }

        rFound = evalOK;
        return value;
    }

    bool setExternalValue(string name, double value)
    {
        HString hname = name.c_str();

        // First try registered data pointer
        if (setRegistered(hname, value))
        {
            return true;
        }

        vector<HString> parts;
        splitString(hname, '.', parts);

        bool isSelfValue = false;
        if (!parts.empty() && parts.front() == "self") {
            isSelfValue = true;
        }

        // Check if this is a local constant, or system parameter
        bool didSet = false;
        if (isSelfValue && parts.size() == 2)
        {
            //! @todo speed this up by not looking every time (use data pointer)
            didSet = mpComponent->setParameterValue(parts[1], to_hstring(value));

            //! @todo we should not be able to set a system parameter from inside a component, but we should not allow setting an internal variable with the same name
            // Try system parameter
//            if (!didSet && mpComponent->getSystemParent() && mpComponent->getSystemParent()->hasParameter(parts[0]))
//            {
//                didSet= mpComponent->getSystemParent()->setParameterValue(parts[0], to_hstring(value));
//            }
        }
        // Check if this is a local port.value pair
        else if (isSelfValue && parts.size() == 3)
        {
            didSet = mpComponent->setParameterValue(parts[1]+"#"+parts[2], to_hstring(value));
        }

        return didSet;
    }

private:
    Component *mpComponent;
};

namespace hopsan {

class NumHopHelperPrivate
{
public:
    NumHopHelperPrivate() : mpHopsanAccess(0) {}
    numhop::VariableStorage mVarStorage;
    HopsanParameterAccess *mpHopsanAccess;
    std::list<numhop::Expression> mExpressions;
};

}



NumHopHelper::NumHopHelper()
{
    mpSystem = 0;
    mpComponent = 0;
    mpPrivate = 0;
    mpPrivate = new NumHopHelperPrivate();
    mpPrivate->mVarStorage.reserveNamedValue("pi", M_PI);
}

NumHopHelper::~NumHopHelper()
{
    if (mpPrivate->mpHopsanAccess)
    {
        delete mpPrivate->mpHopsanAccess;
    }
    delete mpPrivate;
}

void NumHopHelper::setSystem(ComponentSystem *pSystem)
{
    mpSystem = pSystem;

    if (mpPrivate->mpHopsanAccess)
    {
        delete mpPrivate->mpHopsanAccess;
    }
    mpPrivate->mpHopsanAccess = new HopsanSystemAccess(pSystem);
    mpPrivate->mVarStorage.setExternalStorage(mpPrivate->mpHopsanAccess);
    mpPrivate->mVarStorage.setDisallowedInternalNameCharacters(".");
}

void NumHopHelper::setComponent(Component *pComponent)
{
    mpComponent = pComponent;

    if (mpPrivate->mpHopsanAccess)
    {
        delete mpPrivate->mpHopsanAccess;
    }
    mpPrivate->mpHopsanAccess = new HopsanComponentAccess(pComponent);
    mpPrivate->mVarStorage.setExternalStorage(mpPrivate->mpHopsanAccess);
    mpPrivate->mVarStorage.setDisallowedInternalNameCharacters(".");
}

void NumHopHelper::registerDataPtr(const HString &name, double *pData)
{
    if (mpPrivate->mpHopsanAccess)
    {
        mpPrivate->mpHopsanAccess->registerDataPointer(name, pData);
    }
}

bool NumHopHelper::evalNumHopScript(const HString &script, double &rValue, bool doPrintOutput, HString &rOutput)
{
    if (interpretNumHopScript(script, doPrintOutput, rOutput))
    {
        if (doPrintOutput && !rOutput.empty())
        {
            rOutput.append("\n");
        }
        return eval(rValue, doPrintOutput, rOutput);
    }
    mpPrivate->mExpressions.clear();
    return false;
}

bool NumHopHelper::interpretNumHopScript(const HString &script, bool doPrintOutput, HString &rOutput)
{
    list<string> expressions;
    numhop::extractExpressionRows(script.c_str(), '#', expressions);

    mpPrivate->mVarStorage.clearInternalVariables();
    mpPrivate->mExpressions.clear();

    bool allOK=true;
    for (list<string>::iterator it = expressions.begin(); it!=expressions.end(); ++it)
    {
        mpPrivate->mExpressions.push_back(numhop::Expression());
        bool interpretOK = numhop::interpretExpressionStringRecursive(*it, mpPrivate->mExpressions.back());
        if (!interpretOK)
        {
            allOK = false;
        }
        if (doPrintOutput && !interpretOK)
        {
            rOutput.append("Interpreting FAILED in: ").append(it->c_str());
            rOutput.append("\n");
        }
    }
    // Remove the last newline
    if (doPrintOutput && !rOutput.empty())
    {
        rOutput.erase(rOutput.size()-1);
    }
    return allOK;
}

bool NumHopHelper::eval(double &rValue, bool doPrintOutput, HString &rOutput)
{
    bool allOK=!mpPrivate->mExpressions.empty();
    double value=-1;
    for (list<numhop::Expression>::iterator it = mpPrivate->mExpressions.begin(); it!=mpPrivate->mExpressions.end(); ++it)
    {
        numhop::Expression &e = *it;
        bool evalOK;
        value = e.evaluate(mpPrivate->mVarStorage, evalOK);
        if (!evalOK)
        {
            allOK = false;
        }
        if (doPrintOutput)
        {
            rOutput.append("Evaluated ");
            if (evalOK)
            {
                rOutput.append("OK    : ");
            }
            else
            {
                rOutput.append("FAILED: ");
            }
            rOutput.append(e.print().c_str());
            rOutput.append("     Value: ");
            rOutput.append(to_hstring(value).c_str());
            rOutput.append("\n");
        }
    }
    // Remove the last newline
    if (doPrintOutput && !rOutput.empty())
    {
        rOutput.erase(rOutput.size()-1);
    }
    rValue = value;
    return allOK;
}

HVector<HString> NumHopHelper::extractVariableNames(const HString &expression) const
{
    numhop::Expression e(expression.c_str(), numhop::UndefinedT);
    std::set<std::string> variableNames;
    e.extractValidVariableNames(mpPrivate->mVarStorage, variableNames);
    HVector<HString> names;
    names.resize(variableNames.size());
    std::set<std::string>::iterator it;
    size_t ctr=0;
    for (it = variableNames.begin(); it != variableNames.end(); ++it) {
        names[ctr] = it->c_str();
        ++ctr;
    }
    return names;
}

HVector<HString> NumHopHelper::extractNamedValues(const HString &expression)
{
    list<string> expressions;
    numhop::extractExpressionRows(expression.c_str(), '#', expressions);

    HVector<HString> output;
    list<string>::iterator eit;
    for (eit = expressions.begin(); eit != expressions.end(); ++eit) {
        numhop::Expression e(eit->c_str(), numhop::UndefinedT);
        std::set<std::string> namedValues;
        e.extractNamedValues(namedValues);
        std::set<std::string>::iterator it;
        for (it= namedValues.begin(); it != namedValues.end(); ++it) {
            output.append(it->c_str());
        }
    }
    return output;
}

HString NumHopHelper::replaceNamedValue(const HString& expression, const HString &oldName, const HString &newName)
{
    list<string> expressions;
    numhop::extractExpressionRows(expression.c_str(), '#', expressions);

    HString output;
    list<string>::iterator eit;

    for (eit = expressions.begin(); eit != expressions.end(); ++eit) {
        numhop::Expression e(eit->c_str(), numhop::UndefinedT);
        e.replaceNamedValue(oldName.c_str(), newName.c_str());
        output.append(e.print().c_str());
        // For multi-line scripts, append line breaks
        if (expressions.size() > 1) {
            output.append("\n");
        }
    }
    return output;
}
