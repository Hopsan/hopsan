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

#include <string>
#ifdef WIN32
#define _WIN32_WINNT 0x0502
#include "Windows.h"
#else
#include "dlfcn.h"
#endif
#include "win32dll.h"

#include "CoreUtilities/GeneratorHandler.h"

using namespace hopsan;

GeneratorHandler::GeneratorHandler()
{
    mLoadedSuccessfully = false;

#ifdef WIN32
    HINSTANCE lib_ptr;
    lib_ptr = LoadLibrary("HopsanGenerator.dll"); //Load the dll

    if (!lib_ptr)
    {
        //! @todo Error message
        return;
    }

    //Load modelica generator function
    callModelicaGenerator = (call_modelica_generator_t)GetProcAddress(lib_ptr, "callModelicaGenerator");
    if (!callModelicaGenerator)
    {
        //! @todo Error message
        return;
    }

    //Load C++ generator function
    callCppGenerator = (call_cpp_generator_t)GetProcAddress(lib_ptr, "callCppGenerator");
    if (!callCppGenerator)
    {
        //! @todo Error message
        return;
    }

    //Load FMU generator function
    callFmuGenerator = (call_fmu_generator_t)GetProcAddress(lib_ptr, "callFmuGenerator");
    if (!callFmuGenerator)
    {
        //! @todo Error message
        return;
    }

#else
    void *lib_ptr;
    lib_ptr = dlopen("libHopsanGenerator.so", RTLD_NOW);  //Load the dll
    if (!lib_ptr)
    {
        //! @todo Error message
        return;
    }

    //Load modelica generator function
    callModelicaGenerator = (call_modelica_generator_t)dlsym(lib_ptr, "callModelicaGenerator");
    char *dlsym_error = dlerror();
    if (dlsym_error)
    {
        //! @todo Error message
        return;
    }

    //Load C++ generator function
    callCppGenerator = (call_cpp_generator_t)dlsym(lib_ptr, "callCppGenerator");
    dlsym_error = dlerror();
    if (dlsym_error)
    {
        //! @todo Error message
        return;
    }

    //Load FMU generator function
    callFmuGenerator = (call_fmu_generator_t)dlsym(lib_ptr, "callFmuGenerator");
    dlsym_error = dlerror();
    if (dlsym_error)
    {
        //! @todo Error message
        return;
    }
#endif

    //! @todo Success message
    mLoadedSuccessfully = true;
}



GeneratorHandler::~GeneratorHandler()
{
#ifdef WIN32
    FreeLibrary(lib_ptr);
#else
    dlclose(lib_ptr);
#endif
}



bool GeneratorHandler::isLoadedSuccessfully()
{
    return mLoadedSuccessfully;
}


