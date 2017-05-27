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
//! @file   compiler_info.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2013-12-03
//!
//! @brief Contains macros for deciding compiler and architecture used when building
//!
//$Id$

#ifndef COMPILER_INFO_H
#define COMPILER_INFO_H

#include "HopsanCoreMacros.h"

// Decide compiler and architecture 32 or 64 bit version information
// Note! MinGW32 is also defined on MinGW-w64,
// Note2! __GNUC__ is also defined on MinGW (that is why we check for MinGW first)

#if defined(__MINGW32__)
 #define HOPSANCOMPILEDWITH "MinGW GCC " TO_STR(__GNUC__) "." TO_STR(__GNUC_MINOR__) "." TO_STR(__GNUC_PATCHLEVEL__)
 #define HOPSANCOMPILEDWITHGCC
 #ifdef __x86_64__
  #define HOPSANCOMPILED64BIT
 #endif
#elif defined(__GNUC__)
 #define HOPSANCOMPILEDWITH "GNU GCC " TO_STR(__GNUC__) "." TO_STR(__GNUC_MINOR__) "." TO_STR(__GNUC_PATCHLEVEL__)
 #define HOPSANCOMPILEDWITHGCC
 #ifdef __x86_64__
  #define HOPSANCOMPILED64BIT
 #endif
#elif defined(_MSC_VER)
 #define HOPSANCOMPILEDWITH "MSVC " TO_STR(_MSC_VER)
 #define HOPSANCOMPILEDWITHMSVC
 #ifdef _M_X64
  #define HOPSANCOMPILED64BIT
 #endif
#else
 #define HOPSANCOMPILEDWITH "Unknown Compiler"
#endif

//! @todo add LLVM (CLANG) here

#endif // COMPILER_INFO_H
