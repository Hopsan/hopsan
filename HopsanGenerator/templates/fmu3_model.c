#define MODEL_IDENTIFIER fmi3

#include "fmi/fmi3Functions.h"
#include "HopsanEssentials.h"
#include "ComponentSystem.h"
#include "ComponentUtilities/num2string.hpp"
#include "model.hpp"
#include <stdbool.h>
#include <stdio.h>
#include <string>
#include <string.h>

enum fmuStateT { Started, Instantiated, Initializing, Initialized };
fmuStateT state = Started;

<<<data>>>

#define UNUSED(x)(void)(x)

hopsan::HopsanEssentials gHopsanCore;

typedef struct {
    fmi3String instanceName;
    fmi3String instantiationToken;
    fmi3InstanceEnvironment instanceEnvironment;
    fmi3LogMessageCallback logger;
    fmi3IntermediateUpdateCallback intermediateUpdate;
    bool loggingOn;

    hopsan::ComponentSystem *pSystem;
    void *dataPtrs[NUMDATAPTRS+1];        //Data pointer for each value reference, NULL if variable is a parameter
    const char* parNames[NUMDATAPTRS+1];  //Parameter names for value references, NULL for non-parameter variables
} fmuContext;

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

typedef void (*hopsan_message_callback_t) (const char* message, const char* type, void* userState);

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

void forward_message(const char* message, const char* type, void* userState)
{
    fmuContext *fmu = (fmuContext*)userState;
    if (fmu == NULL) {
        return;
    }

    if (fmu->loggingOn == fmi3False) {
        return;
    }

    fmi3Status status = fmi3OK;
    if (strcmp(type, "warning") == 0) {
        status = fmi3Warning;
    }
    else if (strcmp(type, "error") == 0) {
        status = fmi3Error;
    }
    else if (strcmp(type, "fatal") == 0) {
        status = fmi3Fatal;
    }
    else if (strcmp(type, "debug") == 0) {
        status = fmi3OK;
    }
    fmu->logger(fmu->instanceEnvironment, status, type, message);
}

int hopsan_has_message() {
    return (gHopsanCore.checkMessage() > 0) ? 1 : 0;
}

void get_all_hopsan_messages(fmuContext *fmu)
{
    while (hopsan_has_message() > 0) {
        hopsan_get_message(forward_message, (void*)fmu);
    }
}

extern "C" {

const char* fmi3GetVersion(void) {
    return fmi3Version;
}

fmi3Status fmi3SetDebugLogging(fmi3Instance instance,
                               fmi3Boolean loggingOn,
                               size_t nCategories,
                               const fmi3String categories[])
{
    UNUSED(nCategories);
    UNUSED(categories);
    fmuContext *fmu =(fmuContext*)instance;
    fmu->loggingOn = loggingOn;
}

fmi3Instance fmi3InstantiateCoSimulation(fmi3String instanceName,
                                         fmi3String instantiationToken,
                                         fmi3String resourcePath,
                                         fmi3Boolean visible,
                                         fmi3Boolean loggingOn,
                                         fmi3Boolean eventModeUsed,
                                         fmi3Boolean earlyReturnAllowed,
                                         const fmi3ValueReference requiredIntermediateVariables[],
                                         size_t nRequiredIntermediateVariables,
                                         fmi3InstanceEnvironment instanceEnvironment,
                                         fmi3LogMessageCallback logMessage,
                                         fmi3IntermediateUpdateCallback intermediateUpdate)
{
    UNUSED(visible);
    UNUSED(eventModeUsed);
    UNUSED(earlyReturnAllowed);
    UNUSED(requiredIntermediateVariables);
    UNUSED(nRequiredIntermediateVariables);

    fmuContext *fmu = static_cast<fmuContext *>(malloc(sizeof(fmuContext)));

    fmu->instanceName = instanceName;
    fmu->instantiationToken = instantiationToken;
    fmu->instanceEnvironment = instanceEnvironment;
    fmu->logger = logMessage;
    fmu->intermediateUpdate = intermediateUpdate;
    fmu->loggingOn = loggingOn;

    double startT, stopT;      // Dummy variables
    fmu->pSystem = gHopsanCore.loadHMFModel(getModelString().c_str(), startT, stopT);
    if (fmu->pSystem) {
        std::string rl = parseResourceLocation(resourcePath);
        fmu->pSystem->addSearchPath(rl.c_str());
        fmu->pSystem->setDesiredTimestep(TIMESTEP);
        fmu->pSystem->setNumLogSamples(0);
        fmu->pSystem->disableLog();
        if(!fmu->pSystem->checkModelBeforeSimulation())
        {
            get_all_hopsan_messages(fmu);
            if(fmu->loggingOn) {
                fmu->logger(fmu->instanceEnvironment, fmi3Error, "error", "Model cannot be simulated.");
            }
            return NULL;
        }
        get_all_hopsan_messages(fmu);
    }
    else {
        if(fmu->loggingOn) {
            fmu->logger(fmu->instanceEnvironment, fmi3Error, "error", "Model cannot be loaded.");
        }
        return NULL;
    }

    INITDATAPTRS

    if(fmu->loggingOn) {
        fmu->logger(fmu->instanceEnvironment, fmi3OK, "info", "Successfully instantiated FMU");
    }

    state = Instantiated;

    return fmu;
}

void fmi3FreeInstance(fmi3Instance instance)
{
    fmuContext *fmu = (fmuContext*)instance;
    free(fmu);
}

fmi3Status fmi3EnterInitializationMode(fmi3Instance instance,
                                       fmi3Boolean toleranceDefined,
                                       fmi3Float64 tolerance,
                                       fmi3Float64 startTime,
                                       fmi3Boolean stopTimeDefined,
                                       fmi3Float64 stopTime)
{
    UNUSED(toleranceDefined);
    UNUSED(tolerance);
    UNUSED(startTime);
    UNUSED(stopTimeDefined);
    UNUSED(stopTime);
    fmuContext *fmu = (fmuContext*)instance;

    if(fmu == nullptr) {
        return fmi3Error;
    }
    else if(fmu->pSystem == nullptr) {
        if(fmu->loggingOn) {
            fmu->logger(fmu->instanceEnvironment, fmi3Error, "error", "Hopsan system is NULL.");
        }
        return fmi3Error;
    }

    if(fmu->loggingOn) {
        fmu->logger(fmu->instanceEnvironment, fmi3Error, "info", "Entering initialization mode...");
    }

    fmu->pSystem->initialize(startTime, stopTime);
    get_all_hopsan_messages(fmu);

    state = Initializing;

    return fmi3OK;
}

fmi3Status fmi3ExitInitializationMode(fmi3Instance instance) {
    UNUSED(instance);
    state = Initialized;
    return fmi3OK;  //Nothing to do
}

fmi3Status fmi3Terminate(fmi3Instance instance) {
    fmuContext *fmu = (fmuContext*)instance;
    if(fmu) {
        fmu->pSystem->finalize();
        get_all_hopsan_messages(fmu);
        return fmi3OK;
    }
    return fmi3Error;
}

fmi3Status fmi3Reset(fmi3Instance instance) {
    fmuContext *fmu = (fmuContext*)instance;
    if(fmu) {
        fmu->pSystem->finalize();
        get_all_hopsan_messages(fmu);
        state = Instantiated;
        return fmi3OK;
    }
    return fmi3Error;
}

fmi3Status fmi3GetFloat64(fmi3Instance instance,
                          const fmi3ValueReference valueReferences[],
                          size_t nValueReferences,
                          fmi3Float64 values[],
                          size_t nValues) {
    UNUSED(nValues);
    fmuContext *fmu =(fmuContext*)instance;
    fmi3Status status = fmi3OK;
    for(size_t i=0; i<nValueReferences; ++i) {
        if(valueReferences[i] >= NUMDATAPTRS+2) {
            status = fmi3Error;   //Illegal value reference
        }
        else if(valueReferences[i] == 0) {
            values[i] = fmu->pSystem->getTime();
        }
        else if(valueReferences[i] == 1) {
            values[i] = fmu->pSystem->getDesiredTimeStep();
        }
        else {
            values[i] = (*(double*)fmu->dataPtrs[valueReferences[i]]);
        }
    }
    return status;
}

fmi3Status fmi3SetFloat64(fmi3Instance instance,
                          const fmi3ValueReference valueReferences[],
                          size_t nValueReferences,
                          const fmi3Float64 values[],
                          size_t nValues)
{
    UNUSED(nValues);
    fmuContext *fmu =(fmuContext*)instance;
    fmi3Status status = fmi3OK;
    for(size_t i=0; i<nValueReferences; ++i) {
        if(valueReferences[i] >= NUMDATAPTRS+1) {
            status = fmi3Error;
        }
        else {
            if(valueReferences[i] == 1) {
                if(state == Instantiated || state == Initializing) {
                    fmu->pSystem->setDesiredTimestep(values[i]);
                }
            }
            else if(fmu->dataPtrs[valueReferences[i]]) {
                //Non-parameter variable
                (*(double*)fmu->dataPtrs[valueReferences[i]]) = values[i];
            }
            else {
                //Parameter variable (has no data pointer, so use name lookup)
                fmu->pSystem->setSystemParameter(fmu->parNames[valueReferences[i]], to_hstring(values[i]), "double", "", "", true);
            }
        }
    }
    return status;
}

fmi3Status fmi3GetFloat32(fmi3Instance instance,
                          const fmi3ValueReference valueReferences[],
                          size_t nValueReferences,
                          fmi3Float32 values[],
                          size_t nValues)
{
    UNUSED(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);
    return fmi3Error;
}

fmi3Status fmi3SetFloat32(fmi3Instance instance,
                          const fmi3ValueReference valueReferences[],
                          size_t nValueReferences,
                          const fmi3Float32 values[],
                          size_t nValues)
{
    UNUSED(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);
    return fmi3Error;
}

fmi3Status fmi3GetInt64(fmi3Instance instance,
                        const fmi3ValueReference valueReferences[],
                        size_t nValueReferences,
                        fmi3Int64 values[],
                        size_t nValues)
{
    UNUSED(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);
    return fmi3Error;
}

fmi3Status fmi3SetInt64(fmi3Instance instance,
                        const fmi3ValueReference valueReferences[],
                        size_t nValueReferences,
                        const fmi3Int64 values[],
                        size_t nValues)
{
    UNUSED(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);
    return fmi3Error;
}

fmi3Status fmi3GetInt32(fmi3Instance instance,
                        const fmi3ValueReference valueReferences[],
                        size_t nValueReferences,
                        fmi3Int32 values[],
                        size_t nValues)
{
    UNUSED(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);
    return fmi3Error;
}

fmi3Status fmi3SetInt32(fmi3Instance instance,
                        const fmi3ValueReference valueReferences[],
                        size_t nValueReferences,
                        const fmi3Int32 values[],
                        size_t nValues)
{
    UNUSED(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);
    return fmi3Error;
}

fmi3Status fmi3GetInt16(fmi3Instance instance,
                        const fmi3ValueReference valueReferences[],
                        size_t nValueReferences,
                        fmi3Int16 values[],
                        size_t nValues)
{
    UNUSED(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);
    return fmi3Error;
}

fmi3Status fmi3SetInt16(fmi3Instance instance,
                        const fmi3ValueReference valueReferences[],
                        size_t nValueReferences,
                        const fmi3Int16 values[],
                        size_t nValues)
{
    UNUSED(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);
    return fmi3Error;
}

fmi3Status fmi3GetInt8(fmi3Instance instance,
                       const fmi3ValueReference valueReferences[],
                       size_t nValueReferences,
                       fmi3Int8 values[],
                       size_t nValues)
{
    UNUSED(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);
    return fmi3Error;
}

fmi3Status fmi3SetInt8(fmi3Instance instance,
                       const fmi3ValueReference valueReferences[],
                       size_t nValueReferences,
                       const fmi3Int8 values[],
                       size_t nValues)
{
    UNUSED(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);
    return fmi3Error;
}

fmi3Status fmi3GetUInt64(fmi3Instance instance,
                         const fmi3ValueReference valueReferences[],
                         size_t nValueReferences,
                         fmi3UInt64 values[],
                         size_t nValues)
{
    UNUSED(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);
    return fmi3Error;
}

fmi3Status fmi3SetUInt64(fmi3Instance instance,
                         const fmi3ValueReference valueReferences[],
                         size_t nValueReferences,
                         const fmi3UInt64 values[],
                         size_t nValues)
{
    UNUSED(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);
    return fmi3Error;
}

fmi3Status fmi3GetUInt32(fmi3Instance instance,
                         const fmi3ValueReference valueReferences[],
                         size_t nValueReferences,
                         fmi3UInt32 values[],
                         size_t nValues)
{
    UNUSED(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);
    return fmi3Error;
}

fmi3Status fmi3SetUInt32(fmi3Instance instance,
                         const fmi3ValueReference valueReferences[],
                         size_t nValueReferences,
                         const fmi3UInt32 values[],
                         size_t nValues)
{
    UNUSED(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);
    return fmi3Error;
}

fmi3Status fmi3GetUInt16(fmi3Instance instance,
                         const fmi3ValueReference valueReferences[],
                         size_t nValueReferences,
                         fmi3UInt16 values[],
                         size_t nValues)
{
    UNUSED(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);
    return fmi3Error;
}

fmi3Status fmi3SetUInt16(fmi3Instance instance,
                         const fmi3ValueReference valueReferences[],
                         size_t nValueReferences,
                         const fmi3UInt16 values[],
                         size_t nValues)
{
    UNUSED(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);
    return fmi3Error;
}

fmi3Status fmi3GetUInt8(fmi3Instance instance,
                        const fmi3ValueReference valueReferences[],
                        size_t nValueReferences,
                        fmi3UInt8 values[],
                        size_t nValues)
{
    UNUSED(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);
    return fmi3Error;
}

fmi3Status fmi3SetUInt8(fmi3Instance instance,
                        const fmi3ValueReference valueReferences[],
                        size_t nValueReferences,
                        const fmi3UInt8 values[],
                        size_t nValues)
{
    UNUSED(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);
    return fmi3Error;
}

fmi3Status fmi3GetBoolean(fmi3Instance instance,
                          const fmi3ValueReference valueReferences[],
                          size_t nValueReferences,
                          fmi3Boolean values[],
                          size_t nValues)
{
    UNUSED(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);
    return fmi3Error;
}

fmi3Status fmi3GetString(fmi3Instance instance,
                         const fmi3ValueReference valueReferences[],
                         size_t nValueReferences,
                         fmi3String values[],
                         size_t nValues)
{
    UNUSED(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);
    return fmi3Error;
}

fmi3Status fmi3GetBinary(fmi3Instance instance,
                         const fmi3ValueReference valueReferences[],
                         size_t nValueReferences,
                         size_t valueSizes[],
                         fmi3Binary values[],
                         size_t nValues)
{
    UNUSED(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    UNUSED(nValues);
    return fmi3Error;
}

fmi3Status fmi3GetClock(fmi3Instance instance,
                        const fmi3ValueReference valueReferences[],
                        size_t nValueReferences,
                        fmi3Clock values[])
{
    UNUSED(instance);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    return fmi3Error;
}

//Co-simulation
fmi3Status fmi3DoStep(fmi3Instance instance,
                      fmi3Float64 currentCommunicationPoint,
                      fmi3Float64 communicationStepSize,
                      fmi3Boolean noSetFMUStatePriorToCurrentPoint,
                      fmi3Boolean* eventEncountered,
                      fmi3Boolean* terminateSimulation,
                      fmi3Boolean* earlyReturn,
                      fmi3Float64* lastSuccessfulTime)
{
    UNUSED(noSetFMUStatePriorToCurrentPoint);
    UNUSED(eventEncountered);
    UNUSED(terminateSimulation);
    UNUSED(earlyReturn);
    UNUSED(lastSuccessfulTime);

    fmuContext *fmu =(fmuContext *)instance;

    if (fmu == NULL) {
        return fmi3Fatal;
    }
    fmu->pSystem->simulate(currentCommunicationPoint+communicationStepSize);

    get_all_hopsan_messages(fmu);

    return fmi3OK;
}

//Unused functions

fmi3Instance fmi3InstantiateModelExchange(fmi3String, fmi3String, fmi3String, fmi3Boolean, fmi3Boolean,
                                           fmi3InstanceEnvironment, fmi3LogMessageCallback)
{
    return NULL;
}

fmi3Instance fmi3InstantiateScheduledExecution(fmi3String, fmi3String, fmi3String, fmi3Boolean,
                                                                   fmi3Boolean,
                                                                   fmi3InstanceEnvironment, fmi3LogMessageCallback,
                                                                   fmi3ClockUpdateCallback,
                                                                   fmi3LockPreemptionCallback,
                                                                   fmi3UnlockPreemptionCallback)
{
    return NULL;
}

fmi3Status fmi3EnterEventMode(fmi3Instance)
{
    return fmi3Discard;
}


fmi3Status fmi3SetBoolean(fmi3Instance, const fmi3ValueReference[], size_t, const fmi3Boolean[], size_t)
{
    return fmi3Discard;
}

fmi3Status fmi3SetString(fmi3Instance, const fmi3ValueReference[], size_t, const fmi3String[], size_t)
{
    return fmi3Discard;
}

fmi3Status fmi3SetBinary(fmi3Instance, const fmi3ValueReference[], size_t, const size_t[], const fmi3Binary[], size_t)
{
    return fmi3Discard;
}

fmi3Status fmi3SetClock(fmi3Instance, const fmi3ValueReference[], size_t, const fmi3Clock[])
{
    return fmi3Discard;
}

fmi3Status fmi3GetNumberOfVariableDependencies(fmi3Instance, fmi3ValueReference, size_t*)
{
    return fmi3Discard;
}
fmi3Status fmi3GetVariableDependencies(fmi3Instance, fmi3ValueReference, size_t[], fmi3ValueReference[],
                                                           size_t[], fmi3DependencyKind[], size_t)
{
    return fmi3Discard;
}

fmi3Status fmi3GetFMUState(fmi3Instance, fmi3FMUState*)
{
    return fmi3Discard;
}

fmi3Status fmi3SetFMUState(fmi3Instance, fmi3FMUState)
{
    return fmi3Discard;
}

fmi3Status fmi3FreeFMUState(fmi3Instance, fmi3FMUState*)
{
    return fmi3Discard;
}

fmi3Status fmi3SerializedFMUStateSize(fmi3Instance, fmi3FMUState, size_t*)
{
    return fmi3Discard;
}

fmi3Status fmi3SerializeFMUState(fmi3Instance, fmi3FMUState, fmi3Byte[], size_t)
{
    return fmi3Discard;
}

fmi3Status fmi3DeserializeFMUState(fmi3Instance, const fmi3Byte[], size_t, fmi3FMUState*)
{
    return fmi3Discard;
}

fmi3Status fmi3GetDirectionalDerivative(fmi3Instance, const fmi3ValueReference[], size_t,
                                                            const fmi3ValueReference[], size_t, const fmi3Float64[],
                                                            size_t, fmi3Float64[], size_t)
{
    return fmi3Discard;
}

fmi3Status fmi3GetAdjointDerivative(fmi3Instance, const fmi3ValueReference[], size_t,
                                                        const fmi3ValueReference[], size_t, const fmi3Float64[],
                                                        size_t, fmi3Float64[], size_t)
{
    return fmi3Discard;
}

fmi3Status fmi3EnterConfigurationMode(fmi3Instance)
{
    return fmi3Discard;
}

fmi3Status fmi3ExitConfigurationMode(fmi3Instance)
{
    return fmi3Discard;
}

fmi3Status fmi3GetIntervalDecimal(fmi3Instance, const fmi3ValueReference[], size_t,
                                                      fmi3Float64[], fmi3IntervalQualifier[])
{
    return fmi3Discard;
}

fmi3Status fmi3GetIntervalFraction(fmi3Instance, const fmi3ValueReference[], size_t,
                                                       fmi3UInt64[], fmi3UInt64[], fmi3IntervalQualifier[])
{
    return fmi3Discard;
}

fmi3Status fmi3GetShiftDecimal(fmi3Instance, const fmi3ValueReference[], size_t, fmi3Float64[])
{
    return fmi3Discard;
}

fmi3Status fmi3GetShiftFraction(fmi3Instance, const fmi3ValueReference[], size_t,
                                                    fmi3UInt64[], fmi3UInt64[])
{
    return fmi3Discard;
}

fmi3Status fmi3SetIntervalDecimal(fmi3Instance, const fmi3ValueReference[],
                                                      size_t, const fmi3Float64[])
{
    return fmi3Discard;
}

fmi3Status fmi3SetIntervalFraction(fmi3Instance, const fmi3ValueReference[],
                                                       size_t, const fmi3UInt64[], const fmi3UInt64[])
{
    return fmi3Discard;
}

fmi3Status fmi3SetShiftDecimal(fmi3Instance instance, const fmi3ValueReference valueReferences[],
                                                   size_t nValueReferences, const fmi3Float64 shifts[])
{
    return fmi3Discard;
}

fmi3Status fmi3SetShiftFraction(fmi3Instance instance, const fmi3ValueReference valueReferences[],
                                                    size_t nValueReferences, const fmi3UInt64 counters[],
                                                    const fmi3UInt64 resolutions[])
{
    return fmi3Discard;
}

fmi3Status fmi3EvaluateDiscreteStates(fmi3Instance)
{
    return fmi3Discard;
}

fmi3Status fmi3UpdateDiscreteStates(fmi3Instance, fmi3Boolean*, fmi3Boolean*, fmi3Boolean*,
                                                        fmi3Boolean*, fmi3Boolean*, fmi3Float64*)
{
    return fmi3Discard;
}

fmi3Status fmi3EnterContinuousTimeMode(fmi3Instance)
{
    return fmi3Discard;
}

fmi3Status fmi3CompletedIntegratorStep(fmi3Instance, fmi3Boolean, fmi3Boolean*, fmi3Boolean*) {
    return fmi3Discard;
}

fmi3Status fmi3SetTime(fmi3Instance, fmi3Float64)
{
    return fmi3Discard;
}

fmi3Status fmi3SetContinuousStates(fmi3Instance, const fmi3Float64[], size_t)
{
    return fmi3Discard;
}

fmi3Status fmi3GetContinuousStateDerivatives(fmi3Instance, fmi3Float64[], size_t)
{
    return fmi3Discard;
}

fmi3Status fmi3GetEventIndicators(fmi3Instance, fmi3Float64[], size_t)
{
    return fmi3Discard;
}

fmi3Status fmi3GetContinuousStates(fmi3Instance, fmi3Float64[], size_t)
{
    return fmi3Discard;
}

fmi3Status fmi3GetNominalsOfContinuousStates(fmi3Instance, fmi3Float64[], size_t)
{
    return fmi3Discard;
}

fmi3Status fmi3GetNumberOfEventIndicators(fmi3Instance, size_t*)
{
    return fmi3Discard;
}

fmi3Status fmi3GetNumberOfContinuousStates(fmi3Instance, size_t*)
{
    return fmi3Discard;
}

fmi3Status fmi3EnterStepMode(fmi3Instance)
{
    return fmi3Discard;
}
fmi3Status fmi3GetOutputDerivatives(fmi3Instance, const fmi3ValueReference[], size_t,
                                                        const fmi3Int32[], fmi3Float64[], size_t)
{
    return fmi3Discard;
}

fmi3Status fmi3ActivateModelPartition(fmi3Instance, fmi3ValueReference, fmi3Float64)
{
    return fmi3Discard;
}

}
