/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
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

#ifdef WIN32

//! Spcifies that a function or class will only be exported on windows platforms
#define DLLEXPORT __declspec(dllexport)
//! Spcifies that a function or class will only be imported on windows platforms
#define DLLIMPORT __declspec(dllimport)

//!
//! @def DLLIMPORTEXPORT
//! @brief Will either be DLLIMPORT or DLLEXPORT depending on how the files are used when comiling.
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
