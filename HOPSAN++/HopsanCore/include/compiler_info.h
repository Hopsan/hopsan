//!
//! @file   compiler_info.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2013-12-03
//!
//! @brief Contains macros for deciding compiler and architecture used when building
//!
//$Id: common.h 6112 2013-11-06 11:43:30Z robbr48 $

#ifndef COMPILER_INFO_H
#define COMPILER_INFO_H

// Stringify Macro
#ifndef TO_STR_2
 #define TO_STR_2(s) #s
 #define TO_STR(s) TO_STR_2(s)
#endif

// Decide compiler and architecture 32 or 64 bit version information
// Note! MinGW32 is also defined on MinGW-w64,
// Note2! __GNUC__ is also defined on MinGW (that is why we chech for MinGW first)

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
