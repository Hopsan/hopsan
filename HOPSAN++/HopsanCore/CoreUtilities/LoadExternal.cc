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
#include <iostream>

#ifdef WIN32
#include "windows.h"
#else
#include "dlfcn.h"
#endif

using namespace std;
using namespace hopsan;

//! @todo maybe the LoadExternal class is unnecessary, contents are only used in hopsan essentials, could maybe move stuff there

//!LoadExternal Constructor
//LoadExternal::LoadExternal()
//{
//    //ctor
//}

//!This function loads a library with given path
bool LoadExternal::load(string libpath)
{
    //typedef void (*register_contents_t)(ComponentFactory::FactoryPairVectorT *factory_vector_ptr);
    typedef void (*register_contents_t)(ComponentFactory* pComponentFactory, NodeFactory* pNodeFactory);
//! @todo Write some message output if DLL/SO fails to load or similar
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
        cout << "Error opening external lib: " << libpath << endl;
        //cout << dlerror() << endl;
        return false;
        //exit(-1);
    }
    else
    {
        cout << "Succes (probably) opening external lib: " << libpath << endl;
    }
    //Now load the register function
    register_contents_t register_contents = (register_contents_t)GetProcAddress(lib_ptr, "register_contents");
    if (!register_contents)
    {
        cout << "Cannot load symbol 'register_contents': " << GetLastError() << endl;
        //dlclose(handle);
        return false;
    }
#else
    void *lib_ptr;
    //First open the lib
    lib_ptr = dlopen(libpath.c_str(), RTLD_NOW);
    //lib_ptr = dlopen(libpath.c_str(), RTLD_LAZY);
    if (!lib_ptr)
    {
        cout << "Error opening external lib: " << libpath << endl;
        cout << dlerror() << endl;
        return false;
        //exit(-1);
    }
    else
    {
        cout << "Succes (probably) opening external lib: " << libpath << endl;
    }
    //Now load the register function
    register_contents_t register_contents = (register_contents_t)dlsym(lib_ptr, "register_contents");
    const char *dlsym_error = dlerror();
    if (dlsym_error)
    {
        cout << "Cannot load symbol 'register_contents': " << dlsym_error << endl;
        //dlclose(handle);
        return false;
    }

#endif

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
