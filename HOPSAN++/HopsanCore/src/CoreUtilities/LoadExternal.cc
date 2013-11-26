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

#include "CoreUtilities/LoadExternal.h"
#include "Component.h"
#include "Node.h"
#include "CoreUtilities/ClassFactoryStatusCheck.hpp"
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
LoadExternal::LoadExternal(ComponentFactory *pComponentFactory, NodeFactory *pNodeFactory, HopsanCoreMessageHandler *pMessenger)
{
    mpComponentFactory = pComponentFactory;
    mpNodeFactory = pNodeFactory;
    mpMessageHandler = pMessenger;
}

bool LoadExternal::load(const HString &rLibpath)
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

    // Add to libdir path to DLL Load Path
    size_t slashidx = rLibpath.rfind('/');
    if (slashidx!=string::npos)
    {
        //Set search path for dependencies
        const HString libdir = rLibpath.substr(0,slashidx);
        SetDllDirectoryA(libdir.c_str());
        mpMessageHandler->addDebugMessage("SetDllDirectoryA: " + libdir);
    }

    // Load library
    lib_ptr = LoadLibrary(rLibpath.c_str()); //Load the dll

//End of 32-bit

    if (!lib_ptr)
    {
        stringstream ss;
        ss << "Opening external library: " << rLibpath.c_str() << " GetLastError(): " << GetLastError();
        mpMessageHandler->addErrorMessage(ss.str().c_str());
        return false;
    }
    else
    {
        stringstream ss;
        ss << "Success (probably) opening external library: " << rLibpath.c_str();
        mpMessageHandler->addDebugMessage(ss.str().c_str());
    }

    bool isHopsanComponentLib=true;

    //Now get the version hopsan core was compiled against
    get_hopsan_info_t get_hopsan_info = (get_hopsan_info_t)GetProcAddress(lib_ptr, "get_hopsan_info");
    if (!get_hopsan_info)
    {
        stringstream ss;
        ss << "Cannot load symbol 'get_hopsan_info' for: " << rLibpath.c_str() << " Error: " << GetLastError();
        mpMessageHandler->addDebugMessage(ss.str().c_str());
        isHopsanComponentLib=false;
    }

    //Now load the register function
    register_contents_t register_contents = (register_contents_t)GetProcAddress(lib_ptr, "register_contents");
    if (!register_contents)
    {
        stringstream ss;
        ss << "Cannot load symbol 'register_contents' for: " << rLibpath.c_str() << " Error: " << GetLastError();
        mpMessageHandler->addDebugMessage(ss.str().c_str());
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
    lib_ptr = dlopen(rLibpath.c_str(), RTLD_NOW);
    //lib_ptr = dlopen(libpath.c_str(), RTLD_LAZY);
    if (!lib_ptr)
    {
        mpMessageHandler->addErrorMessage("Opening external lib: "+rLibpath+" dlerror(): "+dlerror());
        return false;
    }
    else
    {
        mpMessageHandler->addDebugMessage("Success (probably) opening external lib: "+rLibpath);
    }

    bool isHopsanComponentLib=true;

    //Now get the version hopsan core was compiled against
    get_hopsan_info_t get_hopsan_info = (get_hopsan_info_t)dlsym(lib_ptr, "get_hopsan_info");
    char *dlsym_error = dlerror();

    if (dlsym_error)
    {
        mpMessageHandler->addDebugMessage("Cannot load symbol 'get_hopsan_info' for: "+rLibpath+" Error: "+dlsym_error);
        isHopsanComponentLib = false;
    }

    //Now load the register function
    register_contents_t register_contents = (register_contents_t)dlsym(lib_ptr, "register_contents");
    dlsym_error = dlerror();
    if (dlsym_error)
    {
        mpMessageHandler->addErrorMessage("Cannot load symbol 'register_contents' for: "+rLibpath+" Error: "+dlsym_error);
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
    externalLibInfo.libName = (char*)""; //Init libname to avoid problem if it is missing
    get_hopsan_info(&externalLibInfo);

    if (string(externalLibInfo.libName).empty())
    {
        mpMessageHandler->addErrorMessage("Lib is missing (non-empty) libName when loading lib: " + rLibpath);
        isCorrectVersion = false;
    }

    stringstream ss;
    ss << "ExternalLib: " << rLibpath.c_str() <<  " compiled as: " << externalLibInfo.libCompiledDebugRelease << " against HopsanCore: " << externalLibInfo.hopsanCoreVersion;
    mpMessageHandler->addDebugMessage(ss.str().c_str());

    //Now check if we are compiled against correct version number
    if ( strcmp(externalLibInfo.hopsanCoreVersion, HOPSANCOREVERSION) != 0 )
    {
        stringstream ss;
        ss << "External lib: " << rLibpath.c_str() << " compiled against wrong HopsanCore version: " << externalLibInfo.hopsanCoreVersion << ", current version is: " << HOPSANCOREVERSION;
        mpMessageHandler->addErrorMessage(ss.str().c_str());
        isCorrectVersion = false;
    }

    //Now check if we are compiled against correct debug release version
    if ( strcmp(externalLibInfo.libCompiledDebugRelease, DEBUGRELEASECOMPILED) != 0 )
    {
        stringstream ss;
        ss << "ExternalLib: " << rLibpath.c_str() << " compiled as: " << externalLibInfo.libCompiledDebugRelease << " HopsanCore compiled as: " << DEBUGRELEASECOMPILED << ", You may run into problems!";
        mpMessageHandler->addWarningMessage(ss.str().c_str());
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
    checkClassFactoryStatus(mpComponentFactory, mpMessageHandler);
    checkClassFactoryStatus(mpNodeFactory, mpMessageHandler);

    // Ok everything seems Ok, now register the library ptr and registred components and nodes in map so that we can unload them later
    LoadedLibInfo lelInfo;
    // Remember a ptr to the lib
    lelInfo.mpLib = static_cast<void*>(lib_ptr);

    // Remember the lib name
    lelInfo.mLibName = externalLibInfo.libName;

    // Remeber which components belong to the lib
    ComponentFactory::RegStatusVectorT regCompVec = mpComponentFactory->getRegisterStatus();
    for (size_t i=0; i<regCompVec.size(); ++i)
    {
        if ( regCompVec[i].second == ComponentFactory::RegisteredOk )
        {
            lelInfo.mRegistredComponents.push_back(regCompVec[i].first);
        }
    }

    // Remeber which nodes belong to the lib
    ComponentFactory::RegStatusVectorT regNodeVec = mpNodeFactory->getRegisterStatus();
    for (size_t i=0; i<regNodeVec.size(); ++i)
    {
        if ( regNodeVec[i].second == ComponentFactory::RegisteredOk )
        {
            lelInfo.mRegistredNodes.push_back(regNodeVec[i].first);
        }
    }

    // Insert lib info in map
    mLoadedExtLibsMap.insert( std::pair<HString, LoadedLibInfo>( rLibpath, lelInfo ) );

    //Clear factory status
    mpComponentFactory->clearRegisterStatus();
    mpNodeFactory->clearRegisterStatus();

    mpMessageHandler->addInfoMessage("Loaded library: "+rLibpath);

    return true;
}

//! @brief This function unloads a library and its components and nodes
bool LoadExternal::unLoad(const HString &rLibpath)
{
    LoadedExtLibsMapT::iterator lelit = mLoadedExtLibsMap.find(rLibpath);
    if (lelit != mLoadedExtLibsMap.end())
    {
        for (size_t i=0; i<lelit->second.mRegistredComponents.size(); ++i)
        {
            mpComponentFactory->unRegisterCreatorFunction( lelit->second.mRegistredComponents[i] );
            //! @todo we should check register status to make sure unregistered
        }

        for (size_t i=0; i<lelit->second.mRegistredNodes.size(); ++i)
        {
            mpNodeFactory->unRegisterCreatorFunction( lelit->second.mRegistredNodes[i] );
            //! @todo we should check register status to make sure unregistered
        }

#ifdef WIN32
        HINSTANCE lib_ptr = static_cast<HINSTANCE>(lelit->second.mpLib);
        FreeLibrary(lib_ptr);
        //! @todo handle error messages after close
#else
        void* lib_ptr = lelit->second.mpLib;
        dlclose(lib_ptr);
        //! @todo handle error messages after close
#endif

        // Remove from lib map
        mLoadedExtLibsMap.erase(lelit);

        mpMessageHandler->addInfoMessage("Successfully unloaded: "+rLibpath);
    }
    else
    {
        mpMessageHandler->addWarningMessage("Could not unload: "+rLibpath+", library not found");
    }

    return true;
}

void LoadExternal::getLoadedLibNames(std::vector<HString> &rLibNames)
{
    rLibNames.clear();
    rLibNames.reserve(mLoadedExtLibsMap.size());
    LoadedExtLibsMapT::iterator lelit = mLoadedExtLibsMap.begin();
    for (; lelit!=mLoadedExtLibsMap.end(); ++lelit)
    {
        rLibNames.push_back(lelit->second.mLibName);
    }
}


//! @brief Returns library path (to dll or so file) for a component type
//! @param typeName Type name to look for
//! @param libPath Reference to string where lib path is stored
void LoadExternal::getLibPathByTypeName(const HString &rTypeName, HString &rLibPath)
{
    LoadedExtLibsMapT::iterator lelit = mLoadedExtLibsMap.begin();
    for(; lelit!=mLoadedExtLibsMap.end(); ++lelit)
    {
        for(size_t i=0; i<lelit->second.mRegistredComponents.size(); ++i)
        {
            if(lelit->second.mRegistredComponents[i] == rTypeName)
            {
                rLibPath = lelit->first;
            }
        }
    }
}

//! @brief Returns the components and nodes registered by specified library
//! @param[in] rLibpath Path to library
//! @param[out] rComponents Reference to vector with the loaded component type names
//! @param[out] rNodes Reference to vector with the loaded node type names
void LoadExternal::getLibContents(const HString &rLibpath, std::vector<HString> &rComponents, std::vector<HString> &rNodes)
{
    LoadedExtLibsMapT::iterator lelit = mLoadedExtLibsMap.find(rLibpath);
    if (lelit != mLoadedExtLibsMap.end())
    {
        for (size_t i=0; i<lelit->second.mRegistredComponents.size(); ++i)
        {
            rComponents.push_back(lelit->second.mRegistredComponents[i]);
        }
        for (size_t i=0; i<lelit->second.mRegistredNodes.size(); ++i)
        {
            rNodes.push_back(lelit->second.mRegistredNodes[i]);
        }
    }
}
