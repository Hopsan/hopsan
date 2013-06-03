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
