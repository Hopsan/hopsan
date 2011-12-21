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
//! @file   num2str.hpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2011-12-21
//!
//! @brief Contains help utility functions for converting a number to a string
//!
//$Id: Delay.hpp 3547 2011-10-25 11:48:47Z petno25 $

#ifndef NUM2STRING_HPP
#define NUM2STRING_HPP

#include <string>
#include <sstream>

inline
std::string num2string(const double num)
{
    std::stringstream ss;
    ss << num;
    return ss.str();
}

inline
std::string num2string(const float num)
{
    std::stringstream ss;
    ss << num;
    return ss.str();
}

inline
std::string num2string(const int num)
{
    std::stringstream ss;
    ss << num;
    return ss.str();
}

#endif // NUM2STRING_HPP
