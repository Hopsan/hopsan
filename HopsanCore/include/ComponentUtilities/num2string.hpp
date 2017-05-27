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
