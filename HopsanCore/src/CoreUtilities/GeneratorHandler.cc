/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//$Id$

#include <stdio.h>
#ifndef _WIN32
#include "dlfcn.h"
#endif
#include "win32dll.h"

#include "CoreUtilities/GeneratorHandler.h"
#include "HopsanCoreMacros.h"

using namespace hopsan;

GeneratorHandler::GeneratorHandler()
{
    mLoadedSuccessfully = false;
    mErrorString.clear();

    const HString generatorLibName = TO_STR(DLL_PREFIX) "HopsanGenerator" TO_STR(DEBUG_EXT) TO_STR(DLL_EXT);

#ifdef _WIN32
    mpLib = LoadLibrary(generatorLibName.c_str()); //Load the dll

    if (!mpLib)
    {
        //! @todo Getting Windows Error messages is complicated. Hand-coded for now.
        mErrorString = HString("LoadLibrary failed for: ")+generatorLibName;
        return;
    }

    //Load modelica generator function
    HString name = "callModelicaGenerator";
    callModelicaGenerator = (call_modelica_generator_t)GetProcAddress(mpLib, name.c_str() );
    if (!callModelicaGenerator)
    {
        mErrorString = HString("GetProcAddress failed for: ")+name;
        return;
    }

    //Load C++ generator function
    name = "callCppGenerator";
    callCppGenerator = (call_cpp_generator_t)GetProcAddress(mpLib, name.c_str() );
    if (!callCppGenerator)
    {
        mErrorString = HString("GetProcAddress failed for: ")+name;
        return;
    }

    //Load FMU generator function
    name = "callFmuImportGenerator";
    callFmuImportGenerator = (call_fmu_import_generator_t)GetProcAddress(mpLib, name.c_str() );
    if (!callFmuImportGenerator)
    {
        mErrorString = HString("GetProcAddress failed for: ")+name;
        return;
    }

    //Load FMU generator function
    name = "callFmuExportGenerator";
    callFmuExportGenerator = (call_fmu_export_generator_t)GetProcAddress(mpLib, name.c_str() );
    if (!callFmuExportGenerator)
    {
        mErrorString = HString("GetProcAddress failed for: ")+name;
        return;
    }

    //Load Simulink generator function
    name = "callSimulinkExportGenerator";
    callSimulinkExportGenerator = (call_simulink_export_generator_t)GetProcAddress(mpLib, name.c_str() );
    if (!callSimulinkExportGenerator)
    {
        mErrorString = HString("GetProcAddress failed for: ")+name;
        return;
    }

    //Load Simulink Co-Simulation generator function
    name = "callSimulinkCoSimExportGenerator";
    callSimulinkCoSimExportGenerator = (call_simulink_cosim_export_generator_t)GetProcAddress(mpLib, name.c_str() );
    if (!callSimulinkCoSimExportGenerator)
    {
        mErrorString = HString("GetProcAddress failed for: ")+name;
        return;
    }

    //Load LabVIEW/SIT generator function
    name = "callLabViewSITGenerator";
    callLabViewSITGenerator = (call_lvsit_export_generator_t)GetProcAddress(mpLib, name.c_str() );
    if (!callLabViewSITGenerator)
    {
        mErrorString = HString("GetProcAddress failed for: ")+name;
        return;
    }

    //Load compile component library function
    name = "callComponentLibraryCompiler";
    callComponentLibraryCompiler = (call_complib_compiler_t)GetProcAddress(mpLib, name.c_str() );
    if(!callComponentLibraryCompiler)
    {
        mErrorString = HString("GetProcAddress failed for: ")+name;
        return;
    }

    //Load library generator function
    name = "callLibraryGenerator";
    callLibraryGenerator = (call_library_generator_t)GetProcAddress(mpLib, name.c_str() );
    if(!callLibraryGenerator)
    {
        mErrorString = HString("GetProcAddress failed for: ")+name;
        return;
    }

#else
    // Load the dynamic library
    mpLib = dlopen(generatorLibName.c_str(), RTLD_NOW);
    if(!mpLib)
    {
        //fprintf (stderr, "Failed in dlopen: %s\n", dlerror());
        mErrorString = dlerror();
        return;
    }

    //Load Modelica generator function
    callModelicaGenerator = (call_modelica_generator_t)dlsym(mpLib, "callModelicaGenerator");
    char *dlsym_error = dlerror();
    if(dlsym_error)
    {
        mErrorString = dlsym_error;
        return;
    }

    //Load C++ generator function
    callCppGenerator = (call_cpp_generator_t)dlsym(mpLib, "callCppGenerator");
    dlsym_error = dlerror();
    if(dlsym_error)
    {
        mErrorString = dlsym_error;
        return;
    }

    //Load FMU generator function
    callFmuImportGenerator = (call_fmu_import_generator_t)dlsym(mpLib, "callFmuImportGenerator");
    dlsym_error = dlerror();
    if(dlsym_error)
    {
        mErrorString = dlsym_error;
        return;
    }

    //Load FMU generator function
    callFmuExportGenerator = (call_fmu_export_generator_t)dlsym(mpLib, "callFmuExportGenerator");
    dlsym_error = dlerror();
    if(dlsym_error)
    {
        mErrorString = dlsym_error;
        return;
    }

    //Load Simulink Co-Simulation generator function
    callSimulinkExportGenerator = (call_simulink_export_generator_t)dlsym(mpLib, "callSimulinkExportGenerator");
    dlsym_error = dlerror();
    if(dlsym_error)
    {
        mErrorString = dlsym_error;
        return;
    }

    //! @todo Shall not be here, since Simulink export does not currently work under Linux
    //Load Simulink Co-Simulation generator function
    callSimulinkCoSimExportGenerator = (call_simulink_cosim_export_generator_t)dlsym(mpLib, "callSimulinkCoSimExportGenerator");
    dlsym_error = dlerror();
    if(dlsym_error)
    {
        mErrorString = dlsym_error;
        return;
    }

    //Load LabVIEW/SIT generator function
    callLabViewSITGenerator = (call_lvsit_export_generator_t)dlsym(mpLib, "callLabViewSITGenerator");
    dlsym_error = dlerror();
    if (dlsym_error)
    {
        mErrorString = dlsym_error;
        return;
    }

    //Load compile component library function
    callComponentLibraryCompiler = (call_complib_compiler_t)dlsym(mpLib, "callComponentLibraryCompiler");
    dlsym_error = dlerror();
    if(dlsym_error)
    {
        mErrorString = dlsym_error;
        return;
    }

    //Load library generator function
    callLibraryGenerator = (call_library_generator_t)dlsym(mpLib, "callLibraryGenerator");
    if(dlsym_error)
    {
        mErrorString = dlsym_error;
        return;
    }
#endif

    mLoadedSuccessfully = true;
}



GeneratorHandler::~GeneratorHandler()
{
    if (mpLib)
    {
#ifdef _WIN32
        FreeLibrary(mpLib);
#else
        dlclose(mpLib);
#endif
    }
}



bool GeneratorHandler::isLoadedSuccessfully() const
{
    return mLoadedSuccessfully;
}

bool GeneratorHandler::isLoadedSuccessfully(HString &rError) const
{
    rError = error();
    return isLoadedSuccessfully();
}

HString GeneratorHandler::error() const
{
    return mErrorString;
}
