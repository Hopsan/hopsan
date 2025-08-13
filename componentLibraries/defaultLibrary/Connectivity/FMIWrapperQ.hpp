#ifndef FMIWRAPPERQ_HPP
#define FMIWRAPPERQ_HPP

#define UNDERSCORE 95
#define UPPERCASE_LOW 65
#define UPPERCASE_HIGH 90
#define LOWERCASE_LOW 97
#define LOWERCASE_HIGH 122
#define NUMBERS_LOW 48
#define NUMBERS_HIGH 57


#include "HopsanEssentials.h"
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "CoreUtilities/StringUtilities.h"
#include <algorithm>

#ifdef USEFMI4C
#include "fmi4c.h"
#include <cstdarg>
#endif

//!
//! @file FMIWrapper.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date 2021-01-27
//! @brief Wrapper component for functional mockup units (.fmu)
//!

namespace hopsan {

#ifndef USEFMI4C
class FMIWrapperQ : public ComponentSignal
{
public:
    static Component *Creator()
    {
        return new FMIWrapperQ();
    }

    void configure()
    {

    }

    void reconfigure()
    {

    }

    void initialize()
    {
        stopSimulation("FMIWrapperQ not available: Hopsan is compieled without fmi4c library.");
    }

    void simulateOneTimestep()
    {

    }

    void finalize()
    {

    }

    void deconfigure()
    {

    }
};

#else
void FMIWrapperQ_fmi1Logger(fmi1Component*, fmi1String instanceName, fmi1Status status, fmi1String category, fmi1String message, ...)
{
    HOPSAN_UNUSED(instanceName);
    HOPSAN_UNUSED(status);
    HOPSAN_UNUSED(category);
    va_list args;
    va_start(args, message);
    printf(message, args);  //This is the best we can do, because FMI 1.0 does not allow passing a pointer to the actual component
    va_end(args);
}

void FMIWrapperQ_fmi2Logger(fmi2ComponentEnvironment pComponentEnvironment,
                           fmi2String instanceName,
                           fmi2Status status,
                           fmi2String category,
                           fmi2String message,
                           ...)
{
    HOPSAN_UNUSED(instanceName);
    HOPSAN_UNUSED(category);

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
    case fmi2OK:
        pComponent->addInfoMessage(buffer);
        break;
    case fmi2Warning:
        pComponent->addWarningMessage(buffer);
        break;
    case fmi2Error:
        pComponent->addErrorMessage(buffer);
        break;
    case fmi2Fatal:
        pComponent->addFatalMessage(buffer);
        break;
    default:
        // Discard
        break;
    }
}

void  FMIWrapperQ_fmi3Logger(fmi3InstanceEnvironment instanceEnvironment,
                           fmi3Status status,
                           fmi3String category,
                           fmi3String message)
{
    HOPSAN_UNUSED(category);

    hopsan::Component* pComponent = (hopsan::Component*)instanceEnvironment;
    if (pComponent == NULL) {
        return;
    }

    switch (status) {
    case fmi3OK:
        pComponent->addInfoMessage(message);
        break;
    case fmi3Warning:
        pComponent->addWarningMessage(message);
        break;
    case fmi3Error:
        pComponent->addErrorMessage(message);
        break;
    case fmi3Fatal:
        pComponent->addFatalMessage(message);
        break;
    default:
        // Discard
        break;
    }
}

void FMIWrapperQ_fmi3IntermediateUpdate(
    fmi3InstanceEnvironment instanceEnvironment,
    fmi3Float64  intermediateUpdateTime,
    fmi3Boolean  intermediateVariableSetRequested,
    fmi3Boolean  intermediateVariableGetAllowed,
    fmi3Boolean  intermediateStepFinished,
    fmi3Boolean  canReturnEarly,
    fmi3Boolean* earlyReturnRequested,
    fmi3Float64* earlyReturnTime)
{
    HOPSAN_UNUSED(instanceEnvironment);
    HOPSAN_UNUSED(intermediateUpdateTime);
    HOPSAN_UNUSED(intermediateVariableSetRequested);
    HOPSAN_UNUSED(intermediateVariableGetAllowed);
    HOPSAN_UNUSED(intermediateStepFinished);
    HOPSAN_UNUSED(canReturnEarly);
    HOPSAN_UNUSED(earlyReturnRequested);
    HOPSAN_UNUSED(earlyReturnTime);

    return; // We have no use for intermediate updates in Hopsan at the moment
}

class FMIWrapperQ : public ComponentQ
{
private:
    HFilePath mFmuPath, mLastFmuPath;
    std::map<fmi3ValueReference,double*> mRealOutputs;
    std::map<fmi3ValueReference,double*> mIntOutputs;
    std::map<fmi3ValueReference,double*> mBoolOutputs;
    std::map<fmi3ValueReference,double*> mRealInputs;
    std::map<fmi3ValueReference,double*> mIntInputs;
    std::map<fmi3ValueReference,double*> mBoolInputs;
    std::map<fmi3ValueReference,double> mRealParameters;
    std::map<fmi3ValueReference,bool> mBoolParameters;
    std::map<fmi3ValueReference,int> mIntParameters;
    std::map<fmi3ValueReference,HString> mStringParameters;

    //FMI3 specific variable maps
    std::map<fmi3ValueReference,double*> mFloat64Outputs;
    std::map<fmi3ValueReference,double*> mFloat32Outputs;
    std::map<fmi3ValueReference,double*> mInt64Outputs;
    std::map<fmi3ValueReference,double*> mInt32Outputs;
    std::map<fmi3ValueReference,double*> mInt16Outputs;
    std::map<fmi3ValueReference,double*> mInt8Outputs;
    std::map<fmi3ValueReference,double*> mUInt64Outputs;
    std::map<fmi3ValueReference,double*> mUInt32Outputs;
    std::map<fmi3ValueReference,double*> mUInt16Outputs;
    std::map<fmi3ValueReference,double*> mUInt8Outputs;
    std::map<fmi3ValueReference,double*> mFloat64Inputs;
    std::map<fmi3ValueReference,double*> mFloat32Inputs;
    std::map<fmi3ValueReference,double*> mInt64Inputs;
    std::map<fmi3ValueReference,double*> mInt32Inputs;
    std::map<fmi3ValueReference,double*> mInt16Inputs;
    std::map<fmi3ValueReference,double*> mInt8Inputs;
    std::map<fmi3ValueReference,double*> mUInt64Inputs;
    std::map<fmi3ValueReference,double*> mUInt32Inputs;
    std::map<fmi3ValueReference,double*> mUInt16Inputs;
    std::map<fmi3ValueReference,double*> mUInt8Inputs;
    std::map<fmi3ValueReference,double> mFloat64Parameters;
    std::map<fmi3ValueReference,double> mFloat32Parameters;
    std::map<fmi3ValueReference,int> mInt64Parameters;
    std::map<fmi3ValueReference,int> mInt32Parameters;
    std::map<fmi3ValueReference,int> mInt16Parameters;
    std::map<fmi3ValueReference,int> mInt8Parameters;
    std::map<fmi3ValueReference,int> mUInt64Parameters;
    std::map<fmi3ValueReference,int> mUInt32Parameters;
    std::map<fmi3ValueReference,int> mUInt16Parameters;
    std::map<fmi3ValueReference,int> mUInt8Parameters;

    std::vector<Port*> mPorts;
    HString mPortSpecs, mLastPortSpecs;

    fmiVersion_t mFmiVersion;
    fmuHandle *fmu;
    fmi1InstanceHandle *fmi1_instance;
    fmi2InstanceHandle *fmi2_instance;
    fmi3InstanceHandle *fmi3_instance;
    HString mVisibleOutputs;
    double mTolerance = 1e-4;
    bool mLoggingOn = false;
    bool mReinstantiate = false;
    bool mIsInstantiated = false;


public:
    static Component *Creator()
    {
        return new FMIWrapperQ();
    }

    void configure()
    {
        addConstant("path", "Path to functional mockup unit (FMU)", mFmuPath);
        addConstant("portspecs", "Port specifications", "", mPortSpecs);
        addConstant("reinstantiate", "Create a new FMU instance for every simulation", "", mReinstantiate, mReinstantiate);
        setReconfigurationParameter("path");
        setReconfigurationParameter("portspecs");
    }

    void reconfigure()
    {
        mLastFmuPath = mFmuPath;
        mLastPortSpecs = mPortSpecs;

        deconfigure(); //Make sure to unload FMU and free memory before loading a new one

        for(size_t i=0; i<mPorts.size(); ++i) {
            removePort(mPorts[i]->getName());
        }
        std::vector<HString> parameters;
        this->getParameterNames(parameters);
        for(size_t i=0; i<parameters.size(); ++i) {
            if(parameters[i] != "path" && parameters[i] != "portspecs" && parameters[i] != "reinstantiate") {
                this->unRegisterParameter(parameters[i]);
            }
        }
        mPorts.clear();
        mVisibleOutputs.clear();
        mRealOutputs.clear();
        mBoolOutputs.clear();
        mIntOutputs.clear();
        mRealInputs.clear();
        mBoolInputs.clear();
        mIntInputs.clear();
        mRealParameters.clear();
        mStringParameters.clear();
        mBoolParameters.clear();
        mIntParameters.clear();
        mFloat64Outputs.clear();
        mFloat32Outputs.clear();
        mInt64Outputs.clear();
        mInt32Outputs.clear();
        mInt16Outputs.clear();
        mInt8Outputs.clear();
        mUInt64Outputs.clear();
        mUInt32Outputs.clear();
        mUInt16Outputs.clear();
        mUInt8Outputs.clear();
        mFloat64Inputs.clear();
        mFloat32Inputs.clear();
        mInt64Inputs.clear();
        mInt32Inputs.clear();
        mInt16Inputs.clear();
        mInt8Inputs.clear();
        mUInt64Inputs.clear();
        mUInt32Inputs.clear();
        mUInt16Inputs.clear();
        mUInt8Inputs.clear();
        mFloat64Parameters.clear();
        mFloat32Parameters.clear();
        mInt64Parameters.clear();
        mInt32Parameters.clear();
        mInt16Parameters.clear();
        mInt8Parameters.clear();
        mUInt64Parameters.clear();
        mUInt32Parameters.clear();
        mUInt16Parameters.clear();
        mUInt8Parameters.clear();

        addInfoMessage("Loading FMU from "+mFmuPath+"...");

        HString fmuName = "fmu_"+to_hstring(rand() % 1000000000);
        addDebugMessage("FMU name: "+fmuName);

        fmu = fmi4c_loadFmu(findFilePath(mFmuPath).c_str(), fmuName.c_str());
        if(fmu == nullptr) {
            addErrorMessage("Failed to load FMU file: "+mFmuPath);
            return;
        }
        mFmiVersion = fmi4c_getFmiVersion(fmu);
        if(mFmiVersion == fmiVersionUnknown) {
            //! @todo Implement FMI 1.0 support
            addErrorMessage("The code only supports FMI version 1, 2 and 3");
            return;
        }

        //Parse powerport specifications
        HVector<HString> usedVariables;
        HVector<HString> ports = mPortSpecs.split(';');
        int portNumber = 1;
        for(size_t i=0; i<ports.size(); ++i) {
            ports[i] = ports[i].replace(" ", "");
            HVector<HString> specs = ports[i].split(',');
            HString nodeType, portName;
            if(specs[0].containes(':')) {
                nodeType = specs[0].split(':')[0];
                portName = specs[0].split(':')[1];
            }
            else {
                nodeType = specs[0];
                portName = "P"+to_hstring(portNumber);
            }

            //Check that node is not empty (parameter not given)
            if(nodeType.empty()) {
                continue;
            }

            //Check that node exists in HopsanCore
            hopsan::HopsanEssentials hopsanCore;
            std::vector<HString> v = hopsanCore.getRegisteredNodeTypes();
            if(std::find(v.begin(), v.end(), nodeType) == v.end()) {
                addErrorMessage("Unsupported node type: "+nodeType);
                return;
            }

            //Check port name
            if(!hopsan::isNameValid(portName)) {
                addErrorMessage("Illegal port name: "+portName);
                return;
            }

            //Create dummy node
            hopsan::Node *pDummyNode = hopsanCore.createNode(nodeType);
            size_t expectedNumberOfVariables = pDummyNode->getDataDescriptions()->size();

            //Check that corrent number of variables is given
            size_t numberOfVariables = specs.size()-1;
            if(numberOfVariables != expectedNumberOfVariables)
            {
                addErrorMessage("Error in port type specification: \""+nodeType+"\" requires "+to_hstring(expectedNumberOfVariables)+" variables.");
                return;
            }

            //Check that all variables exist and are unique
            for(size_t i=0; i<numberOfVariables; ++i) {
                if(usedVariables.contains(specs[i+1])) {
                    addErrorMessage("Variable \""+specs[i+1]+"\" can only be used once in the \"portspecs\" parameter.");
                    return;
                }
                std::vector<HString> portNames = getPortNames();
                if(mFmiVersion == fmiVersion1 && fmi1_getVariableByName(fmu, specs[i+1].c_str()) == nullptr) {
                    addErrorMessage("Variable \""+specs[i+1]+"\" does not exist in FMU.");
                    return;
                }
                else if(mFmiVersion == fmiVersion2 && fmi2_getVariableByName(fmu, specs[i+1].c_str()) == nullptr) {
                    addErrorMessage("Variable \""+specs[i+1]+"\" does not exist in FMU.");
                    return;
                }
                else if(mFmiVersion == fmiVersion3 && fmi3_getVariableByName(fmu, specs[i+1].c_str()) == nullptr) {
                    addErrorMessage("Variable \""+specs[i+1]+"\" does not exist in FMU.");
                    return;
                }

                usedVariables.append(specs[i+1]);
            }

            Port *pPort = addPowerPort(portName, nodeType, "");
            mPorts.push_back(pPort);

            ++portNumber;
        }

        if(mFmiVersion == fmiVersion1) {
            if(fmu == NULL) {
                addErrorMessage("Failed to load FMU file: "+mFmuPath);
                return;
            }

            if(fmi1_defaultToleranceDefined(fmu)) {
                mTolerance = fmi1_getDefaultTolerance(fmu);
            }
            addConstant("tol", "Relative tolerance", "", mTolerance, mTolerance);
            addConstant("loggingOn", "Enable FMU logging", "", mLoggingOn, mLoggingOn);

            //Loop through variables in FMU and generate the lists
            for(int i=0; i<fmi1_getNumberOfVariables(fmu); ++i) {
                addDebugMessage("Testing variable...");
                fmi1VariableHandle* var = fmi1_getVariableByIndex(fmu, i+1);
                const char* name = fmi1_getVariableName(var);
                const char* description = fmi1_getVariableDescription(var);
                if(description == NULL) {
                    description = "";
                }
                fmi1DataType type = fmi1_getVariableDataType(var);
                fmi1Causality causality = fmi1_getVariableCausality(var);
                fmi1Variability variability = fmi1_getVariableVariability(var);
                addDebugMessage("Causality = "+to_hstring(causality));
                addDebugMessage("Data type = "+to_hstring(type));
                unsigned int vr = (unsigned int)fmi1_getVariableValueReference(var);

                if(variability == fmi1VariabilityParameter && type == fmi1DataTypeString) {
                    addDebugMessage("String parameter: "+HString(name));
                    const char* startValue = fmi1_getVariableStartString(var);
                    addConstant(toValidHopsanVarName(name), description, "", startValue, mStringParameters[vr]);
                }
                else if(variability == fmi1VariabilityParameter && type == fmi1DataTypeBoolean) {
                    addDebugMessage("Boolean parameter: "+HString(name));
                    bool startValue = fmi1_getVariableStartBoolean(var);
                    addConstant(toValidHopsanVarName(name), description, "", startValue, mBoolParameters[vr]);
                }
                else if(variability == fmi1VariabilityParameter && type == fmi1DataTypeInteger) {
                    addDebugMessage("Integer parameter: "+HString(name));
                    int startValue = fmi1_getVariableStartInteger(var);
                    addConstant(toValidHopsanVarName(name), description, "", startValue, mIntParameters[vr]);
                }
                else if(variability == fmi1VariabilityParameter && type == fmi1DataTypeReal) {
                    addDebugMessage("Real parameter: "+HString(name));
                    double startValue = fmi1_getVariableStartReal(var);
                    addDebugMessage("START VALUE: "+to_hstring(startValue));
                    addConstant(toValidHopsanVarName(name), description, "", startValue, mRealParameters[vr]);
                }
                else if(causality == fmi1CausalityInput && type == fmi1DataTypeReal && !usedVariables.contains(name)) {
                    addDebugMessage("Real input: "+HString(name));
                    double startValue = fmi1_getVariableStartReal(var);
                    addDebugMessage("START VALUE: "+to_hstring(startValue));
                    mPorts.push_back(addInputVariable(toValidHopsanVarName(name), description, "", startValue, &mRealInputs[vr]));
                }
                else if(causality == fmi1CausalityInput && type == fmi1DataTypeInteger) {
                    addDebugMessage("Integer input: "+HString(name));
                    double startValue = (double)fmi1_getVariableStartInteger(var);
                    mPorts.push_back(addInputVariable(toValidHopsanVarName(name), description, "", startValue, &mIntInputs[vr]));
                }
                else if(causality == fmi1CausalityInput && type == fmi1DataTypeBoolean) {
                    addDebugMessage("Boolean input: "+HString(name));
                    double startValue = fmi1_getVariableStartBoolean(var) ? 1.0 : 0.0;
                    mPorts.push_back(addInputVariable(toValidHopsanVarName(name), description, "", startValue, &mBoolInputs[vr]));
                }
                else if(causality == fmi1CausalityOutput && type == fmi1DataTypeReal && !usedVariables.contains(name)) {
                    addDebugMessage("Real output: "+HString(name));
                    mPorts.push_back(addOutputVariable(toValidHopsanVarName(name), description, "", &mRealOutputs[vr]));
                    mVisibleOutputs.append(toValidHopsanVarName(name)+",");
                }
                else if(causality == fmi1CausalityOutput && (type == fmi1DataTypeInteger)) {
                    addDebugMessage("Integer output: "+HString(name));
                    mPorts.push_back(addOutputVariable(toValidHopsanVarName(name), description, "", &mIntOutputs[vr]));
                    mVisibleOutputs.append(toValidHopsanVarName(name)+",");
                }
                else if(causality == fmi1CausalityOutput && (type == fmi1DataTypeBoolean)) {
                    addDebugMessage("Boolean output: "+HString(name));
                    mPorts.push_back(addOutputVariable(toValidHopsanVarName(name), description, "", &mBoolOutputs[vr]));
                    mVisibleOutputs.append(toValidHopsanVarName(name)+",");
                }
                else if(causality == fmi1CausalityInternal && type == fmi1DataTypeReal) {
                    addDebugMessage("Local: "+HString(name));
                    mPorts.push_back(addOutputVariable(toValidHopsanVarName(name), description, "", &mRealOutputs[vr]));
                }
                else if(causality == fmi1CausalityInternal && type == fmi1DataTypeInteger) {
                    addDebugMessage("Local: "+HString(name));
                    mPorts.push_back(addOutputVariable(toValidHopsanVarName(name), description, "", &mIntOutputs[vr]));
                }
                else if(causality == fmi1CausalityInternal && type == fmi1DataTypeBoolean) {
                    addDebugMessage("Local: "+HString(name));
                    mPorts.push_back(addOutputVariable(toValidHopsanVarName(name), description, "", &mBoolOutputs[vr]));
                }
            }
            if(!mVisibleOutputs.empty() && mVisibleOutputs.back() == ',') {
                mVisibleOutputs.erase(mVisibleOutputs.size()-1,1);  //Remove trailing comma
            }
            addConstant("visibleOutputs", "Visible output variables (hidden)", "", mVisibleOutputs, mVisibleOutputs);


            //Instantiate FMU
<<<<<<< HEAD
            if(!mReinstantiate) {
                addDebugMessage("Calling: fmi1InstantiateSlave");
                if(!fmi1_instantiateSlave(fmu, "application/x-fmu-sharedlibrary", 1000, fmi1False, fmi1False, FMIWrapperQ_fmi1Logger, calloc, free, NULL, mLoggingOn)) {
                    addErrorMessage("Hopsan: fmi1InstantiateSlave() failed!");
                    fmu = NULL;
                    return;
                }
                mIsInstantiated = true;
=======
            printf("Hopsan: calling fmi1InstantiateSlave()...");
            fmi1_instance = fmi1_instantiateSlave(fmu, "application/x-fmu-sharedlibrary", 1000, fmi1False, fmi1False, FMIWrapperQ_fmi1Logger, calloc, free, NULL, mLoggingOn);
            if(fmi1_instance == NULL) {
                addErrorMessage("Hopsan: fmi1InstantiateSlave() failed!");
                fmu = NULL;
                return;
>>>>>>> 9529d1b70 (Upgrade fmi4c to v1.1)
            }

            if(NULL == fmu) {
                stopSimulation("Failed to instantiate FMU");
                return;
            }

            addInfoMessage("Successfully instantiated FMU");
        }
        else if(mFmiVersion == fmiVersion2) {
            if(fmi2_defaultToleranceDefined(fmu)) {
                mTolerance = fmi2_getDefaultTolerance(fmu);
            }
            addConstant("tol", "Relative tolerance", "", mTolerance, mTolerance);
            addConstant("loggingOn", "Enable FMU logging", "", mLoggingOn, mLoggingOn);

            //Loop through variables in FMU and generate the lists
            for(int i=0; i<fmi2_getNumberOfVariables(fmu); ++i) {
                fmi2VariableHandle* var = fmi2_getVariableByIndex(fmu, i+1);
                const char* name = fmi2_getVariableName(var);
                const char* description = fmi2_getVariableDescription(var);
                if(description == NULL) {
                    description = "";
                }
                fmi2DataType type = fmi2_getVariableDataType(var);
                fmi2Causality causality = fmi2_getVariableCausality(var);
                unsigned int vr = (unsigned int)fmi2_getVariableValueReference(var);

                if(causality == fmi2CausalityParameter && type == fmi2DataTypeString) {
                    addDebugMessage("String parameter: "+HString(name));
                    const char* startValue = fmi2_getVariableStartString(var);
                    addConstant(toValidHopsanVarName(name), description, "", startValue, mStringParameters[vr]);
                }
                else if(causality == fmi2CausalityParameter && type == fmi2DataTypeBoolean) {
                    addDebugMessage("Boolean parameter: "+HString(name));
                    bool startValue = fmi2_getVariableStartBoolean(var);
                    addConstant(toValidHopsanVarName(name), description, "", startValue, mBoolParameters[vr]);
                }
                else if(causality == fmi2CausalityParameter && type == fmi2DataTypeInteger) {
                    addDebugMessage("Integer parameter: "+HString(name));
                    int startValue = fmi2_getVariableStartInteger(var);
                    addConstant(toValidHopsanVarName(name), description, "", startValue, mIntParameters[vr]);
                }
                else if(causality == fmi2CausalityParameter && type == fmi2DataTypeReal) {
                    addDebugMessage("Real parameter: "+HString(name));
                    double startValue = fmi2_getVariableStartReal(var);
                    addDebugMessage("START VALUE: "+to_hstring(startValue));
                    addConstant(toValidHopsanVarName(name), description, "", startValue, mRealParameters[vr]);
                }
                else if(causality == fmi2CausalityInput && type == fmi2DataTypeReal && !usedVariables.contains(name)) {
                    addDebugMessage("Real input: "+HString(name));
                    double startValue = fmi2_getVariableStartReal(var);
                    addDebugMessage("START VALUE: "+to_hstring(startValue));
                    mPorts.push_back(addInputVariable(toValidHopsanVarName(name), description, "", startValue, &mRealInputs[vr]));
                }
                else if(causality == fmi2CausalityInput && type == fmi2DataTypeInteger) {
                    addDebugMessage("Integer input: "+HString(name));
                    double startValue = (double)fmi2_getVariableStartInteger(var);
                    mPorts.push_back(addInputVariable(toValidHopsanVarName(name), description, "", startValue, &mIntInputs[vr]));
                }
                else if(causality == fmi2CausalityInput && type == fmi2DataTypeBoolean) {
                    addDebugMessage("Boolean input: "+HString(name));
                    double startValue = fmi2_getVariableStartBoolean(var) ? 1.0 : 0.0;
                    mPorts.push_back(addInputVariable(toValidHopsanVarName(name), description, "", startValue, &mBoolInputs[vr]));
                }
                else if(causality == fmi2CausalityOutput && type == fmi2DataTypeReal && !usedVariables.contains(name)) {
                    addDebugMessage("Real output: "+HString(name));
                    mPorts.push_back(addOutputVariable(toValidHopsanVarName(name), description, "", &mRealOutputs[vr]));
                    mVisibleOutputs.append(toValidHopsanVarName(name)+",");
                }
                else if(causality == fmi2CausalityOutput && (type == fmi2DataTypeInteger)) {
                    addDebugMessage("Integer output: "+HString(name));
                    mPorts.push_back(addOutputVariable(toValidHopsanVarName(name), description, "", &mIntOutputs[vr]));
                    mVisibleOutputs.append(toValidHopsanVarName(name)+",");
                }
                else if(causality == fmi2CausalityOutput && (type == fmi2DataTypeBoolean)) {
                    addDebugMessage("Boolean output: "+HString(name));
                    mPorts.push_back(addOutputVariable(toValidHopsanVarName(name), description, "", &mBoolOutputs[vr]));
                    mVisibleOutputs.append(toValidHopsanVarName(name)+",");
                }
                else if(causality == fmi2CausalityLocal && type == fmi2DataTypeReal) {
                    addDebugMessage("Local: "+HString(name));
                    mPorts.push_back(addOutputVariable(toValidHopsanVarName(name), description, "", &mRealOutputs[vr]));
                }
                else if(causality == fmi2CausalityLocal && type == fmi2DataTypeInteger) {
                    addDebugMessage("Local: "+HString(name));
                    mPorts.push_back(addOutputVariable(toValidHopsanVarName(name), description, "", &mIntOutputs[vr]));
                }
                else if(causality == fmi2CausalityLocal && type == fmi2DataTypeBoolean) {
                    addDebugMessage("Local: "+HString(name));
                    mPorts.push_back(addOutputVariable(toValidHopsanVarName(name), description, "", &mBoolOutputs[vr]));
                }
            }
            if(!mVisibleOutputs.empty() && mVisibleOutputs.back() == ',') {
                mVisibleOutputs.erase(mVisibleOutputs.size()-1,1);  //Remove trailing comma
            }
            addConstant("visibleOutputs", "Visible output variables (hidden)", "", mVisibleOutputs, mVisibleOutputs);


            //Instantiate FMU
<<<<<<< HEAD
            if(!mReinstantiate) {
                addDebugMessage("Calling: fmi2Instantiate");
                if(!fmi2_instantiate(fmu, fmi2CoSimulation, FMIWrapperQ_fmi2Logger, calloc, free, NULL, (fmi2ComponentEnvironment*)this, fmi2False, mLoggingOn)) {
                    stopSimulation("Failed to instantiate FMU");
                    fmu = NULL;
                    return;
                }
                mIsInstantiated = true;
=======
            fmi2_instance = fmi2_instantiate(fmu, fmi2CoSimulation, FMIWrapperQ_fmi2Logger, calloc, free, NULL, (fmi2ComponentEnvironment*)this, fmi2False, mLoggingOn);

            if(NULL == fmi2_instance) {
                stopSimulation("Failed to instantiate FMU");
                return;
>>>>>>> 9529d1b70 (Upgrade fmi4c to v1.1)
            }
        }
        else {//FMI 3
            if(fmi3_defaultToleranceDefined(fmu)) {
                mTolerance = fmi3_getDefaultTolerance(fmu);
            }
            addConstant("tol", "Relative tolerance", "", mTolerance, mTolerance);
            addConstant("loggingOn", "Enable FMU logging", "", mLoggingOn, mLoggingOn);

            //Loop through variables in FMU and generate the lists
            for(int i=0; i<fmi3_getNumberOfVariables(fmu); ++i) {
                fmi3VariableHandle* var = fmi3_getVariableByIndex(fmu, i+1);
                const char* name = fmi3_getVariableName(var);
                const char* description = fmi3_getVariableDescription(var);
                if(description == NULL) {
                    description = "";
                }
                fmi3DataType type = fmi3_getVariableDataType(var);
                fmi3Causality causality = fmi3_getVariableCausality(var);
                fmi3ValueReference vr = fmi3_getVariableValueReference(var);

                if(causality == fmi3CausalityParameter && type == fmi3DataTypeString) {
                    addDebugMessage("String parameter: "+HString(name));
                    const char* startValue = fmi3_getVariableStartString(var);
                    addConstant(toValidHopsanVarName(name), description, "", startValue, mStringParameters[vr]);
                }
                else if(causality == fmi3CausalityParameter && type == fmi3DataTypeBoolean) {
                    addDebugMessage("Boolean parameter: "+HString(name));
                    bool startValue = fmi3_getVariableStartBoolean(var);
                    addConstant(toValidHopsanVarName(name), description, "", startValue, mBoolParameters[vr]);
                }
                else if(causality == fmi3CausalityParameter && type == fmi3DataTypeInt64) {
                    addDebugMessage("64-bit integer parameter: "+HString(name));
                    int startValue = (int)fmi3_getVariableStartInt64(var);
                    addConstant(toValidHopsanVarName(name), description, "", startValue, mIntParameters[vr]);
                }
                else if(causality == fmi3CausalityParameter && type == fmi3DataTypeFloat64) {
                    addDebugMessage("64-bit float parameter: "+HString(name));
                    double startValue = (double)fmi3_getVariableStartFloat64(var);
                    addConstant(toValidHopsanVarName(name), description, "", startValue, mFloat64Parameters[vr]);
                }
                else if(causality == fmi3CausalityParameter && type == fmi3DataTypeFloat32) {
                    addDebugMessage("32-bit float parameter: "+HString(name));
                    double startValue = (double)fmi3_getVariableStartFloat32(var);
                    addConstant(toValidHopsanVarName(name), description, "", startValue, mFloat32Parameters[vr]);
                }
                else if(causality == fmi3CausalityParameter && type == fmi3DataTypeInt64) {
                    addDebugMessage("64-bit float parameter: "+HString(name));
                    int startValue = (int)fmi3_getVariableStartInt64(var);
                    addConstant(toValidHopsanVarName(name), description, "", startValue, mInt64Parameters[vr]);
                }
                else if(causality == fmi3CausalityParameter && type == fmi3DataTypeInt32) {
                    addDebugMessage("32-bit float parameter: "+HString(name));
                    int startValue = (int)fmi3_getVariableStartInt32(var);
                    addConstant(toValidHopsanVarName(name), description, "", startValue, mInt32Parameters[vr]);
                }
                else if(causality == fmi3CausalityParameter && type == fmi3DataTypeInt16) {
                    addDebugMessage("16-bit float parameter: "+HString(name));
                    int startValue = (int)fmi3_getVariableStartInt16(var);
                    addConstant(toValidHopsanVarName(name), description, "", startValue, mInt16Parameters[vr]);
                }
                else if(causality == fmi3CausalityParameter && type == fmi3DataTypeInt8) {
                    addDebugMessage("8-bit float parameter: "+HString(name));
                    int startValue = (int)fmi3_getVariableStartInt8(var);
                    addConstant(toValidHopsanVarName(name), description, "", startValue, mInt8Parameters[vr]);
                }
                else if(causality == fmi3CausalityParameter && type == fmi3DataTypeUInt64) {
                    addDebugMessage("64-bit float parameter: "+HString(name));
                    int startValue = (int)fmi3_getVariableStartUInt64(var);
                    addConstant(toValidHopsanVarName(name), description, "", startValue, mUInt64Parameters[vr]);
                }
                else if(causality == fmi3CausalityParameter && type == fmi3DataTypeUInt32) {
                    addDebugMessage("32-bit float parameter: "+HString(name));
                    int startValue = (int)fmi3_getVariableStartUInt32(var);
                    addConstant(toValidHopsanVarName(name), description, "", startValue, mUInt32Parameters[vr]);
                }
                else if(causality == fmi3CausalityParameter && type == fmi3DataTypeUInt16) {
                    addDebugMessage("16-bit float parameter: "+HString(name));
                    int startValue = (int)fmi3_getVariableStartUInt16(var);
                    addConstant(toValidHopsanVarName(name), description, "", startValue, mUInt16Parameters[vr]);
                }
                else if(causality == fmi3CausalityParameter && type == fmi3DataTypeUInt8) {
                    addDebugMessage("8-bit float parameter: "+HString(name));
                    int startValue = (int)fmi3_getVariableStartUInt8(var);
                    addConstant(toValidHopsanVarName(name), description, "", startValue, mUInt8Parameters[vr]);
                }
                else if(causality == fmi3CausalityInput && type == fmi3DataTypeFloat64 && !usedVariables.contains(name)) {
                    addDebugMessage("64-bit float input: "+HString(name));
                    double startValue = fmi3_getVariableStartFloat64(var);
                    mPorts.push_back(addInputVariable(toValidHopsanVarName(name), description, "", startValue, &mFloat64Inputs[vr]));
                }
                else if(causality == fmi3CausalityInput && type == fmi3DataTypeFloat32 && !usedVariables.contains(name)) {
                    addDebugMessage("32-bit float input: "+HString(name));
                    double startValue = fmi3_getVariableStartFloat32(var);
                    mPorts.push_back(addInputVariable(toValidHopsanVarName(name), description, "", startValue, &mFloat32Inputs[vr]));
                }
                else if(causality == fmi3CausalityInput && type == fmi3DataTypeInt64) {
                    addDebugMessage("64-bit integer input: "+HString(name));
                    double startValue = (double)fmi3_getVariableStartInt64(var);
                    mPorts.push_back(addInputVariable(toValidHopsanVarName(name), description, "", startValue, &mInt64Inputs[vr]));
                }
                else if(causality == fmi3CausalityInput && type == fmi3DataTypeInt32) {
                    addDebugMessage("32-bit integer input: "+HString(name));
                    double startValue = (double)fmi3_getVariableStartInt32(var);
                    mPorts.push_back(addInputVariable(toValidHopsanVarName(name), description, "", startValue, &mInt32Inputs[vr]));
                }
                else if(causality == fmi3CausalityInput && type == fmi3DataTypeInt16) {
                    addDebugMessage("16-bit integer input: "+HString(name));
                    double startValue = (double)fmi3_getVariableStartInt16(var);
                    mPorts.push_back(addInputVariable(toValidHopsanVarName(name), description, "", startValue, &mInt16Inputs[vr]));
                }
                else if(causality == fmi3CausalityInput && type == fmi3DataTypeInt8) {
                    addDebugMessage("8-bit integer input: "+HString(name));
                    double startValue = (double)fmi3_getVariableStartInt8(var);
                    mPorts.push_back(addInputVariable(toValidHopsanVarName(name), description, "", startValue, &mInt8Inputs[vr]));
                }
                else if(causality == fmi3CausalityInput && type == fmi3DataTypeUInt64) {
                    addDebugMessage("64-bit unsigned integer input: "+HString(name));
                    double startValue = (double)fmi3_getVariableStartUInt64(var);
                    mPorts.push_back(addInputVariable(toValidHopsanVarName(name), description, "", startValue, &mUInt64Inputs[vr]));
                }
                else if(causality == fmi3CausalityInput && type == fmi3DataTypeUInt32) {
                    addDebugMessage("32-bit unsigned integer input: "+HString(name));
                    double startValue = (double)fmi3_getVariableStartUInt32(var);
                    mPorts.push_back(addInputVariable(toValidHopsanVarName(name), description, "", startValue, &mUInt32Inputs[vr]));
                }
                else if(causality == fmi3CausalityInput && type == fmi3DataTypeUInt16) {
                    addDebugMessage("16-bit unsigned integer input: "+HString(name));
                    double startValue = (double)fmi3_getVariableStartUInt16(var);
                    mPorts.push_back(addInputVariable(toValidHopsanVarName(name), description, "", startValue, &mUInt16Inputs[vr]));
                }
                else if(causality == fmi3CausalityInput && type == fmi3DataTypeUInt8) {
                    addDebugMessage("8-bit unsigned integer input: "+HString(name));
                    double startValue = (double)fmi3_getVariableStartUInt8(var);
                    mPorts.push_back(addInputVariable(toValidHopsanVarName(name), description, "", startValue, &mUInt8Inputs[vr]));
                }
                else if(causality == fmi3CausalityInput && type == fmi3DataTypeBoolean) {
                    addDebugMessage("Boolean input: "+HString(name));
                    double startValue = fmi3_getVariableStartBoolean(var) ? 1.0 : 0.0;
                    mPorts.push_back(addInputVariable(toValidHopsanVarName(name), description, "", startValue, &mBoolInputs[vr]));
                }
                else if(causality == fmi3CausalityOutput && type == fmi3DataTypeFloat64) {
                    addDebugMessage("64-bit float output: "+HString(name));
                    mPorts.push_back(addOutputVariable(toValidHopsanVarName(name), description, "", &mFloat64Outputs[vr]));
                    mVisibleOutputs.append(toValidHopsanVarName(name)+",");
                }
                else if(causality == fmi3CausalityOutput && type == fmi3DataTypeFloat32) {
                    addDebugMessage("32-bit float output: "+HString(name));
                    mPorts.push_back(addOutputVariable(toValidHopsanVarName(name), description, "", &mFloat32Outputs[vr]));
                    mVisibleOutputs.append(toValidHopsanVarName(name)+",");
                }
                else if(causality == fmi3CausalityOutput && (type == fmi3DataTypeInt64)) {
                    addDebugMessage("64-bit integer output: "+HString(name));
                    mPorts.push_back(addOutputVariable(toValidHopsanVarName(name), description, "", &mInt64Outputs[vr]));
                    mVisibleOutputs.append(toValidHopsanVarName(name)+",");
                }
                else if(causality == fmi3CausalityOutput && (type == fmi3DataTypeInt32)) {
                    addDebugMessage("32-bit integer output: "+HString(name));
                    mPorts.push_back(addOutputVariable(toValidHopsanVarName(name), description, "", &mInt32Outputs[vr]));
                    mVisibleOutputs.append(toValidHopsanVarName(name)+",");
                }
                else if(causality == fmi3CausalityOutput && (type == fmi3DataTypeInt16)) {
                    addDebugMessage("16-bit integer output: "+HString(name));
                    mPorts.push_back(addOutputVariable(toValidHopsanVarName(name), description, "", &mInt16Outputs[vr]));
                    mVisibleOutputs.append(toValidHopsanVarName(name)+",");
                }
                else if(causality == fmi3CausalityOutput && (type == fmi3DataTypeInt8)) {
                    addDebugMessage("8-bit integer output: "+HString(name));
                    mPorts.push_back(addOutputVariable(toValidHopsanVarName(name), description, "", &mInt8Outputs[vr]));
                    mVisibleOutputs.append(toValidHopsanVarName(name)+",");
                }
                else if(causality == fmi3CausalityOutput && (type == fmi3DataTypeUInt64)) {
                    addDebugMessage("64-bit unsigned integer output: "+HString(name));
                    mPorts.push_back(addOutputVariable(toValidHopsanVarName(name), description, "", &mUInt64Outputs[vr]));
                    mVisibleOutputs.append(toValidHopsanVarName(name)+",");
                }
                else if(causality == fmi3CausalityOutput && (type == fmi3DataTypeUInt32)) {
                    addDebugMessage("32-bit unsigned integer output: "+HString(name));
                    mPorts.push_back(addOutputVariable(toValidHopsanVarName(name), description, "", &mUInt32Outputs[vr]));
                    mVisibleOutputs.append(toValidHopsanVarName(name)+",");
                }
                else if(causality == fmi3CausalityOutput && (type == fmi3DataTypeUInt16)) {
                    addDebugMessage("16-bit unsigned integer output: "+HString(name));
                    mPorts.push_back(addOutputVariable(toValidHopsanVarName(name), description, "", &mUInt16Outputs[vr]));
                    mVisibleOutputs.append(toValidHopsanVarName(name)+",");
                }
                else if(causality == fmi3CausalityOutput && (type == fmi3DataTypeUInt8)) {
                    addDebugMessage("8-bit unsigned integer output: "+HString(name));
                    mPorts.push_back(addOutputVariable(toValidHopsanVarName(name), description, "", &mUInt8Outputs[vr]));
                    mVisibleOutputs.append(toValidHopsanVarName(name)+",");
                }
                else if(causality == fmi3CausalityOutput && (type == fmi3DataTypeBoolean)) {
                    addDebugMessage("Boolean output: "+HString(name));
                    mPorts.push_back(addOutputVariable(toValidHopsanVarName(name), description, "", &mBoolOutputs[vr]));
                    mVisibleOutputs.append(toValidHopsanVarName(name)+",");
                }
                else if(causality == fmi3CausalityLocal && type == fmi3DataTypeFloat64 && !usedVariables.contains(name)) {
                    addDebugMessage("64-bit float local: "+HString(name));
                    mPorts.push_back(addOutputVariable(toValidHopsanVarName(name), description, "", &mRealOutputs[vr]));
                }
                else if(causality == fmi3CausalityLocal && type == fmi3DataTypeFloat32) {
                    addDebugMessage("32-bit float local: "+HString(name));
                    mPorts.push_back(addOutputVariable(toValidHopsanVarName(name), description, "", &mFloat32Outputs[vr]));
                }
                else if(causality == fmi3CausalityLocal && (type == fmi3DataTypeInt64)) {
                    addDebugMessage("64-bit integer local: "+HString(name));
                    mPorts.push_back(addOutputVariable(toValidHopsanVarName(name), description, "", &mInt64Outputs[vr]));
                }
                else if(causality == fmi3CausalityLocal && (type == fmi3DataTypeInt32)) {
                    addDebugMessage("32-bit integer local: "+HString(name));
                    mPorts.push_back(addOutputVariable(toValidHopsanVarName(name), description, "", &mInt32Outputs[vr]));
                }
                else if(causality == fmi3CausalityLocal && (type == fmi3DataTypeInt16)) {
                    addDebugMessage("16-bit integer local: "+HString(name));
                    mPorts.push_back(addOutputVariable(toValidHopsanVarName(name), description, "", &mInt16Outputs[vr]));
                }
                else if(causality == fmi3CausalityLocal && (type == fmi3DataTypeInt8)) {
                    addDebugMessage("8-bit integer local: "+HString(name));
                    mPorts.push_back(addOutputVariable(toValidHopsanVarName(name), description, "", &mInt8Outputs[vr]));
                }
                else if(causality == fmi3CausalityLocal && (type == fmi3DataTypeUInt64)) {
                    addDebugMessage("64-bit unsigned integer local: "+HString(name));
                    mPorts.push_back(addOutputVariable(toValidHopsanVarName(name), description, "", &mUInt64Outputs[vr]));
                }
                else if(causality == fmi3CausalityLocal && (type == fmi3DataTypeUInt32)) {
                    addDebugMessage("32-bit unsigned integer local: "+HString(name));
                    mPorts.push_back(addOutputVariable(toValidHopsanVarName(name), description, "", &mUInt32Outputs[vr]));
                }
                else if(causality == fmi3CausalityLocal && (type == fmi3DataTypeUInt16)) {
                    addDebugMessage("16-bit unsigned integer local: "+HString(name));
                    mPorts.push_back(addOutputVariable(toValidHopsanVarName(name), description, "", &mUInt16Outputs[vr]));
                }
                else if(causality == fmi3CausalityLocal && (type == fmi3DataTypeUInt8)) {
                    addDebugMessage("8-bit unsigned integer local: "+HString(name));
                    mPorts.push_back(addOutputVariable(toValidHopsanVarName(name), description, "", &mUInt8Outputs[vr]));
                }

            }
            if(!mVisibleOutputs.empty() && mVisibleOutputs.back() == ',') {
                mVisibleOutputs.erase(mVisibleOutputs.size()-1,1);  //Remove trailing comma
            }
            addConstant("visibleOutputs", "Visible output variables (hidden)", "", mVisibleOutputs, mVisibleOutputs);

            //Instantiate FMU
<<<<<<< HEAD
            if(!mReinstantiate) {
                addDebugMessage("Calling: fmi3InstantiateCoSimulation");
                size_t nRequiredIntermediateVariables = 0;
                if(!fmi3_instantiateCoSimulation(fmu, fmi3False, mLoggingOn, fmi3False, fmi3False, NULL, nRequiredIntermediateVariables, this, FMIWrapperQ_fmi3Logger, FMIWrapperQ_fmi3IntermediateUpdate)) {
                    stopSimulation("Failed to instantiate FMU");
                    fmu = NULL;
                    return;
                }
                mIsInstantiated = true;
=======
            size_t nRequiredIntermediateVariables = 0;
            fmi3_instance = fmi3_instantiateCoSimulation(fmu, fmi3False, mLoggingOn, fmi3False, fmi3False, NULL, nRequiredIntermediateVariables, this, FMIWrapperQ_fmi3Logger, FMIWrapperQ_fmi3IntermediateUpdate);

            if (NULL == fmi3_instance) {
                stopSimulation("Failed to instantiate FMU");
                return;
>>>>>>> 9529d1b70 (Upgrade fmi4c to v1.1)
            }
        }
    }

    void initialize()
    {
        if(mFmiVersion == fmiVersion1) {
            if(fmu == NULL) {
                stopSimulation("No FMU file loaded.");
                return;
            }

            if(!mIsInstantiated) {
                addDebugMessage("Calling: fmi1InstantiateSlave");
                if(!fmi1_instantiateSlave(fmu, "application/x-fmu-sharedlibrary", 1000, fmi1False, fmi1False, FMIWrapperQ_fmi1Logger, calloc, free, NULL, mLoggingOn)) {
                    addErrorMessage("Hopsan: fmi1InstantiateSlave() failed!");
                    fmu = NULL;
                    return;
                }
                mIsInstantiated = true;
            }

            //Assign node data pointers
            HVector<HString> ports = mPortSpecs.split(';');
            for(size_t i=0; i<ports.size(); ++i) {
                HVector<HString> specs = ports[i].split(',');
                HString nodeType = specs[0];
                Port *pPort = mPorts[i];
                fmi2ValueReference vr;
                Node *pNode = mPorts[i]->getNodePtr();
                for(size_t j=0; j<specs.size()-1; ++j) {
                    fmi1VariableHandle *var = fmi1_getVariableByName(fmu, specs[j+1].c_str());
                    if(nullptr == var) {
                        stopSimulation("Did not find variable named \""+specs[j+1]+", check your port specifications.");
                        return;
                    }
                    vr = (fmi1ValueReference)fmi1_getVariableValueReference(var);
                    if(pNode->getDataDescription(j)->varType == TLMType) {
                        mRealInputs[vr] = getSafeNodeDataPtr(pPort, (int)j);
                    }
                    else {
                        mRealOutputs[vr] = getSafeNodeDataPtr(pPort, (int)j);
                    }
                }
            }

            //Loop through output variables and assign start values
            for(int i=0; i<fmi1_getNumberOfVariables(fmu); ++i) {
                fmi1VariableHandle *var = fmi1_getVariableByIndex(fmu,i+1);

                fmi1DataType type = fmi1_getVariableDataType(var);
                fmi1Causality causality = fmi1_getVariableCausality(var);
                fmi1ValueReference vr = (fmi1ValueReference)fmi1_getVariableValueReference(var);

                if(causality == fmi1CausalityOutput && type == fmi1DataTypeReal) {
                    (*mRealOutputs[vr]) = fmi1_getVariableStartReal(var);
                }
                else if(causality == fmi1CausalityOutput && type == fmi1DataTypeInteger) {
                    (*mIntOutputs[vr]) = fmi1_getVariableStartInteger(var);
                }
                else if(causality == fmi1CausalityOutput && type == fmi1DataTypeBoolean) {
                    (*mBoolOutputs[vr]) = fmi1_getVariableStartBoolean(var);
                }
            }

            fmi1Status status;

            for(std::map<fmi1ValueReference,double>::iterator it = mRealParameters.begin(); it != mRealParameters.end(); it++) {
                fmi1Real value = (fmi1Real)it->second;
                status = fmi1_setReal(fmi1_instance, &it->first, 1, &value);
            }
            for(std::map<fmi1ValueReference,HString>::iterator it = mStringParameters.begin(); it != mStringParameters.end(); ++it) {
                fmi1String value = it->second.c_str();
                status = fmi1_setString(fmi1_instance, &it->first, 1, &value);
            }
            for(std::map<fmi1ValueReference,bool>::iterator it = mBoolParameters.begin(); it != mBoolParameters.end(); ++it) {
                fmi1Boolean value = (fmi1Boolean)it->second;
                status = fmi1_setBoolean(fmi1_instance, &it->first, 1, &value);
            }
            for(std::map<fmi1ValueReference,int>::iterator it = mIntParameters.begin(); it != mIntParameters.end(); ++it) {
                fmi1Integer value = (fmi1Integer)it->second;
                status = fmi1_setInteger(fmi1_instance, &it->first, 1, &value);
            }

            //Enter initialization mode
<<<<<<< HEAD
            addDebugMessage("Calling: fmi1InitializeSlave");
            status = fmi1_initializeSlave(fmu,mTime,fmi1False,0);
=======
            status = fmi1_initializeSlave(fmi1_instance,mTime,fmi1False,0);
>>>>>>> 9529d1b70 (Upgrade fmi4c to v1.1)
            if(status != fmi1OK) {
                stopSimulation("fmi1InitializeSlave() failed");
                return;
            }
        }
        else if(mFmiVersion == fmiVersion2) {
            if(fmi2_instance == NULL) {
                stopSimulation("No FMU file loaded.");
                return;
            }

            if(!mIsInstantiated) {
                addDebugMessage("Calling: fmi2Instantiate");
                if(!fmi2_instantiate(fmu, fmi2CoSimulation, FMIWrapperQ_fmi2Logger, calloc, free, NULL, (fmi2ComponentEnvironment*)this, fmi2False, mLoggingOn)) {
                    stopSimulation("Failed to instantiate FMU");
                    fmu = NULL;
                    return;
                }
                mIsInstantiated = true;
            }

            //Assign node data pointers
            HVector<HString> ports = mPortSpecs.split(';');
            for(size_t i=0; i<ports.size(); ++i) {
                HVector<HString> specs = ports[i].split(',');
                HString nodeType = specs[0];
                Port *pPort = mPorts[i];
                fmi2ValueReference vr;
                Node *pNode = mPorts[i]->getNodePtr();
                for(size_t j=0; j<specs.size()-1; ++j) {
                    fmi2VariableHandle *var = fmi2_getVariableByName(fmu, specs[j+1].c_str());
                    if(nullptr == var) {
                        stopSimulation("Did not find variable named \""+specs[j+1]+", check your port specifications.");
                        return;
                    }
                    vr = (fmi2ValueReference)fmi2_getVariableValueReference(var);
                    if(pNode->getDataDescription(j)->varType == TLMType) {
                        mRealInputs[vr] = getSafeNodeDataPtr(pPort, (int)j);
                    }
                    else {
                        mRealOutputs[vr] = getSafeNodeDataPtr(pPort, (int)j);
                    }
                }
            }

            //Loop through output variables and assign start values
            for(int i=0; i<fmi2_getNumberOfVariables(fmu); ++i) {
                fmi2VariableHandle *var = fmi2_getVariableByIndex(fmu,i+1);

                fmi2DataType type = fmi2_getVariableDataType(var);
                fmi2Causality causality = fmi2_getVariableCausality(var);
                fmi2ValueReference vr = (fmi2ValueReference)fmi2_getVariableValueReference(var);

                if(causality == fmi2CausalityOutput && type == fmi2DataTypeReal) {
                    (*mRealOutputs[vr]) = fmi2_getVariableStartReal(var);
                }
                else if(causality == fmi2CausalityOutput && type == fmi2DataTypeInteger) {
                    (*mIntOutputs[vr]) = fmi2_getVariableStartInteger(var);
                }
                else if(causality == fmi2CausalityOutput && type == fmi2DataTypeBoolean) {
                    (*mBoolOutputs[vr]) = fmi2_getVariableStartBoolean(var);
                }
            }

            fmi2Status status;

            for(std::map<fmi2ValueReference,double>::iterator it = mRealParameters.begin(); it != mRealParameters.end(); it++) {
                fmi2Real value = (fmi2Real)it->second;
                status = fmi2_setReal(fmi2_instance, &it->first, 1, &value);
            }
            for(std::map<fmi2ValueReference,HString>::iterator it = mStringParameters.begin(); it != mStringParameters.end(); ++it) {
                fmi2String value = it->second.c_str();
                status = fmi2_setString(fmi2_instance, &it->first, 1, &value);
            }
            for(std::map<fmi2ValueReference,bool>::iterator it = mBoolParameters.begin(); it != mBoolParameters.end(); ++it) {
                fmi2Boolean value = (fmi2Boolean)it->second;
                status = fmi2_setBoolean(fmi2_instance, &it->first, 1, &value);
            }
            for(std::map<fmi2ValueReference,int>::iterator it = mIntParameters.begin(); it != mIntParameters.end(); ++it) {
                fmi2Integer value = (fmi2Integer)it->second;
                status = fmi2_setInteger(fmi2_instance, &it->first, 1, &value);
            }

            //Setup experiment
<<<<<<< HEAD
            addDebugMessage("Calling: fmi2SetupExperiment");
            status = fmi2_setupExperiment(fmu, fmi2True, mTolerance, mTime, fmi2False, 0.0);
=======
            status = fmi2_setupExperiment(fmi2_instance, fmi2True, mTolerance, mTime, fmi2False, 0.0);
>>>>>>> 9529d1b70 (Upgrade fmi4c to v1.1)
            if(status != fmi2OK) {
                stopSimulation("fmi2_setupExperiment() failed");
                return;
            }

            //Enter initialization mode
<<<<<<< HEAD
            addDebugMessage("Calling: fmi2_enterInitializationMode");
            status = fmi2_enterInitializationMode(fmu);
=======
            status = fmi2_enterInitializationMode(fmi2_instance);
>>>>>>> 9529d1b70 (Upgrade fmi4c to v1.1)
            if(status != fmi2OK) {
                stopSimulation("fmi2EnterInitializationMode() failed");
                return;
            }

            //Exit initialization mode
<<<<<<< HEAD
            addDebugMessage("Calling: fmi2_exitInitializationMode");
            status = fmi2_exitInitializationMode(fmu);
=======
            status = fmi2_exitInitializationMode(fmi2_instance);
>>>>>>> 9529d1b70 (Upgrade fmi4c to v1.1)
            if(status != fmi2OK) {
                stopSimulation("fmi3ExitInitializationMode() failed");
                return;
            }
        }
        else {
            if(fmi3_instance == NULL) {
                stopSimulation("No FMU file loaded.");
                return;
            }

            if(!mIsInstantiated) {
                addDebugMessage("Calling: fmi3InstantiateCoSimulation");
                size_t nRequiredIntermediateVariables = 0;
                if(!fmi3_instantiateCoSimulation(fmu, fmi3False, mLoggingOn, fmi3False, fmi3False, NULL, nRequiredIntermediateVariables, this, FMIWrapperQ_fmi3Logger, FMIWrapperQ_fmi3IntermediateUpdate)) {
                    stopSimulation("Failed to instantiate FMU");
                    fmu = NULL;
                    return;
                }
                mIsInstantiated = true;
            }

            HVector<HString> ports = mPortSpecs.split(';');
            for(size_t i=0; i<ports.size(); ++i) {
                HVector<HString> specs = ports[i].split(',');
                HString nodeType = specs[0];
                Port *pPort = mPorts[i];
                fmi2ValueReference vr;
                Node *pNode = mPorts[i]->getNodePtr();
                for(size_t j=0; j<specs.size()-1; ++j) {
                    fmi3VariableHandle *var = fmi3_getVariableByName(fmu, specs[j+1].c_str());
                    if(nullptr == var) {
                        stopSimulation("Did not find variable named \""+specs[j+1]+", check your port specifications.");
                        return;
                    }
                    vr = (fmi3ValueReference)fmi3_getVariableValueReference(var);
                    if(pNode->getDataDescription(j)->varType == TLMType) {
                        mFloat64Inputs[vr] = getSafeNodeDataPtr(pPort, (int)j);
                    }
                    else {
                        mFloat64Outputs[vr] = getSafeNodeDataPtr(pPort, (int)j);
                    }
                }
            }

            //Loop through output variables and assign start values
            for(int i=0; i<fmi3_getNumberOfVariables(fmu); ++i) {
                fmi3VariableHandle *var = fmi3_getVariableByIndex(fmu,i+1);

                fmi3DataType type = fmi3_getVariableDataType(var);
                fmi3Causality causality = fmi3_getVariableCausality(var);
                fmi3ValueReference vr = fmi3_getVariableValueReference(var);

                if(causality == fmi3CausalityOutput && type == fmi3DataTypeFloat64) {
                    (*mFloat64Outputs[vr]) = fmi3_getVariableStartFloat64(var);
                }
                else if(causality == fmi3CausalityOutput && type == fmi3DataTypeFloat32) {
                    (*mFloat32Outputs[vr]) = (double)fmi3_getVariableStartFloat32(var);
                }
                else if(causality == fmi3CausalityOutput && type == fmi3DataTypeInt64) {
                    (*mInt64Outputs[vr]) = (double)fmi3_getVariableStartInt64(var);
                }
                else if(causality == fmi3CausalityOutput && type == fmi3DataTypeInt32) {
                    (*mInt32Outputs[vr]) = (double)fmi3_getVariableStartInt32(var);
                }
                else if(causality == fmi3CausalityOutput && type == fmi3DataTypeInt16) {
                    (*mInt16Outputs[vr]) = (double)fmi3_getVariableStartInt16(var);
                }
                else if(causality == fmi3CausalityOutput && type == fmi3DataTypeInt8) {
                    (*mInt8Outputs[vr]) = (double)fmi3_getVariableStartInt8(var);
                }
                else if(causality == fmi3CausalityOutput && type == fmi3DataTypeUInt64) {
                    (*mUInt64Outputs[vr]) = (double)fmi3_getVariableStartUInt64(var);
                }
                else if(causality == fmi3CausalityOutput && type == fmi3DataTypeUInt32) {
                    (*mUInt32Outputs[vr]) = (double)fmi3_getVariableStartUInt32(var);
                }
                else if(causality == fmi3CausalityOutput && type == fmi3DataTypeUInt16) {
                    (*mUInt16Outputs[vr]) = (double)fmi3_getVariableStartUInt16(var);
                }
                else if(causality == fmi3CausalityOutput && type == fmi3DataTypeUInt8) {
                    (*mUInt8Outputs[vr]) = (double)fmi3_getVariableStartUInt8(var);
                }
                else if(causality == fmi3CausalityOutput && type == fmi3DataTypeBoolean) {
                    (*mBoolOutputs[vr]) = fmi3_getVariableStartBoolean(var);
                }
            }

            fmi3Status status;

            for(std::map<fmi3ValueReference,double>::iterator it = mFloat64Parameters.begin(); it != mFloat64Parameters.end(); it++) {
                fmi3Float64 value = (fmi3Float64)it->second;
                status = fmi3_setFloat64(fmi3_instance, &it->first, 1, &value, 1);
            }
            for(std::map<fmi3ValueReference,double>::iterator it = mFloat32Parameters.begin(); it != mFloat32Parameters.end(); it++) {
                fmi3Float32 value = (fmi3Float32)it->second;
                status = fmi3_setFloat32(fmi3_instance, &it->first, 1, &value, 1);
            }
            for(std::map<fmi3ValueReference,HString>::iterator it = mStringParameters.begin(); it != mStringParameters.end(); ++it) {
                fmi3String value = it->second.c_str();
                status = fmi3_setString(fmi3_instance, &it->first, 1, &value, 1);
            }
            for(std::map<fmi3ValueReference,bool>::iterator it = mBoolParameters.begin(); it != mBoolParameters.end(); ++it) {
                fmi3Boolean value = it->second ? 1.0 : 0.0;
                status = fmi3_setBoolean(fmi3_instance, &it->first, 1, &value, 1);
            }
            for(std::map<fmi3ValueReference,int>::iterator it = mInt64Parameters.begin(); it != mInt64Parameters.end(); ++it) {
                fmi3Int64 value = (fmi3Int64)it->second;
                status = fmi3_setInt64(fmi3_instance, &it->first, 1, &value, 1);
            }
            for(std::map<fmi3ValueReference,int>::iterator it = mInt32Parameters.begin(); it != mInt32Parameters.end(); ++it) {
                fmi3Int32 value = (fmi3Int32)it->second;
                status = fmi3_setInt32(fmi3_instance, &it->first, 1, &value, 1);
            }
            for(std::map<fmi3ValueReference,int>::iterator it = mInt16Parameters.begin(); it != mInt16Parameters.end(); ++it) {
                fmi3Int16 value = (fmi3Int16)it->second;
                status = fmi3_setInt16(fmi3_instance, &it->first, 1, &value, 1);
            }
            for(std::map<fmi3ValueReference,int>::iterator it = mInt8Parameters.begin(); it != mInt8Parameters.end(); ++it) {
                fmi3Int8 value = (fmi3Int8)it->second;
                status = fmi3_setInt8(fmi3_instance, &it->first, 1, &value, 1);
            }
            for(std::map<fmi3ValueReference,int>::iterator it = mUInt64Parameters.begin(); it != mUInt64Parameters.end(); ++it) {
                fmi3UInt64 value = (fmi3UInt64)it->second;
                status = fmi3_setUInt64(fmi3_instance, &it->first, 1, &value, 1);
            }
            for(std::map<fmi3ValueReference,int>::iterator it = mUInt32Parameters.begin(); it != mUInt32Parameters.end(); ++it) {
                fmi3UInt32 value = (fmi3UInt32)it->second;
                status = fmi3_setUInt32(fmi3_instance, &it->first, 1, &value, 1);
            }
            for(std::map<fmi3ValueReference,int>::iterator it = mUInt16Parameters.begin(); it != mUInt16Parameters.end(); ++it) {
                fmi3UInt16 value = (fmi3UInt16)it->second;
                status = fmi3_setUInt16(fmi3_instance, &it->first, 1, &value, 1);
            }
            for(std::map<fmi3ValueReference,int>::iterator it = mUInt8Parameters.begin(); it != mUInt8Parameters.end(); ++it) {
                fmi3UInt8 value = (fmi3UInt8)it->second;
                status = fmi3_setUInt8(fmi3_instance, &it->first, 1, &value, 1);
            }

            //Enter initialization mode
            double tstop = 10;
<<<<<<< HEAD
            addDebugMessage("Calling: fmi3EnterInitializationMode");
            status = fmi3_enterInitializationMode(fmu, fmi3False, 0, mTime+mTimestep, fmi3True, tstop);
=======
            status = fmi3_enterInitializationMode(fmi3_instance, fmi3False, 0, mTime+mTimestep, fmi3True, tstop);
>>>>>>> 9529d1b70 (Upgrade fmi4c to v1.1)
            if(status != fmi3OK) {
                stopSimulation("fmi3EnterInitializationMode() failed");
                return;
            }

<<<<<<< HEAD
            //Exit initialization mode'
            addDebugMessage("Calling: fmi3ExitInitializationMode");
            status = fmi3_exitInitializationMode(fmu);
=======
            //Exit initialization mode
            status = fmi3_exitInitializationMode(fmi3_instance);
>>>>>>> 9529d1b70 (Upgrade fmi4c to v1.1)
            if(status != fmi3OK) {
                stopSimulation("fmi3ExitInitializationMode() failed");
                return;
            }
        }
    }

    void simulateOneTimestep()
    {
        if(mFmiVersion == fmiVersion1) {
            if(NULL == fmu) {
                return;
            }

            fmi1Status status;

            //Forward inputs
            std::map<fmi1ValueReference,double*>::iterator it;
            for(it = mRealInputs.begin(); it != mRealInputs.end(); it++) {
                status = fmi1_setReal(fmi1_instance, &it->first, 1, it->second);
            }
            for(it = mIntInputs.begin(); it != mIntInputs.end(); it++) {
                int value = (int)lround(*it->second);
                status = fmi1_setInteger(fmi1_instance, &it->first, 1, &value);
            }
            for(it = mBoolInputs.begin(); it != mBoolInputs.end(); it++) {
                fmi1Boolean value = fmi1Boolean(*it->second);
                status = fmi1_setBoolean(fmi1_instance, &it->first, 1, &value);
            }

            //Take step
            status = fmi1_doStep(fmi1_instance, mTime-mTimestep, mTimestep, fmi1True);
            if (status != fmi1OK) {
                stopSimulation("fmi1DoStep() failed, status = "+to_hstring(status));
                return;
            }

            //Forward outputs
            for(it = mRealOutputs.begin(); it != mRealOutputs.end(); it++) {
                status = fmi1_getReal(fmi1_instance, &it->first, 1, it->second);
            }
            for(it = mIntOutputs.begin(); it != mIntOutputs.end(); it++) {
                int temp;
                status = fmi1_getInteger(fmi1_instance, &it->first, 1, &temp);
                (*it->second) = temp;
            }
            for(it = mBoolOutputs.begin(); it != mBoolOutputs.end(); it++) {
                fmi1Boolean temp;
                status = fmi1_getBoolean(fmi1_instance, &it->first, 1, &temp);
                (*it->second) = temp;
            }
        }
        else if(mFmiVersion == fmiVersion2) {
            if(NULL == fmu) {
                return;
            }

            fmi2Status status;

            //Forward inputs
            std::map<fmi2ValueReference,double*>::iterator it;
            for(it = mRealInputs.begin(); it != mRealInputs.end(); it++) {
                status = fmi2_setReal(fmi2_instance, &it->first, 1, it->second);
            }
            for(it = mIntInputs.begin(); it != mIntInputs.end(); it++) {
                int value = (int)lround(*it->second);
                status = fmi2_setInteger(fmi2_instance, &it->first, 1, &value);
            }
            for(it = mBoolInputs.begin(); it != mBoolInputs.end(); it++) {
                int value = int(*it->second);
                status = fmi2_setBoolean(fmi2_instance, &it->first, 1, &value);
            }

            //Take step
            status = fmi2_doStep(fmi2_instance, mTime-mTimestep, mTimestep, fmi3True);
            if (status != fmi2OK) {
                stopSimulation("fmi2DoStep() failed, status = "+to_hstring(status));
                return;
            }

            //Forward outputs
            for(it = mRealOutputs.begin(); it != mRealOutputs.end(); it++) {
                status = fmi2_getReal(fmi2_instance, &it->first, 1, it->second);
            }
            for(it = mIntOutputs.begin(); it != mIntOutputs.end(); it++) {
                int temp;
                status = fmi2_getInteger(fmi2_instance, &it->first, 1, &temp);
                (*it->second) = temp;
            }
            for(it = mBoolOutputs.begin(); it != mBoolOutputs.end(); it++) {
                int temp;
                status = fmi2_getBoolean(fmi2_instance, &it->first, 1, &temp);
                (*it->second) = temp;
            }
        }
        else { //FMI 3
            if(NULL == fmu) {
                return;
            }
            fmi3Status status;

            //Forward inputs
            std::map<fmi3ValueReference,double*>::iterator itr;
            for(itr = mFloat64Inputs.begin(); itr != mFloat64Inputs.end(); itr++) {
                fmi3Float64 value = (fmi3Float64)(*itr->second);
                status = fmi3_setFloat64(fmi3_instance, &itr->first, 1, &value, 1);
            }
            for(itr = mFloat32Inputs.begin(); itr != mFloat32Inputs.end(); itr++) {
                fmi3Float32 value = (fmi3Float32)(*itr->second);
                status = fmi3_setFloat32(fmi3_instance, &itr->first, 1, &value, 1);
            }
            std::map<fmi3ValueReference,double*>::iterator itb;
            for(itb = mBoolInputs.begin(); itb != mBoolInputs.end(); ++itb) {
                bool value = ((*itb->second) > 0.5);
                status = fmi3_setBoolean(fmi3_instance, &itb->first, 1, &value, 1);
            }
            std::map<fmi3ValueReference,double*>::iterator iti;
            for(iti = mInt64Inputs.begin(); iti != mInt64Inputs.end(); ++iti) {
                fmi3Int64 value = (fmi3Int64)lround(*iti->second);
                status = fmi3_setInt64(fmi3_instance, &iti->first, 1, &value, 1);
            }
            for(iti = mInt32Inputs.begin(); iti != mInt32Inputs.end(); ++iti) {
                fmi3Int32 value = (fmi3Int32)lround(*iti->second);
                status = fmi3_setInt32(fmi3_instance, &iti->first, 1, &value, 1);
            }
            for(iti = mInt16Inputs.begin(); iti != mInt16Inputs.end(); ++iti) {
                fmi3Int16 value = (fmi3Int16)lround(*iti->second);
                status = fmi3_setInt16(fmi3_instance, &iti->first, 1, &value, 1);
            }
            for(iti = mInt8Inputs.begin(); iti != mInt8Inputs.end(); ++iti) {
                fmi3Int8 value = (fmi3Int8)lround(*iti->second);
                status = fmi3_setInt8(fmi3_instance, &iti->first, 1, &value, 1);
            }
            for(iti = mUInt64Inputs.begin(); iti != mUInt64Inputs.end(); ++iti) {
                fmi3UInt64 value = (fmi3UInt64)lround(*iti->second);
                status = fmi3_setUInt64(fmi3_instance, &iti->first, 1, &value, 1);
            }
            for(iti = mUInt32Inputs.begin(); iti != mUInt32Inputs.end(); ++iti) {
                fmi3UInt32 value = (fmi3UInt32)lround(*iti->second);
                status = fmi3_setUInt32(fmi3_instance, &iti->first, 1, &value, 1);
            }
            for(iti = mUInt16Inputs.begin(); iti != mUInt16Inputs.end(); ++iti) {
                fmi3UInt16 value = (fmi3UInt16)lround(*iti->second);
                status = fmi3_setUInt16(fmi3_instance, &iti->first, 1, &value, 1);
            }
            for(iti = mUInt8Inputs.begin(); iti != mUInt8Inputs.end(); ++iti) {
                fmi3UInt8 value = (fmi3UInt8)lround(*iti->second);
                status = fmi3_setUInt8(fmi3_instance, &iti->first, 1, &value, 1);
            }


            //Take step
            bool eventEncountered, terminateSimulation, earlyReturn;
            double lastT;
            status = fmi3_doStep(fmi3_instance, mTime, mTimestep, fmi3True, &eventEncountered, &terminateSimulation, &earlyReturn, &lastT);
            if (status != fmi3OK) {
                stopSimulation("fmi3DoStep() failed, status = "+to_hstring(status));
                return;
            }

            //Forward outputs
            std::map<fmi3ValueReference,double*>::iterator it;
            for(it = mFloat64Outputs.begin(); it != mFloat64Outputs.end(); it++) {
                fmi3Float64 value;
                status = fmi3_getFloat64(fmi3_instance, &it->first, 1, &value, 1);
                (*it->second) = (double)value;
            }
            for(it = mFloat32Outputs.begin(); it != mFloat32Outputs.end(); it++) {
                fmi3Float32 value;
                status = fmi3_getFloat32(fmi3_instance, &it->first, 1, &value, 1);
                (*it->second) = (double)value;
            }
            for(it = mInt64Outputs.begin(); it != mInt64Outputs.end(); it++) {
                fmi3Int64 value;
                status = fmi3_getInt64(fmi3_instance, &it->first, 1, &value, 1);
                (*it->second) = (double)value;
            }
            for(it = mInt32Outputs.begin(); it != mInt32Outputs.end(); it++) {
                fmi3Int32 value;
                status = fmi3_getInt32(fmi3_instance, &it->first, 1, &value, 1);
                (*it->second) = (double)value;
            }
            for(it = mInt16Outputs.begin(); it != mInt16Outputs.end(); it++) {
                fmi3Int16 value;
                status = fmi3_getInt16(fmi3_instance, &it->first, 1, &value, 1);
                (*it->second) = (double)value;
            }
            for(it = mInt8Outputs.begin(); it != mInt8Outputs.end(); it++) {
                fmi3Int8 value;
                status = fmi3_getInt8(fmi3_instance, &it->first, 1, &value, 1);
                (*it->second) = (double)value;
            }
            for(it = mUInt64Outputs.begin(); it != mUInt64Outputs.end(); it++) {
                fmi3UInt64 value;
                status = fmi3_getUInt64(fmi3_instance, &it->first, 1, &value, 1);
                (*it->second) = (double)value;
            }
            for(it = mUInt32Outputs.begin(); it != mUInt32Outputs.end(); it++) {
                fmi3UInt32 value;
                status = fmi3_getUInt32(fmi3_instance, &it->first, 1, &value, 1);
                (*it->second) = (double)value;
            }
            for(it = mUInt16Outputs.begin(); it != mUInt16Outputs.end(); it++) {
                fmi3UInt16 value;
                status = fmi3_getUInt16(fmi3_instance, &it->first, 1, &value, 1);
                (*it->second) = (double)value;
            }
            for(it = mUInt8Outputs.begin(); it != mUInt8Outputs.end(); it++) {
                fmi3UInt8 value;
                status = fmi3_getUInt8(fmi3_instance, &it->first, 1, &value, 1);
                (*it->second) = (double)value;
            }
            for(it = mBoolOutputs.begin(); it != mBoolOutputs.end(); it++) {
                bool temp;
                status = fmi3_getBoolean(fmi3_instance, &it->first, 1, &temp, 1);
                (*it->second) = temp;
            }
        }
    }

    void finalize()
    {
        if(mFmiVersion == fmiVersion1) {
            if(fmi1_instance == NULL) {
                return;
            }
<<<<<<< HEAD
            if(mReinstantiate)
            {
                addDebugMessage("Calling: fmi1Terminate");
                fmi1_terminate(fmu);
                addDebugMessage("Calling: fmi1FreeSlaveInstance");
                mIsInstantiated = false;
                fmi1_freeSlaveInstance(fmu);
            }
            else {
                addDebugMessage("Calling: fmi1Reset");
                fmi1_resetSlave(fmu);
            }
=======
            fmi1_resetSlave(fmi1_instance);
>>>>>>> 9529d1b70 (Upgrade fmi4c to v1.1)
        }
        else if(mFmiVersion == fmiVersion2) {
            if(fmi2_instance == NULL) {
                return;
            }
<<<<<<< HEAD
            if(mReinstantiate) {
                addDebugMessage("Calling: fmi2Terminate");
                fmi2_terminate(fmu);
                addDebugMessage("Calling: fmi2FreeInstance");
                mIsInstantiated = false;
                fmi2_freeInstance(fmu);
            }
            else {
                addDebugMessage("Calling: fmi2Reset");
                fmi2_reset(fmu);
            }

=======
            fmi2_reset(fmi2_instance);
>>>>>>> 9529d1b70 (Upgrade fmi4c to v1.1)
        }
        else {
            if(fmi3_instance == NULL) {
                return;
            }
<<<<<<< HEAD
            if(mReinstantiate) {
                addDebugMessage("Calling: fmi3Terminate");
                fmi3_terminate(fmu);
                addDebugMessage("Calling: fmi3FreeInstance");
                mIsInstantiated = false;
                fmi3_freeInstance(fmu);
            }
            else {
                addDebugMessage("Calling: fmi3Reset");
                fmi3_reset(fmu);
            }

=======
            fmi3_reset(fmi3_instance);
>>>>>>> 9529d1b70 (Upgrade fmi4c to v1.1)
        }
    }

    void deconfigure()
    {
        if(mFmiVersion == fmiVersion1) {
            if(NULL == fmu) {
                return;
            }
<<<<<<< HEAD
            addDebugMessage("Calling: fmi3FreeSlaveInstance");
            mIsInstantiated = false;
            fmi1_freeSlaveInstance(fmu);
=======
            fmi1_freeSlaveInstance(fmi1_instance);
>>>>>>> 9529d1b70 (Upgrade fmi4c to v1.1)
            fmi4c_freeFmu(fmu);
            fmi1_instance = NULL;
            fmu = NULL;
        }
        else if(mFmiVersion == fmiVersion2) {
            if(NULL == fmu) {
                return;
            }
<<<<<<< HEAD
            addDebugMessage("Calling: fmi2FreeInstance");
            mIsInstantiated = false;
            fmi2_freeInstance(fmu);
=======
            fmi2_freeInstance(fmi2_instance);
>>>>>>> 9529d1b70 (Upgrade fmi4c to v1.1)
            fmi4c_freeFmu(fmu);
            fmi2_instance = NULL;
            fmu = NULL;
        }
        else {
            if(NULL == fmu) {
                return;
            }
<<<<<<< HEAD
            addDebugMessage("Calling: fmi3FreeInstance");
            mIsInstantiated = false;
            fmi3_freeInstance(fmu);
=======
            fmi3_freeInstance(fmi3_instance);
>>>>>>> 9529d1b70 (Upgrade fmi4c to v1.1)
            fmi4c_freeFmu(fmu);
            fmi3_instance = NULL;
            fmu = NULL;
        }
    }


    //! @brief Replaces all illegal characters in the string with underscores, so that it can be used as a variable name.
    //! @param [in] rName Input string
    //! @returns Input string with illegal characters replaced with underscore
    //! @todo Check if variable/parameter exist in model already and append number if so
    HString toValidHopsanVarName(const HString &rName)
    {
        HString ret = rName;
        for (size_t i=0; i<ret.size(); ++i) {
            if (!(((ret[i] >= LOWERCASE_LOW) && (ret[i] <= LOWERCASE_HIGH)) ||
                  ((ret[i] >= UPPERCASE_LOW) && (ret[i] <= UPPERCASE_HIGH)) ||
                  ((ret[i] >= NUMBERS_LOW)   && (ret[i] <= NUMBERS_HIGH))   ||
                  (ret[i] == UNDERSCORE)))
            {
                ret[i] = '_';
            }
        }
        return ret;
    }
};

#endif
}

#endif // FMIWRAPPERQ_HPP
