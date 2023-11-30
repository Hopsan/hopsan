#include "fmi/fmiFunctions.h"
#include "HopsanEssentials.h"
#include "ComponentSystem.h"
#include "ComponentUtilities/num2string.hpp"
#include "model.hpp"
#include <stdbool.h>
#include <stdio.h>
#include <string>
#include <string.h>

enum fmuStateT { Started, Instantiated, Initialized };
fmuStateT state = Started;

<<<data>>>

#define UNUSED(x)(void)(x)

hopsan::HopsanEssentials gHopsanCore;

typedef struct {
    fmiString instanceName;
    fmiString instantiationToken;
    fmiCallbackLogger logger;
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
    if(se = std::string::npos) {
        se = 0;     //Resource location did not contain ':', assume an absolute unix path
    }
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

    if (fmu->loggingOn == fmiFalse) {
        return;
    }

    fmiStatus status = fmiOK;
    if (strcmp(type, "warning") == 0) {
        status = fmiWarning;
    }
    else if (strcmp(type, "error") == 0) {
        status = fmiError;
    }
    else if (strcmp(type, "fatal") == 0) {
        status = fmiFatal;
    }
    else if (strcmp(type, "debug") == 0) {
        status = fmiOK;
    }
    fmu->logger(fmu, fmu->instanceName, status, type, message);
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

#define MODEL_IDENTIFIER <<<modelname>>>

const char* fmiGetVersion() {
    return fmiVersion;
}

fmiStatus fmiSetDebugLogging(fmiComponent c,
                               fmiBoolean loggingOn)
{
    fmuContext *fmu =(fmuContext*)c;
    fmu->loggingOn = loggingOn;
}

fmiComponent fmiInstantiateSlave(fmiString instanceName,
                                 fmiString fmuGUID,
                                 fmiString fmuLocation,
                                 fmiString mimeType,
                                 fmiReal timeout,
                                 fmiBoolean visible,
                                 fmiBoolean interactive,
                                 fmiCallbackFunctions functions,
                                 fmiBoolean loggingOn)
{
    UNUSED(mimeType);
    UNUSED(timeout);
    UNUSED(interactive);
    UNUSED(visible);

    fmuContext *fmu = static_cast<fmuContext *>(malloc(sizeof(fmuContext)));

    fmu->instanceName = strdup(instanceName);
    fmu->instantiationToken = fmuGUID;
    fmu->logger = functions.logger;
    fmu->loggingOn = loggingOn;

    double startT, stopT;      // Dummy variables
    fmu->pSystem = gHopsanCore.loadHMFModel(getModelString().c_str(), startT, stopT);
    if (fmu->pSystem) {
        std::string resourceLocation = fmuLocation;
        resourceLocation = resourceLocation+"/resources";
        std::string rl = parseResourceLocation(resourceLocation);
        fmu->pSystem->addSearchPath(rl.c_str());
        fmu->pSystem->setDesiredTimestep(TIMESTEP);
        fmu->pSystem->setNumLogSamples(0);
        fmu->pSystem->disableLog();
        if(!fmu->pSystem->checkModelBeforeSimulation())
        {
            get_all_hopsan_messages(fmu);
            if(fmu->loggingOn) {
                fmu->logger(fmu, fmu->instanceName, fmiError, "error", "Model cannot be simulated.");
            }
            return NULL;
        }
        get_all_hopsan_messages(fmu);
    }

    INITDATAPTRS

    if(fmu->loggingOn) {
        fmu->logger(fmu, fmu->instanceName, fmiOK, "info", "Successfully instantiated FMU");
    }

    state = Instantiated;

    return fmu;
}

fmiStatus fmiInitializeSlave(fmiComponent c,
                             fmiReal startTime,
                             fmiBoolean stopTimeDefined,
                             fmiReal stopTime)
{
    fmuContext *fmu = (fmuContext*)c;

    fmu->startTime = startTime;
    if(stopTimeDefined) {
        fmu->stopTime = stopTime;
    }
    else {
        fmu->stopTime = 0;
    }

    fmu->pSystem->initialize(fmu->startTime, fmu->stopTime);
    get_all_hopsan_messages(fmu);

    state = Initialized;
}

void fmiFreeSlaveInstance(fmiComponent c)
{
    fmuContext *fmu = (fmuContext*)c;
    free(fmu);
}

fmiStatus fmiTerminateSlave(fmiComponent c) {
    fmuContext *fmu = (fmuContext*)c;
    if(fmu) {
        fmu->pSystem->finalize();
        get_all_hopsan_messages(fmu);
        return fmiOK;
    }
    return fmiError;
}

fmiStatus fmiResetSlave(fmiComponent c) {
    fmuContext *fmu = (fmuContext*)c;
    if(fmu) {
        fmu->pSystem->finalize();
        get_all_hopsan_messages(fmu);
        state = Instantiated;
        return fmiOK;
    }
    return fmiError;
}

fmiStatus fmiGetReal(fmiComponent c,
                     const fmiValueReference valueReferences[],
                     size_t nValueReferences,
                     fmiReal values[])
{
    fmuContext *fmu =(fmuContext*)c;
    fmiStatus status = fmiOK;
    for(size_t i=0; i<nValueReferences; ++i) {
        if(valueReferences[i] >= NUMDATAPTRS+1) {
            status = fmiError;   //Illegal value reference
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

fmiStatus fmiSetReal(fmiComponent c,
                     const fmiValueReference valueReferences[],
                     size_t nValueReferences,
                     const fmiReal values[])
{
    fmuContext *fmu =(fmuContext*)c;
    fmiStatus status = fmiOK;
    for(size_t i=0; i<nValueReferences; ++i) {
        if(valueReferences[i] >= NUMDATAPTRS+1) {
            status = fmiError;
        }
        else {
            if(valueReferences[i] == 0) {
                if(state == Instantiated) {
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

fmiStatus fmiGetInteger(fmiComponent c,
                        const fmiValueReference valueReferences[],
                        size_t nValueReferences,
                        fmiInteger values[])
{
    UNUSED(c);
    UNUSED(valueReferences);
    UNUSED(values);
    if (nValueReferences == 0) {
        return fmiOK;
    }
    else {
        return fmiError;
    }
}

fmiStatus fmiSetInteger(fmiComponent c,
                        const fmiValueReference valueReferences[],
                        size_t nValueReferences,
                        const fmiInteger values[])
{
    UNUSED(c);
    UNUSED(valueReferences);
    UNUSED(values);
    if (nValueReferences == 0) {
        return fmiOK;
    }
    else {
        return fmiError;
    }
}

fmiStatus fmiGetBoolean(fmiComponent c,
                        const fmiValueReference valueReferences[],
                        size_t nValueReferences,
                        fmiBoolean values[])
{
    UNUSED(c);
    UNUSED(valueReferences);
    UNUSED(values);
    if (nValueReferences == 0) {
        return fmiOK;
    }
    else {
        return fmiError;
    }
}

fmiStatus fmiSetBoolean(fmiComponent c,
                        const fmiValueReference valueReferences[],
                        size_t nValueReferences,
                        const fmiBoolean values[])
{
    UNUSED(c);
    UNUSED(valueReferences);
    UNUSED(values);
    if (nValueReferences == 0) {
        return fmiOK;
    }
    else {
        return fmiError;
    }
}

fmiStatus fmiGetString(fmiComponent c,
                       const fmiValueReference valueReferences[],
                       size_t nValueReferences,
                       fmiString values[])
{
    UNUSED(c);
    UNUSED(valueReferences);
    UNUSED(values);
    if (nValueReferences == 0) {
        return fmiOK;
    }
    else {
        return fmiError;
    }
}

fmiStatus fmiSetString(fmiComponent c,
                       const fmiValueReference valueReferences[],
                       size_t nValueReferences,
                       const fmiString values[])
{
    UNUSED(c);
    UNUSED(valueReferences);
    UNUSED(values);
    if (nValueReferences == 0) {
        return fmiOK;
    }
    else {
        return fmiError;
    }
}

//Co-simulation
fmiStatus fmiSetRealInputDerivatives(fmiComponent c,
                                     const fmiValueReference valueReferences[],
                                     size_t nValueReferences,
                                     const fmiInteger order[],
                                     const fmiReal value[])
{
    UNUSED(c);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(order);
    UNUSED(value);
    return fmiError;
}

fmiStatus fmiGetRealOutputDerivatives(fmiComponent c,
                                      const fmiValueReference valueReferences[],
                                      size_t nValueReferences,
                                      const fmiInteger order[],
                                      fmiReal value[])
{
    UNUSED(c);
    UNUSED(valueReferences);
    UNUSED(nValueReferences);
    UNUSED(order);
    UNUSED(value);
    return fmiError;
}

fmiStatus fmiDoStep(fmiComponent c,
                    fmiReal currentCommunicationPoint,
                    fmiReal communicationStepSize,
                    fmiBoolean newStep)
{
    UNUSED(newStep);

    fmuContext *fmu =(fmuContext *)c;

    if (fmu == NULL) {
        return fmiFatal;
    }
    fmu->pSystem->simulate(currentCommunicationPoint+communicationStepSize);

    get_all_hopsan_messages(fmu);

    return fmiOK;
}

fmiStatus fmiCancelStep(fmiComponent c)
{
    UNUSED(c);
    return fmiError;
}

fmiStatus fmiGetStatus( fmiComponent c,
                       const fmiStatusKind s,
                       fmiStatus* value)
{
    UNUSED(c);
    UNUSED(s);
    UNUSED(value);
    return fmiDiscard;
}

fmiStatus fmiGetRealStatus(fmiComponent c,
                           const fmiStatusKind s,
                           fmiReal* value)
{
    if(s == fmiLastSuccessfulTime) {
        fmuContext *fmu =(fmuContext *)c;
        (*value) = fmu->pSystem->getTime();
    }
    return fmiDiscard;
}

fmiStatus fmiGetIntegerStatus(fmiComponent c,
                              const fmiStatusKind s,
                              fmiInteger* value)
{
    UNUSED(c);
    UNUSED(s);
    UNUSED(value);
    return fmiDiscard;
}

fmiStatus fmiGetBooleanStatus(fmiComponent c, const fmiStatusKind s,
                              fmiBoolean* value)
{
    UNUSED(c);
    UNUSED(s);
    UNUSED(value);
    return fmiDiscard;
}

fmiStatus fmiGetStringStatus(fmiComponent c, const fmiStatusKind s,
                             fmiString* value)
{
    UNUSED(c);
    UNUSED(s);
    UNUSED(value);
    return fmiDiscard;
}

const char* fmiGetTypesPlatform()
{
    return "standard32";
}

}
