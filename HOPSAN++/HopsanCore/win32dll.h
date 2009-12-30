#ifndef WIN32DLL_H_INCLUDED
#define WIN32DLL_H_INCLUDED

#ifdef WIN32

#define DLLEXPORT __declspec(dllexport)
#define DLLIMPORT __declspec(dllimport)

#ifdef DODLLEXPORT
#define DLLDIR DLLEXPORT /* DLL export */
#else
#define DLLDIR DLLIMPORT /* EXE import */
#endif

#else
//Define nothing on non WIN32 systems
#define DLLEXPORT
#define DLLIMPORT
#define DLLDIR

#endif



#endif // WIN32DLL_H_INCLUDED
