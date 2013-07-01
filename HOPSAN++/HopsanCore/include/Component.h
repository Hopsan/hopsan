/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
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

enum VariameterTypeEnumT {InputVariable, OutputVariable, OtherVariable};

class VariameterDescription
{
public:
    HString mName;
    HString mShortName;
    HString mPortName;
    HString mAlias;
    HString mDataType;
    HString mUnit;
    HString mDescription;
    VariameterTypeEnumT mVariameterType;
    NodeDataVariableTypeEnumT mVarType;
    unsigned int mVariableId;
};

class DLLIMPORTEXPORT Component
{
    friend class ComponentSystem;
    friend class HopsanEssentials; //Need to be able to set typename

public:
    //! @brief Enum type for all CQS types
    enum CQSEnumT {CType, QType, SType, UndefinedCQSType};

    //==========Public functions==========
    // Configureation and simulation functions
    virtual void configure();
    virtual void deconfigure();
    virtual bool preInitialize();
    virtual bool checkModelBeforeSimulation();
    virtual bool initialize(const double startT, const double stopT);
    virtual void simulate(const double stopT);
    virtual void finalize(const double startT, const double Ts);

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

    // Constants
    void addConstant(const HString &rName, const HString &rDescription, const HString &rUnit, double &rData);
    void addConstant(const HString &rName, const HString &rDescription, const HString &rUnit, const double defaultValue, double &rData);
    void addConstant(const HString &rName, const HString &rDescription, const HString &rUnit, int &rData);
    void addConstant(const HString &rName, const HString &rDescription, const HString &rUnit, const int defaultValue, int &rData);
    void addConstant(const HString &rName, const HString &rDescription, const HString &rUnit, HString &rData);
    void addConstant(const HString &rName, const HString &rDescription, const HString &rUnit, const HString &defaultValue, HString &rData);
    void addConstant(const HString &rName, const HString &rDescription, const HString &rUnit, bool &rData);
    void addConstant(const HString &rName, const HString &rDescription, const HString &rUnit, const bool defaultValue, bool &rData);
    void setConstantValue(HString &rName, const double value);
    void setConstantValue(HString &rName, const int value);
    void setConstantValue(HString &rName, const HString &rValue);
    void setConstantValue(HString &rName, const bool value);

    // Variabels and Parameters
    const std::vector<VariameterDescription>* getVariameters();

    //! @todo clean this up /Peter
    virtual void unRegisterParameter(const HString &rName);
    bool hasParameter(const HString &rName) const;
    const std::vector<ParameterEvaluator*> *getParametersVectorPtr() const;
    void getParameterNames(std::vector<HString> &rParameterNames);
    const ParameterEvaluator *getParameter(const HString &rName);
    void getParameterValue(const HString &rName, HString &rValue);
    void* getParameterDataPtr(const HString &rName);
    bool setParameterValue(const HString &rName, const HString &rValue, bool force=false);
    void updateParameters();
    bool checkParameters(HString &errParName);

    // Start values
    double getStartValue(Port* pPort, const size_t idx, const size_t portIdx=0);
    double getDefaultStartValue(Port *pPort, const size_t idx, const size_t portIdx=0);
    void setStartValue(Port* pPort, const size_t idx, const double value);
    void setDefaultStartValue(Port* pPort, const size_t idx, const double value);
    void disableStartValue(Port* pPort, const size_t idx);
    virtual void loadStartValues();
    virtual void loadStartValuesFromSimulation();

    // Ports
    std::vector<Port*> getPortPtrVector();
    Port *getPort(const HString &rPortname) const;
    std::vector<HString> getPortNames();

    // Node Data ptrs
    double *getSafeNodeDataPtr(Port* pPort, const int dataId);
    double *getSafeNodeDataPtr(const HString &rPortName, const int dataId);
    double *getSafeMultiPortNodeDataPtr(Port* pPort, const size_t portIdx, const int dataId);
    double *getSafeMultiPortNodeDataPtr(Port* pPort, const size_t portIdx, const int dataId, const double defaultValue);

    // Node data access
    double readNodeSafeSlow(const HString &rPortName, const HString &rDataName);
    void writeNodeSafeSlow(const HString &rPortName, const HString &rDataName, const double value);

    // System parent
    ComponentSystem *getSystemParent();
    size_t getModelHierarchyDepth();

    //! @todo Should it be possible to set timestep of a component? Should only be possible for a Systemcomponent
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
    void stopSimulation();

    // HopsanEssentials
    HopsanEssentials *getHopsanEssentials();

    //Searchpath
    HString findFilePath(const HString &rFileName);

protected:
    //==========Protected member functions==========
    // Constructor - Destructor
    Component();
    virtual ~Component();

    // Virtual functions
    virtual void initialize(); //!< @todo We should really be able to return sucess true or false from components
    virtual void simulateOneTimestep();
    virtual void finalize();
    virtual void setTimestep(const double timestep);
    inline virtual size_t calcNumSimSteps(const double startT, const double stopT)
    {
        // Round to nearest, we may not get exactly the stop time that we want
        return size_t(std::max(stopT-startT,0.0)/mTimestep+0.5);
    }

    // Interface variable functions
    Port *addInputVariable(const HString &rName, const HString &rDescription, const HString &rUnit, const double defaultValue, double **ppNodeData=0);
    Port *addOutputVariable(const HString &rName, const HString &rDescription, const HString &rUnit, double **ppNodeData=0);
    Port *addOutputVariable(const HString &rName, const HString &rDescription, const HString &rUnit, const double defaultValue, double **ppNodeData=0);

    void initializeAutoSignalNodeDataPtrs();

    // Port functions
    Port* addPowerPort(const HString &rPortName, const HString &rNodeType, const Port::RequireConnectionEnumT reqConnect=Port::Required);
    Port* addPowerPort(const HString &rPortName, const HString &rNodeType, const HString &rDescription, const Port::RequireConnectionEnumT reqConnect=Port::Required);
    Port* addReadPort(const HString &rPortName, const HString &rNodeType, const Port::RequireConnectionEnumT reqConnect=Port::Required);
    Port* addReadPort(const HString &rPortName, const HString &rNodeType, const HString &rDescription, const Port::RequireConnectionEnumT reqConnect=Port::Required);
    Port* addPowerMultiPort(const HString &rPortName, const HString &rNodeType, const Port::RequireConnectionEnumT reqConnect=Port::Required);
    Port* addPowerMultiPort(const HString &rPortName, const HString &rNodeType, const HString &rDescription, const Port::RequireConnectionEnumT reqConnect=Port::Required);
    Port* addReadMultiPort(const HString &rPortname, const HString &rNodetype, const Port::RequireConnectionEnumT reqConnect=Port::Required);
    Port* addReadMultiPort(const HString &rPortName, const HString &rNodeType, const HString &rDescription, const Port::RequireConnectionEnumT reqConnect=Port::Required);
    Port* addWritePort(const HString &rPortName, const HString &rNodeType, const Port::RequireConnectionEnumT reqConnect=Port::Required);
    Port* addWritePort(const HString &rPortName, const HString &rNodeType, const HString &rDescription, const Port::RequireConnectionEnumT reqConnect=Port::Required);

    bool getPort(const HString &rPortname, Port* &rpPort);
    HString renamePort(const HString &rOldname, const HString &rNewname);
    void deletePort(const HString &rName);

    // NodeData ptr function
    //! @todo clean up this mess /Peter
    double *getSafeNodeDataPtr(Port* pPort, const int dataId, const double defaultValue);

    // Unique name functions
    virtual HString determineUniquePortName(const HString &rPortname);

    //==========Protected member variables==========
    ComponentSystem* mpSystemParent;
    bool mInheritTimestep;
    double mTimestep, mDesiredTimestep;
    double mTime;
    size_t mModelHierarchyDepth; //!< This variable containes the depth of the system in the model hierarchy, (used by connect to figure out where to store nodes)
    std::vector<HString> mSearchPaths;

private:
    typedef std::map<HString, Port*> PortPtrMapT;
    typedef std::pair<HString, Port*> PortPtrPairT;

    // Private member functions
    void setSystemParent(ComponentSystem *pComponentSystem);
    void setTypeName(const HString &rTypeName);
    double *getNodeDataPtr(Port* pPort, const int dataId);
    Port* addPort(const HString &rPortName, const PortTypesEnumT portType, const HString &rNodeType, const Port::RequireConnectionEnumT reqConnection);
    Port* addPort(const HString &rPortName, const PortTypesEnumT portType, const HString &rNodeType, const HString &rDescription, const Port::RequireConnectionEnumT reqConnection);

    // Parameter registration
    //! @todo clean this up /Peter
    void registerParameter(const HString &rName, const HString &rDescription, const HString &rUnit, double &rValue);
    void registerParameter(const HString &rName, const HString &rDescription, const HString &rUnit, int &rValue);
    void registerParameter(const HString &rName, const HString &rDescription, const HString &rUnit, HString &rValue);
    void registerParameter(const HString &rName, const HString &rDescription, const HString &rUnit, bool &rValue);

    // Private member variables
    HString mName;
    HString mTypeName;
    HString mSubTypeName;
    ParameterEvaluatorHandler *mpParameters;
    PortPtrMapT mPortPtrMap;
    double mMeasuredTime;
    HopsanEssentials *mpHopsanEssentials;
    HopsanCoreMessageHandler *mpMessageHandler;
    std::vector<VariameterDescription> mVariameters;
    std::map<Port*, double**> mAutoSignalNodeDataPtrPorts;
};



class DLLIMPORTEXPORT ComponentSignal : public Component
{
public:
    CQSEnumT getTypeCQS() const {return SType;}
    bool isComponentSignal() const {return true;}
};


class DLLIMPORTEXPORT ComponentC : public Component
{
    CQSEnumT getTypeCQS() const {return CType;}
    bool isComponentC() const {return true;}
};


class DLLIMPORTEXPORT ComponentQ : public Component
{
    CQSEnumT getTypeCQS() const {return QType;}
    bool isComponentQ() const {return true;}
};

typedef ClassFactory<HString, Component> ComponentFactory;
}

#endif // COMPONENT_H_INCLUDED
