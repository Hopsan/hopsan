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
//! @file   NameStringUtils.cc
//! @author <peter.nordin@liu.se>
//! @date   2013-03-12
//!
//! @brief Contains functions for working with strings, in particualr Name strings.
//!
//$Id: FindUniqueName.h 5130 2013-03-11 15:58:32Z petno25 $

#include "CoreUtilities/NameStringUtils.h"

#define UNDERSCORE 95
#define UPPERCASE_LOW 65
#define UPPERCASE_HIGH 90
#define LOWERCASE_LOW 97
#define LOWERCASE_HIGH 122
#define NUMBERS_LOW 48
#define NUMBERS_HIGH 57

void hopsan::santizeName(std::string &rString)
{
    std::string::iterator it;
    for (it=rString.begin(); it!=rString.end(); ++it)
    {
        if ( !( ((*it >= LOWERCASE_LOW) && (*it <= LOWERCASE_HIGH)) ||
                ((*it >= UPPERCASE_LOW) && (*it <= UPPERCASE_HIGH)) ||
                ((*it >= NUMBERS_LOW)   && (*it <= NUMBERS_HIGH))     ) )
        {
            // Replace invalid char with underscore
            *it = UNDERSCORE;
        }
    }
}

bool hopsan::isNameValid(const std::string &rString)
{
    std::string::const_iterator it;
    for (it=rString.begin(); it!=rString.end(); ++it)
    {
        if ( !( ((*it >= LOWERCASE_LOW) && (*it <= LOWERCASE_HIGH)) ||
                ((*it >= UPPERCASE_LOW) && (*it <= UPPERCASE_HIGH)) ||
                ((*it >= NUMBERS_LOW)   && (*it <= NUMBERS_HIGH))   ||
                (*it == UNDERSCORE)                                   ) )
        {
            // Return if we find invalid character
            return false;
        }
    }
    return true;
}

bool hopsan::isNameValid(const std::string &rString, const std::string &rExceptions)
{
    std::string::const_iterator it;
    for (it=rString.begin(); it!=rString.end(); ++it)
    {
        if ( !( ((*it >= LOWERCASE_LOW) && (*it <= LOWERCASE_HIGH)) ||
                ((*it >= UPPERCASE_LOW) && (*it <= UPPERCASE_HIGH)) ||
                ((*it >= NUMBERS_LOW)   && (*it <= NUMBERS_HIGH))   ||
                (*it == UNDERSCORE) || (rExceptions.find(*it)!=std::string::npos) ) )
        {
            // Return if we find invalid character
            return false;
        }
    }
    return true;
}

std::string hopsan::santizeName(const std::string &rString)
{
    std::string newString = rString;
    santizeName(newString);
    return newString;
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
