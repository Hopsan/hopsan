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

#include "BuildUtilities.h"
#include "CliUtilities.h"
#include "ComponentSystem.h"

#include <cerrno>
#include <fstream>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#if defined(HOPSANCLI_USEGENERATOR)
#include "hopsangenerator.h"
#endif

namespace {
    bool makeDirectory(const std::string &path)
    {
        if (path.empty()) {
            return false;
        }

        std::string currentPath;
        for (size_t i = 0; i < path.size(); ++i)
        {
            currentPath += path[i];
            if (path[i] != '/' && path[i] != '\\' && i + 1 < path.size()) {
                continue;
            }

            if (currentPath.size() == 1 || (currentPath.size() == 3 && currentPath[1] == ':')) {
                continue;
            }

#ifdef _WIN32
            const int result = _mkdir(currentPath.c_str());
#else
            const int result = mkdir(currentPath.c_str(), 0777);
#endif
            if (result != 0 && errno != EEXIST) {
                return false;
            }
        }
        return true;
    }

    void messageHandler(const char* msg, const char type, void* pDummy)
    {
        if (type == 'E') {
            printErrorMessage(msg);
        } else if (type == 'W') {
            printWarningMessage(msg);
        } else {
            printMessage(msg);
        }
    }
}

bool buildComponentLibrary(const std::string &rLibraryXML, std::string &rOutput)
{
#if defined(HOPSANCLI_USEGENERATOR)
    const std::string hopsanRootPath = getCurrentExecPath()+"/..";
    constexpr auto cflags = "";
    constexpr auto lflags = "";
    constexpr auto compilerPath = "";

    callComponentLibraryCompiler(rLibraryXML.c_str(), cflags, lflags, hopsanRootPath.c_str(), compilerPath, &messageHandler, nullptr);
    return true;
#else
    printErrorMessage("This HopsanCLI is not built with HopsanGenerator support");
    return false;
#endif
}

bool exportFmu(const std::string &rOutputPath, const std::string &rModelPath, hopsan::ComponentSystem *pSystem, int version, const std::string &rCompilerPath)
{
#if defined(HOPSANCLI_USEGENERATOR)
    if (!makeDirectory(rOutputPath)) {
        printErrorMessage("Unable to create FMI export directory: "+rOutputPath);
        return false;
    }

    std::ifstream sourceModel(rModelPath.c_str(), std::ios::binary);
    if (!sourceModel.is_open()) {
        printErrorMessage("Unable to open model file for FMI export: "+rModelPath);
        return false;
    }

    const std::string modelFilePath = rOutputPath+"/"+pSystem->getName().c_str()+".hmf";
    std::ofstream stagedModel(modelFilePath.c_str(), std::ios::binary);
    if (!stagedModel.is_open()) {
        printErrorMessage("Unable to stage model file for FMI export: "+modelFilePath);
        return false;
    }
    stagedModel << sourceModel.rdbuf();
    if (!stagedModel.good()) {
        printErrorMessage("Failed to stage model file for FMI export: "+modelFilePath);
        return false;
    }

    const std::string hopsanRootPath = getCurrentExecPath()+"/..";

    stagedModel.close();
    sourceModel.close();

    return callFmuExportGenerator(rOutputPath.c_str(), pSystem, nullptr, 0,
                                  hopsanRootPath.c_str(), rCompilerPath.c_str(), version, 64,
                                  &messageHandler, nullptr);
#else
    (void)rOutputPath;
    (void)rModelPath;
    (void)pSystem;
    (void)version;
    (void)rCompilerPath;
    printErrorMessage("This HopsanCLI is not built with HopsanGenerator support");
    return false;
#endif
}
