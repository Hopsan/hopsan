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
#include "FMI2/fmi2_import.h"
#include "JM/jm_portability.h"


void localLogger(jm_callbacks *c, jm_string module, jm_log_level_enu_t log_level, jm_string message) { }

namespace hopsan {

//!
//! @brief
//! @ingroup HydraulicComponents
//!
class <<<className>>> : public ComponentQ
{

private:
    //Node data pointers
    <<<localvars>>>

    fmi1_callback_functions_t callBackFunctions;
    jm_callbacks callbacks;
    fmi_import_context_t* context;
    fmi_version_enu_t version;
    fmi1_status_t fmistatus;
    jm_status_enu_t status;
    int k;

    fmi1_import_t* fmu;

public:
    static Component *Creator()
    {
        return new <<<className>>>();
    }

    void configure()
    {
        //Add constants
        <<<addconstants>>>

        //Add input variables
        <<<addinputs>>>

        //Add output variables
        <<<addoutputs>>>
    }

    void initialize()
    {
        addInfoMessage("Initializing FMU 2.0 import");

        const char* FMUPath = "<<<fmupath>>>";
        const char* tmpPath = "<<<temppath>>>";

        callbacks.malloc = malloc;
        callbacks.calloc = calloc;
        callbacks.realloc = realloc;
        callbacks.free = free;
        callbacks.logger = localLogger;
        callbacks.log_level = jm_log_level_debug;
        callbacks.context = 0;

        context = fmi_import_allocate_context(&callbacks);

        version = fmi_import_get_fmi_version(context, FMUPath, tmpPath);

        if(version != fmi_version_1_enu)
        {
            addErrorMessage("The code only supports version 2.0\n");
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
            addErrorMessage("Only CS 2.0 is supported by this code\n");
            stopSimulation();
             return;
        }

        callBackFunctions.logger = fmi1_log_forwarding;
        callBackFunctions.allocateMemory = calloc;
        callBackFunctions.freeMemory = free;

        status = fmi1_import_create_dllfmu(fmu, callBackFunctions, 1);
        if (status == jm_status_error)
        {
            std::stringstream ss;
            ss << "Could not create the DLL loading mechanism(C-API) (error: " << fmi1_import_get_last_error(fmu) << ").";
            addErrorMessage(ss.str().c_str());
            stopSimulation();
            return;
        }

        //Instantiate FMU
        fmi1_string_t mimeType = "";
        fmi1_string_t instanceName = "Test CS model instance";
        fmi1_string_t fmuLocation = "";
        fmi1_real_t timeout = 0.0;
        fmi1_boolean_t visible = fmi1_false;
        fmi1_boolean_t interactive = fmi1_false;
        status = fmi1_import_instantiate_slave(fmu, instanceName, fmuLocation, mimeType, timeout, visible, interactive);
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
        fmi1_import_destroy_dllfmu(fmu);
        fmi1_import_free(fmu);
        fmi_import_free_context(context);
    }
};
}

#endif
