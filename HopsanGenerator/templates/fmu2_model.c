#define MODEL_IDENTIFIER fmi2

#include "fmi/fmi2Functions.h"
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
    fmi2String instanceName;
    fmi2String instantiationToken;
    fmi2ComponentEnvironment componentEnvironment;
    fmi2CallbackLogger logger;
    bool loggingOn;
    double startTime;
    double stopTime;

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

    if (fmu->loggingOn == fmi2False) {
        return;
    }

    fmi2Status status = fmi2OK;
    if (strcmp(type, "warning") == 0) {
        status = fmi2Warning;
    }
    else if (strcmp(type, "error") == 0) {
        status = fmi2Error;
    }
    else if (strcmp(type, "fatal") == 0) {
        status = fmi2Fatal;
    }
    else if (strcmp(type, "debug") == 0) {
        status = fmi2OK;
    }
    fmu->logger(fmu->componentEnvironment, fmu->instanceName, status, type, message);
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

const char* fmi2GetVersion(void) {
    return fmi2Version;
}

fmi2Status fmi2SetDebugLogging(fmi2Component c,
                               fmi2Boolean loggingOn,
                               size_t nCategories,
                               const fmi2String categories[])
{
    UNUSED(nCategories);
    UNUSED(categories);
    fmuContext *fmu =(fmuContext*)c;
    fmu->loggingOn = loggingOn;
}

fmi2Component fmi2Instantiate(fmi2String instanceName,
                              fmi2Type fmuType,
                              fmi2String fmuGUID,
                              fmi2String fmuResourceLocation,
                              const fmi2CallbackFunctions* functions,
                              fmi2Boolean visible,
                              fmi2Boolean loggingOn)
{
    UNUSED(visible);

    if(fmuType != fmi2CoSimulation) {
        return NULL;
    }

    fmuContext *fmu = static_cast<fmuContext *>(malloc(sizeof(fmuContext)));

    fmu->instanceName = strdup(instanceName);
    fmu->instantiationToken = fmuGUID;
    fmu->componentEnvironment = functions->componentEnvironment;
    fmu->logger = functions->logger;
    fmu->loggingOn = loggingOn;

    double startT, stopT;      // Dummy variables
    fmu->pSystem = gHopsanCore.loadHMFModel(getModelString().c_str(), startT, stopT);
    if (fmu->pSystem) {
        std::string rl = parseResourceLocation(fmuResourceLocation);
        fmu->pSystem->addSearchPath(rl.c_str());
        fmu->pSystem->setDesiredTimestep(TIMESTEP);
        fmu->pSystem->setNumLogSamples(0);
        fmu->pSystem->disableLog();
        if(!fmu->pSystem->checkModelBeforeSimulation())
        {
            get_all_hopsan_messages(fmu);
            if(fmu->loggingOn) {
                fmu->logger(fmu->componentEnvironment, fmu->instanceName, fmi2Error, "eror", "Model cannot be simulated.");
            }
            return NULL;
        }
        get_all_hopsan_messages(fmu);
    }

    INITDATAPTRS

    if(fmu->loggingOn) {
        fmu->logger(fmu->componentEnvironment, fmu->instanceName, fmi2OK, "info", "Successfully instantiated FMU");
    }

    state = Instantiated;
    return fmu;
}

fmi2Status fmi2SetupExperiment(fmi2Component c,
                               fmi2Boolean toleranceDefined,
                               fmi2Real tolerance,
                               fmi2Real startTime,
                               fmi2Boolean stopTimeDefined,
                               fmi2Real stopTime)
{
    UNUSED(toleranceDefined);
    UNUSED(tolerance);

    fmuContext *fmu = (fmuContext*)c;

    fmu->startTime = startTime;
    if(stopTimeDefined) {
        fmu->stopTime = stopTime;
    }
    else {
        fmu->stopTime = 0;
    }

    return fmi2OK;
}

void fmi2FreeInstance(fmi2Component c)
{
    fmuContext *fmu = (fmuContext*)c;
    free(fmu);
}

fmi2Status fmi2EnterInitializationMode(fmi2Component c)
{
    fmuContext *fmu = (fmuContext*)c;

    fmu->pSystem->initialize(fmu->startTime, fmu->stopTime);
    get_all_hopsan_messages(fmu);

    state = Initializing;
    return fmi2OK;
}

fmi2Status fmi2ExitInitializationMode(fmi2Component c) {
    UNUSED(c);
    state = Initialized;
    return fmi2OK;  //Nothing to do
}

fmi2Status fmi2Terminate(fmi2Component c) {
    fmuContext *fmu = (fmuContext*)c;
    if(fmu) {
        fmu->pSystem->finalize();
        get_all_hopsan_messages(fmu);
        return fmi2OK;
    }
    state = Instantiated;
    return fmi2Error;
}

fmi2Status fmi2Reset(fmi2Component c) {
    fmuContext *fmu = (fmuContext*)c;
    if(fmu) {
        fmu->pSystem->finalize();
        get_all_hopsan_messages(fmu);
        state = Instantiated;
        return fmi2OK;
    }

    return fmi2Error;
}

fmi2Status fmi2GetReal(fmi2Component c,
                       const fmi2ValueReference valueReferences[],
                       size_t nValueReferences,
                       fmi2Real values[])
{
    fmuContext *fmu =(fmuContext*)c;
    fmi2Status status = fmi2OK;
    for(size_t i=0; i<nValueReferences; ++i) {
        if(valueReferences[i] >= NUMDATAPTRS+1) {
            status = fmi2Error;   //Illegal value reference
        }
        else if(valueReferences[i]==0) {
            values[i] = fmu->pSystem->getDesiredTimeStep();
        }
        else {
            if(fmu->dataPtrs[valueReferences[i]]) {
                values[i] = (*(double*)fmu->dataPtrs[valueReferences[i]]);
            }
            else {
                hopsan::HString valueStr;
                fmu->pSystem->getParameterValue(fmu->parNames[valueReferences[i]], valueStr);
                bool ok;
                values[i] = valueStr.toDouble(&ok);
            }
        }
    }
    return status;
}

fmi2Status fmi2SetReal(fmi2Component c,
                       const fmi2ValueReference valueReferences[],
                       size_t nValueReferences,
                       const fmi2Real values[])
{
    fmuContext *fmu =(fmuContext*)c;
    fmi2Status status = fmi2OK;
    for(size_t i=0; i<nValueReferences; ++i) {
        if(valueReferences[i] >= NUMDATAPTRS+1) {
            status = fmi2Error;
        }
        else {
            if(valueReferences[i] == 0) {
                if(state == Instantiated || state == Initializing) {
                    fmu->pSystem->setDesiredTimestep(values[i]);
                }
            }
            else if(fmu->dataPtrs[valueReferences[i]]) {
                (*(double*)fmu->dataPtrs[valueReferences[i]]) = values[i];
            }
            else {
                fmu->pSystem->setSystemParameter(fmu->parNames[valueReferences[i]], to_hstring(values[i]), "double", "", "", true);
            }
        }
    }
    return status;
}

fmi2Status fmi2GetInteger(fmi2Component c,
                          const fmi2ValueReference valueReferences[],
                          size_t nValueReferences,
                          fmi2Integer values[])
{
    UNUSED(c);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    return fmi2Error;
}

fmi2Status fmi2SetInteger(fmi2Component c,
                          const fmi2ValueReference valueReferences[],
                          size_t nValueReferences,
                          const fmi2Integer values[])
{
    UNUSED(c);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    return fmi2Error;
}

fmi2Status fmi2GetBoolean(fmi2Component c,
                          const fmi2ValueReference valueReferences[],
                          size_t nValueReferences,
                          fmi2Boolean values[])
{
    UNUSED(c);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    return fmi2Error;
}

fmi2Status fmi2SetBoolean(fmi2Component c,
                          const fmi2ValueReference valueReferences[],
                          size_t nValueReferences,
                          const fmi2Boolean values[])
{
    UNUSED(c);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    return fmi2Error;
}

fmi2Status fmi2GetString(fmi2Component c,
                         const fmi2ValueReference valueReferences[],
                         size_t nValueReferences,
                         fmi2String values[])
{
    UNUSED(c);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    return fmi2Error;
}

fmi2Status fmi2SetString(fmi2Component c,
                         const fmi2ValueReference valueReferences[],
                         size_t nValueReferences,
                         const fmi2String values[])
{
    UNUSED(c);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(values);
    return fmi2Error;
}

fmi2Status fmi2GetFMUstate (fmi2Component c, fmi2FMUstate* FMUstate)
{
    UNUSED(c);
    UNUSED(FMUstate);
    return fmi2Error;
}

fmi2Status fmi2SetFMUstate (fmi2Component c, fmi2FMUstate FMUstate)
{
    UNUSED(c);
    UNUSED(FMUstate);
    return fmi2Error;
}

fmi2Status fmi2FreeFMUstate(fmi2Component c, fmi2FMUstate* FMUstate)
{
    UNUSED(c);
    UNUSED(FMUstate);
    return fmi2Error;
}

fmi2Status fmi2SerializedFMUstateSize(fmi2Component c,
                                      fmi2FMUstate FMUstate,
                                      size_t *size)
{
    UNUSED(c);
    UNUSED(FMUstate);
    UNUSED(size);
    return fmi2Error;
}

fmi2Status fmi2SerializeFMUstate (fmi2Component c, fmi2FMUstate FMUstate,
                                 fmi2Byte serializedState[], size_t size)
{
    UNUSED(c);
    UNUSED(FMUstate);
    UNUSED(serializedState);
    UNUSED(size);
    return fmi2Error;
}

fmi2Status fmi2DeSerializeFMUstate (fmi2Component c,
                                   const fmi2Byte serializedState[],
                                   size_t size, fmi2FMUstate* FMUstate)
{
    UNUSED(c);
    UNUSED(serializedState);
    UNUSED(size);
    UNUSED(FMUstate);
    return fmi2Error;
}

fmi2Status fmi2GetDirectionalDerivative(fmi2Component c,
                                        const fmi2ValueReference vUnknown_ref[],
                                        size_t nUnknown,
                                        const fmi2ValueReference vKnown_ref[] ,
                                        size_t nKnown,
                                        const fmi2Real dvKnown[],
                                        fmi2Real dvUnknown[])
{
    UNUSED(c);
    UNUSED(vUnknown_ref);
    UNUSED(nUnknown);
    UNUSED(vKnown_ref);
    UNUSED(nKnown);
    UNUSED(dvKnown);
    UNUSED(dvUnknown);
    return fmi2Error;
}

//Co-simulation
fmi2Status fmi2GetRealOutputDerivatives(fmi2Component c,
                                        const fmi2ValueReference valueReferences[],
                                        size_t nValueReferences,
                                        const fmi2Integer order[],
                                        fmi2Real value[])
{
    UNUSED(c);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(order);
    UNUSED(value);
    return fmi2Error;
}

fmi2Status fmi2SetRealInputDerivatives(fmi2Component c,
                                       const fmi2ValueReference valueReferences[],
                                       size_t nValueReferences,
                                       const fmi2Integer order[],
                                       const fmi2Real value[])
{
    UNUSED(c);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(order);
    UNUSED(value);
    return fmi2Error;
}

fmi2Status fmi2DoStep(fmi2Component c,
                      fmi2Real currentCommunicationPoint,
                      fmi2Real communicationStepSize,
                      fmi2Boolean newStep)
{
    UNUSED(newStep);

    fmuContext *fmu =(fmuContext *)c;

    if (fmu == NULL) {
        return fmi2Fatal;
    }
    fmu->pSystem->simulate(currentCommunicationPoint+communicationStepSize);

    get_all_hopsan_messages(fmu);

    return fmi2OK;
}

fmi2Status fmi2CancelStep(fmi2Component c)
{
    UNUSED(c);
    return fmi2Error;   //Should never be called
}

fmi2Status fmi2GetStatus (fmi2Component c,
                         const fmi2StatusKind s,
                         fmi2Status* value)
{
    UNUSED(c);
    UNUSED(s);
    UNUSED(value);
    return fmi2Discard;
}

fmi2Status fmi2GetRealStatus (fmi2Component c,
                             const fmi2StatusKind s,
                             fmi2Real* value)
{
    if(s == fmi2LastSuccessfulTime) {
        fmuContext *fmu =(fmuContext *)c;
        (*value) = fmu->pSystem->getTime();
    }
    return fmi2Discard;
}

fmi2Status fmi2GetIntegerStatus(fmi2Component c,
                                const fmi2StatusKind s,
                                fmi2Integer* value)
{
    UNUSED(c);
    UNUSED(s);
    UNUSED(value);
    return fmi2Discard;
}

fmi2Status fmi2GetBooleanStatus(fmi2Component c,
                                const fmi2StatusKind s,
                                fmi2Boolean* value)
{
    if(s == fmi2Terminated) {
        fmuContext *fmu =(fmuContext *)c;
        (*value) = fmu->pSystem->wasSimulationAborted();
    }
    return fmi2Discard;
}

fmi2Status fmi2GetStringStatus (fmi2Component c,
                               const fmi2StatusKind s,
                               fmi2String* value)
{
    UNUSED(c);
    UNUSED(s);
    UNUSED(value);
    return fmi2Discard;
}

const char* fmi2GetTypesPlatform()
{
    return "default";
}

}
