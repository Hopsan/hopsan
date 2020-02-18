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
//! @file   HmfLoader.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2011-03-20
//!
//! @brief Contains the HopsanCore hmf loader functions
//!
//$Id$

#include <iostream>
#include <cassert>
#include <cstring>
#include "CoreUtilities/HmfLoader.h"
#include "CoreUtilities/HopsanCoreMessageHandler.h"
#include "CoreUtilities/NumHopHelper.h"
#include "ComponentUtilities/num2string.hpp"
#include "HopsanEssentials.h"
#include "CoreUtilities/StringUtilities.h"
#include "HopsanCoreVersion.h"

#include "hopsan_rapidxml.hpp"

using namespace std;
using namespace hopsan;


// vvvvvvvvvv Help Functions vvvvvvvvvv
namespace oldversionformat{

int getGenerationVersion(const HString &version)
{
    HString tempStr;
    for(size_t i=0; i<version.size() && version.at(i) != '.'; ++i)
    {
        tempStr.append(version.at(i));
    }

    bool dummy;
    return tempStr.toLongInt(&dummy);
}

int getMajorVersion(const HString &version)
{
    HString tempStr;
    size_t i;
    for(i=0; i<version.size() && version.at(i) != '.'; ++i) {}
    for(++i; i<version.size() && version.at(i) != '.'; ++i)
    {
        tempStr.append(version.at(i));
    }

    bool dummy;
    return tempStr.toLongInt(&dummy);
}

int getMinorVersion(const HString &version)
{
    HString tempStr;
    size_t i;
    for(i=0; i<version.size() && version.at(i) != '.'; ++i) {}
    for(++i; i<version.size() && version.at(i) != '.'; ++i) {}
    for(++i; i<version.size() && version.at(i) != 'x'; ++i)
    {
        tempStr.append(version.at(i));
    }

    bool dummy;
    if(tempStr == "")
        return -1;

    return tempStr.toLongInt(&dummy);
}


char getHotfixLetter(const HString &version)
{
    HString tempStr;
    size_t i;
    for(i=0; i<version.size() && version.at(i) != '.'; ++i) {}
    for(++i; i<version.size() && version.at(i) != '.'; ++i) {}
    for(++i; i<version.size() && version.at(i) != 'x'; ++i)
    {
        if(!isdigit(version.at(i)))
            tempStr.append(version.at(i));
    }

    if(tempStr.size() > 1)
        return ' ';

    return tempStr.at(0);
}


int getRevisionNumber(const HString &version)
{
    HString tempStr;
    size_t i;
    for(i=1; i<version.size() && version.at(i-1) != '_' && version.at(i) != 'r'; ++i) {}
    for(++i; i<version.size(); ++i)
    {
        tempStr.append(version.at(i));
    }

    bool dummy;
    if(tempStr == "")
        return -1;

    return tempStr.toLongInt(&dummy);
}


bool isVersionGreaterThan(HString version1, HString version2)
{
    int gen1 = oldversionformat::getGenerationVersion(version1);
    int gen2 = oldversionformat::getGenerationVersion(version2);
    int maj1 = oldversionformat::getMajorVersion(version1);
    int maj2 = oldversionformat::getMajorVersion(version2);
    int min1 = oldversionformat::getMinorVersion(version1);
    int min2 = oldversionformat::getMinorVersion(version2);
    char letter1 = getHotfixLetter(version1);
    char letter2 = getHotfixLetter(version2);
    int rev1 = getRevisionNumber(version1);
    int rev2 = getRevisionNumber(version2);

    if(gen1 > gen2)
        return true;
    if(gen1 < gen2)
        return false;

    if(maj1 > maj2)
        return true;
    if(maj1 < maj2)
        return false;

    // Assume that revision build is higher generation than release builds,
    // this is a completely absurd assumtion (but this is legacy code)
    if( (min1 == -1) && (min2 > -1) )
        return true;
    if( (min1 > -1) && (min2 == -1) )
        return false;

    if(min1 > min2)
        return true;
    if(min1 < min2)
        return false;

    if(letter1 > letter2)
        return true;
    if(letter1 < letter2)
        return false;

    if(rev1 > rev2)
        return true;
    if(rev1 < rev2)
        return false;

    return false;
}

}

namespace {

//! @brief Helpfunction to strip filename from path
HString stripFilenameFromPath(HString filePath)
{
    size_t pos = filePath.rfind('/');
    // On windows, also check for backslash in path, if backslash is found use that
#ifdef _WIN32
    size_t pos_bs = filePath.rfind('\\');
    if (pos_bs != HString::npos)
    {
        pos = pos_bs;
    }
#endif
    if (pos != HString::npos)
    {
        filePath.erase(pos+1);
    }
    return filePath;
}

//! @brief Splits a full name into comp port and variable names
//! @todo this should be in a more "global" place since it may be usefull elsewhere
void splitFullName(const HString &rFullName, HString &rCompName, HString &rPortName, HString &rVarName)
{
    vector<HString> parts;
    parts.reserve(3);

    size_t start=0, end=0;
    while (end != HString::npos)
    {
        end = rFullName.find_first_of('#', start);
        parts.push_back(rFullName.substr(start, end-start));
        start = end+1;
        if (end == HString::npos)
        {
            break;
        }
    }

    if (parts.size() == 1)
    {
        rCompName.clear();
        rPortName.clear();
        rVarName = parts[0];
    }
    else if (parts.size() == 3)
    {
        rCompName = parts[0];
        rPortName = parts[1];
        rVarName = parts[2];
    }
    else
    {
        rCompName.clear();
        rPortName.clear();
        rVarName = "ERROR_in_splitFullName()";
    }
}

void updateOldModelFileParameter(rapidxml::xml_node<> *pParameterNode, const HString &rHmfCoreVersion)
{
    if (isVersionAGreaterThanB("0.6.0", rHmfCoreVersion) || rHmfCoreVersion.containes("0.6.x_r") )
    {
        if (pParameterNode)
        {
            // Fix renamed node data vaariables
            HString name = readStringAttribute(pParameterNode,"name","").c_str();
            name.replace("::","#"); //!< @todo remove this after 0.7 (it is used to update models prior to 0.6
            if (name.containes("#"))
            {
                // split string
                HString part1 = name.substr(0, name.rfind(':')+1);
                HString part2 = name.substr(name.rfind(':')+1);

                if (part2 == "Angular Velocity")
                {
                    part2 = "AngularVelocity";
                }
                else if (part2 == "Equivalent Inertia")
                {
                    part2 = "EquivalentInertia";
                }
                else if (part2 == "CharImp")
                {
                    part2 = "CharImpedance";
                }
                writeStringAttribute(pParameterNode, "name", (part1+part2).c_str());
            }

            // Fix parameter names with illegal chars
            if (!isNameValid(name.c_str()))
            {
                if (name == "sigma^2")
                {
                    name = "std_dev";
                }

                name.replace(",","");
                name.replace(".","");
                name.replace(" ","_");

                writeStringAttribute(pParameterNode, "name", name.c_str());
            }
        }
    }
}

void updateOldModelFileComponent(rapidxml::xml_node<> */*pComponentNode*/, const HString &/*rHmfCoreVersion*/)
{
    // Typos (no specific version)

}

void updateRenamedComponentType(rapidxml::xml_node<> *pNode, const string &rOldType, const string &rNewType)
{
    if(readStringAttribute(pNode, "typename", "") == rOldType)
    {
        writeStringAttribute(pNode, "typename", rNewType);
    }
}

void updateRenamedPort(rapidxml::xml_node<> *pNode, const string &rComponentType, const HString &rOldName, const HString &rNewName)
{
    if(readStringAttribute(pNode, "typename", "") == rComponentType)
    {
        // Rename startvalue parameters
        rapidxml::xml_node<> *pParamNode = getGrandChild(pNode, "parameters", "parameter");
        while (pParamNode)
        {
            HString paramName = readStringAttribute(pParamNode, "name").c_str();
            if (paramName.containes(rOldName+"#"))
            {
                paramName.replace(rOldName+"#", rNewName+"#");
                writeStringAttribute(pParamNode, "name", paramName.c_str());
            }
            pParamNode = pParamNode->next_sibling("parameter");
        }

        // Now try to find all connections, and replace portname
        string compName = readStringAttribute(pNode, "name");
        rapidxml::xml_node<> *pConnNode = getGrandChild(pNode->parent()->parent(),"connections","connect");
        while (pConnNode)
        {
            string startComp = readStringAttribute(pConnNode, "startcomponent");
            string endComp = readStringAttribute(pConnNode, "endcomponent");

            if (startComp == compName)
            {
                if (readStringAttribute(pConnNode, "startport").c_str() == rOldName)
                {
                    writeStringAttribute(pConnNode, "startport", rNewName.c_str());
                }
            }
            if (endComp == compName)
            {
                if (readStringAttribute(pConnNode, "endport").c_str() == rOldName)
                {
                    writeStringAttribute(pConnNode, "endport", rNewName.c_str());
                }
            }

            pConnNode = pConnNode->next_sibling("connect");
        }
    }
}

void updateRenamedParameter(rapidxml::xml_node<> *pNode, const string &rComponentType, const string &rOldName, const string &rNewName)
{
    if(readStringAttribute(pNode,"typename") == rComponentType)
    {
        rapidxml::xml_node<> *pParamNode = getGrandChild(pNode, "parameters", "parameter");
        while (pParamNode)
        {
            if (readStringAttribute(pParamNode, "name") == rOldName)
            {
                writeStringAttribute(pParamNode, "name", rNewName);
            }
            pParamNode = pParamNode->next_sibling("parameter");
        }
    }
}


size_t loadValuesFromHopsanParameterFile(rapidxml::xml_node<> *pComponentNode, Component *pComponent)
{
    size_t numUpdated = 0;
    try {
        if (pComponentNode != 0) {
            bool typenameMatch = (pComponent->getTypeName() == readStringAttribute(pComponentNode, "typename").c_str()) &&
                    (pComponent->getSubTypeName() == readStringAttribute(pComponentNode, "subtypename").c_str());
            if (typenameMatch) {
                rapidxml::xml_node<> *pParameters = pComponentNode->first_node("parameters");
                if (pParameters != 0) {
                    rapidxml::xml_node<> *pParameter = pParameters->first_node("parameter");
                    while (pParameter != 0) {

                        const HString name = readStringAttribute(pParameter, "name", "").c_str();
                        const HString newValue = readStringAttribute(pParameter, "value", "").c_str();

                        HString currentValue;
                        pComponent->getParameterValue(name, currentValue);
                        if (newValue != currentValue) {
                            bool setOK = pComponent->setParameterValue(name, newValue);
                            if (setOK) {
                                HString message = "Parameter: "+name+" was updated from "+currentValue+" to "+newValue;
                                pComponent->addInfoMessage(message, "ParameterUpdated");
                                ++numUpdated;
                            }
                            else {
                                HString message = "Parameter: "+name+" could not be updated";
                                pComponent->addWarningMessage(message, "ParameterUpdated");
                            }
                        }

                        pParameter = pParameter->next_sibling("parameter");
                    }
                }
            }
            else {
                addCoreLogMessage("hopsan::loadHopsanParameterFile(): Component type mismatch.");
                pComponent->addErrorMessage("Component type mismatch when loading parameter values.");
            }
        }
    }
    catch(std::exception &e) {
        const HString msg = HString("hopsan::loadFromHopsanParameterFile(): Error reading file: ")+e.what();
        addCoreLogMessage(msg);
        pComponent->addErrorMessage(msg);
    }
    return numUpdated;
}

size_t loadValuesFromHopsanParameterFile(rapidxml::xml_node<> *pSystemNode, ComponentSystem *pSystem)
{
    size_t numUpdated = 0;
    try {
        if (pSystemNode != 0) {
            bool typenameMatch = (pSystem->getTypeName() == readStringAttribute(pSystemNode, "typename").c_str()) &&
                    (pSystem->getSubTypeName() == readStringAttribute(pSystemNode, "subtypename").c_str());
            if (typenameMatch) {
                // Load systems own parameters
                numUpdated += loadValuesFromHopsanParameterFile(pSystemNode, static_cast<Component*>(pSystem));

                // Load contained components and subsystems recursively
                rapidxml::xml_node<> *pXmlObjects = pSystemNode->first_node("objects");
                if (pXmlObjects) {
                    rapidxml::xml_node<> *pXmlComponentOrSystem = pXmlObjects->first_node();
                    while (pXmlComponentOrSystem != 0) {
                        char* elementName = pXmlComponentOrSystem->name();
                        // Ignore non system or components, eg. systemports or any user added data
                        if (strcmp(elementName, "component")==0 || strcmp(elementName, "system")==0) {

                            HString name = readStringAttribute(pXmlComponentOrSystem, "name", "").c_str();

                            if (strcmp(pXmlComponentOrSystem->name(), "system") == 0) {
                                ComponentSystem* pSubSystem = pSystem->getSubComponentSystem(name);
                                if (pSubSystem) {
                                    numUpdated += loadValuesFromHopsanParameterFile(pXmlComponentOrSystem, pSubSystem);
                                }
                                else {
                                    addCoreLogMessage("hopsan::loadHopsanParameterFile(): Subsystem not found: "+name);
                                    pSystem->addErrorMessage("Subsystem not found: "+name);
                                }
                            }
                            else {
                                Component* pSubComponent = pSystem->getSubComponent(name);
                                if (pSubComponent) {
                                    numUpdated += loadValuesFromHopsanParameterFile(pXmlComponentOrSystem, pSubComponent);
                                }
                                else {
                                    addCoreLogMessage("hopsan::loadHopsanParameterFile(): Subcomponent not found: "+name);
                                    pSystem->addErrorMessage("Subcomponent not found: "+name);
                                }
                            }
                        }

                        pXmlComponentOrSystem = pXmlComponentOrSystem->next_sibling();
                    }
                }
            }
            else {
                addCoreLogMessage("hopsan::loadHopsanParameterFile(): System type mismatch.");
                pSystem->addErrorMessage("System type mismatch when loading parameter values.");
            }
        }
    }
    catch(std::exception &e) {
        const HString msg = HString("hopsan::loadFromHopsanParameterFile(): Error reading file: ")+e.what();
        addCoreLogMessage(msg);
        pSystem->addErrorMessage(msg);
    }
    return numUpdated;
}

}

//! @brief Go through all parameter values an prepend self# to parameter names in expressions if they point to a local parameter
void hopsan::autoPrependSelfToParameterExpressions(Component* pComponent) {

    std::vector<HString> localParameterNames;
    pComponent->getParameterNames(localParameterNames);
    const std::vector<ParameterEvaluator*> *pParameters = pComponent->getParametersVectorPtr();
    for (size_t i=0; i<pParameters->size(); ++i) {
        ParameterEvaluator* pParameter = (*pParameters)[i];
        // Only double parameters can have expressions
        if ((pParameter->getType() == "double") && !pParameter->getValue().isNummeric()) {
            HString parameterValueExpression = pParameter->getValue();
            // Extract all named values from the parameter expression
            HVector<HString> namedValues = NumHopHelper::extractNamedValues(parameterValueExpression);
            bool needReplaceParameterValue = false;
            for (size_t j=0; j<namedValues.size(); ++j) {
                HString fixedNamedValue = namedValues[j];
                fixedNamedValue.replace('.', '#'); // Replace . with # since you can not enter # in parameter input dialog (since is a separator char that can not be in given names)
                // If a named value from the expression exists as a parameter name in the current component, prepend self. to the name
                if (std::find(localParameterNames.begin(), localParameterNames.end(), fixedNamedValue) != localParameterNames.end()) {
                    parameterValueExpression = NumHopHelper::replaceNamedValue(parameterValueExpression, namedValues[j], "self."+namedValues[j]);
                    needReplaceParameterValue = true;
                }
            }
            // If any parameter name was changed, then set the new parameter value expression
            if (needReplaceParameterValue) {
                const bool force = true;
                pParameter->setParameterValue(parameterValueExpression, 0, force);
            }
        }
    }
}


//! @brief This help function loads a component
void loadComponent(rapidxml::xml_node<> *pComponentNode, ComponentSystem* pSystem, HopsanEssentials *pHopsanEssentials)
{
    HString typeName = readStringAttribute(pComponentNode, "typename", "ERROR_NO_TYPE_GIVEN").c_str();
    HString subTypeName = readStringAttribute(pComponentNode, "subtypename", "").c_str();
    HString displayName = readStringAttribute(pComponentNode, "name", typeName.c_str()).c_str();

    bool disabled = readBoolAttribute(pComponentNode, "disabled", false);

    Component *pComp = pHopsanEssentials->createComponent(typeName.c_str());
    if (pComp != 0)
    {
        pComp->setName(displayName);
        pComp->setSubTypeName(subTypeName.c_str());
        pComp->setDisabled(disabled);
        pSystem->addComponent(pComp);

        // Load parameters
        //! @todo should be able to load parameters and system parameters with same help function
        rapidxml::xml_node<> *pParams = pComponentNode->first_node("parameters");
        if (pParams)
        {
            const HString coreVersionOfModelFile = readStringAttribute(pComponentNode->document()->first_node(), "hopsancoreversion").c_str();
            rapidxml::xml_node<> *pParam = pParams->first_node("parameter");
            while (pParam != 0)
            {
                updateOldModelFileParameter(pParam, coreVersionOfModelFile);

                HString paramName = readStringAttribute(pParam, "name", "ERROR_NO_PARAM_NAME_GIVEN").c_str();
                HString val = readStringAttribute(pParam, "value", "ERROR_NO_PARAM_VALUE_GIVEN").c_str();

                //! @todo this is a hack to update old parameters, remove at some point in the future
                if (!pComp->hasParameter(paramName))
                {
                    if (paramName.find("#") == HString::npos)
                    {
                        paramName=paramName+"#Value";
                    }
                }

                // We need force=true here to make sure that parameters with system variable names are set even if they can not yet be evaluated
                //! @todo why cant they be evaluated, if everything loaded in correct order that should work
                bool ok = pComp->setParameterValue(paramName, val, true);
                if(!ok)
                {
                    pComp->addWarningMessage("Failed to set parameter: "+paramName+"="+val);
                }

                pParam = pParam->next_sibling("parameter");
            }

            if (isVersionAGreaterThanB("2.14.0", coreVersionOfModelFile)) {
                autoPrependSelfToParameterExpressions(pComp);
            }
        }

        // Load modifyable signal quantities
        rapidxml::xml_node<> *pXmlPorts = pComponentNode->first_node("ports");
        if (pXmlPorts)
        {
            rapidxml::xml_node<> *pXmlPort = pXmlPorts->first_node("port");
            while (pXmlPort != 0)
            {
                HString quantity = readStringAttribute(pXmlPort, "signalquantity", "").c_str();
                if (!quantity.empty())
                {
                    HString portName = readStringAttribute(pXmlPort, "name", "").c_str();
                    Port *pPort = pComp->getPort(portName);
                    if (pPort)
                    {
                        pPort->setSignalNodeQuantityOrUnit(quantity);
                    }
                }
                pXmlPort = pXmlPort->next_sibling("port");
            }
        }
    }
}


//! @brief This help function loads a connection
void loadConnection(rapidxml::xml_node<> *pConnectNode, ComponentSystem* pSystem)
{
    string startcomponent = readStringAttribute(pConnectNode, "startcomponent", "ERROR_NOSTARTCOMPNAME_GIVEN");
    string startport = readStringAttribute(pConnectNode, "startport", "ERROR_NOSTARTPORTNAME_GIVEN");
    string endcomponent = readStringAttribute(pConnectNode, "endcomponent", "ERROR_NOENDCOMPNAME_GIVEN");
    string endport = readStringAttribute(pConnectNode, "endport", "ERROR_NOENDPORTNAME_GIVEN");

    santizeName(startcomponent.c_str());
    santizeName(startport.c_str());
    santizeName(endcomponent.c_str());
    santizeName(endport.c_str());

    pSystem->connect(startcomponent.c_str(), startport.c_str(), endcomponent.c_str(), endport.c_str());
}

//! @brief This help function loads a SystemPort
void loadSystemPort(rapidxml::xml_node<> *pSysPortNode, ComponentSystem* pSystem)
{
    string name = readStringAttribute(pSysPortNode, "name", "ERROR_NO_NAME_GIVEN");
    pSystem->addSystemPort(name.c_str());
}

//! @brief Help function to load system parameters
void loadSystemParameters(rapidxml::xml_node<> *pSysNode, ComponentSystem* pSystem)
{
    // Load system parameters
    rapidxml::xml_node<> *pParameters = pSysNode->first_node("parameters");
    if (pParameters)
    {
        const HString coreVersionOfModelFile = readStringAttribute(pSysNode->document()->first_node(), "hopsancoreversion").c_str();
        rapidxml::xml_node<> *pParameter = pParameters->first_node("parameter");
        while (pParameter != 0)
        {
            updateOldModelFileParameter(pParameter, coreVersionOfModelFile);

            string paramName = readStringAttribute(pParameter, "name", "ERROR_NO_PARAM_NAME_GIVEN");
            string val = readStringAttribute(pParameter, "value", "ERROR_NO_PARAM_VALUE_GIVEN");
            string type = readStringAttribute(pParameter, "type", "ERROR_NO_PARAM_TYPE_GIVEN");
            //! @todo maybe type should be data type or value type or something
            string quantityORunit = readStringAttribute(pParameter, "quantity", readStringAttribute(pParameter, "unit", ""));
            string description = readStringAttribute(pParameter, "description", "");

            // Here we use force=true to make sure system parameters load even if they do not evaluate
            //! @todo if system parameters are loaded in the correct order (top to bottom) they should evaluate, why don't they?
            bool ok = pSystem->setOrAddSystemParameter(paramName.c_str(), val.c_str(), type.c_str(), description.c_str(), quantityORunit.c_str(), true);
            if(!ok)
            {
                pSystem->addErrorMessage(HString("Failed to load parameter: ")+(paramName+"="+val).c_str());
            }

            pParameter = pParameter->next_sibling("parameter");
        }

        if (isVersionAGreaterThanB("2.14.0", coreVersionOfModelFile)) {
            autoPrependSelfToParameterExpressions(pSystem);
        }
    }
}

void loadAliases(rapidxml::xml_node<> *pAliasesNode, ComponentSystem* pSystem)
{
    rapidxml::xml_node<> *pAlias = pAliasesNode->first_node("alias");
    while(pAlias)
    {
        HString type = readStringAttribute(pAlias, "type", "ERROR_NO_ALIAS_TYPE_GIVEN").c_str();
        HString alias = readStringAttribute(pAlias, "name", "ERROR_NO_ALIAS_NAME_GIVEN").c_str();
        HString fullName = readStringNodeValue(pAlias->first_node("fullname"), "ERROR_NO_FULLNAME_GIVEN").c_str();

        HString comp, port, var;
        splitFullName(fullName, comp, port, var);
        //cout << "splitOut: " << fullName << " ! " << comp << " ! " << port << " ! " << var << endl;

        if (type == "variable" || type == "Variable")
        {
            //! @todo check bool and display warning if false
            pSystem->getAliasHandler().setVariableAlias(alias.c_str(), comp.c_str(), port.c_str(), var.c_str());
        }


        pAlias = pAlias->next_sibling("alias");
    }
}


//! @brief This function loads a subsystem
void loadSystemContents(rapidxml::xml_node<> *pSysNode, ComponentSystem* pSystem, HopsanEssentials* pHopsanEssentials, const HString rootFilePath="")
{
    string typeName = readStringAttribute(pSysNode, "typename", "ERROR_NO_TYPE_GIVEN");
    string displayName = readStringAttribute(pSysNode, "name", typeName );
    pSystem->setName(displayName.c_str());

    bool componentDisabled = readBoolAttribute(pSysNode, "disabled", false);
    pSystem->setDisabled(componentDisabled);

    rapidxml::xml_node<> *pSimtimeNode = pSysNode->first_node("simulationtime");
    double Ts = readDoubleAttribute(pSimtimeNode, "timestep", 0.001);
    pSystem->setDesiredTimestep(Ts);
    pSystem->setInheritTimestep(readBoolAttribute(pSimtimeNode,"inherit_timestep",true));

    // Load number of log samples
    rapidxml::xml_node<> *pLogSettingsNode = pSysNode->first_node("simulationlogsettings");
    pSystem->setLogStartTime(readDoubleAttribute(pLogSettingsNode, "starttime", pSystem->getLogStartTime()));
    pSystem->setNumLogSamples(readIntAttribute(pLogSettingsNode, "numsamples", pSystem->getNumLogSamples()));
    //! @deprecated 20131002 keep this old way of loading for a while for backwards compatibility
    if(hasAttribute(pSysNode,  "logsamples"))
    {
        pSystem->setNumLogSamples(readIntAttribute(pSysNode, "logsamples", pSystem->getNumLogSamples()));
    }

    //! @todo we really need defines for allof these "strings"

    // Load system parameters (needed before objects are loaded as they may be using sys-parameters)
    loadSystemParameters(pSysNode, pSystem);

    // Load NumHop script
    pSystem->setNumHopScript(readStringNodeValue(pSysNode->first_node("numhopscript"), "").c_str());

    // Load contents
    rapidxml::xml_node<> *pObjects = pSysNode->first_node("objects");
    if (pObjects)
    {
        rapidxml::xml_node<> *pObject = pObjects->first_node();
        while (pObject != 0)
        {
            if (strcmp(pObject->name(), "component")==0)
            {
                updateOldModelFileComponent(pObject, readStringAttribute(pObject->document()->first_node(), "hopsancoreversion", "").c_str());
                loadComponent(pObject, pSystem, pHopsanEssentials);
            }
            else if (strcmp(pObject->name(), "system")==0)
            {
                bool isExternal = hasAttribute(pObject, "external_path");
                ComponentSystem* pSys;

                if (isExternal)
                {
                    double dummy1,dummy2;
                    HString externalPath = stripFilenameFromPath(rootFilePath) + readStringAttribute(pObject,"external_path","").c_str();
                    cout << "externalPath: " << externalPath.c_str() << endl;
                    pSys = loadHopsanModelFile(externalPath, pHopsanEssentials, dummy1, dummy2);
                    if (pSys != 0)
                    {
                        // Add new system to parent
                        pSystem->addComponent(pSys);
                        // load overwriten parameter values
                        loadSystemParameters(pObject, pSys);
                        // Overwrite name
                        string displayNameExt = readStringAttribute(pObject, "name", typeName );
                        pSys->setName(displayNameExt.c_str());
                        // Make sure system knows its an externally loaded system
                        pSys->setExternalModelFilePath(readStringAttribute(pObject,"external_path","").c_str());
                    }
                }
                else
                {
                    // Get the typename for this new subsystem
                    string newTypeName = readStringAttribute(pObject, "typename", "UNSUPORTED_SYSTEM_TYPENAME");
                    // Create the appropriate subsystem
                    if (newTypeName == HOPSAN_BUILTIN_TYPENAME_CONDITIONALSUBSYSTEM)
                    {
                        pSys = pHopsanEssentials->createConditionalComponentSystem();
                    }
                    else if (newTypeName == HOPSAN_BUILTIN_TYPENAME_SUBSYSTEM)
                    {
                        pSys = pHopsanEssentials->createComponentSystem();
                    }
                    else
                    {
                        //! @todo don't know how to report this error, but it is unlikely to happen
                        return;
                    }
                    // Add new system to parent
                    pSystem->addComponent(pSys);
                    // Load system contents
                    loadSystemContents(pObject, pSys, pHopsanEssentials, rootFilePath);
                }
            }
            else if (strcmp(pObject->name(), "systemport")==0)
            {
                loadSystemPort(pObject,pSystem);
            }

            pObject = pObject->next_sibling();
        }
    }

    // Load connections
    rapidxml::xml_node<> *pConnections = pSysNode->first_node("connections");
    if (pConnections)
    {
        rapidxml::xml_node<> *pConnection = pConnections->first_node();
        while (pConnection != 0)
        {
            if (strcmp(pConnection->name(), "connect")==0)
            {
                loadConnection(pConnection, pSystem);
            }
            pConnection = pConnection->next_sibling();
        }
    }

    // Load system parameters again in case we have c-component subsystems with startvalues
    //! @todo this is an ugly hack to be forced to load again
    loadSystemParameters(pSysNode, pSystem);

    // Load aliases
    rapidxml::xml_node<> *pAliases = pSysNode->first_node("aliases");
    if (pAliases)
    {
        loadAliases(pAliases, pSystem);
    }
}

// The actual model load function
ComponentSystem* loadHopsanModelFileActual(const rapidxml::xml_document<> &rDoc, const HString &rFilePath, HopsanEssentials* pHopsanEssentials, double &rStartTime, double &rStopTime)
{
    try
    {
        rapidxml::xml_node<> *pRootNode = rDoc.first_node();

        //Check for correct root node name
        if (strcmp(pRootNode->name(), "hopsanmodelfile")==0)
        {
            // Check version
            HString savedwithcoreversion = readStringAttribute(pRootNode, "hopsancoreversion", "0").c_str();
            pHopsanEssentials->getCoreMessageHandler()->addDebugMessage("Model saved with core version: " + savedwithcoreversion);
            if (isVersionAGreaterThanB("0.6.0", savedwithcoreversion) || (isVersionAGreaterThanB(savedwithcoreversion, "0.6.x") && isVersionAGreaterThanB("0.6.x_r5500", savedwithcoreversion)))
            {
                pHopsanEssentials->getCoreMessageHandler()->addErrorMessage("This hmf model was saved with HopsanCoreVersion: "+savedwithcoreversion+". This old version is not supported by the HopsanCore hmf loader, resave the model with HopsanGUI");
                return 0;
            }


            rapidxml::xml_node<> *pSysNode = pRootNode->first_node("system");
            if (pSysNode != 0)
            {
                //! @todo more error check
                //We only want to read toplevel simulation time settings here
                rapidxml::xml_node<> *pSimtimeNode = pSysNode->first_node("simulationtime");
                rStartTime = readDoubleAttribute(pSimtimeNode, "start", 0);
                rStopTime = readDoubleAttribute(pSimtimeNode, "stop", 2);
                ComponentSystem * pSys = pHopsanEssentials->createComponentSystem(); //Create root system
                loadSystemContents(pSysNode, pSys, pHopsanEssentials, rFilePath);

                pSys->addSearchPath(stripFilenameFromPath(rFilePath));
                return pSys;
            }
            else
            {
                addCoreLogMessage("hopsan::loadHopsanModelFileActual(): No system found in file.");
                pHopsanEssentials->getCoreMessageHandler()->addErrorMessage(rFilePath+" Has no system to load");
            }
        }
        else
        {
            addCoreLogMessage("hopsan::loadHopsanModelFileActual(): Wrong root tag name.");
            pHopsanEssentials->getCoreMessageHandler()->addErrorMessage(rFilePath+" Has wrong root tag name: "+pRootNode->name());
            cout << "Not correct hmf file root node name: " << pRootNode->name() << endl;
        }
    }
    catch(std::exception &e)
    {
        addCoreLogMessage("hopsan::loadHopsanModelFileActual(): Unable to parse xml doc.");
        pHopsanEssentials->getCoreMessageHandler()->addErrorMessage("Unable to parse xml doc");
        cout << "throws: " << e.what() << endl;
    }

    addCoreLogMessage("hopsan::loadHopsanModelFileActual(): Failed.");

    // We failed, return 0 ptr
    return 0;
}


// vvvvvvvvvv The public function vvvvvvvvvv

int hopsan::getEpochVersion(const HString& version)
{
  bool ok = false;
  int epoch = -1;
  HVector<HString> parts = version.split('.');
  if (!parts.empty())
  {
    epoch = parts[0].toLongInt(&ok);
  }
  return ok ? epoch : -1;
}

int hopsan::getMajorVersion(const HString& version)
{
  bool ok = false;
  int major = -1;
  HVector<HString> parts = version.split('.');
  if (parts.size() > 1)
  {
    major = parts[1].toLongInt(&ok);
  }
  return ok ? major : -1;
}

int hopsan::getMinorVersion(const HString& version)
{
  bool ok = false;
  int minor = -1;
  HVector<HString> parts = version.split('.');
  if (parts.size() > 2)
  {
    minor = parts[2].toLongInt(&ok);
  }
  return ok ? minor : -1;
}

//! @brief Check if one Hopsan version number is larger then an other
//! @param [in] versionA The version to check
//! @param [in] versionB The version to compare to
//! @returns true if versionA > versionB
bool hopsan::isVersionAGreaterThanB(const HString& versionA, const HString& versionB)
{
  return compareHopsanVersions(versionA, versionB) > 0;
}

//! @brief Compare two Hopsan version numbers
//! @param [in] versionA The first version number to check
//! @param [in] versionB The second version number to check
//! @returns 1 if versionA > versionB, 0 if versionA == versionB, -1 if versionA < versionB
int hopsan::compareHopsanVersions(const HString& versionA, const HString& versionB)
{
  HVector<HString> parts1, parts2;
  HString branch1, branch2;
  //! @todo Maybe ~branchname should not really be part of the version number but only the release name
  parts1 = versionA.split('~');
  parts2 = versionB.split('~');
  if (parts1.size() > 1)
  {
    branch1 = parts1[1];
  }
  if (parts2.size() > 1)
  {
    branch2 = parts2[1];
  }
  parts1 = parts1[0].split('.');
  parts2 = parts2[0].split('.');

  size_t minSize = std::min(parts1.size(), parts2.size());
  for (size_t i=0; i<minSize; ++i)
  {
    bool ok1,ok2;
    long int v1 = parts1[i].toLongInt(&ok1);
    long int v2 = parts2[i].toLongInt(&ok2);
    if (ok1 && ok2) {
        if (v1 > v2)
        {
            return 1;
        }
        else if (v1 < v2)
        {
            return -1;
        }
    }

    // Handle comparison of the old version number format
    if ( (i==0 && v1==0) || !(ok1 && ok2))
    {
      return oldversionformat::isVersionGreaterThan(versionA, versionB);
    }
  }
  // If we get here, then the numbers are the same up until minSize

  // Treat the shortest one as "larger", it probalby indicates a "stable" release
  if (parts1.size() < parts2.size())
  {
    return 1;
  }
  else if (parts1.size() > parts2.size())
  {
    return -1;
  }

  //! @todo Compare branchnames, but this will likely never be required
  return 0;
}


//! @brief This function is used to load a HMF file.
//! @param [in] filePath The name (path) of the HMF file
//! @param [out] rStartTime A reference to the starttime variable
//! @param [out] rStopTime A reference to the stoptime variable
//! @returns A pointer to the rootsystem of the loaded model
//! @todo if possible merge the two differen main load functions
ComponentSystem* hopsan::loadHopsanModelFile(const HString &rFilePath, HopsanEssentials* pHopsanEssentials, double &rStartTime, double &rStopTime)
{
    addCoreLogMessage("hopsan::loadHopsanModelFile("+rFilePath+")");
    try
    {
        rapidxml::file<> hmfFile(rFilePath.c_str());
        rapidxml::xml_document<> doc;
        doc.parse<0>(hmfFile.data());

        return loadHopsanModelFileActual(doc, rFilePath, pHopsanEssentials, rStartTime, rStopTime);
    }
    catch(std::exception &e)
    {
        addCoreLogMessage("hopsan::loadHopsanModelFile(): Unable to open file.");
        pHopsanEssentials->getCoreMessageHandler()->addErrorMessage("Could not open file: "+rFilePath);
        cout << "Could not open file, throws: " << e.what() << endl;
    }
    addCoreLogMessage("hopsan::loadHopsanModelFile(): Failed.");
    // We failed, return 0 ptr
    return 0;
}


//! @brief This function is used to load a HMF file from model string.
//! @param [in] xmlModel The xml representation of the model
//! @returns A pointer to the rootsystem of the loaded model
ComponentSystem* hopsan::loadHopsanModel(std::vector<unsigned char> xmlVector, HopsanEssentials* pHopsanEssentials)
{
    double start,stop;
    return loadHopsanModelFile((char*) &xmlVector[0], pHopsanEssentials, start, stop);
}


//! @brief This function is used to load a HMF file from model string.
//! @param [in] xmlModel The xml representation of the model
//! @returns A pointer to the rootsystem of the loaded model
ComponentSystem* hopsan::loadHopsanModel(const char* xmlStr, HopsanEssentials* pHopsanEssentials, double &rStartTime, double &rStopTime)
{
    // Rapid xml requires a non-const char* to work with, we need to copy the text
    char *str = new char[strlen(xmlStr)+1];
    strcpy(str, xmlStr);
    ComponentSystem *pSystem = loadHopsanModel(str, pHopsanEssentials, rStartTime, rStopTime);
    delete [] str;
    return pSystem;
}


ComponentSystem* hopsan::loadHopsanModel(char* xmlStr, HopsanEssentials* pHopsanEssentials, double &rStartTime, double &rStopTime)
{
    try
    {
        rapidxml::xml_document<> doc;
        doc.parse<0>( xmlStr);

        return loadHopsanModelFileActual(doc, "", pHopsanEssentials, rStartTime, rStopTime);
    }
    catch(std::exception &e)
    {
        pHopsanEssentials->getCoreMessageHandler()->addErrorMessage("Could not parse xml string");
        cout << "throws: " << e.what() << endl;
    }

    // We failed, return 0 ptr
    return 0;
}




//! @brief This function is used to load a Hopsan Parameter File (HPF).
//! @param [in] filePath The file path to the HPF file
//! @param [in] pMessageHandler The Hopsan Core message handler
//! @param [in] pSystem The top-level system in the model to load parameters for
//! @return Number of changed parameters
size_t hopsan::loadHopsanParameterFile(const HString &filePath, HopsanCoreMessageHandler *pMessageHandler, Component *pComponentOrSystem)
{
    size_t numUpdated = 0;
    addCoreLogMessage("hopsan::loadHopsanParameterFile("+filePath+")");
    try
    {
        rapidxml::file<> hmfFile(filePath.c_str());

        rapidxml::xml_document<> doc;
        doc.parse<0>(hmfFile.data());

        rapidxml::xml_node<> *pRootNode = doc.first_node();

        //Check for correct root node name
        if (strcmp(pRootNode->name(), "hopsanparameterfile")==0) {

            if (pComponentOrSystem->isComponentSystem()) {
                rapidxml::xml_node<> *pSysNode = pRootNode->first_node("system");
                if (pSysNode != 0) {
                    ComponentSystem* pSystem = dynamic_cast<ComponentSystem*>(pComponentOrSystem);
                    numUpdated = loadValuesFromHopsanParameterFile(pSysNode, pSystem);
                }
                else {
                    addCoreLogMessage("hopsan::loadHopsanParameterFile(): No system element found in file.");
                    pMessageHandler->addErrorMessage(filePath+" Has no system element to load");
                }
            }
            else {
                rapidxml::xml_node<> *pComponentNode = pRootNode->first_node("component");
                if (pComponentNode != 0) {
                    numUpdated = loadValuesFromHopsanParameterFile(pComponentNode, pComponentOrSystem);
                }
                else {
                    addCoreLogMessage("hopsan::loadHopsanParameterFile(): No component element found in file.");
                    pMessageHandler->addErrorMessage(filePath+" Has no system element to load");
                }
            }
            pMessageHandler->addInfoMessage("Updated: "+to_hstring(numUpdated)+" parameters from File: "+filePath);

        }
        else {
            addCoreLogMessage("hopsan::loadHopsanParameterFile(): Wrong root tag name.");
            pMessageHandler->addErrorMessage(filePath+" Has wrong root tag name: "+pRootNode->name());
        }
    }
    catch(std::exception &e)
    {
        addCoreLogMessage(HString("hopsan::loadHopsanParameterFile(): Unable to open file: ")+e.what());
        pMessageHandler->addErrorMessage(HString("Exception: ")+e.what()+", Could not open file: "+filePath);
    }
    return numUpdated;
}

