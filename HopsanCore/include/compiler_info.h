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
 #define HOPSANCOMPILEDWITH "MinGW GCC " TO_STR(__GNUC__) "." TO_STR(__GNUC_MINOR__)
 #define HOPSANCOMPILEDWITHGCC
 #ifdef __x86_64__
  #define HOPSANCOMPILED64BIT
 #endif
#elif defined(__GNUC__)
 #define HOPSANCOMPILEDWITH "GNU GCC " TO_STR(__GNUC__) "." TO_STR(__GNUC_MINOR__)
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
