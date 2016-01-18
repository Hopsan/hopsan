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
