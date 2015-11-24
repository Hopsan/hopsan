#include "CoreUtilities/NumHopHelper.h"
#include "ComponentSystem.h"
#include "CoreUtilities/StringUtilities.h"
#include "ComponentUtilities/num2string.hpp"

using namespace std;
using namespace hopsan;

#ifdef USENUMHOP
#include "numhop.h"

class HopsanParameterAccess : public numhop::ExternalVariableStorage
{
public:
    HopsanParameterAccess(ComponentSystem *pSystem)
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
            if (parts.size() == 3)
            {
                Component* pC = mpSystem->getSubComponent(parts[0]);
                if (pC)
                {
                    pC->getParameterValue(parts[1]+"#"+parts[2], value);
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
            if (parts.size() == 3)
            {
                Component* pC = mpSystem->getSubComponent(parts[0]);
                if (pC)
                {
                    return pC->setParameterValue(parts[1]+"#"+parts[2], to_hstring(value));
                }
            }
        }
        return false;
    }

private:
    ComponentSystem *mpSystem;
};

namespace hopsan {

class NumHopHelperPrivate
{
public:
    NumHopHelperPrivate() : mpHopsanAccess(0) {}
    numhop::VariableStorage mVarStorage;
    HopsanParameterAccess *mpHopsanAccess;
};

}

#endif

NumHopHelper::NumHopHelper()
{
    mpSystem = 0;
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
    mpPrivate->mpHopsanAccess = new HopsanParameterAccess(pSystem);
    mpPrivate->mVarStorage.setExternalStorage(mpPrivate->mpHopsanAccess);
    mpPrivate->mVarStorage.setDissallowedInternalNameCharacters(".");
#endif
}

bool NumHopHelper::evalNumHopScript(const HString &script, bool doPrintOutput, HString &rOutput)
{
#ifdef USENUMHOP
    list<string> expressions;
    numhop::extractExpressionRows(script.c_str(), '#', expressions);

    bool allOK=true;
    for (list<string>::iterator it = expressions.begin(); it!=expressions.end(); ++it)
    {
        numhop::Expression2 e;
        numhop::interpretExpressionStringRecursive2(*it, e);
        bool evalOK;
        double value = e.evaluate(mpPrivate->mVarStorage, evalOK);
        if (doPrintOutput)
        {
            rOutput.append("Evaluated ");
            if (evalOK)
            {
                rOutput.append("OK    : ");
            }
            else
            {
                allOK=false;
                rOutput.append("FAILED: ");
            }
            rOutput.append(e.print().c_str());
            rOutput.append("     Value: ");
            rOutput.append(to_hstring(value).c_str());
            rOutput.append("\n");
        }
    }
    return allOK;
#else
    rOutput = "Error: NumHop is not pressent!";
    return false;
#endif
}

