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

#include <stdio.h>
#ifdef _WIN32
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

#ifdef _WIN32
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

    //Load library generator function
    callLibraryGenerator = (call_library_generator_t)GetProcAddress(lib_ptr, "callLibraryGenerator");
    if(!callLibraryGenerator)
    {
        //! @todo Error message
        return;
    }

#else
    void *lib_ptr;
#ifdef QT_NO_DEBUG // magse: why NO_DEBUG?
#ifdef Q_OS_OSX
    lib_ptr = dlopen("libHopsanGenerator.dylib", RTLD_NOW);  //Load the dylib
#else
    lib_ptr = dlopen("libHopsanGenerator.so", RTLD_NOW);  //Load the dll
#endif
#else
#ifdef Q_OS_OSX
    lib_ptr = dlopen("libHopsanGenerator_d.dylib", RTLD_NOW);  //Load the dylib
#else
    lib_ptr = dlopen("libHopsanGenerator_d.so", RTLD_NOW);  //Load the dll
#endif
#endif
    if(!lib_ptr)
    {
        fprintf (stderr, "%s\n", dlerror());
        //! @todo Error message
        return;
    }

    //Load Modelica generator function
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

    //Load Simulink Co-Simulation generator function
    callSimulinkExportGenerator = (call_simulink_export_generator_t)dlsym(lib_ptr, "callSimulinkExportGenerator");
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

    //Load library generator function
    callLibraryGenerator = (call_library_generator_t)dlsym(lib_ptr, "callLibraryGenerator");
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
#ifdef _WIN32
    FreeLibrary(lib_ptr);
#else
    dlclose(lib_ptr);
#endif
}



bool GeneratorHandler::isLoadedSuccessfully()
{
    return mLoadedSuccessfully;
}


