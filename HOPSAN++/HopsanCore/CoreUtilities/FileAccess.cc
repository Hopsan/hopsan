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

FileAccess::FileAccess()
{
    //Nothing
}

//! @todo Update this code
void FileAccess::loadModel(string filename, ComponentSystem* pModelSystem, double *startTime, double *stopTime)
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




    }
    else
    {
        cout << "Not correct hmf file root node name" << endl;
        assert(false);
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
            //Add new component

        }
        else if (pObject->name() == "system")
        {
            //Add new system
        }
        pObject = pObject->next_sibling();
    }

}

void FileAccess::loadComponent(rapidxml::xml_node<> *pComponentNode, ComponentSystem* pSystem)
{
    //Load the component

}

void FileAccess::loadConnection(rapidxml::xml_node<> *pConnectNode, ComponentSystem* pSystem)
{
    //Load the component

}

