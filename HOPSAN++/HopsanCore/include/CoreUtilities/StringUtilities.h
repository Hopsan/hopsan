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
//! @file   StringUtilities.h
//! @author <peter.nordin@liu.se>
//! @date   2013-03-12
//!
//! @brief Contains functions for working with strings, in particualr Name strings.
//!
//$Id$

#ifndef STRINGUTILITIES_H
#define STRINGUTILITIES_H

#include <sstream>
#include "HopsanTypes.h"

namespace hopsan {

void santizeName(HString &rName);
HString santizeName(const HString &rName);

bool isNameValid(const HString &rString);
bool isNameValid(const HString &rString, const HString &rExceptions);

//! @brief Help function for create a unique name among names from one STL Container
template<typename ContainerT>
HString findUniqueName(const ContainerT &rContainer, HString name)
{
    // New name must not be empty, empty name is "reserved" to be used in the API to indicate that we want to manipulate the current root system
    if (name.empty())
    {
        name = "noName";
    }

    // Make sure name is sane
    santizeName(name);

    size_t ctr = 1; //The suffix number
    while(rContainer.find(name) != rContainer.end())
    {
        //strip suffix
        size_t foundpos = name.rfind('_');
        if (foundpos != HString::npos)
        {
            if (foundpos+1 < name.size())
            {
                unsigned char nr = name.at(foundpos+1);
                //cout << "nr after _: " << nr << endl;
                //Check the ascii code for the charachter
                if ((nr >= 48) && (nr <= 57))
                {
                    //Is number lets assume that the _ found is the beginning of a suffix
                    name.erase(foundpos, HString::npos);
                }
            }
        }
        //cout << "ctr: " << ctr << " stripped tempname: " << name << endl;

        //add new suffix
        std::stringstream suffix;
        suffix << ctr;
        name.append("_").append(suffix.str().c_str());
        ++ctr;
        //cout << "ctr: " << ctr << " appended tempname: " << name << endl;
    }
    //cout << name << endl;

    return name.c_str();
}

//inline bool contains(const HString &rString, const HString &rPattern)
//{
//    return rString.find(rPattern) != HString::npos;
//}

//HString &replace(HString &rString, const HString &rOld, const HString &rNew);

//void copyString(char** c, std::string s);

}

#endif // STRINGUTILITIES_H
