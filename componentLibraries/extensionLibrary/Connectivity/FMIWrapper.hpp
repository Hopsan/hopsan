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

#ifndef FMIWRAPPER_HPP
#define FMIWRAPPER_HPP

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

#include "FMI/fmi_import_context.h"
#include <FMI1/fmi1_import.h>
#include <FMI2/fmi2_import.h>
#include <JM/jm_portability.h>

//!
//! @file FMIWrapper.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date 2021-01-27
//! @brief Wrapper component for functional mockup units (.fmu)
//!

using namespace hopsan;

void jmLogger(jm_callbacks *c, jm_string module, jm_log_level_enu_t log_level, jm_string message)
{
    (void)module;
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

void fmiLogger(fmi2ComponentEnvironment pComponentEnvironment, fmi2_string_t instanceName, fmi2_status_t status, fmi2_string_t category, fmi2_string_t message, ...)
{
    hopsan::Component* pComponent = (hopsan::Component*)pComponentEnvironment;
    if (pComponent == nullptr) {
        return;
    }

    char buffer[512];
    va_list args;
    va_start(args, message);
    vsnprintf(buffer, 512, message, args);
    va_end(args);

    switch (status) {
    // Typically info messages are not something we want to see in Hopsan, so show them as debug type
    case fmi2_status_ok:
        pComponent->addDebugMessage(buffer);
        break;
    case fmi2_status_warning:
        pComponent->addWarningMessage(buffer);
        break;
    case fmi2_status_error:
        pComponent->addErrorMessage(buffer);
        break;
    case fmi2_status_fatal:
        pComponent->addFatalMessage(buffer);
        break;
    default:
        // Discard
        break;
    }
}



class FMIWrapper : public ComponentSignal
{
private:
    TempDirectoryHandle *mpTempDir;
    HString mFmuPath, mLastFmuPath;
    std::map<fmi2_value_reference_t,double*> mOutputs;
    std::map<fmi2_value_reference_t,double*> mInputs;
    std::map<fmi2_value_reference_t,double> mParameters;
    std::vector<Port*> mPorts;

    jm_callbacks callbacks;
    jm_status_enu_t status;
    fmi_import_context_t* context;
    fmi_version_enu_t version;
    fmi2_callback_functions_t fmiCallbackFunctions;
    fmi2_status_t fmistatus;
    fmi2_import_t* fmu;


public:
    static Component *Creator()
    {
        return new FMIWrapper();
    }

    void configure()
    {
        addConstant("path", "Path to functional mockup unit (FMU)", "", mFmuPath);
    }

    void reconfigure()
    {
        if(mFmuPath == mLastFmuPath) {
            return; //Path did not change, do nothing
        }
        mLastFmuPath = mFmuPath;

        deconfigure(); //Make sure to unload FMU and free memory before loading a new one

        for(const auto *port : mPorts) {
            removePort(port->getName());
        }
        std::vector<HString> parameters;
        this->getParameterNames(parameters);
        for(const auto &parameter : parameters) {
            if(parameter != "path") {
                this->unRegisterParameter(parameter);
            }
        }
        mPorts.clear();
        mOutputs.clear();
        mInputs.clear();
        mParameters.clear();

        callbacks.malloc = malloc;
        callbacks.calloc = calloc;
        callbacks.realloc = realloc;
        callbacks.free = free;
        callbacks.logger = jmLogger;
        callbacks.log_level = jm_log_level_debug;
        callbacks.context = static_cast<jm_voidp>(this);

        context = fmi_import_allocate_context(&callbacks);

        addInfoMessage("Loading FMU from "+mFmuPath+"...");

        mpTempDir = new TempDirectoryHandle("fmu");
        if(!mpTempDir->isValid()) {
            addErrorMessage("Unable to create temp directory: "+mpTempDir->path());
            return;
        }

        addDebugMessage("Using temporary directory: "+mpTempDir->path());

        version = fmi_import_get_fmi_version(context, mFmuPath.c_str(), mpTempDir->path().c_str());
        if(version != fmi_version_2_0_enu) {
            //! @todo Implement FMI 1.0 support
            addErrorMessage("The code only supports version 2.0");
            return;
        }

        addDebugMessage("FMU version: 2.0");

        fmu = fmi2_import_parse_xml(context, mpTempDir->path().c_str(), nullptr);
        if(!fmu) {
            addErrorMessage("Parsing model description failed");
            return;
        }

        addDebugMessage("Successfully parsed model description");

        if(fmi2_import_get_fmu_kind(fmu) == fmi2_fmu_kind_me) {
            addErrorMessage("Only FMI for co-simulation is supported");
            return;
        }

        addDebugMessage("FMU supports FMI for co-simulation");

        fmi2_fmu_kind_enu_t fmuKind = fmi2_import_get_fmu_kind(fmu);
        if (fmuKind == fmi2_fmu_kind_me_and_cs) {
            fmuKind = fmi2_fmu_kind_cs;
        }

        //Loop through variables in FMU and generate the lists
        fmi2_import_variable_list_t *pVarList = fmi2_import_get_variable_list(fmu,0);
        for(size_t i=0; i<fmi2_import_get_variable_list_size(pVarList); ++i)
        {
            fmi2_import_variable_t *pVar = fmi2_import_get_variable(pVarList, i);

            const char* name = fmi2_import_get_variable_name(pVar);
            const char* description = fmi2_import_get_variable_description(pVar);
            if(description == nullptr) {
                description = "";
            }
            fmi2_base_type_enu_t type = fmi2_import_get_variable_base_type(pVar);
            fmi2_causality_enu_t causality = fmi2_import_get_causality(pVar);
            fmi2_value_reference_t vr = fmi2_import_get_variable_vr(pVar);

            if(causality == fmi2_causality_enu_parameter)
            {
                addDebugMessage("Parameter: "+HString(name));
                addConstant(name, description, "", mParameters[vr]);
            }
            else if(causality == fmi2_causality_enu_input && type == fmi2_base_type_real)
            {
                addDebugMessage("Input: "+HString(name));
                mPorts.push_back(addInputVariable(name, description, "", 0, &mInputs[vr]));
            }
            else if(causality == fmi2_causality_enu_output && type == fmi2_base_type_real)
            {
                addDebugMessage("Output: "+HString(name));
                mPorts.push_back(addOutputVariable(name, description, "", &mOutputs[vr]));
            }
        }

        fmiCallbackFunctions.logger = fmiLogger;
        fmiCallbackFunctions.allocateMemory = calloc;
        fmiCallbackFunctions.freeMemory = free;
        fmiCallbackFunctions.componentEnvironment = (fmi2ComponentEnvironment*)this;

        //Load FMU binary
        status = fmi2_import_create_dllfmu(fmu, fmi2_fmu_kind_cs, &fmiCallbackFunctions);
        if (status == jm_status_error) {
            stopSimulation("Failed to load .dll/.so file (error: "+HString(fmi2_import_get_last_error(fmu))+")");
            return;
        }

        addDebugMessage("Successfully loaded .dll/.so file");

        //Instantiate FMU
        HString instanceName = getName();
        fmi2_string_t fmuResourceLocation = "file:///home/robbr48/Documents/Hopsan/import/FMU/hopsan_tutorial/resources";
        fmi2_boolean_t visible = fmi2_false;
        jm_status_enu_t jmstatus = fmi2_import_instantiate(fmu, instanceName.c_str(), fmi2_cosimulation, fmuResourceLocation, visible);
        if (jmstatus == jm_status_error) {
            stopSimulation("Failed to instantiate FMU");
            return;
        }

        addInfoMessage("Successfully instantiated FMU");

    }

    void initialize()
    {
        addInfoMessage("Initializing FMU 2.0 import");

        for(const auto &parameter : mParameters) {
            fmistatus = fmi2_import_set_real(fmu, &parameter.first, 1, &parameter.second);
        }

        //Setup experiment
        fmistatus = fmi2_import_setup_experiment(fmu, fmi2_false, 0, mTime, fmi2_false, 10);
        if(fmistatus != fmi2_status_ok) {
            stopSimulation("fmi2_import_setup_experiment() failed");
            return;
        }

        //Enter initialization mode
        fmistatus = fmi2_import_enter_initialization_mode(fmu);
        if(fmistatus != fmi2_status_ok) {
            stopSimulation("fmi2_import_enter_initialization_mode() failed");
            return;
        }

        //Exit initialization mode
        fmistatus = fmi2_import_exit_initialization_mode(fmu);
        if(fmistatus != fmi2_status_ok) {
            stopSimulation("fmi2_import_exit_initialization_mode() failed");
            return;
        }
    }

    void simulateOneTimestep()
    {
        //Read inputs
        for(const auto &input : mInputs) {
            fmistatus = fmi2_import_set_real(fmu, &input.first, 1, input.second);
        }

        //Take step
        fmistatus = fmi2_import_do_step(fmu, mTime, mTimestep, true);
        if (fmistatus != fmi2_status_ok) {
            stopSimulation("fmi2_import_do_step failed");
            return;
        }

        //Write outputs
        for(const auto &output : mOutputs) {
            fmistatus = fmi2_import_get_real(fmu, &output.first, 1, output.second);
        }
    }

    void finalize()
    {
        if(fmu) {
            fmistatus = fmi2_import_terminate(fmu);
        }
    }

    void deconfigure()
    {
        if(fmu) {
            fmi2_import_free_instance(fmu);
            fmi2_import_destroy_dllfmu(fmu);
            fmi2_import_free(fmu);
            fmu = nullptr;
        }

        if(context) {
            fmi_import_free_context(context);
            context = nullptr;
        }

        delete mpTempDir;
    }
};

#endif // FMIWRAPPER_HPP
