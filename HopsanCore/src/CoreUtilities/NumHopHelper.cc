/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

//$Id$

#include "CoreUtilities/NumHopHelper.h"
#include "ComponentSystem.h"
#include "CoreUtilities/StringUtilities.h"
#include "ComponentUtilities/num2string.hpp"

using namespace std;
using namespace hopsan;

#include "numhop.h"

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

        // First check if the name represents a system parameter in this system
        if (mpSystem->hasParameter(hname))
        {
            return mpSystem->evaluateDoubleParameter(hname, rFound);
        }
        else
        {
            double value;
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
                        value = pC->evaluateDoubleParameter(parts[1], rFound);
                        // If not found, then try to add "Value", in case user is lazy and have not specified it for input or output start values
                        if (!rFound)
                        {
                            value = pC->evaluateDoubleParameter(parts[1]+"#Value", rFound);
                        }
                    }
                    else if (parts.size() == 3)
                    {
                        value = pC->evaluateDoubleParameter(parts[1]+"#"+parts[2], rFound);
                    }
                    return value;
                }
            }
        }
        rFound = false;
        return -1;
    }

    bool setExternalValue(string name, double value)
    {
        HString hname = name.c_str();

        // First check if the name represents a system parameter in this system
        if (mpSystem->hasParameter(hname))
        {
            return mpSystem->setParameterValue(hname, to_hstring(value));
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

        double value;
        HString valstring;
        vector<HString> parts;
        splitString(hname, '.', parts);

        bool evalOK=false;
        // Check if this is a local constant, or system parameter
        if (parts.size() == 1)
        {
            mpComponent->getParameterValue(parts[0], valstring);
            // The value!=parts[0] is a hack to avoid infinite recursion when the name of the system parameter (value) is the same as the parameter name
            if (!valstring.empty() && valstring!=parts[0])
            {
                value = mpComponent->evaluateDoubleParameter(parts[0], evalOK);
            }
            // Try system parameter, value not found
            if (!evalOK && mpComponent->getSystemParent())
            {
                value = mpComponent->getSystemParent()->evaluateDoubleParameter(parts[0], evalOK);
            }
        }
        // Check if this is a local port.value pair
        else if (parts.size() == 2)
        {
            value = mpComponent->evaluateDoubleParameter(parts[0]+"#"+parts[1], evalOK);
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

        // Check if this is a local constant, or system parameter
        bool didSet = false;
        if (parts.size() == 1)
        {
            //! @todo speed this up by not looking every time (use data pointer)
            didSet = mpComponent->setParameterValue(parts[0], to_hstring(value));

            //! @todo we should not be able to set a system parameter from inside a component, but we should not allow setting an internal variable with the same name
            // Try system parameter
//            if (!didSet && mpComponent->getSystemParent() && mpComponent->getSystemParent()->hasParameter(parts[0]))
//            {
//                didSet= mpComponent->getSystemParent()->setParameterValue(parts[0], to_hstring(value));
//            }
        }
        // Check if this is a local port.value pair
        else if (parts.size() == 2)
        {
            didSet = mpComponent->setParameterValue(parts[0]+"#"+parts[1], to_hstring(value));
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
    double value;
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

