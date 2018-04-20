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

#if defined(HOPSANCLI_USEGENERATOR)
#include "hopsangenerator.h"
#endif

namespace {
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
