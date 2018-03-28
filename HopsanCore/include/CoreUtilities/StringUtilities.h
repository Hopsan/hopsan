/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   StringUtilities.h
//! @author <peter.nordin@liu.se>
//! @date   2013-03-12
//!
//! @brief Contains functions for working with strings, in particular Name strings.
//!
//$Id$

#ifndef STRINGUTILITIES_H
#define STRINGUTILITIES_H

#include <sstream>
#include <vector>
#include "HopsanTypes.h"

namespace hopsan {

void HOPSANCORE_DLLAPI santizeName(HString &rName);
HString HOPSANCORE_DLLAPI santizeName(const HString &rName);

bool HOPSANCORE_DLLAPI isNameValid(const HString &rString);
bool HOPSANCORE_DLLAPI isNameValid(const HString &rString, const HString &rExceptions);
void HOPSANCORE_DLLAPI splitString(const HString &rString, const char delim, std::vector<HString> &rParts);

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
                //Check the ascii code for the character
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
