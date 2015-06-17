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
//! @file   win32dll.h
//! @author <peter.nordin@liu.se>
//! @date   2009-12-26
//!
//! @brief Handles DLLIMPORT, DLLEXPORT and DLLIMPORTEXPORT for win32 dll support
//!
//$Id$

#ifndef WIN32DLL_H_INCLUDED
#define WIN32DLL_H_INCLUDED

#ifdef _WIN32

//! Specifies that a function or class will only be exported on windows platforms
#define DLLEXPORT __declspec(dllexport)
//! Specifies that a function or class will only be imported on windows platforms
#define DLLIMPORT __declspec(dllimport)

//!
//! @def DLLIMPORTEXPORT
//! @brief Will either be DLLIMPORT or DLLEXPORT depending on how the files are used when compiling.
//!
//! If the files are compiled as part of a lib it will be DLLEXPORT
//! If the files are included as a lib they will be DLLIMPORT
//! This only applies to Windows operation systems
//!

#ifdef STATICCORE
#define DLLIMPORTEXPORT
#elif defined DOCOREDLLEXPORT
#define DLLIMPORTEXPORT DLLEXPORT /* DLL export */
#else
#define DLLIMPORTEXPORT DLLIMPORT /* EXE import */
#endif


#else
//Define nothing on non WIN32 systems
#define DLLEXPORT
#define DLLIMPORT
#define DLLIMPORTEXPORT

#endif



#endif // WIN32DLL_H_INCLUDED
