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

#include "fmu_hopsan.h"
#include "HopsanCore.h"
#include "HopsanEssentials.h"
#include <string>
#include "model.hpp"
#include <cassert>

namespace {

static double fmu_time=0;
static hopsan::ComponentSystem *spCoreComponentSystem = 0;
static std::vector<std::string> sComponentNames;
hopsan::HopsanEssentials gHopsanCore;

double *dataPtrs[<<<nports>>>];

std::string parseResourceLocation(std::string uri)
{
    // The resource location is an URI according to rfc3986 on the following format
    // schema://authority/path or schema:/path
    // authority is expected to be empty if included
    // only the 'file' schema is supported by Hopsan
    std::string::size_type se = uri.find_first_of(':');
    std::string schema = uri.substr(0,se);
    // If the next two chars are // then authority is included (may be empty)
    std::string::size_type pb;
    if (uri.substr(se+1,2) == "//") {
        pb = uri.find_first_of('/', se+3);
    } else {
        pb = uri.find_first_of('/', se);
    }
    // Now we know were the path begins (pb), but is it a unix or windows path
    // Check windows
    if (uri.substr(pb+2,2) == ":/") {
        // Skip first /
        pb++;
    }
    std::string path = uri.substr(pb);
#ifdef _WIN32
    std::string::size_type i = path.find_first_of('/');
    while (i != std::string::npos) {
        path.replace(i, 1, 1, '\\');
        i = path.find_first_of('/');
    }
#endif
    return path;
}
}

extern "C" {

int hopsan_instantiate(const char *resourceLocation)
{
    double startT, stopT;      // Dummy variables
    spCoreComponentSystem = gHopsanCore.loadHMFModel(getModelString().c_str(), startT, stopT);
    if (spCoreComponentSystem)
    {
        std::string rl = parseResourceLocation(resourceLocation);
        spCoreComponentSystem->addSearchPath(rl.c_str());

        // Get pointers to I/O data variables
        <<<setdataptrs>>>

        // Initialize system
        spCoreComponentSystem->setDesiredTimestep(<<<timestep>>>);
        spCoreComponentSystem->setNumLogSamples(0);
        //! @todo disableLog does not work without setNumLogsamples0
        spCoreComponentSystem->disableLog();
        if (spCoreComponentSystem->checkModelBeforeSimulation())
        {
            return 1; // C true
        }
    }
    return 0;  // C false
}

int hopsan_initialize(double startT, double stopT)
{
    return spCoreComponentSystem->initialize(startT, stopT) ? 1 : 0;
}


void hopsan_simulate(double stopTime)
{
    spCoreComponentSystem->simulate(stopTime);
}

void hopsan_finalize()
{
    spCoreComponentSystem->finalize();
}

int hopsan_has_message()
{
    return (gHopsanCore.checkMessage() > 0) ? 1 : 0;
}

void hopsan_get_message(hopsan_message_callback_t message_callback, void* userState)
{
    hopsan::HString message, type, tag;
    gHopsanCore.getMessage(message, type, tag);

    // Replace any # with ## (# is reserved by FMI for value references)
    // # is used as escape character in this case
    message.replace("#", "##");

    // Replace any single % since we do not use printf format strings inside Hopsan
    // The FMI standard assuems that message is a printf format string
    // Use %% to print %
    message.replace("%", "%%");

    message_callback(message.c_str(), type.c_str(), userState);
}

double hopsan_get_real(int vr)
{
    return (*dataPtrs[vr]);
}

int hopsan_get_integer(int vr)
{
    //Write code here
}

int hopsan_get_boolean(int vr)
{
    //Write code here
}

const char* hopsan_get_string(int vr)
{
    //Write code here
}

void hopsan_set_real(int vr, double value)
{
    (*dataPtrs[vr]) = value;
}

void hopsan_set_integer(int vr, int value)
{
    //Write code here
}

void hopsan_set_boolean(int vr, int value)
{
    //Write code here
}

void hopsan_set_string(int vr, const char* value)
{
    //Write code here
}

}
