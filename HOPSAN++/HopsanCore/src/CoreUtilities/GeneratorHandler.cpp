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

#include <stdio.h>
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
#ifdef QT_NO_DEBUG
    lib_ptr = LoadLibrary("HopsanGenerator.dll"); //Load the dll
#else
    lib_ptr = LoadLibrary("HopsanGenerator_d.dll"); //Load the dll
#endif

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
    callFmuImportGenerator = (call_fmu_import_generator_t)GetProcAddress(lib_ptr, "callFmuImportGenerator");
    if (!callFmuImportGenerator)
    {
        //! @todo Error message
        return;
    }

    //Load FMU generator function
    callFmuExportGenerator = (call_fmu_export_generator_t)GetProcAddress(lib_ptr, "callFmuExportGenerator");
    if (!callFmuExportGenerator)
    {
        //! @todo Error message
        return;
    }

    //Load Simulink generator function
    callSimulinkExportGenerator = (call_simulink_export_generator_t)GetProcAddress(lib_ptr, "callSimulinkExportGenerator");
    if (!callSimulinkExportGenerator)
    {
        //! @todo Error message
        return;
    }

    //Load Simulink Co-Simulation generator function
    callSimulinkCoSimExportGenerator = (call_simulink_cosim_export_generator_t)GetProcAddress(lib_ptr, "callSimulinkCoSimExportGenerator");
    if (!callSimulinkCoSimExportGenerator)
    {
        //! @todo Error message
        return;
    }

    //Load LabVIEW/SIT generator function
    callLabViewSITGenerator = (call_lvsit_export_generator_t)GetProcAddress(lib_ptr, "callLabViewSITGenerator");
    if (!callLabViewSITGenerator)
    {
        //! @todo Error message
        return;
    }

    //Load compile component library function
    callComponentLibraryCompiler = (call_complib_compiler_t)GetProcAddress(lib_ptr, "callComponentLibraryCompiler");
    if(!callComponentLibraryCompiler)
    {
        //! @todo Error message
        return;
    }

#else
    void *lib_ptr;
#ifdef QT_NO_DEBUG
    lib_ptr = dlopen("libHopsanGenerator.so", RTLD_NOW);  //Load the dll
#else
    lib_ptr = dlopen("libHopsanGenerator_d.so", RTLD_NOW);  //Load the dll
#endif
    if(!lib_ptr)
    {
        fprintf (stderr, "%s\n", dlerror());
        //! @todo Error message
        return;
    }

    //Load modelica generator function
    callModelicaGenerator = (call_modelica_generator_t)dlsym(lib_ptr, "callModelicaGenerator");
    char *dlsym_error = dlerror();
    if(dlsym_error)
    {
        //! @todo Error message
        return;
    }

    //Load C++ generator function
    callCppGenerator = (call_cpp_generator_t)dlsym(lib_ptr, "callCppGenerator");
    dlsym_error = dlerror();
    if(dlsym_error)
    {
        //! @todo Error message
        return;
    }

    //Load FMU generator function
    callFmuImportGenerator = (call_fmu_import_generator_t)dlsym(lib_ptr, "callFmuImportGenerator");
    dlsym_error = dlerror();
    if(dlsym_error)
    {
        //! @todo Error message
        return;
    }

    //Load FMU generator function
    callFmuExportGenerator = (call_fmu_export_generator_t)dlsym(lib_ptr, "callFmuExportGenerator");
    dlsym_error = dlerror();
    if(dlsym_error)
    {
        //! @todo Error message
        return;
    }

    //! @todo Shall not be here, since Simulink export does not currently work under Linux
    //Load Simulink Co-Simulation generator function
    callSimulinkCoSimExportGenerator = (call_simulink_cosim_export_generator_t)dlsym(lib_ptr, "callSimulinkCoSimExportGenerator");
    dlsym_error = dlerror();
    if(dlsym_error)
    {
        //! @todo Error message
        return;
    }

    //Load LabVIEW/SIT generator function
    callLabViewSITGenerator = (call_lvsit_export_generator_t)dlsym(lib_ptr, "callLabViewSITGenerator");
    dlsym_error = dlerror();
    if (dlsym_error)
    {
        //! @todo Error message
        return;
    }

    //Load compile component library function
    callComponentLibraryCompiler = (call_complib_compiler_t)dlsym(lib_ptr, "callComponentLibraryCompiler");
    dlsym_error = dlerror();
    if(dlsym_error)
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


