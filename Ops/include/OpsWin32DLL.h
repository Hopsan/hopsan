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
//! @file   OpsWin32DLL.h
//! @brief Handles symbol export/import macros for win32 dll support
//!
//$Id$

#ifndef OPSWIN32DLL_H_INCLUDED
#define OPSWIN32DLL_H_INCLUDED

#ifdef _WIN32

#if defined(OPS_DLLEXPORT)
#define OPS_DLLAPI __declspec(dllexport)
#elif defined(OPS_DLLIMPORT)
#define OPS_DLLAPI __declspec(dllimport)
#else /*Ops static library*/
#define OPS_DLLAPI
#endif

#else

// Define empty on non-windows systems
#define OPS_DLLAPI

#endif

#endif // OPSWIN32DLL_H_INCLUDED
