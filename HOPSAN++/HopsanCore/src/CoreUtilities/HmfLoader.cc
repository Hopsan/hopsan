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

#include "hopsan_rapidxml.hpp"

using namespace std;
using namespace hopsan;

// vvvvvvvvvv Help Functions vvvvvvvvvv
//! @brief Helpfunction to strip filename from path
//! @note Assumes that dir separator is forward slash /
std::string stripFilenameFromPath(std::string filePath)
{
    //cout << "Stripping from: " << filePath << endl;
    size_t pos = filePath.rfind('/');
    if (pos != std::string::npos)
    {
        filePath.erase(pos+1);
    }
    //cout << "Stripped: " << filePath << endl;
    return filePath;
}

//! @brief Splits a full name into comp port and variable names
//! @todo this should be in a more "global" place since it may be usefull elsewhere
void splitFullName(const std::string &rFullName, std::string &rCompName, std::string &rPortName, std::string &rVarName)
{
    vector<string> parts;
    parts.reserve(3);

    size_t start=0, end=0;
    while (end != string::npos)
    {
        end = rFullName.find_first_of('#', start);
        parts.push_back(rFullName.substr(start, end-start));
        start = end+1;
        if (end == string::npos)
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

void updateOldModelFileParameter(rapidxml::xml_node<> *pParameterNode, const std::string &rHmfCoreVersion)
{
    if (rHmfCoreVersion < "0.6.0" || (rHmfCoreVersion > "0.6.x" && rHmfCoreVersion < "0.6.x_r5135"))
    {
        if (pParameterNode)
        {
            // Fix renamed node data vaariables
            string name = readStringAttribute(pParameterNode,"name","");
            if (contains(name, "::"))
            {
                // split string
                string part1 = name.substr(0, name.rfind(":")+1);
                string part2 = name.substr(name.rfind(":")+1);

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
                writeStringAttribute(pParameterNode, "name", part1+part2);
            }
            // Fix parameter names with illegal chars
            else if (!isNameValid(name))
            {
                if (name == "sigma^2")
                {
                    name = "std_dev";
                }

                replace(name,",","");
                replace(name,".","");
                replace(name," ","_");

                writeStringAttribute(pParameterNode, "name", name);
            }
        }
    }
}

void updateOldModelFileComponent(rapidxml::xml_node<> *pComponentNode, const std::string &rHmfCoreVersion)
{
    // Typos (no specific version)
    if(readStringAttribute(pComponentNode, "typename", "") == "MechanicTranslationalMassWithCoulumbFriction")
    {
        writeStringAttribute(pComponentNode, "typename", "MechanicTranslationalMassWithCoulombFriction");
    }
}



//! @brief This help function loads a component
void loadComponent(rapidxml::xml_node<> *pComponentNode, ComponentSystem* pSystem, HopsanEssentials *pHopsanEssentials)
{
    string typeName = readStringAttribute(pComponentNode, "typename", "ERROR_NO_TYPE_GIVEN");
    string subTypeName = readStringAttribute(pComponentNode, "subtypename", "");
    string displayName = readStringAttribute(pComponentNode, "name", typeName);

    Component *pComp = pHopsanEssentials->createComponent(typeName);
    if (pComp != 0)
    {
        pComp->setName(displayName);
        pComp->setSubTypeName(subTypeName);
        pSystem->addComponent(pComp);

        //Load parameters
        //! @todo should be able to load parameters and system parmaeters with same help function
        rapidxml::xml_node<> *pParams = pComponentNode->first_node("parameters");
        if (pParams)
        {
            rapidxml::xml_node<> *pParam = pParams->first_node("parameter");
            while (pParam != 0)
            {
                updateOldModelFileParameter(pParam, readStringAttribute(pComponentNode->document()->first_node(), "hopsancoreversion"));

                string paramName = readStringAttribute(pParam, "name", "ERROR_NO_PARAM_NAME_GIVEN");
                string val = readStringAttribute(pParam, "value", "ERROR_NO_PARAM_VALUE_GIVEN");

                // We need force=true here to make sure that parameters with system variable names are set even if they can not yet be evaluated
                //! @todo why cant they be evaluated, if everything loaded in correct order that should work
                bool ok = pComp->setParameterValue(paramName, val, true);
                if(!ok)
                {
                    pComp->addErrorMessage("Failed to load parameter: "+paramName+"="+val);
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

    santizeName(startcomponent);
    santizeName(startport);
    santizeName(endcomponent);
    santizeName(endport);

    pSystem->connect(startcomponent, startport, endcomponent, endport);
}

//! @brief This help function loads a SystemPort
void loadSystemPort(rapidxml::xml_node<> *pSysPortNode, ComponentSystem* pSystem)
{
    string name = readStringAttribute(pSysPortNode, "name", "ERROR_NO_NAME_GIVEN");
    pSystem->addSystemPort(name);
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
            updateOldModelFileParameter(pParameter, readStringAttribute(pSysNode->document()->first_node(), "hopsancoreversion"));

            string paramName = readStringAttribute(pParameter, "name", "ERROR_NO_PARAM_NAME_GIVEN");
            string val = readStringAttribute(pParameter, "value", "ERROR_NO_PARAM_VALUE_GIVEN");
            string type = readStringAttribute(pParameter, "type", "ERROR_NO_PARAM_TYPE_GIVEN");
            //! @todo maybe type should be data type or value type or something

            // Here we use force=true to make sure system parameters laoded even if they do not evaluate
            //! @todo if system parameters are loaded in the correct order (top to bottom) they should evaluete, why dont they?
            bool ok = pSystem->setSystemParameter(paramName, val, type, "", "", true);
            if(!ok)
            {
                pSystem->addErrorMessage("Failed to load parameter: "+paramName+"="+val);
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
        string type = readStringAttribute(pAlias, "type", "ERROR_NO_ALIAS_TYPE_GIVEN");
        string alias = readStringAttribute(pAlias, "name", "ERROR_NO_ALIAS_NAME_GIVEN");
        string fullName = readStringNodeValue(pAlias->first_node("fullname"), "ERROR_NO_FULLNAME_GIVEN");

        string comp, port, var;
        splitFullName(fullName, comp, port, var);
        //cout << "splitOut: " << fullName << " ! " << comp << " ! " << port << " ! " << var << endl;

        if (type == "variable" || type == "Variable")
        {
            //! @todo check bool and display warning if false
            pSystem->getAliasHandler().setVariableAlias(alias, comp, port, var);
        }


        pAlias = pAlias->next_sibling("alias");
    }
}


//! @brief This function loads a subsystem
void loadSystemContents(rapidxml::xml_node<> *pSysNode, ComponentSystem* pSystem, HopsanEssentials* pHopsanEssentials, const std::string rootFilePath="")
{
    string typeName = readStringAttribute(pSysNode, "typename", "ERROR_NO_TYPE_GIVEN");
    string displayName = readStringAttribute(pSysNode, "name", typeName );
    pSystem->setName(displayName);

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
                updateOldModelFileComponent(pObject, readStringAttribute(pObject->document()->first_node(), "hopsancoreversion", ""));
                loadComponent(pObject, pSystem, pHopsanEssentials);
            }
            else if (strcmp(pObject->name(), "system")==0)
            {
                bool isExternal = hasAttribute(pObject, "external_path");
                ComponentSystem* pSys;

                if (isExternal)
                {
                    double dummy1,dummy2;
                    string externalPath = stripFilenameFromPath(rootFilePath) + readStringAttribute(pObject,"external_path","");
                    cout << "externalPath: " << externalPath << endl;
                    pSys = loadHopsanModelFile(externalPath, pHopsanEssentials, dummy1, dummy2);
                    if (pSys != 0)
                    {
                        // load overwriten parameter values
                        loadSystemParameters(pObject, pSys);
                        // Overwrite name
                        string displayNameExt = readStringAttribute(pObject, "name", typeName );
                        pSys->setName(displayNameExt);
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


// vvvvvvvvvv The public function vvvvvvvvvv

//! @brief This function is used to load a HMF file.
//! @param [in] filePath The name (path) of the HMF file
//! @param [out] rStartTime A reference to the starttime variable
//! @param [out] rStopTime A reference to the stoptime variable
//! @returns A pointer to the rootsystem of the loaded model
ComponentSystem* hopsan::loadHopsanModelFile(const std::string filePath, HopsanEssentials* pHopsanEssentials, double &rStartTime, double &rStopTime)
{
    addLogMess("hopsan::loadHopsanModelFile("+filePath+")");
    try
    {
        rapidxml::file<> hmfFile(filePath.c_str());

        rapidxml::xml_document<> doc;
        doc.parse<0>(hmfFile.data());

        rapidxml::xml_node<> *pRootNode = doc.first_node();

        //Check for correct root node name
        if (strcmp(pRootNode->name(), "hopsanmodelfile")==0)
        {
            rapidxml::xml_node<> *pSysNode = pRootNode->first_node("system");
            if (pSysNode != 0)
            {
                //! @todo more error check
                //We only want to read toplevel simulation time settings here
                rapidxml::xml_node<> *pSimtimeNode = pSysNode->first_node("simulationtime");
                rStartTime = readDoubleAttribute(pSimtimeNode, "start", 0);
                rStopTime = readDoubleAttribute(pSimtimeNode, "stop", 2);
                ComponentSystem * pSys = pHopsanEssentials->createComponentSystem(); //Create root system
                loadSystemContents(pSysNode, pSys, pHopsanEssentials, filePath);

                pSys->addSearchPath(stripFilenameFromPath(filePath));
                return pSys;
            }
            else
            {
                addLogMess("hopsan::loadHopsanModelFile(): No system found in file.");
                pHopsanEssentials->getCoreMessageHandler()->addErrorMessage(filePath+" Has no system to load");
            }
        }
        else
        {
            addLogMess("hopsan::loadHopsanModelFile(): Wrong root tag name.");
            pHopsanEssentials->getCoreMessageHandler()->addErrorMessage(filePath+" Has wrong root tag name: "+pRootNode->name());
            cout << "Not correct hmf file root node name: " << pRootNode->name() << endl;
        }
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
ComponentSystem* hopsan::loadHopsanModelFileFromStdString(std::string xmlStr, HopsanEssentials* pHopsanEssentials)
{
    char *cstr = new char[xmlStr.length() + 1];
    strcpy(cstr, xmlStr.c_str());
    ComponentSystem *pSystem = loadHopsanModelFile(cstr, pHopsanEssentials);
    delete [] cstr;
    return pSystem;
}


ComponentSystem* hopsan::loadHopsanModelFile(char* xmlStr, HopsanEssentials* pHopsanEssentials)
{
    std::string filePath("");

    try
    {
        rapidxml::xml_document<> doc;
        doc.parse<0>( xmlStr);

        rapidxml::xml_node<> *pRootNode = doc.first_node();

        //Check for correct root node name
        if (strcmp(pRootNode->name(), "hopsanmodelfile")==0)
        {
            rapidxml::xml_node<> *pSysNode = pRootNode->first_node("system");
            if (pSysNode != 0)
            {
                //! @todo more error check
                ComponentSystem * pSys = pHopsanEssentials->createComponentSystem(); //Create root system
                loadSystemContents(pSysNode, pSys, pHopsanEssentials, filePath);

                return pSys;
            }
            else
            {
                pHopsanEssentials->getCoreMessageHandler()->addErrorMessage(filePath+" Has no system to load");
            }
        }
        else
        {
            pHopsanEssentials->getCoreMessageHandler()->addErrorMessage(filePath+" Has wrong root tag name: "+pRootNode->name());
            cout << "Not correct hmf file root node name: " << pRootNode->name() << endl;
        }
    }
    catch(std::exception &e)
    {
        pHopsanEssentials->getCoreMessageHandler()->addErrorMessage("Could not open file: "+filePath);
        cout << "Could not open file, throws: " << e.what() << endl;
    }

    // We failed, return 0 ptr
    return 0;
}




//! @brief This function is used to load a HMF file.
//! @param [in] filePath The name (path) of the HMF file
//! @param [out] rStartTime A reference to the starttime variable
//! @param [out] rStopTime A reference to the stoptime variable
//! @returns A pointer to the rootsystem of the loaded model
void hopsan::loadHopsanParameterFile(const std::string filePath, HopsanEssentials* pHopsanEssentials, ComponentSystem *pSystem)
{
    addLogMess("hopsan::loadHopsanParameterFile("+filePath+")");
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
                std::map<std::string, std::pair<std::vector<std::string>, std::vector<std::string> > > parMap;

                //Load contents
                rapidxml::xml_node<> *pObjects = pSysNode->first_node("objects");
                if (pObjects)
                {
                    rapidxml::xml_node<> *pComponent = pObjects->first_node("component");
                    while (pComponent != 0)
                    {
                        std::string name = readStringAttribute(pComponent, "name", "");

                        std::vector<std::string> parameterNames;
                        std::vector<std::string> parameterValues;
                        rapidxml::xml_node<> *pParameters = pComponent->first_node("parameters");
                        if (pParameters != 0)
                        {
                            rapidxml::xml_node<> *pParameter = pParameters->first_node("parameter");
                            while (pParameter != 0)
                            {
                                parameterNames.push_back(readStringAttribute(pParameter, "name", ""));
                                parameterValues.push_back(readStringAttribute(pParameter, "value", ""));
                                pParameter = pParameter->next_sibling();
                            }
                        }

                        std::pair<std::vector<std::string>, std::vector<std::string> > parameters;
                        parameters = std::pair<std::vector<std::string>, std::vector<std::string> >(parameterNames, parameterValues);

                        parMap.insert(std::pair<std::string, std::pair<std::vector<std::string>, std::vector<std::string> > >(name, parameters));

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
