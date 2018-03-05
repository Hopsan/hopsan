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
//! @file   win32dll.h
//! @brief Configure symbol export/import for Windows DLL linking
//!
//$Id$

#ifndef WIN32DLL_H_INCLUDED
#define WIN32DLL_H_INCLUDED

#ifdef _WIN32

//! Specifies that a function or class will be exported from a DLL  on Windows platforms
#define DLLEXPORT __declspec(dllexport)
//! Specifies that a function or class will be imported from a DLL on windows platforms
#define DLLIMPORT __declspec(dllimport)

//!
//! @def HOPSANCORE_DLLAPI
//! @brief Export or import symbol from a DLL
//!
//! If the core is compiled as a DLL, it should have the value DLLEXPORT,
//! When an application depends on the core as a DLL and includes the header files,
//! it should havevthe value DLLIMPORT
//! If the core is build as astatic library, this macro should be empty.
//! On non Windows platforms, it shall always be empty!
//!

#if defined HOPSANCORE_DLLEXPORT
#define HOPSANCORE_DLLAPI DLLEXPORT
#elif defined HOPSANCORE_DLLIMPORT
#define HOPSANCORE_DLLAPI DLLIMPORT
#else
#define HOPSANCORE_DLLAPI
#endif

#else

// Define nothing on non windows systems
#define DLLEXPORT
#define DLLIMPORT
#define HOPSANCORE_DLLAPI

#endif

#endif
