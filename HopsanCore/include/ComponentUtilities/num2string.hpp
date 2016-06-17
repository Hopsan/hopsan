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
//! @file   num2string.hpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2011-12-21
//!
//! @brief Contains help utility functions for converting a number to a string
//!
//$Id$

#ifndef NUM2STRING_HPP
#define NUM2STRING_HPP

#include <sstream>
#include <HopsanTypes.h>

template<typename T>
inline hopsan::HString to_hstring(const T num, std::streamsize precision=17)
{
    std::stringstream ss;
    ss.precision(precision);
    ss << num;
    return ss.str().c_str();
}

#endif // NUM2STRING_HPP
