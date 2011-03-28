//!
//! @file   FileAccess.cc
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-02-03
//!
//! @brief Contains the file access functions
//!
//$Id$

#include <iostream>
#include <cassert>
#include <string>
#include "FileAccess.h"
#include "../Component.h"
#include "../HopsanCore.h"

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
        //Convert char* to dstring, assume null terminated strings
        return string(pAttr->value());
    }
    else
    {
        return defaultValue;
    }

}


FileAccess::FileAccess()
{
    //Nothing
}

//! @todo Update this code
ComponentSystem* FileAccess::loadModel(string filename, double *startTime, double *stopTime)
{
    rapidxml::file<> hmfFile(filename.c_str());

    rapidxml::xml_document<> doc;
    doc.parse<0>(hmfFile.data());

    rapidxml::xml_node<> *pRootNode = doc.first_node();

    //Check for correct root node name
    if (pRootNode->name() == "hopsanmodelfile")
    {
        rapidxml::xml_node<> *pSysNode = pRootNode->first_node("system");
        //! @todo error check
        //We only want to read toplevel simulation time settings here
        rapidxml::xml_node<> *pSimtimeNode = pSysNode->first_node("simulationtime");
        *startTime = readDoubleAttribute(pSimtimeNode, "start", 0);
        *stopTime = readDoubleAttribute(pSimtimeNode, "stop", 2);

        ComponentSystem * pSys = HopsanEssentials::getInstance()->CreateComponentSystem(); //Create root system
        loadSystemContents(pSysNode, pSys);

        return pSys;
    }
    else
    {
        cout << "Not correct hmf file root node name" << endl;
        assert(false);
        return 0;
    }
}


//! @brief This function can be used to load subsystem contents from a stream into an existing subsystem
//! @todo Update this code
void FileAccess::loadSystemContents(rapidxml::xml_node<> *pSysNode, ComponentSystem* pSystem)
{
    rapidxml::xml_node<> *pSimtimeNode = pSysNode->first_node("simulationtime");
    double Ts = readDoubleAttribute(pSimtimeNode, "timestep", 0.001);
    pSystem->setDesiredTimestep(Ts);

    //Load contents
    rapidxml::xml_node<> *pObject = pSysNode->first_node("objects")->first_node();
    while (pObject != 0)
    {
        if (pObject->name() == "component")
        {
            loadComponent(pObject, pSystem);

        }
        else if (pObject->name() == "system")
        {
            //Add new system
            ComponentSystem * pSys = HopsanEssentials::getInstance()->CreateComponentSystem();
            pSystem->addComponent(pSys);
            loadSystemContents(pObject, pSys);
        }
        pObject = pObject->next_sibling();
    }

    //Load connections
    rapidxml::xml_node<> *pConnection = pSysNode->first_node("conections")->first_node();
    while (pConnection != 0)
    {
        if (pConnection->name() == "connnect")
        {
            loadConnection(pConnection, pSystem);
        }
        pConnection->next_sibling();
    }


}

void FileAccess::loadComponent(rapidxml::xml_node<> *pComponentNode, ComponentSystem* pSystem)
{
    string typeName = readStringAttribute(pComponentNode, "typename", "ERROR_NO_TYPE_GIVEN");
    string displayName =  readStringAttribute(pComponentNode, "displayname", typeName);

    Component *pComp = HopsanEssentials::getInstance()->CreateComponent(typeName);
    pComp->setName(displayName);
    pSystem->addComponent(pComp);

    //Load parameters
    rapidxml::xml_node<> *pParam = pComponentNode->first_node("parameters")->first_node();
    while (pParam != 0)
    {
        if (pParam->name() == "parameter")
        {
            string paramName = readStringAttribute(pParam, "name", "ERROR_NO_PARAM_NAME_GIVEN");
            double val = readDoubleAttribute(pParam, "value", 0);

            pComp->setParameterValue(paramName, val);
        }
        pParam->next_sibling();
    }

    //Load startvalues
    rapidxml::xml_node<> *pStartValue = pComponentNode->first_node("startvalues")->first_node();
    while (pStartValue != 0)
    {
        if (pStartValue->name() == "startvalue")
        {
            string portName = readStringAttribute(pStartValue, "portname", "ERROR_NO_PARTNAME_GIVEN");
            string variableName = readStringAttribute(pStartValue, "variable", "ERROR_NO_PARTNAME_GIVEN");
            double val = readDoubleAttribute(pParam, "value", 0);

            //! @todo how do I transfrom variable name into variable index?
            //pComp->setStartValue();
        }
        pStartValue->next_sibling();
    }
}

void FileAccess::loadConnection(rapidxml::xml_node<> *pConnectNode, ComponentSystem* pSystem)
{
    string startcomponent = readStringAttribute(pConnectNode, "startcomponent", "ERROR_NOSTARTCOMPNAME_GIVEN");
    string startport = readStringAttribute(pConnectNode, "startport", "ERROR_NOSTARTPORTNAME_GIVEN");
    string endcomponent = readStringAttribute(pConnectNode, "endcomponent", "ERROR_NOENDCOMPNAME_GIVEN");
    string endport = readStringAttribute(pConnectNode, "endport", "ERROR_NOENDPORTNAME_GIVEN");

    pSystem->connect(startcomponent, startport, endcomponent, endport);
}

