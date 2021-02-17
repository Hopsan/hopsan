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
    if (pComponent == NULL) {
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
    std::map<fmi2_value_reference_t,double> mRealParameters;
    std::map<fmi2_value_reference_t,bool> mBoolParameters;
    std::map<fmi2_value_reference_t,int> mIntParameters;
    std::map<fmi2_value_reference_t,HString> mStringParameters;
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
        setReconfigurationParameter("path");
    }

    void reconfigure()
    {
        if(mFmuPath == mLastFmuPath) {
            return; //Path did not change, do nothing
        }
        mLastFmuPath = mFmuPath;

        deconfigure(); //Make sure to unload FMU and free memory before loading a new one

        for(int i=0; i<mPorts.size(); ++i) {
            removePort(mPorts[i]->getName());
        }
        std::vector<HString> parameters;
        this->getParameterNames(parameters);
        for(int i=0; i<parameters.size(); ++i) {
            if(parameters[i] != "path") {
                this->unRegisterParameter(parameters[i]);
            }
        }
        mPorts.clear();
        mOutputs.clear();
        mInputs.clear();
        mRealParameters.clear();
        mStringParameters.clear();
        mBoolParameters.clear();
        mIntParameters.clear();

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

        fmu = fmi2_import_parse_xml(context, mpTempDir->path().c_str(), NULL);
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
            if(description == NULL) {
                description = "";
            }
            fmi2_base_type_enu_t type = fmi2_import_get_variable_base_type(pVar);
            fmi2_causality_enu_t causality = fmi2_import_get_causality(pVar);
            fmi2_value_reference_t vr = fmi2_import_get_variable_vr(pVar);

            if(causality == fmi2_causality_enu_parameter && type == fmi2_base_type_str)
            {
                addDebugMessage("String parameter: "+HString(name));
                const char* startValue = fmi2_import_get_string_variable_start(fmi2_import_get_variable_as_string(pVar));
                addConstant(name, description, "", startValue, mStringParameters[vr]);
            }
            else if(causality == fmi2_causality_enu_parameter && type == fmi2_base_type_bool)
            {
                addDebugMessage("Boolean parameter: "+HString(name));
                bool startValue = fmi2_import_get_boolean_variable_start(fmi2_import_get_variable_as_boolean(pVar));
                addConstant(name, description, "", startValue, mBoolParameters[vr]);
            }
            else if(causality == fmi2_causality_enu_parameter && type == fmi2_base_type_int)
            {
                addDebugMessage("Integer parameter: "+HString(name));
                int startValue = fmi2_import_get_integer_variable_start(fmi2_import_get_variable_as_integer(pVar));
                addConstant(name, description, "", startValue, mIntParameters[vr]);
            }
            else if(causality == fmi2_causality_enu_parameter)
            {
                addDebugMessage("Real parameter: "+HString(name));
                double startValue = fmi2_import_get_real_variable_start(fmi2_import_get_variable_as_real(pVar));
                addConstant(name, description, "", startValue, mRealParameters[vr]);
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
            addErrorMessage("Failed to load .dll/.so file (error: "+HString(fmi2_import_get_last_error(fmu))+")");
            return;
        }

        addDebugMessage("Successfully loaded .dll/.so file");

        //Instantiate FMU
        HString instanceName = getName();
        HString resourceDir = "file://"+mpTempDir->path()+"/resources";
        fmi2_boolean_t visible = fmi2_false;
        jm_status_enu_t jmstatus = fmi2_import_instantiate(fmu, instanceName.c_str(), fmi2_cosimulation, resourceDir.c_str(), visible);
        if (jmstatus == jm_status_error) {
            stopSimulation("Failed to instantiate FMU");
            return;
        }

        addInfoMessage("Successfully instantiated FMU");

    }

    void initialize()
    {
        addInfoMessage("Initializing FMU 2.0 import");

        std::map<fmi2_value_reference_t,double>::iterator itr;
        for(itr = mRealParameters.begin(); itr != mRealParameters.end(); itr++) {
            fmistatus = fmi2_import_set_real(fmu, &itr->first, 1, &itr->second);
        }
        std::map<fmi2_value_reference_t,HString>::iterator its;
        for(its = mStringParameters.begin(); its != mStringParameters.end(); ++its) {
            const char* value = its->second.c_str();
            fmistatus = fmi2_import_set_string(fmu, &its->first, 1, &value);
        }
        std::map<fmi2_value_reference_t,bool>::iterator itb;
        for(itb = mBoolParameters.begin(); itb != mBoolParameters.end(); ++itb) {
            int value = int(itb->second);
            fmistatus = fmi2_import_set_boolean(fmu, &its->first, 1, &value);
        }
        std::map<fmi2_value_reference_t,int>::iterator iti;
        for(iti = mIntParameters.begin(); iti != mIntParameters.end(); ++iti) {
            fmistatus = fmi2_import_set_integer(fmu, &its->first, 1, &iti->second);
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
        std::map<fmi2_value_reference_t,double*>::iterator it;
        for(it = mInputs.begin(); it != mInputs.end(); it++) {
            fmistatus = fmi2_import_set_real(fmu, &it->first, 1, it->second);
        }

        //Take step
        fmistatus = fmi2_import_do_step(fmu, mTime, mTimestep, true);
        if (fmistatus != fmi2_status_ok) {
            stopSimulation("fmi2_import_do_step failed");
            return;
        }

        //Write outputs
        for(it = mOutputs.begin(); it != mOutputs.end(); it++) {
            fmistatus = fmi2_import_get_real(fmu, &it->first, 1, it->second);
        }
    }

    void finalize()
    {
        if(fmu) {
            fmistatus = fmi2_import_reset(fmu);
        }
    }

    void deconfigure()
    {
        if(fmu) {
            fmistatus = fmi2_import_terminate(fmu);
            fmi2_import_free_instance(fmu);
            fmi2_import_destroy_dllfmu(fmu);
            fmi2_import_free(fmu);
            fmu = NULL;
        }

        if(context) {
            fmi_import_free_context(context);
            context = NULL;
        }

        delete mpTempDir;
    }
};

#endif // FMIWRAPPER_HPP
