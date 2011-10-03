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
//! @file   LoadExternal.cc
//! @author <peter.nordin@liu.se>
//! @date   2009-12-22
//!
//! @brief Contains the ExternalLoader class
//!
//$Id$

#include "LoadExternal.h"
#include "../Component.h"
#include "../Node.h"
#include "ClassFactoryStatusCheck.hpp"
#include "version.h"

#include <sstream>
#include <cstring>

#ifdef WIN32
#include "windows.h"
#include "../win32dll.h"
#else
#include "dlfcn.h"
#endif


using namespace std;
using namespace hopsan;

//! @todo maybe the LoadExternal class is unnecessary, contents are only used in hopsan essentials, could maybe move stuff there

//!This function loads a library with given path
bool LoadExternal::load(string libpath)
{
    typedef void (*register_contents_t)(ComponentFactory* pComponentFactory, NodeFactory* pNodeFactory);
    typedef void (*get_hopsan_info_t)(HopsanExternalLibInfoT *pHopsanExternalLibInfo);

    bool hopsanInfoExists = true; //!< @todo temporary variable, remove later

#ifdef WIN32
    HINSTANCE lib_ptr;

//Use this for 64-bit compilation
//    int len;
//    int slength = (int)libpath.length() + 1;
//    len = MultiByteToWideChar(CP_ACP, 0, libpath.c_str(), slength, 0, 0);
//    wchar_t* buf = new wchar_t[len];
//    MultiByteToWideChar(CP_ACP, 0, libpath.c_str(), slength, buf, len);
//    std::wstring r(buf);
//    delete[] buf;
//    LPCWSTR wstr = r.c_str();
//    lib_ptr = LoadLibrary(wstr);
//End of 64-bit

//Use this for 32-bit compilation
    lib_ptr = LoadLibrary(libpath.c_str());
//End of 32-bit

    if (!lib_ptr)
    {
        stringstream ss;
        ss << "Error opening external lib: " << libpath << " Error: " << GetLastError();
        gCoreMessageHandler.addErrorMessage(ss.str());
        return false;
    }
    else
    {
        stringstream ss;
        ss << "Succes (probably) opening external lib: " << libpath;
        gCoreMessageHandler.addDebugMessage(ss.str());
    }

    //Now get the version hopsan core was compiled against
    get_hopsan_info_t get_hopsan_info = (get_hopsan_info_t)GetProcAddress(lib_ptr, "get_hopsan_info");
    if (!get_hopsan_info)
    {
        //#warning for 0.5.0 release, mak sure we run this check and abort / return ERROR
        stringstream ss;
        ss << "Cannot load symbol 'get_hopsan_info' for: " << libpath << " Error: " << GetLastError();
        gCoreMessageHandler.addDebugMessage(ss.str());
//        return false;
        hopsanInfoExists = false;
    }

    //Now load the register function
    register_contents_t register_contents = (register_contents_t)GetProcAddress(lib_ptr, "register_contents");
    if (!register_contents)
    {
        stringstream ss;
        ss << "Cannot load symbol 'register_contents' for: " << libpath << " Error: " << GetLastError();
        gCoreMessageHandler.addErrorMessage(ss.str());
        return false;
    }
#else
    void *lib_ptr;
    //First open the lib
    lib_ptr = dlopen(libpath.c_str(), RTLD_NOW);
    //lib_ptr = dlopen(libpath.c_str(), RTLD_LAZY);
    if (!lib_ptr)
    {
        stringstream ss;
        ss << "Error opening external lib: " << libpath << " Error: " << dlerror();
        gCoreMessageHandler.addErrorMessage(ss.str());
        return false;
    }
    else
    {
        stringstream ss;
        ss << "Succes (probably) opening external lib: " << libpath;
        gCoreMessageHandler.addDebugMessage(ss.str());
    }

    //Now get the version hopsan core was compiled against
    get_hopsan_info_t get_hopsan_info = (get_hopsan_info_t)dlsym(lib_ptr, "get_hopsan_info");
    char *dlsym_error = dlerror();

    if (dlsym_error)
    {
        //#warning for 0.5.0 release, make sure we run this check and abort / return ERROR
        stringstream ss;
        ss << "Cannot load symbol 'get_hopsan_info' for: " << libpath << " Error: " << dlsym_error;
        gCoreMessageHandler.addDebugMessage(ss.str());
//        return false;
        hopsanInfoExists = false;
    }

    //Now load the register function
    register_contents_t register_contents = (register_contents_t)dlsym(lib_ptr, "register_contents");
    dlsym_error = dlerror();
    if (dlsym_error)
    {
        stringstream ss;
        ss << "Cannot load symbol 'register_contents' for: " << libpath << " Error: " << dlsym_error;
        gCoreMessageHandler.addErrorMessage(ss.str());
        return false;
    }

#endif

    if (hopsanInfoExists)
    {
        HopsanExternalLibInfoT externalLibInfo;
        get_hopsan_info(&externalLibInfo);

        stringstream ss;
        ss << "ExternalLib: " << libpath <<  " compiled as: " << externalLibInfo.libCompiledDebugRelease << " against HopsanCore: " << externalLibInfo.hopsanCoreVersion;
        gCoreMessageHandler.addDebugMessage(ss.str());

        //Now check if we are compiled against correct version number
        if ( strcmp(externalLibInfo.hopsanCoreVersion, HOPSANCOREVERSION) != 0 )
        {
            stringstream ss;
            ss << "External lib: " << libpath << " compiled against wrong HopsanCore version: " << externalLibInfo.hopsanCoreVersion << ", current version is: " << HOPSANCOREVERSION;
            gCoreMessageHandler.addErrorMessage(ss.str());
            return false;
        }

        //Now check if we are compiled against correct debug release version
        if ( strcmp(externalLibInfo.libCompiledDebugRelease, DEBUGRELEASECOMPILED) != 0 )
        {
            stringstream ss;
            ss << "ExternalLib: " << libpath << " compiled as: " << externalLibInfo.libCompiledDebugRelease << " HopsanCore compiled as: " << DEBUGRELEASECOMPILED << ", You may run into problems!";
            gCoreMessageHandler.addWarningMessage(ss.str());
            //return false;
        }
    }

    //! @todo if we abort loading in code above we must remember to run dlclose or similar on the dll handle, right now we do not do that
    register_contents(mpComponentFactory, mpNodeFactory);

    //Check for register errors and status
    checkClassFactoryStatus<ComponentFactory>(mpComponentFactory);
    checkClassFactoryStatus<NodeFactory>(mpNodeFactory);

    //Clear factory status
    mpComponentFactory->clearRegisterStatusMap();
    mpNodeFactory->clearRegisterStatusMap();

    return true;
}

//!This function sets the node and component factory pointers
void LoadExternal::setFactory(ComponentFactory* pComponentFactory, NodeFactory* pNodeFactory)
{
    mpComponentFactory = pComponentFactory;
    mpNodeFactory = pNodeFactory;
}
