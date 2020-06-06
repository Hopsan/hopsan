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
//! @file   HopsanCoreMacros.h
//! @author <peter.nordin@liu.se>
//!
//! @brief Contains HopsanCore global macro definitions
//!
//$Id$

#ifndef HOPSANCOREMACROS_H
#define HOPSANCOREMACROS_H

//! @brief This macro can be used to indicate that a function argument is unused and avoids a compiler warning
#define HOPSAN_UNUSED(x) (void)x;

// Stringify Macro
#ifndef TO_STR_2
 #define TO_STR_2(s) #s
 //! @brief This macro is used to turn a defined value into a string
 #define TO_STR(s) TO_STR_2(s)
#endif

#if defined _WIN32
#define SHAREDLIB_PREFIX ""
#define SHAREDLIB_SUFFIX "dll"
#elif defined __APPLE__
#define SHAREDLIB_PREFIX "lib"
#define SHAREDLIB_SUFFIX "dylib"
#else
#define SHAREDLIB_PREFIX "lib"
#define SHAREDLIB_SUFFIX "so"
#endif

#endif // HOPSANCOREMACROS_H
