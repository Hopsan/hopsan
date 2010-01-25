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

#define DLLEXPORT __declspec(dllexport)
#define DLLIMPORT __declspec(dllimport)

#ifdef DOCOREDLLEXPORT
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
