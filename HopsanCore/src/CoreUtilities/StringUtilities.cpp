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
//! @file   StringUtilities.cpp
//! @author <peter.nordin@liu.se>
//! @date   2013-03-12
//!
//! @brief Contains functions for working with strings, in particualr Name strings.
//!
//$Id$

#include "CoreUtilities/StringUtilities.h"
//#include <stdio.h>
//#include <iostream>
//#include <stdlib.h>

#define UNDERSCORE 95
#define UPPERCASE_LOW 65
#define UPPERCASE_HIGH 90
#define LOWERCASE_LOW 97
#define LOWERCASE_HIGH 122
#define NUMBERS_LOW 48
#define NUMBERS_HIGH 57

void hopsan::santizeName(HString &rName)
{
    for (size_t i=0; i<rName.size(); ++i)
    {
        if ( !( ((rName[i] >= LOWERCASE_LOW) && (rName[i] <= LOWERCASE_HIGH)) ||
                ((rName[i] >= UPPERCASE_LOW) && (rName[i] <= UPPERCASE_HIGH)) ||
                ((rName[i] >= NUMBERS_LOW)   && (rName[i] <= NUMBERS_HIGH))     ) )
        {
            // Replace invalid char with underscore
            rName[i] = UNDERSCORE;
        }
    }
}

bool hopsan::isNameValid(const HString &rString)
{
    for (size_t i=0; i<rString.size(); ++i)
    {
        if ( !( ((rString[i] >= LOWERCASE_LOW) && (rString[i] <= LOWERCASE_HIGH)) ||
                ((rString[i] >= UPPERCASE_LOW) && (rString[i] <= UPPERCASE_HIGH)) ||
                ((rString[i] >= NUMBERS_LOW)   && (rString[i] <= NUMBERS_HIGH))   ||
                (rString[i] == UNDERSCORE)                                   ) )
        {
            // Return if we find invalid character
            return false;
        }
    }
    return true;
}

bool hopsan::isNameValid(const HString &rString, const HString &rExceptions)
{
    for (size_t i=0; i<rString.size(); ++i)
    {
        if ( !( ((rString[i] >= LOWERCASE_LOW) && (rString[i] <= LOWERCASE_HIGH)) ||
                ((rString[i] >= UPPERCASE_LOW) && (rString[i] <= UPPERCASE_HIGH)) ||
                ((rString[i] >= NUMBERS_LOW)   && (rString[i] <= NUMBERS_HIGH))   ||
                (rString[i] == UNDERSCORE) || (rExceptions.containes(rString[i])) ) )
        {
            // Return if we find invalid character
            return false;
        }
    }
    return true;
}

hopsan::HString hopsan::santizeName(const HString &rName)
{
    HString name = rName;
    santizeName(name);
    return name;
}

//! @deprecated Use HString::split(char delim) instead
void hopsan::splitString(const HString &rString, const char delim, std::vector<HString> &rParts)
{
    rParts.clear();
    std::string item;
    std::stringstream ss(rString.c_str());
    while(getline(ss, item, delim))
    {
        rParts.push_back(item.c_str());
    }
}
