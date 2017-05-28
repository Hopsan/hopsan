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

#ifndef GENERATORHANDLER_H
#define GENERATORHANDLER_H

#ifdef _WIN32
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0502
#include "Windows.h"
#endif
#include <vector>
#include "win32dll.h"
#include "HopsanTypes.h"

namespace hopsan {

class ComponentSystem;

class DLLIMPORTEXPORT GeneratorHandler
{
public:
    GeneratorHandler();
    ~GeneratorHandler();

    bool isLoadedSuccessfully();

    typedef void (*call_modelica_generator_t)(HString path, hopsan::HString gccPath, bool showDialog, int solver, bool compile, hopsan::HString coreIncludePath, hopsan::HString binPath);
    typedef void (*call_cpp_generator_t)(HString hppPath, hopsan::HString gccPath, bool compile, hopsan::HString coreIncludePath, hopsan::HString binPath);
    typedef void (*call_fmu_import_generator_t)(HString path, HString targetPath, HString coreIncludePath, HString binPath, HString gccPath, bool showDialog);
    typedef void (*call_fmu_export_generator_t)(HString path, hopsan::ComponentSystem *pSystem, HString coreIncludePath, HString binPath, HString gccPath, int version, bool x64, bool showDialog);
    typedef void (*call_simulink_export_generator_t)(HString path, HString modelName, hopsan::ComponentSystem *pSystem, bool disablePortLabels, HString coreIncludePath, HString binPath, bool showDialog);
    typedef void (*call_simulink_cosim_export_generator_t)(HString path, hopsan::ComponentSystem *pSystem, bool disablePortLabels, int compiler, HString coreIncludePath, HString binPath, bool showDialog);
    typedef void (*call_lvsit_export_generator_t)(HString path, hopsan::ComponentSystem *pSystem, HString coreIncludePath, HString binPath, bool showDialog);
    typedef void (*call_complib_compiler_t)(HString path, HString extraCFlags, HString extraLFlags, HString coreIncludePath, HString binPath, HString gccPath, bool showDialog);
    typedef void (*call_library_generator_t)(HString path, std::vector<HString> hppFiles, bool showDialog);

    call_modelica_generator_t callModelicaGenerator;
    call_cpp_generator_t callCppGenerator;
    call_fmu_import_generator_t callFmuImportGenerator;
    call_fmu_export_generator_t callFmuExportGenerator;
    call_simulink_export_generator_t callSimulinkExportGenerator;
    call_simulink_cosim_export_generator_t callSimulinkCoSimExportGenerator;
    call_lvsit_export_generator_t callLabViewSITGenerator;
    call_complib_compiler_t callComponentLibraryCompiler;
    call_library_generator_t callLibraryGenerator;

private:
    bool mLoadedSuccessfully;

#ifdef _WIN32
    HINSTANCE mpLib;
#else
    void* mpLib;
#endif
};

}

#endif // GENERATORHANDLER_H
