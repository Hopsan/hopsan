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
//! @file   StringUtilities.cc
//! @author <peter.nordin@liu.se>
//! @date   2013-03-12
//!
//! @brief Contains functions for working with strings, in particualr Name strings.
//!
//$Id$

#include "CoreUtilities/StringUtilities.h"
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>

#define UNDERSCORE 95
#define UPPERCASE_LOW 65
#define UPPERCASE_HIGH 90
#define LOWERCASE_LOW 97
#define LOWERCASE_HIGH 122
#define NUMBERS_LOW 48
#define NUMBERS_HIGH 57

void hopsan::santizeName(HString &rName)
{
    //! @todo HString do not have iterators, need to work with temp string for now
    //! @todo whay do we even need iterators can use normal index instead
    std::string str = rName.c_str();
    std::string::iterator it;
    for (it=str.begin(); it!=str.end(); ++it)
    {
        if ( !( ((*it >= LOWERCASE_LOW) && (*it <= LOWERCASE_HIGH)) ||
                ((*it >= UPPERCASE_LOW) && (*it <= UPPERCASE_HIGH)) ||
                ((*it >= NUMBERS_LOW)   && (*it <= NUMBERS_HIGH))     ) )
        {
            // Replace invalid char with underscore
            *it = UNDERSCORE;
        }
    }
    // Copy back the string
    rName = str.c_str();
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
    std::string sexceptions = rExceptions.c_str(); //!< @todo convert since we dont have find in hstring yet
    for (size_t i=0; i<rString.size(); ++i)
    {
        if ( !( ((rString[i] >= LOWERCASE_LOW) && (rString[i] <= LOWERCASE_HIGH)) ||
                ((rString[i] >= UPPERCASE_LOW) && (rString[i] <= UPPERCASE_HIGH)) ||
                ((rString[i] >= NUMBERS_LOW)   && (rString[i] <= NUMBERS_HIGH))   ||
                (rString[i] == UNDERSCORE) || (sexceptions.find(rString[i])!=std::string::npos) ) )
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

std::string &hopsan::replace(std::string &rString, const std::string &rOld, const std::string &rNew)
{
    size_t pos = rString.find(rOld);
    while (pos!=std::string::npos)
    {
        rString.replace(pos, rOld.size(), rNew);
        pos = rString.find(rOld);
    }
    return rString;
}


//! @brief Copies a std::string to a char* (with memory allocation)
//! @param [out] c Target string
//! @param [in] s Source string
void hopsan::copyString(char** c, std::string s)
{
    *c = (char *)realloc(*c, (strlen(s.c_str())+1)*sizeof(char));
    strcpy(*c, s.c_str());
}



