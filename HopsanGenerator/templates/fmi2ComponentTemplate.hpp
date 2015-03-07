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

class <<<className>>> : public ComponentQ
{

private:
    //Node data pointers
    <<<localvars>>>

    fmi2_callback_functions_t callBackFunctions;
    jm_callbacks callbacks;
    fmi_import_context_t* context;
    fmi_version_enu_t version;
    jm_status_enu_t status;
    fmi2_status_t fmistatus;
    int k;

    fmi2_import_t* fmu;

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

        //Add ports
        <<<addports>>>
    }

    void initialize()
    {
        <<<setnodedatapointers>>>

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

        if(version != fmi_version_2_0_enu)
        {
            addErrorMessage("The code only supports version 2.0\n");
            stopSimulation();
            return;
        }

        fmu = fmi2_import_parse_xml(context, tmpPath, 0);

        if(!fmu)
        {
            addErrorMessage("Error parsing XML, exiting\n");
            stopSimulation();
            return;
        }

        if(fmi2_import_get_fmu_kind(fmu) == fmi2_fmu_kind_me)
        {
            addErrorMessage("Only CS 2.0 is supported by this code\n");
            stopSimulation();
             return;
        }

        callBackFunctions.logger = fmi2_log_forwarding;
        callBackFunctions.allocateMemory = calloc;
        callBackFunctions.freeMemory = free;
        callBackFunctions.componentEnvironment = fmu;

        status = fmi2_import_create_dllfmu(fmu, fmi2_fmu_kind_cs, &callBackFunctions);
        if (status == jm_status_error)
        {
            std::stringstream ss;
            ss << "Could not create the DLL loading mechanism(C-API) (error: " << fmi2_import_get_last_error(fmu) << ").";
            addErrorMessage(ss.str().c_str());
            stopSimulation();
            return;
        }

        //Instantiate FMU
        fmi2_string_t instanceName = "Test CS model instance";
        fmi2_string_t fmuLocation = "";
        fmi2_boolean_t visible = fmi2_false;
        jm_status_enu_t jmstatus = fmi2_import_instantiate(fmu, instanceName, fmi2_cosimulation, fmuLocation, visible);
        if (jmstatus == jm_status_error)
        {
            addErrorMessage("fmi2_import_instantiate() failed!");
            stopSimulation();
            return;
        }

        //Setup experiment
        fmi2_real_t relativeTol = 1e-4;
        fmistatus = fmi2_import_setup_experiment(fmu, fmi2_true, relativeTol, mTime, fmi2_false, 10);
        if(fmistatus != fmi2_status_ok)
        {
            addErrorMessage("fmi2_import_setup_experiment() failed!");
            stopSimulation();
            return;
        }

        //Enter initialization mode
        fmistatus = fmi2_import_enter_initialization_mode(fmu);
        if(fmistatus != fmi2_status_ok)
        {
            addErrorMessage("fmi2_import_enter_initialization_mode() failed!");
            stopSimulation();
            return;
        }

        //Set parameters
        double value;
        fmi2_value_reference_t vr;
>>>setpars>>>        vr = <<<vr>>>;
        value = <<<var>>>;
        fmistatus = fmi2_import_set_real(fmu, &vr, 1, &value);
        <<<setpars<<<

        //Exit initialization mode
        fmistatus = fmi2_import_exit_initialization_mode(fmu);
        if(fmistatus != fmi2_status_ok)
        {
            addErrorMessage("fmi2_import_exit_initialization_mode() failed!");
            stopSimulation();
            return;
        }
    }


    void simulateOneTimestep()
    {
        //Read inputs
        fmi2_value_reference_t vr;
        double value;
>>>readvars>>>        vr = <<<vr>>>;
        value = (*<<<var>>>);
        fmistatus = fmi2_import_set_real(fmu, &vr, 1, &value);
        <<<readvars<<<

        fmi2_import_do_step(fmu, mTime, mTimestep, true);

        double rValue;
>>>writevars>>>        vr = <<<vr>>>;
        fmistatus = fmi2_import_get_real(fmu, &vr, 1, &rValue);
        (*<<<var>>>) = rValue;
        <<<writevars<<<
    }


    void finalize()
    {
        fmistatus = fmi2_import_terminate(fmu);

        fmi2_import_free_instance(fmu);

        fmi2_import_destroy_dllfmu(fmu);

        fmi2_import_free(fmu);
        fmi_import_free_context(context);

        addInfoMessage("Everything seems to be OK since you got this far=)!");
    }
};
}

#endif
