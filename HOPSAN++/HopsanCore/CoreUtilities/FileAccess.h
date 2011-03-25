//!
//! @file   FileAccess.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-02-03
//!
//! @brief Contains the file access functions
//!
//$Id$

#ifndef FILEACCESS_H_INCLUDED
#define FILEACCESS_H_INCLUDED


#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

#include "../win32dll.h"
#include "../ComponentEssentials.h"
#include "../HopsanEssentials.h"

#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
//#include "rapidxml_print.hpp"

namespace hopsan {

//    std::string readName(std::stringstream &rTextStream);
//    std::string readName(std::string namestring);
//    std::string addQuotes(std::string str);

    class DLLIMPORTEXPORT FileAccess
    {
    public:
        FileAccess();

        void loadModel(std::string filename, ComponentSystem* pModelSystem, double *startTime, double *stopTime);
        void loadSystemContents(rapidxml::xml_node<> *pSysNode, ComponentSystem *pSystem);
        void loadComponent(rapidxml::xml_node<> *pComponentNode, ComponentSystem *pSystem);
        void loadConnection(rapidxml::xml_node<> *pConnectNode, ComponentSystem *pSystem);

    private:


    };
}

#endif // FILEACCESS_INCLUDED

