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

#ifndef <<<headerguard>>>
#define <<<headerguard>>>

#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stddef.h>
#include <sstream>
#include <iostream>

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

#include "FMI/fmi_import_context.h"
#include "FMI1/fmi1_import.h"
#include "JM/jm_portability.h"

namespace {
void jmLogger(jm_callbacks *c, jm_string module, jm_log_level_enu_t log_level, jm_string message)
{
    hopsan::Component* pComponent = (hopsan::Component*)(c->context);
    if (pComponent) {
        switch (log_level) {
        case jm_log_level_fatal:
            pComponent->addFatalMessage(message);
            break;
        case jm_log_level_error:
            pComponent->addErrorMessage(message);
            break;
        case jm_log_level_warning:
            pComponent->addWarningMessage(message);
            break;
        // Typically the jm logger info messages are not something we want to see in Hopsan, so show them as debug type
        case jm_log_level_verbose:
        case jm_log_level_info:
        case jm_log_level_debug:
            pComponent->addDebugMessage(message);
            break;
        default:
            break;
        }
    }
}

} // end anonymous-namespace

namespace hopsan {

class <<<className>>> : public <<<classParent>>>
{

private:
    //Node data pointers
    <<<localvars>>>

    jm_callbacks jmCallbacks;
    jm_status_enu_t status;
    fmi_import_context_t* context;
    fmi_version_enu_t version;
    fmi1_callback_functions_t fmiCallbackFunctions;
    fmi1_status_t fmistatus;
    fmi1_import_t* fmu;
    int k;

public:
    static Component *Creator()
    {
        return new <<<className>>>();
    }

    void configure()
    {
        //Init fmu pointers
        context = 0;
        fmu = 0;

        //Add constants
        <<<addconstants>>>

        //Add input variables
        <<<addinputs>>>

        //Add output variables
        <<<addoutputs>>>
    }

    void initialize()
    {
        addInfoMessage("Initializing FMU 1.0 import");

        const char* FMUPath = "<<<fmupath>>>";
        const char* tmpPath = "<<<temppath>>>";

        jmCallbacks.malloc = malloc;
        jmCallbacks.calloc = calloc;
        jmCallbacks.realloc = realloc;
        jmCallbacks.free = free;
        jmCallbacks.logger = jmLogger;
        jmCallbacks.log_level = jm_log_level_debug;
        jmCallbacks.context = (jm_voidp)this;   // This pointer is used by the jmLogger callback

        context = fmi_import_allocate_context(&jmCallbacks);
        version = fmi_import_get_fmi_version(context, FMUPath, tmpPath);

        if(version != fmi_version_1_enu)
        {
            addErrorMessage("The code only supports version 1.0\n");
            stopSimulation();
            return;
        }

        fmu = fmi1_import_parse_xml(context, tmpPath);

        if(!fmu)
        {
            addErrorMessage("Error parsing XML, exiting\n");
            stopSimulation();
            return;
        }

        if(fmi1_import_get_fmu_kind(fmu) == fmi1_fmu_kind_enu_me)
        {
            addErrorMessage("Only CS 1.0 is supported by this code\n");
            stopSimulation();
             return;
        }

        // Since FMI1 does not support sending a void* to this environment, we use the default log forwarder
        fmiCallbackFunctions.logger = fmi1_log_forwarding;
        fmiCallbackFunctions.allocateMemory = calloc;
        fmiCallbackFunctions.freeMemory = free;

        status = fmi1_import_create_dllfmu(fmu, fmiCallbackFunctions, 1);
        if (status == jm_status_error)
        {
            std::stringstream ss;
            ss << "Could not create the DLL loading mechanism(C-API) (error: " << fmi1_import_get_last_error(fmu) << ").";
            addErrorMessage(ss.str().c_str());
            stopSimulation();
            return;
        }

        //Instantiate FMU
        HString instanceName = getName();
        fmi1_string_t mimeType = NULL;
        fmi1_string_t fmuLocation = "<<<fmulocation>>>";
        fmi1_real_t timeout = 0.0;
        fmi1_boolean_t visible = fmi1_false;
        fmi1_boolean_t interactive = fmi1_false;
        status = fmi1_import_instantiate_slave(fmu, instanceName.c_str(), fmuLocation, mimeType, timeout, visible, interactive);
        if (status == jm_status_error)
        {
            addErrorMessage("fmi1_import_instantiate_slave() failed!");
            stopSimulation();
            return;
        }

        //Enter initialization mode
        fmistatus = fmi1_import_initialize_slave(fmu, mTime, fmi1_false, 10);
        if(fmistatus != fmi1_status_ok)
        {
            addErrorMessage("fmi1_import_initialize_slave() failed!");
            stopSimulation();
            return; 
        }

          //Set parameters
          fmi1_value_reference_t vr;
>>>setpars>>>        vr = <<<vr>>>;
        double value = <<<var>>>;
        fmistatus = fmi1_import_set_real(fmu, &vr, 1, &value);
        <<<setpars<<<
    }


    void simulateOneTimestep()
    {
        //Read inputs
        fmi1_value_reference_t vr;
        double value;
>>>readvars>>>        vr = <<<vr>>>;
        value = (*<<<var>>>);
        fmistatus = fmi1_import_set_real(fmu, &vr, 1, &value);
        <<<readvars<<<

        fmi1_import_do_step(fmu, mTime, mTimestep, true);

        double rValue;
>>>writevars>>>        vr = <<<vr>>>;
        fmistatus = fmi1_import_get_real(fmu, &vr, 1, &rValue);
        (*<<<var>>>) = rValue;
        <<<writevars<<<
    }


    void finalize()
    {
        if (fmu)
        {
            fmi1_import_destroy_dllfmu(fmu);
            fmi1_import_free(fmu);
            fmu = 0;
        }

        if (context)
        {
            fmi_import_free_context(context);
            context = 0;
        }
    }
};
}

#endif
