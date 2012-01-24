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
#include "HopsanEssentials.h"

#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
//#include "rapidxml_print.hpp"

using namespace std;
using namespace hopsan;

// vvvvvvvvvv Help Functions vvvvvvvvvv

//! @brief Helpfunction, reads a double xml attribute
double readDoubleAttribute(rapidxml::xml_node<> *pNode, string attrName, double defaultValue)
{
    if (pNode!=0)
    {
        rapidxml::xml_attribute<> *pAttr = pNode->first_attribute(attrName.c_str());
        if (pAttr)
        {
            //Convert char* to double, assume null terminated strings
            return atof(pAttr->value());
        }
    }

    return defaultValue;
}

//! @brief Helpfunction, reads a string xml attribute
string readStringAttribute(rapidxml::xml_node<> *pNode, string attrName, string defaultValue)
{
    if (pNode)
    {
        rapidxml::xml_attribute<> *pAttr = pNode->first_attribute(attrName.c_str());
        if (pAttr)
        {
            //Convert char* to string, assume null terminated strings
            return string(pAttr->value());
        }
    }

    return defaultValue;
}

//! @brief Check if node has attribute
bool hasAttribute(rapidxml::xml_node<> *pNode, string attrName)
{
    if (pNode)
    {
        if(pNode->first_attribute(attrName.c_str()))
        {
            return true;
        }
    }
    return false;
}

//! @brief Helpfunction to strip filename from path
//! @note Assumes that dir separator is forward slash /
std::string stripFilenameFromPath(std::string filePath)
{
    cout << "Stripping from: " << filePath << endl;
    size_t pos = filePath.rfind('/');
    if (pos != std::string::npos)
    {
        filePath.erase(pos+1);
    }
    cout << "Stripped: " << filePath << endl;
    return filePath;
}


//! @brief This help function loads a component
void loadComponent(rapidxml::xml_node<> *pComponentNode, ComponentSystem* pSystem, HopsanEssentials *pHopsanEssentials)
{
    string typeName = readStringAttribute(pComponentNode, "typename", "ERROR_NO_TYPE_GIVEN");
    string displayName =  readStringAttribute(pComponentNode, "name", typeName);

    Component *pComp = pHopsanEssentials->createComponent(typeName);
    if (pComp != 0)
    {
        pComp->setName(displayName);
        //cout << "------------------------before add comp: "  << typeName << " " << displayName << " " << pComp->getName() << endl;
        pSystem->addComponent(pComp);
        //cout << "------------------------after add comp: "  << typeName << " " << displayName << " " << pComp->getName() << endl;

        //Load parameters
        rapidxml::xml_node<> *pParams = pComponentNode->first_node("parameters");
        if (pParams)
        {
            rapidxml::xml_node<> *pParam = pParams->first_node();
            while (pParam != 0)
            {
                if (strcmp(pParam->name(), "parameter")==0)
                {
                    string paramName = readStringAttribute(pParam, "name", "ERROR_NO_PARAM_NAME_GIVEN");
                    string val = readStringAttribute(pParam, "value", "ERROR_NO_PARAM_VALUE_GIVEN");

                    pComp->setParameterValue(paramName, val);
                }
                pParam = pParam->next_sibling();
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

    pSystem->connect(startcomponent, startport, endcomponent, endport);
}

//! @brief This help function loads a SystemPort
void loadSystemPort(rapidxml::xml_node<> *pSysPortNode, ComponentSystem* pSystem)
{
    string name = readStringAttribute(pSysPortNode, "name", "ERROR_NO_NAME_GIVEN");
    pSystem->addSystemPort(name);
}


//! @brief This function loads a subsystem
//! @todo load inherit timestep
void loadSystemContents(rapidxml::xml_node<> *pSysNode, ComponentSystem* pSystem, HopsanEssentials* pHopsanEssentials, const std::string rootFilePath="")
{
    string typeName = readStringAttribute(pSysNode, "typename", "ERROR_NO_TYPE_GIVEN");
    string displayName = readStringAttribute(pSysNode, "name", typeName );
    pSystem->setName(displayName);

    rapidxml::xml_node<> *pSimtimeNode = pSysNode->first_node("simulationtime");
    double Ts = readDoubleAttribute(pSimtimeNode, "timestep", 0.001);
    pSystem->setDesiredTimestep(Ts);

    //Load contents
    rapidxml::xml_node<> *pObjects = pSysNode->first_node("objects");
    if (pObjects)
    {
        rapidxml::xml_node<> *pObject = pObjects->first_node();
        while (pObject != 0)
        {
            if (strcmp(pObject->name(), "component")==0)
            {
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
                    pSys = loadHopsanModelFile(externalPath,pHopsanEssentials,dummy1,dummy2);
                    if (pSys != 0)
                    {
                        // Remove external_path attribute from node so that it wont be loaded again when we load additional info
                        pObject->remove_attribute(pObject->first_attribute("external_path"));
                        cout << "Have external attribute: " << hasAttribute(pObject, "external_path") << endl;
                        // load overwriten parameter values, and other things maybe
                        loadSystemContents(pObject, pSys, pHopsanEssentials, externalPath);
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
    rapidxml::xml_node<> *pParameters = pSysNode->first_node("parameters");
    if (pParameters)
    {
        rapidxml::xml_node<> *pParameter = pParameters->first_node("parameter");
        while (pParameter != 0)
        {
            string paramName = readStringAttribute(pParameter, "name", "ERROR_NO_PARAM_NAME_GIVEN");
            string val = readStringAttribute(pParameter, "value", "ERROR_NO_PARAM_VALUE_GIVEN");
            string type = readStringAttribute(pParameter, "type", "ERROR_NO_PARAM_TYPE_GIVEN");
            //! @todo maybe type should be data type or value type or something

            pSystem->setSystemParameter(paramName, val, type);

            pParameter = pParameter->next_sibling("parameter");
        }
    }

    //! @todo load ALIASES
}

// vvvvvvvvvv The public function vvvvvvvvvv

//! @brief This function is used to load a HMF file.
//! @param [in] filePath The name (path) of the HMF file
//! @param [out] rStartTime A reference to the starttime variable
//! @param [out] rStopTime A reference to the stoptime variable
//! @returns A pointer to the rootsystem of the loaded model
ComponentSystem* hopsan::loadHopsanModelFile(const std::string filePath, HopsanEssentials* pHopsanEssentials, double &rStartTime, double &rStopTime)
{
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

                return pSys;
            }
            else
            {
                getCoreMessageHandlerPtr()->addErrorMessage(filePath+" Has no system to load");
            }
        }
        else
        {
            getCoreMessageHandlerPtr()->addErrorMessage(filePath+" Has wrong root tag name: "+pRootNode->name());
            cout << "Not correct hmf file root node name: " << pRootNode->name() << endl;
        }
    }
    catch(std::exception &e)
    {
        getCoreMessageHandlerPtr()->addErrorMessage("Could not open file: "+filePath);
        cout << "Could not open file, throws: " << e.what() << endl;
    }

    // We failed, return 0 ptr
    return 0;
}
