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

class <<<className>>> : public <<<classParent>>>
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

        //Init fmu pointers
        context = 0;
        fmu = 0;
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

        //Set parameters
        fmi2_value_reference_t vr;
  >>>setpars>>>        vr = <<<vr>>>;
          fmistatus = <<<setparfunction>>>(fmu, &vr, 1, <<<var>>>);
        <<<setpars<<<

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

        fmistatus = fmi2_import_do_step(fmu, mTime, mTimestep, true);
        if (fmistatus != fmi2_status_ok)
        {
            stopSimulation("fmi2_import_do_Step did NOT return status fmi2_Status_ok");
            return;
        }

        //Write outputs
>>>writevars>>>        vr = <<<vr>>>;
        fmistatus = fmi2_import_get_real(fmu, &vr, 1, &value);
        (*<<<var>>>) = value;
        <<<writevars<<<
    }


    void finalize()
    {
        if (fmu)
        {
            fmistatus = fmi2_import_terminate(fmu);

            fmi2_import_free_instance(fmu);

            fmi2_import_destroy_dllfmu(fmu);

            fmi2_import_free(fmu);
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
