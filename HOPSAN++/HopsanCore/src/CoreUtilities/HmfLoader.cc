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
        //! @todo should be able to load parameters and system parmaeters with same help function
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

                    // We need force=true here to make sure that parameters with system variable names are set even if they can not yet be evaluated
                    //! @todo why cant they be evaluated, if everything loaded in correct order that should work
                    pComp->setParameterValue(paramName, val, true);
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
            string paramName = readStringAttribute(pParameter, "name", "ERROR_NO_PARAM_NAME_GIVEN");
            string val = readStringAttribute(pParameter, "value", "ERROR_NO_PARAM_VALUE_GIVEN");
            string type = readStringAttribute(pParameter, "type", "ERROR_NO_PARAM_TYPE_GIVEN");
            //! @todo maybe type should be data type or value type or something

            // Here we use force=true to make sure system parameters laoded even if they do not evaluate
            //! @todo if system parameters are loaded in the correct order (top to bottom) they should evaluete, why dont they?
            bool success = pSystem->setSystemParameter(paramName, val, type, "", "", true);
            if(!success)
            {
                getCoreMessageHandlerPtr()->addErrorMessage("Could not set parameter: " + paramName);
            }

            pParameter = pParameter->next_sibling("parameter");
        }
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

//! @brief This function is used to load a HMF file from model string.
//! @param [in] xmlModel The xml representation of the model
//! @returns A pointer to the rootsystem of the loaded model
ComponentSystem* hopsan::loadHopsanModelFile(std::vector<unsigned char> xmlVector, HopsanEssentials* pHopsanEssentials)
{
    std::string filePath("");

    try
    {
        rapidxml::xml_document<> doc;
        doc.parse<0>( (char*) &xmlVector[0]);

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
