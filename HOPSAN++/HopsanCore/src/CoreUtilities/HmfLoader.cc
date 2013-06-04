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
//! @file   HmfLoader.cc
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
#include "HopsanEssentials.h"
#include "CoreUtilities/StringUtilities.h"
#include "version.h"

#include "hopsan_rapidxml.hpp"

using namespace std;
using namespace hopsan;

// vvvvvvvvvv Help Functions vvvvvvvvvv
//! @brief Helpfunction to strip filename from path
//! @note Assumes that dir separator is forward slash /
HString stripFilenameFromPath(HString filePath)
{
    //cout << "Stripping from: " << filePath << endl;
    size_t pos = filePath.rfind('/');
    if (pos != HString::npos)
    {
        filePath.erase(pos+1);
    }
    //cout << "Stripped: " << filePath << endl;
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
    if (rHmfCoreVersion < "0.6.0" || rHmfCoreVersion.containes("0.6.x_r") )
    {
        if (pParameterNode)
        {
            // Fix renamed node data vaariables
            HString name = readStringAttribute(pParameterNode,"name","").c_str();
            if (name.containes("::"))
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

void updateRenamedComponentType(rapidxml::xml_node<> *pNode, const string oldType, const string newType)
{
    if(readStringAttribute(pNode, "typename", "") == oldType)
    {
        writeStringAttribute(pNode, "typename", newType);
    }
}

void updateRenamedPort(rapidxml::xml_node<> *pNode, const string componentType, const HString oldName, const HString newName)
{
    if(readStringAttribute(pNode, "typename", "") == componentType)
    {
        // Rename startvalue parameters
        rapidxml::xml_node<> *pParamNode = getGrandChild(pNode, "parameters", "parameter");
        while (pParamNode)
        {
            HString paramName = readStringAttribute(pParamNode, "name").c_str();
            if (paramName.containes(oldName+"::"))
            {
                paramName.replace(oldName+"::", newName+"::");
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
                if (readStringAttribute(pConnNode, "startport").c_str() == oldName)
                {
                    writeStringAttribute(pConnNode, "startport", newName.c_str());
                }
            }
            if (endComp == compName)
            {
                if (readStringAttribute(pConnNode, "endport").c_str() == oldName)
                {
                    writeStringAttribute(pConnNode, "endport", newName.c_str());
                }
            }

            pConnNode = pConnNode->next_sibling("connect");
        }
    }
}

void updateRenamedParameter(rapidxml::xml_node<> *pNode, const string componentType, const string oldName, const string newName)
{
    if(readStringAttribute(pNode,"typename") == componentType)
    {
        rapidxml::xml_node<> *pParamNode = getGrandChild(pNode, "parameters", "parameter");
        while (pParamNode)
        {
            if (readStringAttribute(pParamNode, "name") == oldName)
            {
                writeStringAttribute(pParamNode, "name", newName);
            }
            pParamNode = pParamNode->next_sibling("parameter");
        }
    }
}



//! @brief This help function loads a component
void loadComponent(rapidxml::xml_node<> *pComponentNode, ComponentSystem* pSystem, HopsanEssentials *pHopsanEssentials)
{
    HString typeName = readStringAttribute(pComponentNode, "typename", "ERROR_NO_TYPE_GIVEN").c_str();
    HString subTypeName = readStringAttribute(pComponentNode, "subtypename", "").c_str();
    HString displayName = readStringAttribute(pComponentNode, "name", typeName.c_str()).c_str();

    Component *pComp = pHopsanEssentials->createComponent(typeName.c_str());
    if (pComp != 0)
    {
        pComp->setName(displayName);
        pComp->setSubTypeName(subTypeName.c_str());
        pSystem->addComponent(pComp);

        //Load parameters
        //! @todo should be able to load parameters and system parmaeters with same help function
        rapidxml::xml_node<> *pParams = pComponentNode->first_node("parameters");
        if (pParams)
        {
            rapidxml::xml_node<> *pParam = pParams->first_node("parameter");
            while (pParam != 0)
            {
                updateOldModelFileParameter(pParam, readStringAttribute(pComponentNode->document()->first_node(), "hopsancoreversion").c_str());

                HString paramName = readStringAttribute(pParam, "name", "ERROR_NO_PARAM_NAME_GIVEN").c_str();
                HString val = readStringAttribute(pParam, "value", "ERROR_NO_PARAM_VALUE_GIVEN").c_str();

                //! @todo this is a hack to update old parameters, remove at some point in the future
                if (!pComp->hasParameter(paramName))
                {
                    if (paramName.find("::") == HString::npos)
                    {
                        paramName=paramName+"::Value";
                    }
                }

                // We need force=true here to make sure that parameters with system variable names are set even if they can not yet be evaluated
                //! @todo why cant they be evaluated, if everything loaded in correct order that should work
                bool ok = pComp->setParameterValue(paramName.c_str(), val.c_str(), true);
                if(!ok)
                {
                    pComp->addWarningMessage("Failed to set parameter: "+paramName+"="+val);
                }

                pParam = pParam->next_sibling("parameter");
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
    //Load system parameters
    rapidxml::xml_node<> *pParameters = pSysNode->first_node("parameters");
    if (pParameters)
    {
        rapidxml::xml_node<> *pParameter = pParameters->first_node("parameter");
        while (pParameter != 0)
        {
            updateOldModelFileParameter(pParameter, readStringAttribute(pSysNode->document()->first_node(), "hopsancoreversion").c_str());

            string paramName = readStringAttribute(pParameter, "name", "ERROR_NO_PARAM_NAME_GIVEN");
            string val = readStringAttribute(pParameter, "value", "ERROR_NO_PARAM_VALUE_GIVEN");
            string type = readStringAttribute(pParameter, "type", "ERROR_NO_PARAM_TYPE_GIVEN");
            //! @todo maybe type should be data type or value type or something

            // Here we use force=true to make sure system parameters laoded even if they do not evaluate
            //! @todo if system parameters are loaded in the correct order (top to bottom) they should evaluete, why dont they?
            bool ok = pSystem->setSystemParameter(paramName.c_str(), val.c_str(), type.c_str(), "", "", true);
            if(!ok)
            {
                pSystem->addErrorMessage(HString("Failed to load parameter: ")+(paramName+"="+val).c_str());
            }

            pParameter = pParameter->next_sibling("parameter");
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

    rapidxml::xml_node<> *pSimtimeNode = pSysNode->first_node("simulationtime");
    double Ts = readDoubleAttribute(pSimtimeNode, "timestep", 0.001);
    pSystem->setDesiredTimestep(Ts);
    pSystem->setInheritTimestep(readBoolAttribute(pSimtimeNode,"inherit_timestep",true));

    //Load number of log samples
    if(hasAttribute(pSysNode,  "logsamples"))
    {
        pSystem->setNumLogSamples(readIntAttribute(pSysNode, "logsamples", pSystem->getNumLogSamples()));
    }

    //! @todo we really need defines for allof these "strings"

    //Load contents
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
                        // load overwriten parameter values
                        loadSystemParameters(pObject, pSys);
                        // Overwrite name
                        string displayNameExt = readStringAttribute(pObject, "name", typeName );
                        pSys->setName(displayNameExt.c_str());
                        // Add new system to parent
                        pSystem->addComponent(pSys);
                    }
                }
                else
                {
                    pSys = pHopsanEssentials->createComponentSystem();
                    // Load system contents
                    loadSystemContents(pObject, pSys, pHopsanEssentials, rootFilePath);
                    // Add new system to parent
                    pSystem->addComponent(pSys);
                }
            }
            else if (strcmp(pObject->name(), "systemport")==0)
            {
                loadSystemPort(pObject,pSystem);
            }

            pObject = pObject->next_sibling();
        }
    }

    //Load connections
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

    //Load system parameters
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
            if (savedwithcoreversion < "0.6.0" || (savedwithcoreversion > "0.6.x" && savedwithcoreversion < "0.6.x_r5500"))
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
                addLogMess("hopsan::loadHopsanModelFile(): No system found in file.");
                pHopsanEssentials->getCoreMessageHandler()->addErrorMessage(rFilePath+" Has no system to load");
            }
        }
        else
        {
            addLogMess("hopsan::loadHopsanModelFile(): Wrong root tag name.");
            pHopsanEssentials->getCoreMessageHandler()->addErrorMessage(rFilePath+" Has wrong root tag name: "+pRootNode->name());
            cout << "Not correct hmf file root node name: " << pRootNode->name() << endl;
        }
    }
    catch(std::exception &e)
    {
        addLogMess("hopsan::loadHopsanModelFile(): Unable to parse xml doc.");
        pHopsanEssentials->getCoreMessageHandler()->addErrorMessage("Unable to parse xml doc");
        cout << "throws: " << e.what() << endl;
    }

    addLogMess("hopsan::loadHopsanModelFile(): Failed.");

    // We failed, return 0 ptr
    return 0;
}


// vvvvvvvvvv The public function vvvvvvvvvv

//! @brief This function is used to load a HMF file.
//! @param [in] filePath The name (path) of the HMF file
//! @param [out] rStartTime A reference to the starttime variable
//! @param [out] rStopTime A reference to the stoptime variable
//! @returns A pointer to the rootsystem of the loaded model
//! @todo if possible merge the two differen main load functions
ComponentSystem* hopsan::loadHopsanModelFile(const HString filePath, HopsanEssentials* pHopsanEssentials, double &rStartTime, double &rStopTime)
{
    addLogMess(("hopsan::loadHopsanModelFile("+filePath+")").c_str());
    try
    {
        rapidxml::file<> hmfFile(filePath.c_str());
        rapidxml::xml_document<> doc;
        doc.parse<0>(hmfFile.data());

        return loadHopsanModelFileActual(doc, filePath, pHopsanEssentials, rStartTime, rStopTime);
    }
    catch(std::exception &e)
    {
        addLogMess("hopsan::loadHopsanModelFile(): Unable to open file.");
        pHopsanEssentials->getCoreMessageHandler()->addErrorMessage("Could not open file: "+filePath);
        cout << "Could not open file, throws: " << e.what() << endl;
    }
    addLogMess("hopsan::loadHopsanModelFile(): Failed.");
    // We failed, return 0 ptr
    return 0;
}


//! @brief This function is used to load a HMF file from model string.
//! @param [in] xmlModel The xml representation of the model
//! @returns A pointer to the rootsystem of the loaded model
ComponentSystem* hopsan::loadHopsanModelFile(std::vector<unsigned char> xmlVector, HopsanEssentials* pHopsanEssentials)
{
    return loadHopsanModelFile((char*) &xmlVector[0], pHopsanEssentials);
}


//! @brief This function is used to load a HMF file from model string.
//! @param [in] xmlModel The xml representation of the model
//! @returns A pointer to the rootsystem of the loaded model
ComponentSystem* hopsan::loadHopsanModelFile(const char* xmlStr, HopsanEssentials* pHopsanEssentials)
{
    // Rapid xml requires a non-const char* to work with, we need to copy the text
    char *str = new char[strlen(xmlStr)+1];
    strcpy(str, xmlStr);
    ComponentSystem *pSystem = loadHopsanModelFile(str, pHopsanEssentials);
    delete [] str;
    return pSystem;
}


ComponentSystem* hopsan::loadHopsanModelFile(char* xmlStr, HopsanEssentials* pHopsanEssentials)
{
    try
    {
        double startT, stopT;
        rapidxml::xml_document<> doc;
        doc.parse<0>( xmlStr);

        return loadHopsanModelFileActual(doc, "", pHopsanEssentials, startT, stopT);

//        rapidxml::xml_node<> *pRootNode = doc.first_node();

//        //Check for correct root node name
//        if (strcmp(pRootNode->name(), "hopsanmodelfile")==0)
//        {
//            // Check version
//            HString savedwithcoreversion = readStringAttribute(pRootNode, "hopsancoreversion", "0").c_str();
//            pHopsanEssentials->getCoreMessageHandler()->addErrorMessage(savedwithcoreversion);
//            if (savedwithcoreversion < "0.6.0" || (savedwithcoreversion > "0.6.x" && savedwithcoreversion < "0.6.x_r5216"))
//            {
//                pHopsanEssentials->getCoreMessageHandler()->addErrorMessage("This hmf model was saved with HopsanCoreVersion: "+savedwithcoreversion+". This old version is not supported by the HopsanCore hmf loader, resave the model with HopsanGUI");
//                return 0;
//            }

//            rapidxml::xml_node<> *pSysNode = pRootNode->first_node("system");
//            if (pSysNode != 0)
//            {
//                //! @todo more error check
//                ComponentSystem * pSys = pHopsanEssentials->createComponentSystem(); //Create root system
//                loadSystemContents(pSysNode, pSys, pHopsanEssentials, filePath);

//                return pSys;
//            }
//            else
//            {
//                pHopsanEssentials->getCoreMessageHandler()->addErrorMessage(filePath+" Has no system to load");
//            }
//        }
//        else
//        {
//            pHopsanEssentials->getCoreMessageHandler()->addErrorMessage(filePath+" Has wrong root tag name: "+pRootNode->name());
//            cout << "Not correct hmf file root node name: " << pRootNode->name() << endl;
//        }
    }
    catch(std::exception &e)
    {
        pHopsanEssentials->getCoreMessageHandler()->addErrorMessage("Could not parse xml string");
        cout << "throws: " << e.what() << endl;
    }

    // We failed, return 0 ptr
    return 0;
}




//! @brief This function is used to load a HMF file.
//! @param [in] filePath The name (path) of the HMF file
//! @param [out] rStartTime A reference to the starttime variable
//! @param [out] rStopTime A reference to the stoptime variable
//! @returns A pointer to the rootsystem of the loaded model
void hopsan::loadHopsanParameterFile(const HString filePath, HopsanEssentials* pHopsanEssentials, ComponentSystem *pSystem)
{
    addLogMess(("hopsan::loadHopsanParameterFile("+filePath+")").c_str());
    try
    {
        rapidxml::file<> hmfFile(filePath.c_str());

        rapidxml::xml_document<> doc;
        doc.parse<0>(hmfFile.data());

        rapidxml::xml_node<> *pRootNode = doc.first_node();

        //Check for correct root node name
        if (strcmp(pRootNode->name(), "hopsanparameterfile")==0)
        {
            rapidxml::xml_node<> *pSysNode = pRootNode->first_node("system");
            if (pSysNode != 0)
            {
                std::map<HString, std::pair<std::vector<HString>, std::vector<HString> > > parMap;

                //Load contents
                rapidxml::xml_node<> *pObjects = pSysNode->first_node("objects");
                if (pObjects)
                {
                    rapidxml::xml_node<> *pComponent = pObjects->first_node("component");
                    while (pComponent != 0)
                    {
                        HString name = readStringAttribute(pComponent, "name", "").c_str();

                        std::vector<HString> parameterNames;
                        std::vector<HString> parameterValues;
                        rapidxml::xml_node<> *pParameters = pComponent->first_node("parameters");
                        if (pParameters != 0)
                        {
                            rapidxml::xml_node<> *pParameter = pParameters->first_node("parameter");
                            while (pParameter != 0)
                            {
                                parameterNames.push_back(readStringAttribute(pParameter, "name", "").c_str());
                                parameterValues.push_back(readStringAttribute(pParameter, "value", "").c_str());
                                pParameter = pParameter->next_sibling();
                            }
                        }

                        std::pair<std::vector<HString>, std::vector<HString> > parameters;
                        parameters = std::pair<std::vector<HString>, std::vector<HString> >(parameterNames, parameterValues);

                        parMap.insert(std::pair<HString, std::pair<std::vector<HString>, std::vector<HString> > >(name, parameters));

                        pComponent = pComponent->next_sibling();
                    }
                }


                pSystem->loadParameters(parMap);
                return;
               // loadSystemContents(pSysNode, pSys, pHopsanEssentials, filePath);

            }
            else
            {
                addLogMess("hopsan::loadHopsanParameterFile(): No system found in file.");
                pHopsanEssentials->getCoreMessageHandler()->addErrorMessage(filePath+" Has no system to load");
            }
        }
        else
        {
            addLogMess("hopsan::loadHopsanParameterFile(): Wrong root tag name.");
            pHopsanEssentials->getCoreMessageHandler()->addErrorMessage(filePath+" Has wrong root tag name: "+pRootNode->name());
            cout << "Not correct hmf file root node name: " << pRootNode->name() << endl;
        }
    }
    catch(std::exception &e)
    {
        addLogMess("hopsan::loadHopsanParameterFile(): Unable to open file.");
        pHopsanEssentials->getCoreMessageHandler()->addErrorMessage("Could not open file: "+filePath);
        cout << "Could not open file, throws: " << e.what() << endl;
    }

    addLogMess("hopsan::loadHopsanParameterFile(): Failed.");

    // We failed, return 0 ptr
    return;
}
