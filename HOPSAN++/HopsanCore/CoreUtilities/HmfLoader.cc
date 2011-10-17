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
#include "HmfLoader.h"
#include "../HopsanEssentials.h"
#include "../ComponentSystem.h"

using namespace std;
using namespace hopsan;

//Help functions
double readDoubleAttribute(rapidxml::xml_node<> *pNode, string attrName, double defaultValue)
{
    rapidxml::xml_attribute<> *pAttr = pNode->first_attribute(attrName.c_str());
    if (pAttr)
    {
        //Convert char* to double, assume null terminated strings
        return atof(pAttr->value());
    }
    else
    {
        return defaultValue;
    }
}

string readStringAttribute(rapidxml::xml_node<> *pNode, string attrName, string defaultValue)
{
    rapidxml::xml_attribute<> *pAttr = pNode->first_attribute(attrName.c_str());
    if (pAttr)
    {
        //Convert char* to string, assume null terminated strings
        return string(pAttr->value());
    }
    else
    {
        return defaultValue;
    }

}


HmfLoader::HmfLoader()
{
    //Nothing
}

//! @todo Update this code
ComponentSystem* HmfLoader::loadModel(string filename, double &rStartTime, double &rStopTime)
{
    cout << "Loading from file: " << filename << endl;
    //! @todo do not crash if file is missing, send error message
    rapidxml::file<> hmfFile(filename.c_str());

    rapidxml::xml_document<> doc;
    doc.parse<0>(hmfFile.data());

    rapidxml::xml_node<> *pRootNode = doc.first_node();

    //Check for correct root node name
    if (strcmp(pRootNode->name(), "hopsanmodelfile")==0)
    {
        rapidxml::xml_node<> *pSysNode = pRootNode->first_node("system");
        //! @todo error check
        //We only want to read toplevel simulation time settings here
        rapidxml::xml_node<> *pSimtimeNode = pSysNode->first_node("simulationtime");
        rStartTime = readDoubleAttribute(pSimtimeNode, "start", 0);
        rStopTime = readDoubleAttribute(pSimtimeNode, "stop", 2);

        ComponentSystem * pSys = HopsanEssentials::getInstance()->CreateComponentSystem(); //Create root system
        loadSystemContents(pSysNode, pSys);

        return pSys;
    }
    else
    {
        cout << "Not correct hmf file root node name: " << pRootNode->name() << endl;
        assert(false);
        return 0;
    }
}


//! @brief This function can be used to load subsystem contents from a stream into an existing subsystem
//! @todo Update this code
void HmfLoader::loadSystemContents(rapidxml::xml_node<> *pSysNode, ComponentSystem* pSystem)
{
    rapidxml::xml_node<> *pSimtimeNode = pSysNode->first_node("simulationtime");
    assert(pSimtimeNode != 0); //!< @todo smarter error handling
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
                loadComponent(pObject, pSystem);
            }
            else if (strcmp(pObject->name(), "system")==0)
            {
                //Add new system
                ComponentSystem * pSys = HopsanEssentials::getInstance()->CreateComponentSystem();
                pSystem->addComponent(pSys);
                loadSystemContents(pObject, pSys);
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
}

void HmfLoader::loadComponent(rapidxml::xml_node<> *pComponentNode, ComponentSystem* pSystem)
{
    string typeName = readStringAttribute(pComponentNode, "typename", "ERROR_NO_TYPE_GIVEN");
    string displayName =  readStringAttribute(pComponentNode, "name", typeName);

    Component *pComp = HopsanEssentials::getInstance()->CreateComponent(typeName);
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

void HmfLoader::loadConnection(rapidxml::xml_node<> *pConnectNode, ComponentSystem* pSystem)
{
    string startcomponent = readStringAttribute(pConnectNode, "startcomponent", "ERROR_NOSTARTCOMPNAME_GIVEN");
    string startport = readStringAttribute(pConnectNode, "startport", "ERROR_NOSTARTPORTNAME_GIVEN");
    string endcomponent = readStringAttribute(pConnectNode, "endcomponent", "ERROR_NOENDCOMPNAME_GIVEN");
    string endport = readStringAttribute(pConnectNode, "endport", "ERROR_NOENDPORTNAME_GIVEN");

    pSystem->connect(startcomponent, startport, endcomponent, endport);
}

