#include "CoreUtilities/NumHopHelper.h"
#include "ComponentSystem.h"
#include "CoreUtilities/StringUtilities.h"
#include "ComponentUtilities/num2string.hpp"

using namespace std;
using namespace hopsan;

#ifdef USENUMHOP
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
        HString value;

        // First check if the name represents a system parameter in this system
        if (mpSystem->hasParameter(hname))
        {
            mpSystem->getParameterValue(hname, value);
            return value.toDouble(&rFound);
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
                        pC->getParameterValue(parts[1], value);
                    }
                    else if (parts.size() == 3)
                    {
                        pC->getParameterValue(parts[1]+"#"+parts[2], value);
                    }
                    if (!value.empty())
                    {
                        return value.toDouble(&rFound);
                    }
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
                        return pC->setParameterValue(parts[1], to_hstring(value));
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

        HString value;
        vector<HString> parts;
        splitString(hname, '.', parts);

        // Check if this is a local constant, or system parameter
        if (parts.size() == 1)
        {
            mpComponent->getParameterValue(parts[0], value);
            // Try system parameter
            if (value.empty() && mpComponent->getSystemParent())
            {
                mpComponent->getSystemParent()->getParameterValue(parts[0], value);
            }
        }
        // Check if this is a local port.value pair
        else if (parts.size() == 2)
        {
            mpComponent->getParameterValue(parts[0]+"#"+parts[1], value);
        }

        if (!value.empty())
        {
            return value.toDouble(&rFound);
        }
        else
        {
            rFound = false;
            return -1;
        }
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
            // Try system parameter
            if (!didSet && mpComponent->getSystemParent() && mpComponent->getSystemParent()->hasParameter(parts[0]))
            {
                didSet= mpComponent->getSystemParent()->setParameterValue(parts[0], to_hstring(value));
            }
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

#endif

NumHopHelper::NumHopHelper()
{
    mpSystem = 0;
    mpComponent = 0;
    mpPrivate = 0;
#ifdef USENUMHOP
    mpPrivate = new NumHopHelperPrivate();
#endif

}

NumHopHelper::~NumHopHelper()
{
#ifdef USENUMHOP
    if (mpPrivate->mpHopsanAccess)
    {
        delete mpPrivate->mpHopsanAccess;
    }
    delete mpPrivate;
#endif
}

void NumHopHelper::setSystem(ComponentSystem *pSystem)
{
#ifdef USENUMHOP
    mpSystem = pSystem;

    if (mpPrivate->mpHopsanAccess)
    {
        delete mpPrivate->mpHopsanAccess;
    }
    mpPrivate->mpHopsanAccess = new HopsanSystemAccess(pSystem);
    mpPrivate->mVarStorage.setExternalStorage(mpPrivate->mpHopsanAccess);
    mpPrivate->mVarStorage.setDisallowedInternalNameCharacters(".");
#endif
}

void NumHopHelper::setComponent(Component *pComponent)
{
#ifdef USENUMHOP
    mpComponent = pComponent;

    if (mpPrivate->mpHopsanAccess)
    {
        delete mpPrivate->mpHopsanAccess;
    }
    mpPrivate->mpHopsanAccess = new HopsanComponentAccess(pComponent);
    mpPrivate->mVarStorage.setExternalStorage(mpPrivate->mpHopsanAccess);
    mpPrivate->mVarStorage.setDisallowedInternalNameCharacters(".");
#endif
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
#ifdef USENUMHOP
    if (interpretNumHopScript(script, doPrintOutput, rOutput))
    {
        if (doPrintOutput)
        {
            rOutput.append("\n");
        }
        return eval(rValue, doPrintOutput, rOutput);
    }
#else
    rOutput = "Error: NumHop is not pressent!";
#endif
    mpPrivate->mExpressions.clear();
    return false;
}

bool NumHopHelper::interpretNumHopScript(const HString &script, bool doPrintOutput, HString &rOutput)
{
#ifdef USENUMHOP
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
    if (doPrintOutput)
    {
        rOutput.erase(rOutput.size()-1);
    }
    return allOK;
#else
    rOutput = "Error: NumHop is not pressent!";
    return false;
#endif
}

bool NumHopHelper::eval(double &rValue, bool doPrintOutput, HString &rOutput)
{
#ifdef USENUMHOP
    bool allOK=true;
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
    if (doPrintOutput)
    {
        rOutput.erase(rOutput.size()-1);
    }
    rValue = value;
    return allOK;
#else
    rOutput = "Error: NumHop is not pressent!";
    return false;
#endif
}

