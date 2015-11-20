/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
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

void DLLIMPORTEXPORT santizeName(HString &rName);
HString DLLIMPORTEXPORT santizeName(const HString &rName);

bool DLLIMPORTEXPORT isNameValid(const HString &rString);
bool DLLIMPORTEXPORT isNameValid(const HString &rString, const HString &rExceptions);
void DLLIMPORTEXPORT splitString(const HString &rString, const char delim, std::vector<HString> &rParts);

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
