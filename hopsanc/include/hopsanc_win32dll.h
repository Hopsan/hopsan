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
//! @author <peter.nordin@liu.se>
//! @date   2009-12-26
//!
//! @brief Handles DLLIMPORT, DLLEXPORT and HOPSANC_DLLAPI for win32 dll support
//!
//$Id$

#ifndef HOPSANC_WIN32DLL_H
#define HOPSANC_WIN32DLL_H

#ifdef _WIN32


//!
//! @def HOPSANC_DLLAPI
//! @brief Will either be DLLIMPORT or DLLEXPORT depending on how the files are used when compiling.
//!
//! If the files are compiled as part of a lib it will be DLLEXPORT
//! If the files are included as a lib they will be DLLIMPORT
//! This only applies to Windows operation systems
//!



#if defined HOPSANC_DLLEXPORT
#define HOPSANC_DLLAPI __declspec(dllexport)
#elif defined HOPSANC_DLLIMPORT
#define HOPSANC_DLLAPI __declspec(dllimport)
#else /* Static library */
#define HOPSANC_DLLAPI
#endif

#else
// Define empty on non WIN32 systems
#define HOPSANC_DLLAPI

#endif

#endif
