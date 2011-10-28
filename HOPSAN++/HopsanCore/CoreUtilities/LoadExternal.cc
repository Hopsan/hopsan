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
#include "Component.h"
#include "Node.h"
#include "ClassFactoryStatusCheck.hpp"
#include "version.h"

#include <sstream>
#include <cstring>

#ifdef WIN32
#define _WIN32_WINNT 0x0502
#include "win32dll.h"
#include "Windows.h"
#else
#include "dlfcn.h"
#endif


using namespace std;
using namespace hopsan;

//!This function loads a library with given path
bool LoadExternal::load(string libpath)
{
    typedef void (*register_contents_t)(ComponentFactory* pComponentFactory, NodeFactory* pNodeFactory);
    typedef void (*get_hopsan_info_t)(HopsanExternalLibInfoT *pHopsanExternalLibInfo);

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

    //Calculate path without filename
    string libdir = libpath;
    while(libdir.at(libdir.size()-1) != '/')
    {
        libdir.erase(libdir.size()-1, 1);
    }
    getCoreMessageHandlerPtr()->addDebugMessage(libdir.c_str());
    SetDllDirectoryA(libdir.c_str());       //Set search path for dependencies
    lib_ptr = LoadLibrary(libpath.c_str()); //Load the dll

//End of 32-bit

    if (!lib_ptr)
    {
        stringstream ss;
        ss << "Error opening external library: " << libpath << " Error: " << GetLastError();
        gCoreMessageHandler.addDebugMessage(ss.str());
        return false;
    }
    else
    {
        stringstream ss;
        ss << "Success (probably) opening external library: " << libpath;
        gCoreMessageHandler.addDebugMessage(ss.str());
    }

    bool isHopsanComponentLib=true;

    //Now get the version hopsan core was compiled against
    get_hopsan_info_t get_hopsan_info = (get_hopsan_info_t)GetProcAddress(lib_ptr, "get_hopsan_info");
    if (!get_hopsan_info)
    {
        stringstream ss;
        ss << "Cannot load symbol 'get_hopsan_info' for: " << libpath << " Error: " << GetLastError();
        gCoreMessageHandler.addDebugMessage(ss.str());
        isHopsanComponentLib=false;
    }

    //Now load the register function
    register_contents_t register_contents = (register_contents_t)GetProcAddress(lib_ptr, "register_contents");
    if (!register_contents)
    {
        stringstream ss;
        ss << "Cannot load symbol 'register_contents' for: " << libpath << " Error: " << GetLastError();
        gCoreMessageHandler.addDebugMessage(ss.str());
        isHopsanComponentLib=false;
    }

    if(!isHopsanComponentLib)
    {
        FreeLibrary(lib_ptr);
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

    bool isHopsanComponentLib=true;

    //Now get the version hopsan core was compiled against
    get_hopsan_info_t get_hopsan_info = (get_hopsan_info_t)dlsym(lib_ptr, "get_hopsan_info");
    char *dlsym_error = dlerror();

    if (dlsym_error)
    {
        stringstream ss;
        ss << "Cannot load symbol 'get_hopsan_info' for: " << libpath << " Error: " << dlsym_error;
        gCoreMessageHandler.addDebugMessage(ss.str());
        isHopsanComponentLib = false;
    }

    //Now load the register function
    register_contents_t register_contents = (register_contents_t)dlsym(lib_ptr, "register_contents");
    dlsym_error = dlerror();
    if (dlsym_error)
    {
        stringstream ss;
        ss << "Cannot load symbol 'register_contents' for: " << libpath << " Error: " << dlsym_error;
        gCoreMessageHandler.addErrorMessage(ss.str());
        isHopsanComponentLib=false;
    }

    if(!isHopsanComponentLib)
    {
        dlclose(lib_ptr);
        return false;
    }

#endif

    bool isCorrectVersion = true;

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
        isCorrectVersion = false;
    }

    //Now check if we are compiled against correct debug release version
    if ( strcmp(externalLibInfo.libCompiledDebugRelease, DEBUGRELEASECOMPILED) != 0 )
    {
        stringstream ss;
        ss << "ExternalLib: " << libpath << " compiled as: " << externalLibInfo.libCompiledDebugRelease << " HopsanCore compiled as: " << DEBUGRELEASECOMPILED << ", You may run into problems!";
        gCoreMessageHandler.addWarningMessage(ss.str());
        //isCorrectVersion = false;
    }

    if (!isCorrectVersion)
    {
        // If wrong version free lib and return false
#ifdef WIN32
        FreeLibrary(lib_ptr);
#else
        dlclose(lib_ptr);
#endif
        return false;
    }

    register_contents(mpComponentFactory, mpNodeFactory);

    //Check for register errors and status
    checkClassFactoryStatus<ComponentFactory>(mpComponentFactory);
    checkClassFactoryStatus<NodeFactory>(mpNodeFactory);

    // Ok everything seems Ok, now register the library ptr and registreed components in map so that we can unload it later
    LoadedLibInfoPairT lelInfoPair(static_cast<void*>(lib_ptr), vector<string>() );
    ComponentFactory::RegStatusVectorT regCompVec = mpComponentFactory->getRegisterStatusMap();
    for (size_t i=0; i<regCompVec.size(); ++i)
    {
        if ( regCompVec[i].second == ComponentFactory::REGISTEREDOK )
        {
            lelInfoPair.second.push_back(regCompVec[i].first);
        }
    }
    mLoadedExtLibsMap.insert( std::pair<string, LoadedLibInfoPairT>( libpath, lelInfoPair ) );
    //!  @todo We also need to remember what nodes were loaded

    //Clear factory status
    mpComponentFactory->clearRegisterStatusMap();
    mpNodeFactory->clearRegisterStatusMap();

    return true;
}

bool LoadExternal::unLoad(std::string libpath)
{
    stringstream ss;
    LoadedExtLibsMapT::iterator lelit = mLoadedExtLibsMap.find(libpath);
    if (lelit != mLoadedExtLibsMap.end())
    {
        for (size_t i=0; i<lelit->second.second.size(); ++i)
        {
            mpComponentFactory->unRegisterCreatorFunction( lelit->second.second[i] );
            //! @todo we should check register status to make sure unregistered
        }

        //! @todo we should really need to unregister nodes also

#ifdef WIN32
        HINSTANCE lib_ptr = static_cast<HINSTANCE>(lelit->second.first);
        FreeLibrary(lib_ptr);
        //! @todo handle error messages after close
#else
        void* lib_ptr = (*lelit).second.first;
        dlclose(lib_ptr);
        //! @todo handle error messages after close
#endif
        ss << "Sucessfully unloaded: " << libpath;
        gCoreMessageHandler.addInfoMessage(ss.str());
    }
    else
    {
        ss << "Could not unload: " << libpath << ", library not found";
        gCoreMessageHandler.addWarningMessage(ss.str());
    }

    return true;
}

//!This function sets the node and component factory pointers
void LoadExternal::setFactory(ComponentFactory* pComponentFactory, NodeFactory* pNodeFactory)
{
    mpComponentFactory = pComponentFactory;
    mpNodeFactory = pNodeFactory;
}
