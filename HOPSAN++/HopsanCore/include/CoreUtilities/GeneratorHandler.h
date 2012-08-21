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

#ifndef GENERATORHANDLER_H
#define GENERATORHANDLER_H

#include "win32dll.h"

namespace hopsan {

class DLLIMPORTEXPORT GeneratorHandler
{
public:
    GeneratorHandler();
    ~GeneratorHandler();

    bool isLoadedSuccessfully();

    typedef void (*call_modelica_generator_t)(std::string code, std::string codeIncludeDir, std::string binDir, bool showDialog);
    typedef void (*call_cpp_generator_t)(std::string cppCode, std::string coreIncludePath, std::string binPath, bool showDialog);
    typedef void (*call_fmu_generator_t)(std::string path, std::string coreIncludePath, std::string binPath, bool showDialog);

    call_modelica_generator_t callModelicaGenerator;
    call_cpp_generator_t callCppGenerator;
    call_fmu_generator_t callFmuGenerator;

private:
    bool mLoadedSuccessfully;

#ifdef WIN32
    HINSTANCE lib_ptr;
#else
    void* lib_ptr;
#endif
};

}

#endif // GENERATORHANDLER_H
