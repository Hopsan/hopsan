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

//!
//! @file   Component.h
//! @author FluMeS
//! @date   2009-12-20
//!
//! @brief Contains Component base classes
//!
//$Id$

#ifndef COMPONENT_H_INCLUDED
#define COMPONENT_H_INCLUDED

#include "Node.h"
#include "Port.h"
#include "Parameters.h"
#include "win32dll.h"
#include <map>
#include <list>
#include <algorithm>

namespace hopsan {

#pragma pack(push, 1)
typedef struct HopsanExternalLibInfo
{
    char* hopsanCoreVersion;
    char* libCompiledDebugRelease;
    char* libName;
}HopsanExternalLibInfoT;
#pragma pack(pop)

//Forward declaration
class ComponentSystem;
class HopsanEssentials;
class HopsanCoreMessageHandler;
class NumericalIntegrationSolver;

enum VariameterTypeEnumT {InputVariable, OutputVariable, OtherVariable};

class VariameterDescription
{
public:
    HString mName;
    HString mShortName;
    HString mPortName;
    HString mNodeType;
    HString mAlias;
    HString mDataType;
    HString mUnit;
    HString mQuantity;
    bool mUserModifiableQuantity;
    HString mDescription;
    VariameterTypeEnumT mVariameterType;
    NodeDataVariableTypeEnumT mVarType;
    size_t mVariableId;
};

class HOPSANCORE_DLLAPI Component
{
    friend class ComponentSystem;
    friend class ConditionalComponentSystem;
    friend class HopsanEssentials; //Need to be able to set typename
    friend class NumericalIntegrationSolver;

public:
    //! @brief Enum type for all CQS types
    enum CQSEnumT {CType, QType, SType, UndefinedCQSType};

    //==========Public functions==========
    // Configuration and simulation functions
    virtual void configure();
    virtual void deconfigure();
    virtual bool preInitialize();
    virtual bool checkModelBeforeSimulation();
    virtual bool initialize(const double startT, const double stopT);
    virtual void simulate(const double stopT);

    //Enabled or disabled?
    void setDisabled(bool value);
    bool isDisabled() const;

    // Timestep functions
    void setDesiredTimestep(const double timestep);
    void setInheritTimestep(const bool inherit=true);
    bool doesInheritTimestep() const;

    // Name and type
    void setName(HString name);
    const HString &getName() const;
    const HString &getTypeName() const;
    const HString &getSubTypeName() const;
    void setSubTypeName(const HString &rSubTypeName);

    // Component type identification
    virtual CQSEnumT getTypeCQS() const;
    HString getTypeCQSString() const;
    virtual bool isComponentC() const;
    virtual bool isComponentQ() const;
    virtual bool isComponentSystem() const;
    virtual bool isComponentSignal() const;
    virtual bool isExperimental() const;
    virtual bool isObsolete() const;

    // Constants
    void addConstant(const HString &rName, const HString &rDescription, const HString &rUnit, double &rData);
    void addConstant(const HString &rName, const HString &rDescription, const HString &rUnit, int &rData);
    void addConstant(const HString &rName, const HString &rDescription, const HString &rUnit, HString &rData);
    void addConstant(const HString &rName, const HString &rDescription, const HString &rUnit, bool &rData);
    void addConstant(const HString &rName, const HString &rDescription, const HString &rQuantityOrUnit, const double defaultValue, double &rData);
    void addConstant(const HString &rName, const HString &rDescription, const HString &rUnit, const int defaultValue, int &rData);
    void addConstant(const HString &rName, const HString &rDescription, const HString &rUnit, const HString &defaultValue, HString &rData);
    void addConstant(const HString &rName, const HString &rDescription, const HString &rUnit, const bool defaultValue, bool &rData);
    void addConstant(const HString &rName, const HString &rDescription, const HString &rQuantity, const HString &rUnit, const double defaultValue, double &rData);
    void addConstant(const HString &rName, const HString &rDescription, HTextBlock &rData);
    void addConditionalConstant(const HString &rName, const HString &rDescription, std::vector<HString> &rConditions, int &rData);
    void addConditionalConstant(const HString &rName, const HString &rDescription, std::vector<HString> &rConditions, const int defaultValue, int &rData);

    void setConstantValue(const HString &rName, const double value);
    void setConstantValue(const HString &rName, const int value);
    void setConstantValue(const HString &rName, const HString &rValue);
    void setConstantValue(const HString &rName, const char* value);
    void setConstantValue(const HString &rName, const bool value);

    // Variables and Parameters
    const std::vector<VariameterDescription>* getVariameters();
    virtual std::list<HString> getModelAssets() const;
    virtual size_t loadParameterValues(const HString &rFilePath);

    //! @todo clean this mess up /Peter
    void registerConditionalParameter(const HString &rName, const HString &rDescription, std::vector<HString> &rConditions, int &rValue);
    virtual void unRegisterParameter(const HString &rName);
    bool hasParameter(const HString &rName) const;
    void getParameterNames(std::vector<HString> &rParameterNames);
    const std::vector<ParameterEvaluator*> *getParametersVectorPtr() const;
    const ParameterEvaluator *getParameter(const HString &rName);
    void getParameterValue(const HString &rName, HString &rValue);
    void* getParameterDataPtr(const HString &rName);
    bool setParameterValue(const HString &rName, const HString &rValue, bool force=false);
    bool checkParameters(HString &errParName);
    void evaluateParameters();
    bool evaluateParameter(const HString &rName, HString &rEvaluatedParameterValue, const HString &rType);

    // Start values
    void setDefaultStartValue(Port* pPort, const size_t idx, const double value);
    void setDefaultStartValue(const HString &rPortName, const HString &rDataName, const double value);
    double getDefaultStartValue(Port *pPort, const size_t idx, const size_t portIdx=0);
    double getDefaultStartValue(const HString &rPortName, const HString &rDataName, const size_t portIdx=0);
    void disableStartValue(Port* pPort, const size_t idx);
    void disableStartValue(const HString &rPortName, const size_t idx);
    void disableStartValue(const HString &rPortName, const HString &rDataName);
    virtual void loadStartValues();
    virtual void loadStartValuesFromSimulation();

    // Ports
    std::vector<Port*> getPortPtrVector() const;
    Port *getPort(const HString &rPortname) const;
    std::vector<HString> getPortNames();

    // Node Data pointers
    double *getSafeNodeDataPtr(Port* pPort, const int dataId);
    double *getSafeNodeDataPtr(const HString &rPortName, const int dataId);
    double *getSafeMultiPortNodeDataPtr(Port* pPort, const size_t portIdx, const int dataId);
    double *getSafeMultiPortNodeDataPtr(Port* pPort, const size_t portIdx, const int dataId, const double defaultValue);

    // Node data access
    double readNodeSafe(const HString &rPortName, const HString &rDataName, const size_t subPortIdx=0);
    double readNodeSafe(const HString &rPortName, const size_t dataId, const size_t subPortIdx=0);
    void writeNodeSafe(const HString &rPortName, const HString &rDataName, const double value, const size_t subPortIdx=0);
    void writeNodeSafe(const HString &rPortName, const size_t dataId, const double value, const size_t subPortIdx=0);

    // System parent
    ComponentSystem *getSystemParent() const;
    size_t getModelHierarchyDepth() const;

    //! @todo Should it be possible to set timestep of a component? Should only be possible for a System component
    //void setTimestep(const double timestep);
    double getTimestep() const;
    double *getTimePtr();
    //! @brief Get the current simulation time
    //! @ingroup ConvenientSimulationFunctions
    inline double getTime() const {return mTime;}

    void setMeasuredTime(const double time);
    double getMeasuredTime() const;

    void addDebugMessage(const HString &rMessage, const HString &rTag="") const;
    void addWarningMessage(const HString &rMessage, const HString &rTag="") const;
    void addErrorMessage(const HString &rMessage, const HString &rTag="") const;
    void addInfoMessage(const HString &rMessage, const HString &rTag="") const;
    void addFatalMessage(const HString &rMessage, const HString &rTag="") const;

    // Stop a running simulation
    void stopSimulation(const HString &rReason="");

    // HopsanEssentials
    HopsanEssentials *getHopsanEssentials();

    // Search path
    HString findFilePath(const HString &rFileName) const;
    std::vector<HString> getSearchPaths() const;

    //Numerical integration members
    virtual void reInitializeValuesFromNodes();
    virtual void solveSystem();
    virtual double getStateVariableDerivative(int /*i*/);
    virtual double getStateVariableSecondDerivative(int /*i*/);

protected:
    //==========Protected member functions==========
    // Constructor - Destructor
    Component();
    virtual ~Component();

    // Virtual functions
    virtual void initialize(); //!< @todo Maybe we should be able to return success true or false from components
    virtual void simulateOneTimestep();
    virtual void finalize();
    virtual void setTimestep(const double timestep);
    inline virtual size_t calcNumSimSteps(const double startT, const double stopT)
    {
        // Round to nearest, we may not get exactly the stop time that we want
        return size_t(std::max(stopT-startT,0.0)/mTimestep+0.5);
    }

    // Interface variable functions
    Port *addInputVariable(const HString &rName, const HString &rDescription, const HString &rQuantityOrUnit, const double defaultValue, double **ppNodeData=0);
    Port *addOutputVariable(const HString &rName, const HString &rDescription, const HString &rQuantityOrUnit, double **ppNodeData=0);
    Port *addOutputVariable(const HString &rName, const HString &rDescription, const HString &rQuantityOrUnit, const double defaultValue, double **ppNodeData=0);

    void initializeAutoSignalNodeDataPtrs();

    // Port functions
    Port* addPort(const HString &rPortName, const PortTypesEnumT portType, const HString &rNodeType, const HString &rDescription, const Port::RequireConnectionEnumT reqConnection);
    Port* addPowerPort(const HString &rPortName, const HString &rNodeType, const HString &rDescription="", const Port::RequireConnectionEnumT reqConnect=Port::Required);
    Port* addReadPort(const HString &rPortName, const HString &rNodeType, const HString &rDescription="", const Port::RequireConnectionEnumT reqConnect=Port::Required);
    Port* addPowerMultiPort(const HString &rPortName, const HString &rNodeType, const HString &rDescription="", const Port::RequireConnectionEnumT reqConnect=Port::Required);
    Port* addReadMultiPort(const HString &rPortName, const HString &rNodeType, const HString &rDescription="", const Port::RequireConnectionEnumT reqConnect=Port::Required);
    Port* addWritePort(const HString &rPortName, const HString &rNodeType, const HString &rDescription="", const Port::RequireConnectionEnumT reqConnect=Port::Required);

    void removePort(const HString &rPortName);

    bool getPort(const HString &rPortname, Port* &rpPort);
    HString renamePort(const HString &rOldname, const HString &rNewname);
    void deletePort(const HString &rName);

    // Unique name functions
    virtual HString determineUniquePortName(const HString &rPortname);

    //==========Protected member variables==========
    ComponentSystem* mpSystemParent;
    bool mInheritTimestep;
    double mTimestep, mDesiredTimestep;
    double mTime;
    size_t mModelHierarchyDepth; //!< This variable contains the depth of the system in the model hierarchy, (used by connect to figure out where to store nodes)
    std::vector<HString> mSearchPaths;

private:
    typedef std::map<HString, Port*> PortPtrMapT;
    typedef std::pair<HString, Port*> PortPtrPairT;

    // Private member functions
    void setSystemParent(ComponentSystem *pComponentSystem);
    void setTypeName(const HString &rTypeName);
    double *getNodeDataPtr(Port* pPort, const int dataId);

    // Parameter registration
    void registerParameter(const HString &rName, const HString &rDescription, const HString &rQuantity, const HString &rUnit, double &rValue);
    void registerParameter(const HString &rName, const HString &rDescription, const HString &rUnit, int &rValue);
    void registerParameter(const HString &rName, const HString &rDescription, const HString &rUnit, HString &rValue);
    void registerParameter(const HString &rName, const HString &rDescription, const HString &rUnit, bool &rValue);
    void registerParameter(const HString &rName, const HString &rDescription, HTextBlock &rValue);

    // Private member variables
    HString mName;
    HString mTypeName;
    HString mSubTypeName;
    ParameterEvaluatorHandler *mpParameters;
    PortPtrMapT mPortPtrMap;
    std::vector<Port*> mPortPtrVector;
    double mMeasuredTime;
    HopsanEssentials *mpHopsanEssentials;
    HopsanCoreMessageHandler *mpMessageHandler;
    std::vector<VariameterDescription> mVariameters;
    std::map<Port*, double**> mAutoSignalNodeDataPtrPorts;
    bool mIsDisabled;
};



class HOPSANCORE_DLLAPI ComponentSignal : public Component
{
public:
    CQSEnumT getTypeCQS() const {return SType;}
    bool isComponentSignal() const {return true;}
};


class HOPSANCORE_DLLAPI ComponentC : public Component
{
    CQSEnumT getTypeCQS() const {return CType;}
    bool isComponentC() const {return true;}
};


class HOPSANCORE_DLLAPI ComponentQ : public Component
{
    CQSEnumT getTypeCQS() const {return QType;}
    bool isComponentQ() const {return true;}
};

typedef ClassFactory<HString, Component> ComponentFactory;
}

#endif // COMPONENT_H_INCLUDED
