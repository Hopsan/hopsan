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
//! @file   hopsan_rapidxml.hpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2012-05-30
//!
//! @brief Contains some convenient help functions for using rapid_xml in Hopsan
//! @note This file is not included with HopsanCore because we want to keep the include folder free from external dependencies
//!
//$Id$

#include <string>
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
//#include "rapidxml_print.hpp"

//! @brief Helpfunction, reads a double xml attribute
double readDoubleAttribute(const rapidxml::xml_node<> *pNode, const std::string attrName, const double defaultValue)
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

//! @brief Helpfunction, reads a int xml attribute
int readIntAttribute(const rapidxml::xml_node<> *pNode, const std::string attrName, const int defaultValue)
{
    if (pNode!=0)
    {
        rapidxml::xml_attribute<> *pAttr = pNode->first_attribute(attrName.c_str());
        if (pAttr)
        {
            //Convert char* to int, assume null terminated strings
            return atoi(pAttr->value());
        }
    }

    return defaultValue;
}

//! @brief Helpfunction, reads a string xml attribute
std::string readStringAttribute(const rapidxml::xml_node<> *pNode, const std::string attrName, const std::string defaultValue)
{
    if (pNode)
    {
        rapidxml::xml_attribute<> *pAttr = pNode->first_attribute(attrName.c_str());
        if (pAttr)
        {
            //Convert char* to string, assume null terminated strings
            return std::string(pAttr->value());
        }
    }

    return defaultValue;
}

bool readBoolAttribute(const rapidxml::xml_node<> *pNode, const std::string attrName, const bool defaultValue)
{
    if (pNode)
    {
        rapidxml::xml_attribute<> *pAttr = pNode->first_attribute(attrName.c_str());
        if (pAttr)
        {
            //Convert char* to string, assume null terminated strings
            std::string boolStr = std::string(pAttr->value());
            if ((boolStr == "true") || (boolStr == "True") || (boolStr == "1") )
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }

    return defaultValue;
}

//! @brief Check if node has attribute
bool hasAttribute(rapidxml::xml_node<> *pNode, std::string attrName)
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

std::string readStringNodeValue(rapidxml::xml_node<> *pNode, const std::string defaultValue)
{
    if (pNode)
    {
        return std::string(pNode->value());
    }
    else
    {
        return defaultValue;
    }
}

int readIntNodeValue(rapidxml::xml_node<> *pNode, const int defaultValue)
{
    if (pNode)
    {
        return atoi(pNode->value());
    }
    else
    {
        return defaultValue;
    }
}
